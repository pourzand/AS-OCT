"""Utils for tensorboard
TODO
----
@Youngwon
doc_string
"""
import os
import numpy as np
import matplotlib.pyplot as plt
from keras.callbacks import TensorBoard
import keras.backend as k
import tensorflow as tf
"""check for v2
https://www.tensorflow.org/guide/migrate
"""
# import tensorflow.compat.v1 as tf
# tf.disable_v2_behavior()
# from tensorflow.compat.v1.keras.callbacks import TensorBoard
# from tensorflow.compat.v1.keras import backend as k

import matplotlib
import matplotlib.cm

"""For segmentation
"""
class TensorBoardSegmentationWrapper(TensorBoard):
    '''Tensorboard wrapper for segmentation
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol:
                input images: number of modals
                bottleneck images : number of filters
                output images: 2 (GT, predict)
    '''
    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False,
                 tb_data_steps=1, zcut=[0,0], downsampling_scale = 1,
                 **kwargs):
        super(TensorBoardSegmentationWrapper, self).__init__(**kwargs)
        self.write_weights_histogram = write_weights_histogram
        self.write_weights_images = write_weights_images
        self.write_graph=write_graph
        self.tb_data_steps = tb_data_steps
        self.validation_data = validation_data
        # print("[root|DEBUG|cnn_tensorboard_utils.py 47] ", len(self.validation_data), len(self.validation_data[0]), self.validation_data[0][0].shape, self.validation_data[0][1].shape)
        self.img_shape = validation_data[0][0].shape[1:]
        self.zcut = zcut
        self.downsampling_scale = downsampling_scale
        # print("[root|DEBUG|cnn_tensorboard_utils.py 51] initial image_shape: %s" % list(self.img_shape))
        
        # if self.embeddings_data is None and self.validation_data:
        #     self.embeddings_data = self.validation_data
        self.embeddings_data = self.validation_data
    
    def normalize(self, value):
        vmin = tf.reduce_min(value)
        vmax = tf.reduce_max(value)
        value = (value - vmin) / (vmax - vmin)
        return value
        
    def colorize(self, value, cmap=None):
        """
        ref: https://gist.github.com/jimfleming/c1adfdb0f526465c99409cc143dea97b
        A utility function for TensorFlow that maps a grayscale image to a matplotlib
        colormap for use with TensorBoard image summaries.
        By default it will normalize the input value to the range 0..1 before mapping
        to a grayscale colormap.
        Arguments:
          - value: 2D Tensor of shape [height, width] or 3D Tensor of shape
            [height, width, 1].
          - vmin: the minimum value of the range used for normalization.
            (Default: value minimum)
          - vmax: the maximum value of the range used for normalization.
            (Default: value maximum)
          - cmap: a valid cmap named for use with matplotlib's `get_cmap`.
            (Default: 'gray')
        Example usage:
        ```
        output = tf.random_uniform(shape=[256, 256, 1])
        output_color = colorize(output, vmin=0.0, vmax=1.0, cmap='viridis')
        tf.summary.image('output', output_color)
        ```
        Returns a 3D tensor of shape [height, width, 3].
        """
        """squeeze last dim if it exists"""
        value = tf.squeeze(value)
        """quantize"""
        indices = tf.to_int32(tf.round(tf.clip_by_value(value, -1, 1) * 255))
        """gather"""
        cm = matplotlib.cm.get_cmap(cmap if cmap is not None else 'gist_yarg')
        try: color_attr = cm.colors
        except: color_attr = cm(np.arange(0,cm.N)) 
        colors = tf.constant(color_attr, dtype=tf.float32)
        value = tf.gather(colors, indices)
        return value

    def resize2D(self, x, target_shape):
        # input x = (batch, x, y, filter)
        x = tf.image.resize_images(x, size=target_shape[0:2])
        return x
    
    def resize3D(self, x, target_shape, zcut):
        # input x = (z, x, y, filter)
        x = k.stack([tf.image.resize_images(x[i], size=target_shape[1:3]) for i in range(target_shape[0]+np.sum(zcut))])  # x = (z, x, y, filter)
        x = x[zcut[0]:]
        if zcut[1] > 0: x = x[:-zcut[1]]
        return x
    
    def tile_patches_medical(self, x, shape):
        if len(shape) == 4:
            """For 3D"""
            # input x = (z, x, y, filter)
            x = tf.transpose(x, [0,3,1,2]) # x = (z, filter, x, y)
            x = k.reshape(x,[shape[0],shape[3],shape[1]*shape[2],1]) # (z, filter, x*y, 1)
            x = tf.transpose(x, perm=[2,0,1,3]) # (x*y, z, filter, 1)
            tiled_x = tf.batch_to_space_nd(x, shape[1:3], [[0,0],[0,0]])
        elif len(shape) == 3:
            """For 2D"""
            # input x = (batch, x, y, filter)
            nbatch = self.validation_data.batch_size
            x = tf.transpose(x, [0,3,1,2]) # x = (batch, filter, x, y)
            x = k.reshape(x,[nbatch,shape[2],shape[0]*shape[1],1]) # (batch, filter, x*y, 1)
            x = tf.transpose(x, perm=[2,0,1,3]) # (x*y, batch, filter, 1)
            tiled_x = tf.batch_to_space_nd(x, shape[0:2], [[0,0],[0,0]])
        else:
            raise ValueError('image must be 2D or 3D')
        return tiled_x
    
    def _summary_histogram_per_weight(self, summary, mapped_weight_name, weight):
        summary.histogram(mapped_weight_name, weight)

    def _summary_grad_histogram_per_weight(self, summary, mapped_weight_name, weight, model):
        grads = model.optimizer.get_gradients(model.total_loss, weight)
        def is_indexed_slices(grad):
            return type(grad).__name__ == 'IndexedSlices'
        grads = [
            grad.values if is_indexed_slices(grad) else grad
            for grad in grads]
        summary.histogram('{}_grad'.format(mapped_weight_name), grads)

    def _summary_weight_images_per_weight(self, summary, mapped_weight_name, weight):
        w_img = tf.squeeze(weight)
        shape = k.int_shape(w_img)
        if len(shape) == 2:
            """dense layer kernel case"""
            if shape[0] > shape[1]:
                w_img = tf.transpose(w_img)
                shape = k.int_shape(w_img)
            w_img = tf.reshape(w_img, [1,
                                        shape[0],
                                        shape[1],
                                        1])
        elif len(shape) == 3:  
            """1d convnet case"""
            if k.image_data_format() == 'channels_last':
                """
                switch to channels_first to display
                every kernel as a separate image
                """
                w_img = tf.transpose(w_img, perm=[2, 0, 1])
                shape = k.int_shape(w_img)
            w_img = tf.reshape(w_img, [shape[0],
                                        shape[1],
                                        shape[2],
                                        1])
        elif len(shape) == 4:
            """conv2D"""
            # input_dim * output_dim, width, hieght
            w_img = tf.transpose(w_img, perm=[2, 3, 0, 1])
            shape = k.int_shape(w_img)
            w_img = tf.reshape(w_img, [shape[0]*shape[1],
                                        shape[2],
                                        shape[3],
                                        1])
        elif len(shape) == 5:
            """conv3D"""
            # input_dim * output_dim*depth, width, hieght
            w_img = tf.transpose(w_img, perm=[3, 4, 0, 1, 2])
            shape = k.int_shape(w_img)
            w_img = tf.reshape(w_img, [shape[0]*shape[1]*shape[2],
                                        shape[3],
                                        shape[4],
                                        1])
        elif len(shape) == 1:
            """bias case"""
            w_img = tf.reshape(w_img, [1,
                                        shape[0],
                                        1,
                                        1])
        summary.image(mapped_weight_name, w_img)

    def _summary_3d_image(self, model):
        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += 2 # gt 1channel + pred 1channel
        input_shape[0] = input_shape[0] - np.sum(self.zcut)
        input_shape[1:3] = np.round(input_shape[1:3] * self.downsampling_scale, 0)
        assert (input_shape[0] > 0, 'Please check the zcut setting for 3D inputs. It should be small than image size')
        tot_pred_image = []
        for i in range(self.batch_size):
            input_img = self.resize3D(model.inputs[0][i], target_shape= input_shape, zcut=self.zcut)
            input_img_norm = k.concatenate([self.normalize(input_img[...,j:j+1]) for j in range(self.img_shape[-1])], axis=-1)
            gt = self.resize3D(model.targets[0][i], target_shape= input_shape, zcut=self.zcut)
            pred = self.resize3D(model.outputs[0][i], target_shape= input_shape, zcut=self.zcut)
            pred_image_list = [input_img_norm, gt, pred]
            pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
            pred_image_col = self.colorize(pred_image, cmap='gray') # 'gist_yarg', 'inferno'
            tot_pred_image.append(pred_image_col)
        tot_pred_image = k.stack(tot_pred_image)
        return tot_pred_image

    def _summary_2d_image(self, model):
        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += 2 # gt 1channel + pred 1channel
        input_shape[0:2] = np.round(input_shape[0:2] * self.downsampling_scale, 0)
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 221] {input_shape}')

        input_img = self.resize2D(model.inputs[0], target_shape= input_shape)
        input_img_norm = self.normalize(input_img)
        gt = self.resize2D(model.targets[0], target_shape= input_shape)
        pred = self.resize2D(model.outputs[0], target_shape= input_shape)
        pred_image_list = [input_img_norm, gt, pred]
        tot_pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 229] {k.int_shape(tot_pred_image)}')
        tot_pred_image_col = k.stack([self.colorize(tot_pred_image, cmap='gray')])
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 231] {k.int_shape(tot_pred_image_col)}')
        return tot_pred_image_col

    def _summary_image(self, summary, model):
        if len(self.img_shape) == 4:
            """for 3D images"""
            tot_pred_image = self._summary_3d_image(model)  
        elif len(self.img_shape) == 3:
            """for 2D images"""
            tot_pred_image = self._summary_2d_image(model)
        else:
            raise ValueError(f'image with shape {len(self.img_shape)} is not supported.')
        shape = k.int_shape(tot_pred_image)
        assert len(shape) == 4 and shape[-1] in [1, 3, 4]
        summary.image('prediction', tot_pred_image, max_outputs=self.batch_size)

    def _summary_embedding(self, writer, model):
        embeddings_layer_names = self.embeddings_layer_names
        if not embeddings_layer_names:
            embeddings_layer_names = [layer.name for layer in model.layers
                                        if type(layer).__name__ == 'Embedding']
        self.assign_embeddings = []
        embeddings_vars = {}

        self.batch_id = batch_id = tf.placeholder(tf.int32)
        self.step = step = tf.placeholder(tf.int32)

        for layer in model.layers:
            if layer.name in embeddings_layer_names:
                embedding_input = model.get_layer(layer.name).output
                embedding_size = int(np.prod(embedding_input.shape[1:]))
                embedding_input = tf.reshape(embedding_input,
                                                (step, embedding_size))
                shape = (self.embeddings_data[0].shape[0], embedding_size)
                embedding = tf.Variable(tf.zeros(shape),
                                        name=layer.name + '_embedding')
                embeddings_vars[layer.name] = embedding
                batch = tf.assign(embedding[batch_id:batch_id + step],
                                    embedding_input)
                self.assign_embeddings.append(batch)

        self.saver = tf.train.Saver(list(embeddings_vars.values()))

        embeddings_metadata = {}

        if not isinstance(self.embeddings_metadata, str):
            embeddings_metadata = self.embeddings_metadata
        else:
            embeddings_metadata = {layer_name: self.embeddings_metadata
                                    for layer_name in embeddings_vars.keys()}

        config = projector.ProjectorConfig()

        for layer_name, tensor in embeddings_vars.items():
            embedding = config.embeddings.add()
            embedding.tensor_name = tensor.name

            if layer_name in embeddings_metadata:
                embedding.metadata_path = embeddings_metadata[layer_name]

        projector.visualize_embeddings(writer, config)

    def set_model(self, model):
        self.model = model
        if k.backend() == 'tensorflow':
            self.sess = k.get_session()
        if self.histogram_freq and self.merged is None:
            """summary weight"""
            for layer in self.model.layers:
                for weight in layer.weights:
                    mapped_weight_name = 'weight_%s' % weight.name.replace(':', '_')
                    if self.write_weights_histogram: self._summary_histogram_per_weight(tf.summary, mapped_weight_name, weight)
                    if self.write_grads: self._summary_grad_histogram_per_weight(tf.summary, mapped_weight_name, weight, model)
                    if self.write_weights_images: self._summary_weight_images_per_weight(tf.summary, mapped_weight_name, weight)
                if hasattr(layer, 'output'):
                    if isinstance(layer.output, list):
                        for i, output in enumerate(layer.output):
                            tf.summary.histogram('{}_out_{}'.format(layer.name, i), output)
                    else: tf.summary.histogram('{}_out'.format(layer.name), layer.output)
            """summary images, references, and predictions"""
            if self.write_images: self._summary_image(tf.summary, model)
        self.merged = tf.summary.merge_all()
        """
        TODO: fix
        """
        self.tf_physical_ids = tf.placeholder(tf.string, shape=(None,))
        self.summary_physical_ids = tf.summary.text('patient_ids', self.tf_physical_ids)
        
        """tensor graph & file write"""
        if self.write_graph: 
            self.writer = tf.summary.FileWriter(self.log_dir, self.sess.graph)
        else: 
            self.writer = tf.summary.FileWriter(self.log_dir)
        
        """embedding
        TODO: code
        """
        if self.embeddings_freq: self._summary_embedding(self.writer, model)
    
    def on_epoch_end(self, epoch, logs=None):
        logs = logs or {}

        if not self.validation_data and self.histogram_freq:
            raise ValueError("If printing histograms, validation_data must be "
                             "provided, and cannot be a generator.")
        if self.embeddings_data is None and self.embeddings_freq:
            raise ValueError("To visualize embeddings, embeddings_data must "
                             "be provided.")
            
        if self.validation_data and self.histogram_freq:
            if epoch == 0 or (epoch+1) % self.histogram_freq == 0:

                val_data = self.validation_data
                tensors = (self.model.inputs +
                           self.model.targets +
                           self.model.sample_weights)

                if self.model.uses_learning_phase:
                    tensors += [k.learning_phase()]

                for i in range(self.tb_data_steps):
                    x, y = val_data[i]
                    physical_ids = val_data.sampler.get_current_physical_id()
                    if type(x) != list:
                        x = [x]
                    if type(y) != list:
                        y = [y]
                    if self.model.uses_learning_phase:
                        batch_val = x + y + [np.ones(self.batch_size, dtype=np.float32) for tmp in range(len(self.model.sample_weights))] + [0.0]
                    else:
                        batch_val = x + y + [np.ones(self.batch_size, dtype=np.float32) for tmp in range(len(self.model.sample_weights))]
                    
                    assert len(batch_val) == len(tensors)
                    feed_dict = dict(zip(tensors, batch_val))
                    # print("[root|DEBUG|cnn_tensorboard_utils.py 364] ", x[0].shape, y[0].shape, tensors)
                    result = self.sess.run([self.merged], feed_dict=feed_dict)
                    summary_str = result[0]
                    self.writer.add_summary(summary_str, epoch)
                    
                    summary_patient_id = self.sess.run([self.summary_physical_ids], feed_dict={self.tf_physical_ids: physical_ids})
                    self.writer.add_summary(summary_patient_id[0], epoch)
                    
        if self.embeddings_freq and self.embeddings_data is not None:
            if epoch == 0 or epoch % self.embeddings_freq == 0:
                embeddings_data = self.embeddings_data
                for i in range(self.tb_data_steps):
                    if type(self.model.input) == list:
                        feed_dict = {model_input: embeddings_data[i][idx]
                                     for idx, model_input in enumerate(self.model.input)}
                    else:
                        feed_dict = {self.model.input: embeddings_data[i]}

                    feed_dict.update({self.batch_id: i, self.step: self.batch_size})

                    if self.model.uses_learning_phase:
                        feed_dict[k.learning_phase()] = False

                    self.sess.run(self.assign_embeddings, feed_dict=feed_dict)
                    self.saver.save(self.sess,
                                    os.path.join(self.log_dir, 'keras_embedding.ckpt'),
                                    epoch)
                    
        for name, value in logs.items():
            if name in ['batch', 'size']:
                continue
            summary = tf.Summary()
            summary_value = summary.value.add()
            if isinstance(value, np.ndarray):
                summary_value.simple_value = value.item()
            else:
                summary_value.simple_value = value
            summary_value.tag = name
            self.writer.add_summary(summary, epoch)        
        
        self.writer.flush()

