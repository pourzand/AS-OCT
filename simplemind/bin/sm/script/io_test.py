import os
import h5py
import numpy as np

class Writer(object):
    def __init__(self, fname, xdims, labels, ydims=None, overwrite=False):
        """Creates a writer object that is capable of storing data in a stream like manner.
        :param fname: The file to be created for writing. 
        :param xdims: A list containing the dimension/shape of X.
        :param labels: A list of labels which each item in Y represents.
        :param ydims: A list containing the dimension/shape of Y if it is different from labels.
        :param overwrite: Disable file exists exception. Will delete existing file.
        """
        if os.path.exists(fname):
            if not overwrite:
                raise IOError("%s already exists! Use overwrite=True if you want to replace it.")
            os.remove(fname)
        self.store = h5py.File(fname)
        self.X = self.store.create_dataset("X", (0,)+tuple(xdims), maxshape=(None,)+tuple(xdims))
        if ydims is None:
            self.Y = self.store.create_dataset("Y", (0,len(labels)), maxshape=(None,len(labels)))
        else:
            self.Y = self.store.create_dataset("Y", (0,)+tuple(ydims), maxshape=(None,)+tuple(ydims))
        self.labels = list(labels)
        
        maxlen = max([len(i) for i in self.labels])
        en_labels = [str(n).encode("ascii", "ignore") for n in self.labels]
        self.store.create_dataset("label", (len(en_labels),1), 'S%s' % maxlen, en_labels)
        self._closed = False

    def __del__(self):
        if not self._closed:
            self.close()
            
    def add(self, x, y):      
        for dataset, val in zip((self.X, self.Y), (x,y)):
            new_shape = list(dataset.shape)
            ind = new_shape[0]
            new_shape[0] += 1
            dataset.resize(tuple(new_shape))
            dataset[ind,:] = val

    def close(self):
        self.store.close()
        self._closed = True
        
    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.close()
        return False
        
class Reader(object):
    def __init__(self, fname):
        if not os.path.exists(fname):
            raise IOError("Cannot find %s" % fname)
        self.store = h5py.File(fname)
        self.X = self.store["X"]
        self.Y = self.store["Y"]
        self.labels = {s.tostring().rstrip(b'\0').decode("utf-8", 'ignore'):i for s, i in zip(self.store["label"], range(self.store["label"].shape[0]))}
        self._closed = False
        
    def get_labels(self):
        return sorted(self.labels.keys(), key=lambda x: self.labels[x])
        
    def get_X(self):
        return self.X
        
    def get_Y(self, labels=None):
        if labels is None:
            return self.Y
        else:
            index = tuple([self.labels[i] for i in labels])
            return self.Y[:, index]
        
    def __del__(self):
        if not self._closed:
            self.close()
        
    def close(self):
        self.store.close()
        self._closed = True
        
    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.close()
        return False
