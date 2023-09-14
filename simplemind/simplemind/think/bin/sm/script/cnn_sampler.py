"""Sampler classes family for CNN

Spec
---
batch_generator
ABC base_sampler
simple_uniform_sampler

TODO 
---
@youngwonchoi
    1. simple_oversampling_sampler
"""
import numpy as np
from keras.utils import Sequence

# import __qia__

class batch_generator(Sequence):
    """Batch Generator
    Inherit the keras.utils.Sequence class
    to avoid duplicating data to multiple workers
    
    Attributes
    ----------
    self.reader : object
        instanse of a reader class of reader class family
        from cnn_reader
    self.sampler: object
        instanse of a sampler class of sampler class family
        from cnn_sampler
    self.batch_size : int
        size of one mini-batch
    self.steps : int
        length of the generator
    
    Methods
    -------
    on_epoch_end()
        set the callback for the epoch end
    __len__()
    __getitem__()

    Usage
    -----
    Use this batch generator when using multiple workers
    to avoid duplicating data

    Examples
    --------

    """
    def __init__(self, reader, sampler, augmentator):
        """
        Parameters
        ----------
        reader : object
            instance of a reader class of reader class family
            from cnn_reader
        sampler : object
            instance of a sampler class of sampler class family
            from cnn_sampler
        augmentator : object
            instance of a augmentator class from cnn_augmentation
        """
        self.reader = reader
        self.sampler = sampler
        self.augmentator = augmentator
        self.batch_size = sampler.get_batch_size()
        self.steps = sampler.get_steps()
        self.reader.on_training_start()
        self.sampler.on_training_start()
        self.augmentator.on_training_start()
            
    def __len__(self):
        return self.steps

    def __getitem__(self, idx):
        idx_list = self.sampler.get_batch_idxs(idx)
        # print(f'Case: {idx_list[0]}')
        # print(self.reader.get_train_set_case_info(idx_list[0]))
        mini_batches = self.reader.get_mini_batch(idx_list)
        return self.augmentator.augment(mini_batches)
    
    def on_epoch_end(self):
        self.reader.on_epoch_end()
        self.sampler.on_epoch_end()
        self.augmentator.on_epoch_end()