class TensorBoardSegmentationAttentionWrapper(TensorBoardSegmentationWrapper):
    '''Tensorboard wrapper for segmentation
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol:
                input images: number of modals
                bottleneck images : number of filters
                output images: 2 (GT, predict)
    '''
    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False, 
                 tb_data_steps=1, zcut=[0,0], downsampling_scale = 1,
                 **kwargs):
        super(TensorBoardSegmentationAttentionWrapper, self).__init__(validation_data, write_graph, write_weights_histogram, write_weights_images, 
                                                               tb_data_steps, zcut, downsampling_scale, **kwargs)
    
    def set_model(self, model):
        self.model = model
        if k.backend() == 'tensorflow':
            self.sess = k.get_session()
        if self.histogram_freq and self.merged is None:
            """summary weight"""
            for layer in self.model.layers:
                for weight in layer.weights:
                    mapped_weight_name = 'weight_%s' % weight.name.replace(':', '_')
                    if self.write_weights_histogram: self._summary_histogram_per_weight(tf.summary, mapped_weight_name, weight)
                    if self.write_grads: self._summary_grad_histogram_per_weight(tf.summary, mapped_weight_name, weight, model)
                    if self.write_weights_images: self._summary_weight_images_per_weight(tf.summary, mapped_weight_name, weight)
                if hasattr(layer, 'output'):
                    if isinstance(layer.output, list):
                        for i, output in enumerate(layer.output):
                            tf.summary.histogram('{}_out_{}'.format(layer.name, i), output)
                    else: tf.summary.histogram('{}_out'.format(layer.name), layer.output)
            
            """search_area_attention_intensity"""
            attention_layer_list = [l for l in model.layers if 'search_area_attention_' in l.name]
            for layer in attention_layer_list:
                mapped_weight_name = 'intensity_weight_%s' % layer.name
                value = tf.squeeze(layer.intensity_weight)
                tf.summary.scalar(mapped_weight_name, value)
                mapped_weight_name = 'intensity_%s' % layer.name
                value = tf.squeeze(layer.intensity)
                tf.summary.scalar(mapped_weight_name, value)
                # for weight in layer.weights:
                #     if 'intensity' in weight.name:
                #         tf.summary.scalar(mapped_weight_name, weight)
                        
            """summary images, references, and predictions"""
            if self.write_images: self._summary_image(tf.summary, model)
        self.merged = tf.summary.merge_all()
        """
        TODO: fix
        """
        self.tf_physical_ids = tf.placeholder(tf.string, shape=(None,))
        self.summary_physical_ids = tf.summary.text('patient_ids', self.tf_physical_ids)
        
        """tensor graph & file write"""
        if self.write_graph: 
            self.writer = tf.summary.FileWriter(self.log_dir, self.sess.graph)
        else: 
            self.writer = tf.summary.FileWriter(self.log_dir)
        
        """embedding
        TODO: code
        """
        if self.embeddings_freq: self._summary_embedding(self.writer, model)

    def _summary_3d_image(self, model):
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 476] TB layer: {[l.name for l in model.layers]}')
        attention_layer_list = [l for l in model.layers if 'search_area_attention_' in l.name]
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 478] TB attention layer: {attention_layer_list}')

        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += 2 + len(attention_layer_list) * (self.img_shape[-1]) # gt 1channel, pred 1 channel, number of attention layers * (attnetion + attention_gated_input)
        input_shape[0] = input_shape[0] - np.sum(self.zcut)
        input_shape[1:3] = np.round(input_shape[1:3] * self.downsampling_scale, 0)
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 486] input_shape: {input_shape}')
        assert (input_shape[0] > 0, 'Please check the zcut setting for 3D inputs. It should be small than image size')
        tot_pred_image = []
        for i in range(self.batch_size):
            input_img = self.resize3D(model.inputs[0][i], target_shape= input_shape, zcut=self.zcut)
            input_img_norm = k.concatenate([self.normalize(input_img[...,j:j+1]) for j in range(self.img_shape[-1])], axis=-1)
            attention_norm_gated_list = []
            for attention_layer in attention_layer_list:
                attention = attention_layer.output[1][i]
                # ratio =  k.int_shape(model.inputs[0][i])[0] // k.int_shape(attention)[0]
                # attention = k.repeat_elements(attention, ratio, axis=0)
                # print(f'[root|DEBUG|cnn_tensorboard_utils.py 497] {k.int_shape(attention)}')
                attention_resize = self.resize3D(attention, target_shape= input_shape, zcut=self.zcut)
                # attention_norm = self.normalize(attention_resize)
                attention_norm = attention_resize
                attention_norm_gated_list.append(attention_norm)
                attention_gated = attention_layer.output[0][i]
                # print(f'[root|DEBUG|cnn_tensorboard_utils.py 500] {k.int_shape(attention_gated)}')
                attention_gated_resize = self.resize3D(attention_gated, target_shape= input_shape, zcut=self.zcut)
                attention_gated_norm = self.normalize(attention_gated_resize)
                attention_norm_gated_list.append(attention_gated_norm)
            
            gt = self.resize3D(model.targets[0][i], target_shape= input_shape, zcut=self.zcut)
            pred = self.resize3D(model.outputs[0][i], target_shape= input_shape, zcut=self.zcut)
            pred_image_list = [input_img_norm] + attention_norm_gated_list + [gt, pred]
            pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
            pred_image_col = self.colorize(pred_image, cmap='gray') # 'gist_yarg', 'inferno'
            tot_pred_image.append(pred_image_col)
        tot_pred_image = k.stack(tot_pred_image)
        return tot_pred_image

    def _summary_2d_image(self, model):
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 518] TB layer: {[l.name for l in model.layers]}')
        attention_layer_list = [l for l in model.layers if 'search_area_attention_' in l.name]
        # print(f'[root|DEBUG|cnn_tensorboard_utils.py 520] TB attention layer: {attention_layer_list}')
                
        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += 2 + len(attention_layer_list) * (self.img_shape[-1]) # gt 1channel, pred 1 channel, number of attention layers * (attnetion + attention_gated_input)
        input_shape[0:2] = input_shape[0:2] * self.downsampling_scale
        input_shape[0:2] = np.round(input_shape[0:2] * self.downsampling_scale, 0)
        
        input_img = self.resize2D(model.inputs[0], target_shape= input_shape)
        input_img_norm = self.normalize(input_img)
        attention_norm_gated_list = []
        for attention_layer in attention_layer_list:
            attention = attention_layer.output[1]
            # ratio =  k.int_shape(model.inputs[0])[0] // k.int_shape(attention)[0]
            # attention = k.repeat_elements(attention, ratio, axis=0)
            attention_resize = self.resize2D(attention, target_shape= input_shape)
            # attention_norm = self.normalize(attention_resize)
            attention_norm = attention_resize
            attention_norm_gated_list.append(attention_norm)
            attention_gated = attention_layer.output[0]
            attention_gated_resize = self.resize2D(attention_gated, target_shape= input_shape)
            attention_gated_norm = self.normalize(attention_gated_resize)
            attention_norm_gated_list.append(attention_gated_norm)        
            
        gt = self.resize2D(model.targets[0], target_shape= input_shape)
        pred = self.resize2D(model.outputs[0], target_shape= input_shape)
        pred_image_list = [input_img_norm] + attention_norm_gated_list + [gt, pred]
        tot_pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
        tot_pred_image_col = k.stack([self.colorize(tot_pred_image, cmap='gray')])
        return tot_pred_image_col


"""for debugging
"""
class TensorBoardSimpleWrapper(TensorBoardSegmentationWrapper):
    '''
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol: 
                   input images: number of modals
                   bottleneck images : number of filters
                   output images: 2 (GT, predict)
        TODO: fix one person as reference
    '''
    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False, 
                 tb_data_steps=1, zcut=[0,0], downsampling_scale=1.,
                 **kwargs):
        super(TensorBoardSimpleWrapper, self).__init__(validation_data, write_graph, write_weights_histogram, write_weights_images, 
                                                               tb_data_steps, zcut, downsampling_scale, **kwargs)

    def set_model(self, model):
        self.model = model
        if k.backend() == 'tensorflow':
            self.sess = k.get_session()
        if self.histogram_freq and self.merged is None:
            """summary weight"""
            for layer in self.model.layers:
                for weight in layer.weights:
                    mapped_weight_name = 'weight_%s' % weight.name.replace(':', '_')
                    if self.write_weights_histogram: self._summary_histogram_per_weight(tf.summary, mapped_weight_name, weight)
                    if self.write_grads: self._summary_grad_histogram_per_weight(tf.summary, mapped_weight_name, weight, model)
                    if self.write_weights_images: self._summary_weight_images_per_weight(tf.summary, mapped_weight_name, weight)
                if hasattr(layer, 'output'):
                    if isinstance(layer.output, list):
                        for i, output in enumerate(layer.output):
                            tf.summary.histogram('{}_out_{}'.format(layer.name, i), output)
                    else: tf.summary.histogram('{}_out'.format(layer.name), layer.output)
            
        self.merged = tf.summary.merge_all()
        """
        TODO: fix
        """
        self.tf_physical_ids = tf.placeholder(tf.string, shape=(None,))
        self.summary_physical_ids = tf.summary.text('patient_ids', self.tf_physical_ids)
        
        """tensor graph & file write"""
        if self.write_graph: 
            self.writer = tf.summary.FileWriter(self.log_dir, self.sess.graph)
        else: 
            self.writer = tf.summary.FileWriter(self.log_dir)
        
        """embedding
        TODO: code
        """
        if self.embeddings_freq: self._summary_embedding(self.writer, model)