class base_sampler(object):
    """Base Sampler Class
    Abstract base class of a class family for sampling mini-batch.
    Primal example of the child calss is simple_uniform_sampler class. 
    This subclass is simply build a mini-batch with uniform sampling.

    Parameters
    ----------
    config_model : dict
        dictionary of model configuration information
    log : object
        logging class instance

    Attributes
    ----------
    self.train_idx : ndarray
        ndarray of int index set mapping cases within training set
    self.log : object
        logging class instance
    self.batch_size : int (default = 1)
        size of mini-batch
    self.sequential : bool (default = False)
        whether to build mini-batches sequentially.
        If not, indexes for mini-batch will be randomly sampled 
        based on self.probability
    self.replace : bool (default = False)
        When the index for mini-batch is sampled randomly,
        whether to sample index with replace.
        If sequential = True, this attribute will be ignored.
    self.subsets_per_epoch : int (default = None)
        Subset of cases to used for one epoch. As default, 
        whole set of train_idx will be used.
    self.steps_per_epoch : int (default = None)
        Number of mini-batch (iterations) within one epoch.
        (lentgh of the sampler) If not specified, 
        it will be calculated autometically based on
        size of given training-idx and batch-size.
    self.epoch_idxs : ndarray
        ndarray of index set. If self.subsets_per_epoch is None,
        This will be self.train_idx. If not, subset of index with
        size self.subsets_per_epoch will be uniformly sampled from
        self.train_idx.
    self.probability : ndarray
        ndarray of probability (float; range=[0,1])
        weight for random sampling index to build a mini-batch
        The probabilities associated with each entry in self.epoch_idxs.

    Methods
    -------
    

    Usage
    -----
    You can inherit this class to implement a new 
    sampler class with a own probability sampling function, 
    if you need to specify a sampling weight.

    Examples
    --------

    """
    def __init__(self, train_idx, log, batch_size,
                sequential = False, replace = False,
                subsets_per_epoch = None, steps_per_epoch = None):
        """
        Parameters
        ----------
        train_idx : ndarray
            ndarray of int index set mapping cases within training set
        log : object
            logging class instance
        batch_size : int
            size of mini-batch
        sequential : bool (default = False)
            whether to build mini-batches sequentially.
            If not, indexes for mini-batch will be randomly sampled 
            based on self.probability
        replace : bool (default = False)
            When the index for mini-batch is sampled randomly,
            whether to sample index with replace.
            If sequential = True, this attribute will be ignored.
        subsets_per_epoch : int (default = None)
            Size of the subset of cases to used for one epoch. 
            If not specified, whole set of train_idx will be used.
        steps_per_epoch : int (default = None)
            Number of mini-batches (iterations) within one epoch.
            If the value is note given, number of mini-batches per one epoch
            will be calculated autometically based on
            size of given training-idx and batch-size.
        """
        
        self.train_idx = train_idx
        self.log = log
        
        self.batch_size = batch_size
        self.sequential = sequential
        self.replace = replace
        self.subsets_per_epoch = subsets_per_epoch
        if steps_per_epoch == None:
            if self.subsets_per_epoch == None:
                self.steps_per_epoch = np.ceil(self.train_idx.shape[0]/self.batch_size).astype(np.int)
            else: 
                self.steps_per_epoch = np.ceil(self.subsets_per_epoch/self.batch_size).astype(np.int)
        else:
            self.steps_per_epoch = None
        
        self.probability = None
        self.epoch_idxs = None
    
    def set_probability_vector(self): #, information=None):
        """Set the probability Vector for Probability sampling
        Set the weight of random sampling (self.probability) to be
        a float 1-D ndarray. If self.subsets_per_epoch is None,
        The size of self.probability will be as same as the length 
        of self.training_idx. Else, The size of self.probability will be 
        self.subsets_per_epoch.
        The sum of self.probability should be 1. (proper probability)

        Parameters
        ----------
        information : None
            any information to build a probability.
            For example, this can be a class of y.
            It will be ignored if information is not given.

        Examples
        --------

        Raises
        ------
        NotImplementedError
            You should have to implement this function for generating 
            a sampler subclass

        TODO: set information to reader class and get it as a parameter
        """
        raise NotImplementedError()
        
    def probability_sampling(self, idxs, size, replace=False):
        """ Probability based Index Sampling Functions
        
        Parameters
        ----------
        idxs : ndarray
            index set
        size : int
            size of sample
        replace : bool (default = False)
            whether to sample idxs with replacement or without replacement

        Returns
        -------
        ndarray
            sampled index set with given size
        """
        try:
            return np.random.choice(idxs, size=size, replace=replace, p=self.probability)
        except Exception as e:
            self.log.error(e)
            raise ValueError(f'Probability sampling failed. {e}')
    
    def on_training_start(self):
        """callback function for training start
        """
        self._set_epoch_idxs()
        if self.sequential:
            """TODO: needed?"""
            np.random.shuffle(self.epoch_idxs)
        else: 
            self.set_probability_vector()
        
    def on_epoch_end(self):
        """callback function for epoch end
        """
        if self.sequential:
            np.random.shuffle(self.epoch_idxs)
        else:
            self._set_epoch_idxs()
            self.set_probability_vector()
    
    def _set_epoch_idxs(self):
        """Set the subset of index set self.epoch_idxs 
        to be used in one epoch.
        If self.subsets_per_epoch is None, self.epoch_idxs will
        be self.train_idx. If not, self.subsets_per_epoch indexs
        will be uniformly sampled from self.train_idx.
        """
        if self.subsets_per_epoch != None: 
            self.epoch_idxs = np.random.choice(self.train_idx, self.subsets_per_epoch, replace=False)
        else: 
            self.epoch_idxs = self.train_idx

    def get_batch_idxs(self, i):
        """
        Return the set of indexs for {i}th mini-batch.
        """
        if self.sequential:
            idxs = self.epoch_idxs[i*self.batch_size:min((i+1)*self.batch_size, self.epoch_idxs.shape[0])]
        else: 
            idxs = self.probability_sampling(self.epoch_idxs, self.batch_size, replace=self.replace)
        self.current_physical_id = [str(i) for i in idxs]
        # self.get_physical_id(self.epoch_idxs[idxs])
        return idxs
        
    # def get_physical_id(self, idxs):
    #     return self.reader.get_x_list(idxs)[:,0]
    
    """getter"""
    def get_proability_vector(self):
        return self.probability
    
    def get_batch_size(self):
        return self.batch_size
    
    def get_steps(self):
        return self.steps_per_epoch

    def get_epoch_idxs(self):
        return self.epoch_idxs

    def get_current_physical_id(self):
        return self.current_physical_id