"""for point detection network
"""
class TensorBoardPointDetectionWrapper(TensorBoardSegmentationWrapper):
    '''
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol: 
                   input images: number of modals
                   bottleneck images : number of filters
                   output images: 2 (GT, predict)
        TODO: fix one person as reference
    '''
    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False, 
                 tb_data_steps=1, zcut=[0,0], downsampling_scale=1.,
                 **kwargs):
        super(TensorBoardPointDetectionWrapper, self).__init__(validation_data, write_graph, write_weights_histogram, write_weights_images, 
                                                               tb_data_steps, zcut, downsampling_scale, **kwargs)

    def set_model(self, model):
        self.model = model
        if k.backend() == 'tensorflow':
            self.sess = k.get_session()
        if self.histogram_freq and self.merged is None:
            """summary weight"""
            for layer in self.model.layers:
                for weight in layer.weights:
                    mapped_weight_name = 'weight_%s' % weight.name.replace(':', '_')
                    if self.write_weights_histogram: self._summary_histogram_per_weight(tf.summary, mapped_weight_name, weight)
                    if self.write_grads: self._summary_grad_histogram_per_weight(tf.summary, mapped_weight_name, weight, model)
                    if self.write_weights_images: self._summary_weight_images_per_weight(tf.summary, mapped_weight_name, weight)
                if hasattr(layer, 'output'):
                    if isinstance(layer.output, list):
                        for i, output in enumerate(layer.output):
                            tf.summary.histogram('{}_out_{}'.format(layer.name, i), output)
                    else: tf.summary.histogram('{}_out'.format(layer.name), layer.output)
            
        self.merged = tf.summary.merge_all()
        """
        TODO: fix
        """
        self.tf_physical_ids = tf.placeholder(tf.string, shape=(None,))
        self.summary_physical_ids = tf.summary.text('patient_ids', self.tf_physical_ids)

        self.pred_x = tf.placeholder(tf.string, shape=(None,))
        self.pred_y = tf.placeholder(tf.string, shape=(None,))
        self.summary_physical_ids = tf.summary.text('patient_ids', self.tf_physical_ids)
        
        """tensor graph & file write"""
        if self.write_graph: 
            self.writer = tf.summary.FileWriter(self.log_dir, self.sess.graph)
        else: 
            self.writer = tf.summary.FileWriter(self.log_dir)
        
        """embedding
        TODO: code
        """
        if self.embeddings_freq: self._summary_embedding(self.writer, model)

class TensorBoardPointDetectionMultiTaskWrapper(TensorBoardPointDetectionWrapper):
    '''
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol: 
                   input images: number of modals
                   bottleneck images : number of filters
                   output images: 2 (GT, predict)
        TODO: fix one person as reference
    '''
    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False, 
                 tb_data_steps=1, zcut=[0,0], downsampling_scale=1.,
                 **kwargs):
        super(TensorBoardPointDetectionMultiTaskWrapper, self).__init__(validation_data, write_graph, write_weights_histogram, write_weights_images, 
                                                               tb_data_steps, zcut, downsampling_scale, **kwargs)

    def set_model(self, model):
        self.model = model
        if k.backend() == 'tensorflow':
            self.sess = k.get_session()
        if self.histogram_freq and self.merged is None:
            """summary weight"""
            for layer in self.model.layers:
                for weight in layer.weights:
                    mapped_weight_name = 'weight_%s' % weight.name.replace(':', '_')
                    if self.write_weights_histogram: self._summary_histogram_per_weight(tf.summary, mapped_weight_name, weight)
                    if self.write_grads: self._summary_grad_histogram_per_weight(tf.summary, mapped_weight_name, weight, model)
                    if self.write_weights_images: self._summary_weight_images_per_weight(tf.summary, mapped_weight_name, weight)
                if hasattr(layer, 'output'):
                    if isinstance(layer.output, list):
                        for i, output in enumerate(layer.output):
                            tf.summary.histogram('{}_out_{}'.format(layer.name, i), output)
                    else: tf.summary.histogram('{}_out'.format(layer.name), layer.output)
            
        self.merged = tf.summary.merge_all()
        """
        TODO: fix
        """
        self.tf_physical_ids = tf.placeholder(tf.string, shape=(None,))
        self.summary_physical_ids = tf.summary.text('patient_ids', self.tf_physical_ids)

        self.pred_x = tf.placeholder(tf.string, shape=(None,))
        self.pred_y = tf.placeholder(tf.string, shape=(None,))
        self.summary_physical_ids = tf.summary.text('patient_ids', self.tf_physical_ids)
        
        """tensor graph & file write"""
        if self.write_graph: 
            self.writer = tf.summary.FileWriter(self.log_dir, self.sess.graph)
        else: 
            self.writer = tf.summary.FileWriter(self.log_dir)
        
        """embedding
        TODO: code
        """
        if self.embeddings_freq: self._summary_embedding(self.writer, model)