class simple_uniform_sampler(base_sampler):
    """Simple Uniform Sampler Class
    A primal child class of within a class family for sampling mini-batch.
    This subclass is simply build a mini-batch with uniform sampling.

    Parameters
    ----------
    config_model : dict
        dictionary of model configuration information
    log : object
        logging class instance

    Attributes
    ----------
    self.train_idx : ndarray
        ndarray of int index set mapping cases within training set
    self.log : object
        logging class instance
    self.batch_size : int (default = 1)
        size of mini-batch
    self.sequential : bool (default = False)
        whether to build mini-batches sequentially.
        If not, indexes for mini-batch will be randomly sampled 
        based on self.probability
    self.replace : bool (default = False)
        When the index for mini-batch is sampled randomly,
        whether to sample index with replace.
        If sequential = True, this attribute will be ignored.
    self.subsets_per_epoch : int (default = None)
        Subset of cases to used for one epoch. As default, 
        whole set of train_idx will be used.
    self.steps_per_epoch : int (default = None)
        Number of mini-batch (iterations) within one epoch.
        (lentgh of the sampler) If not specified, 
        it will be calculated autometically based on
        size of given training-idx and batch-size.
    self.epoch_idxs : ndarray
        ndarray of index set. If self.subsets_per_epoch is None,
        This will be self.train_idx. If not, subset of index with
        size self.subsets_per_epoch will be uniformly sampled from
        self.train_idx.
    self.probability : ndarray
        ndarray of probability (float; range=[0,1])
        weight for random sampling index to build a mini-batch
        The probabilities associated with each entry in self.epoch_idxs.

    Methods
    -------
    

    Usage
    -----

    Examples
    --------

    """
    def __init__(self, train_idx, log, batch_size,
                sequential = False, replace = False,
                subsets_per_epoch = None, steps_per_epoch = None):
        """
        Parameters
        ----------
        train_idx : ndarray
            ndarray of int index set mapping cases within training set
        log : object
            logging class instance
        batch_size : int
            size of mini-batch
        sequential : bool (default = False)
            whether to build mini-batches sequentially.
            If not, indexes for mini-batch will be randomly sampled 
            based on self.probability
        replace : bool (default = False)
            When the index for mini-batch is sampled randomly,
            whether to sample index with replace.
            If sequential = True, this attribute will be ignored.
        subsets_per_epoch : int (default = None)
            Size of the subset of cases to used for one epoch. 
            If not specified, whole set of train_idx will be used.
        steps_per_epoch : int (default = None)
            Number of mini-batches (iterations) within one epoch.
            If the value is note given, number of mini-batches per one epoch
            will be calculated autometically based on
            size of given training-idx and batch-size.
        """
        super(simple_uniform_sampler,self).__init__(train_idx, log, batch_size, 
                                                    sequential, replace, 
                                                    subsets_per_epoch, steps_per_epoch)
    
    def set_probability_vector(self): #, information=None):
        """Set the probability Vector for Probability sampling
        Set the weight of random sampling (self.probability) to be
        a float 1-D ndarray. If self.subsets_per_epoch is None,
        The size of self.probability will be as same as the length 
        of self.training_idx. Else, The size of self.probability will be 
        self.subsets_per_epoch.
        The sum of self.probability should be 1. (proper probability)

        Parameters
        ----------
        information : None
            any information to build a probability.
            For example, this can be a class of y.
            It will be ignored if information is not given.

        Examples
        --------
        """
        self.probability = np.ones(self.epoch_idxs.shape[0])
        self.probability = self.probability / np.sum(self.probability)