"""For classification
"""
class TensorBoardClassificationWrapper(TensorBoardSegmentationWrapper):
    '''
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol: 
                   input images: number of modals
                   bottleneck images : number of filters
                   output images: 2 (GT, predict)
        TODO: fix one person as reference
    '''
    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False, 
                 tb_data_steps=1, zcut=[0,0], downsampling_scale=1.,
                 **kwargs):
        super(TensorBoardClassificationWrapper, self).__init__(validation_data, write_graph, write_weights_histogram, write_weights_images, 
                                                               tb_data_steps, zcut, downsampling_scale, **kwargs)

    def _summary_3d_image(self, model):
        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[0] = input_shape[0] - np.sum(self.zcut)
        input_shape[1:3] = input_shape[1:3] * self.downsampling_scale
        assert (input_shape[0] > 0, 'Please check the zcut setting for 3D inputs. It should be small than image size')
        tot_pred_image = []
        for i in range(self.batch_size):
            # title = tf.strings.format("predicted: {}, label: {}", model.outputs[0][i], model.targets[0][i])
            input_img = self.resize3D(model.inputs[0][i], target_shape= input_shape, zcut=self.zcut)
            input_img_norm = k.stack([self.normalize(input_img[...,j:j+1]) for j in range(self.img_shape[-1])])
            pred_image = self.tile_patches_medical(input_img_norm, shape=input_shape)
            pred_image_col = self.colorize(pred_image, cmap='inferno')
            # pred_image_col = k.stack([self.colorize(pred_img, cmap='gray') for pred_img in pred_image]) # 'gist_yarg', 'inferno'
            tot_pred_image.append(pred_image_col)
        tot_pred_image = k.stack(tot_pred_image)
        return tot_pred_image

    def _summary_2d_image(self, model):
        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[0:2] = input_shape[0:2] * self.downsampling_scale
        
        # tot_title = tf.strings.format("predicted: {}, label: {}", model.outputs[0], model.targets[0])
        input_img = self.resize2D(model.inputs[0], target_shape= input_shape)
        input_img_norm = self.normalize(input_img)
        tot_pred_image = self.tile_patches_medical(input_img_norm, shape=input_shape)
        tot_pred_image_col = self.colorize(tot_pred_image, cmap='gray')
        # tot_pred_image_col = k.stack([self.colorize(pred_img, cmap='gray') for pred_img in tot_pred_image])
        return tot_pred_image_col

class TensorBoardClassificationAttentionWrapper(TensorBoardClassificationWrapper):
    '''
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol: 
                   input images: number of modals
                   bottleneck images : number of filters
                   output images: 2 (GT, predict)
        TODO: fix one person as reference
    '''

    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False, 
                 tb_data_steps=1, zcut=[0,0], downsampling_scale=1.,
                 **kwargs):
        super(TensorBoardClassificationAttentionWrapper, self).__init__(validation_data, write_graph, write_weights_histogram, write_weights_images, 
                                                               tb_data_steps, zcut, downsampling_scale, **kwargs)
    
    
    def _summary_3d_image(self, model):
        attention_layer_list = [l for l in model.layers if 'attention' in l.name]
        
        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += len(attention_layer_list)
        input_shape[0] = input_shape[0] - np.sum(self.zcut)
        input_shape[1:3] = input_shape[1:3] * self.downsampling_scale
        assert (input_shape[0] > 0, 'Please check the zcut setting for 3D inputs. It should be small than image size')
        tot_pred_image = []
        for i in range(self.batch_size):
            # title = tf.strings.format("predicted: {}, label: {}", model.outputs[0][i], model.targets[0][i])
            input_img = self.resize3D(model.inputs[0][i], target_shape= input_shape, zcut=self.zcut)
            input_img_norm = k.stack([self.normalize(input_img[...,j:j+1]) for j in range(self.img_shape[-1])])
            attention_norm_list = []
            for attention_layer in attention_layer_list:
                attention = attention_layer.output[1][i]
                # ratio =  k.int_shape(model.inputs[0][i])[0] // k.int_shape(attention)[0]
                # attention = k.repeat_elements(attention, ratio, axis=0)
                attention_resize = self.resize3D(attention, target_shape= input_shape, zcut=self.zcut)
                attention_norm = self.normalize(attention_resize)
                attention_norm_list.append(attention_norm)
            
            pred_image_list = [input_img_norm] + attention_norm_list
            pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
            pred_image_col = self.colorize(pred_image, cmap='inferno')
            # pred_image_col = k.stack([self.colorize(pred_img, cmap='gray') for pred_img in pred_image]) # 'gist_yarg', 'inferno'
            tot_pred_image.append(pred_image_col)
        tot_pred_image = k.stack(tot_pred_image)
        return tot_pred_image

    def _summary_2d_image(self, model):
        attention_layer_list = [l for l in model.layers if 'attention' in l.name]
        
        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += len(attention_layer_list)
        input_shape[0:2] = input_shape[0:2] * self.downsampling_scale
        
        input_img = self.resize2D(model.inputs[0], target_shape= input_shape)
        input_img_norm = self.normalize(input_img)
        attention_norm_list = []
        for attention_layer in attention_layer_list:
            # title = tf.strings.format("predicted: {}, label: {}", model.outputs[0], model.targets[0])
            attention = attention_layer.output[1]
            # ratio =  k.int_shape(model.inputs[0])[0] // k.int_shape(attention)[0]
            # attention = k.repeat_elements(attention, ratio, axis=0)
            attention_resize = self.resize2D(attention, target_shape= input_shape)
            attention_norm = self.normalize(attention_resize)
            attention_norm_list.append(attention_norm)
            
        pred_image_list = [input_img_norm] + attention_norm_list
        tot_pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
        tot_pred_image_col = self.colorize(tot_pred_image, cmap='gray')
        # tot_pred_image_col = k.stack([self.colorize(pred_img, cmap='gray') for pred_img in tot_pred_image])
        return tot_pred_image_col

class TensorBoardClassificationGuidedAttentionWrapper(TensorBoardClassificationAttentionWrapper):
    '''
    Sets the self.validation_data property for use with TensorBoard callback.
    
    Image Summary with multi-modal medical 3D volumes:  
        Thumbnail of nrow x ncol 2D images (of one person) 
            nrow: number of slice (z-axis)
            ncol: 
                   input images: number of modals
                   bottleneck images : number of filters
                   output images: 2 (GT, predict)
        TODO: fix one person as reference
    '''

    def __init__(self, validation_data, write_graph=False, write_weights_histogram = True, write_weights_images=False, 
                 tb_data_steps=1, zcut=[0,0], downsampling_scale=1.,
                 **kwargs):
        super(TensorBoardClassificationGuidedAttentionWrapper, self).__init__(**kwargs)
        self.write_weights_histogram = write_weights_histogram
        self.write_weights_images = write_weights_images
        self.tb_data_steps = tb_data_steps
        self.validation_data = validation_data
        self.img_shape = validation_data[0][0][0].shape[1:]
        self.zcut = zcut
        self.downsampling_scale = downsampling_scale
        
        if self.embeddings_data is None and self.validation_data:
            self.embeddings_data = self.validation_data
    
    def _summary_3d_image(self, model):
        attention_layer_list = [l for l in model.layers if 'attention' in l.name]
        marginal_attention_layer = [l for l in model.layers if 'marginal_attention' in l.name][0]
        prior_layer = [l for l in model.layers if 'prior' in l.name][0]

        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += 2 + len(attention_layer_list)
        input_shape[0] = input_shape[0] - np.sum(self.zcut)
        input_shape[1:3] = input_shape[1:3] * self.downsampling_scale
        assert (input_shape[0] > 0, 'Please check the zcut setting for 3D inputs. It should be small than image size')

        marginal_attention = marginal_attention_layer.output[0]
        marginal_attention = k.repeat_elements(marginal_attention, k.int_shape(model.inputs[0][0])[0] // k.int_shape(marginal_attention)[0], axis=0)
        marginal_attention = self.resize3D(marginal_attention, target_shape= input_shape, zcut=self.zcut)
        marginal_attention_norm = self.normalize(marginal_attention)
        prior = prior_layer.output[0]
        prior = k.repeat_elements(prior, k.int_shape(model.inputs[0][0])[0] // k.int_shape(prior)[0], axis=0)
        prior = self.resize3D(prior, target_shape= input_shape, zcut=self.zcut)
        prior_norm = self.normalize(prior)

        tot_pred_image = []
        for i in range(self.batch_size):
            # title = tf.strings.format("predicted: {}, label: {}", model.outputs[0][i], model.targets[0][i])
            input_img = self.resize3D(model.inputs[0][i], target_shape= input_shape, zcut=self.zcut)
            input_img_norm = k.stack([self.normalize(input_img[...,j:j+1]) for j in range(self.img_shape[-1])])
            attention_norm_list = []
            for attention_layer in attention_layer_list:
                attention = attention_layer.output[1][i]
                # ratio =  k.int_shape(model.inputs[0][i])[0] // k.int_shape(attention)[0]
                # attention = k.repeat_elements(attention, ratio, axis=0)
                attention_resize = self.resize3D(attention, target_shape= input_shape, zcut=self.zcut)
                attention_norm = self.normalize(attention_resize)
                attention_norm_list.append(attention_norm)
            
            pred_image_list = [input_img_norm] + attention_norm_list + [marginal_attention_norm, prior_norm]
            pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
            pred_image_col = self.colorize(pred_image, cmap='inferno')
            # pred_image_col = k.stack([self.colorize(pred_img, cmap='gray') for pred_img in pred_image]) # 'gist_yarg', 'inferno'
            tot_pred_image.append(pred_image_col)
        tot_pred_image = k.stack(tot_pred_image)
        return tot_pred_image

    def _summary_2d_image(self, model):
        attention_layer_list = [l for l in model.layers if 'attention' in l.name]
        marginal_attention_layer = [l for l in model.layers if 'marginal_attention' in l.name][0]
        prior_layer = [l for l in model.layers if 'prior' in l.name][0]

        input_shape = []
        input_shape[:] = self.img_shape[:]
        input_shape = np.array(input_shape)
        input_shape[-1] += len(attention_layer_list)
        input_shape[0:2] = input_shape[0:2] * self.downsampling_scale
        
        marginal_attention = marginal_attention_layer.output[0]
        marginal_attention = k.repeat_elements(marginal_attention, k.int_shape(model.inputs[0][0])[0] // k.int_shape(marginal_attention)[0], axis=0)
        marginal_attention = self.resize2D(marginal_attention, target_shape= input_shape, zcut=self.zcut)
        marginal_attention_norm = self.normalize(marginal_attention)
        prior = prior_layer.output[0]
        prior = k.repeat_elements(prior, k.int_shape(model.inputs[0][0])[0] // k.int_shape(prior)[0], axis=0)
        prior = self.resize2D(prior, target_shape= input_shape, zcut=self.zcut)
        prior_norm = self.normalize(prior)

        input_img = self.resize2D(model.inputs[0], target_shape= input_shape)
        input_img_norm = self.normalize(input_img)
        attention_norm_list = []
        for attention_layer in attention_layer_list:
            # title = tf.strings.format("predicted: {}, label: {}", model.outputs[0], model.targets[0])
            attention = attention_layer.output[1]
            # ratio =  k.int_shape(model.inputs[0])[0] // k.int_shape(attention)[0]
            # attention = k.repeat_elements(attention, ratio, axis=0)
            attention_resize = self.resize2D(attention, target_shape= input_shape)
            attention_norm = self.normalize(attention_resize)
            attention_norm_list.append(attention_norm)
            
        pred_image_list = [input_img_norm] + attention_norm_list + [marginal_attention_norm, prior_norm]
        tot_pred_image = self.tile_patches_medical(k.concatenate(pred_image_list, axis=-1), shape=input_shape)
        tot_pred_image_col = k.stack([self.colorize(tot_pred_image, cmap='gray')])
        # tot_pred_image_col = k.stack([self.colorize(pred_img, cmap='gray') for pred_img in tot_pred_image])
        return tot_pred_image_col