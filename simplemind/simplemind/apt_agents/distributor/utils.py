import yaml, os


class MapFunc():
    def __init__(self, pool=None, pool2=None, map=None, map_close=None, map_join=None, map_is_alive=None):
        self.pool = pool
        if map is not None:
            self._map = map
        elif pool is not None:
            self._map = pool.map
        else:
            raise("Map not defined.")
        
        if map_close is not None:
            self._map_close = map_close
        elif pool is not None:
            self._map_close = pool.close
        else:
            self._map_close = self.return_none
        
        if map_join is not None:
            self._map_join = map_join
        elif pool is not None:
            self._map_join = pool.join
        else:
            self._map_join = self.return_none        
        
        if map_is_alive is not None:
            self._map_is_alive = map_is_alive
        else:
            self._map_is_alive = self.return_none
                
    def return_none(self):
        return None
    def map(self, *args):
        return self._map(*args)
        
    def close(self):
        return self._map_close()
        
    def join(self):
        return self._map_join()

    def is_alive(self):
        return self._map_is_alive()





import time

#### Supporting functions ####

### default hashfunc for reference  ###
# def hashfunc(x):
#     t = hashlib.new('ripemd160')
#     t.update("|".join([repr(i) if not isfunction(i) else i.__name__ for i in x]).encode('utf-8'))
#     return t.hexdigest()

### TODO: FINISH THIS
def argshash_workpath_generator(func, args, dist_rootpath, wp_hashfunc, log_rootpath, logpath_hashfunc, dag=False):
    if not dag:
        arghash = wp_hashfunc(args)
        wp = os.path.join(dist_rootpath, "%s-%s" % (func.__name__, arghash))
    else:
        arghash = wp_hashfunc(args)
        wp = os.path.join(dist_rootpath, "dag", "%s.condor" % (arghash))

    lp = os.path.join(log_rootpath, logpath_hashfunc(args))

    return wp, lp

def ga_hashfunc(info):
    epoch_time = str(int(time.time()))
    return "%s_%s_%s"%(info.get("parameter_set", "NA"), info.get("id", "NA"), epoch_time)

def ga2_0_hashfunc(x):
    parameter_set_id = x[0]
    input_info = x[1]
    epoch_time = str(int(time.time()))
    return "%s_%s_%s"%(parameter_set_id, input_info.get("id", "NA"), epoch_time)

def log_hashfunc(info):
    return os.path.join(info.get("parameter_set", "NA"), info.get("id", "NA"))


### TODO: Determine if this is condor specific and move if necessary
from simplemind.apt_agents.distributor.condor.src.creator import new_sh
def condor_creator(condor_script, condor_template, compute_env=None, log_path=None):
    ## can insert a different job creator triggered by something in compute_env later
    
    ### Formerly "default_creator", simplified to call `new_sh` directly
    job = new_sh(condor_script, compute_env=compute_env, log_path=log_path)

    return job

### Filler function for taking arguments but doing nothing
def filler_func(*args, **kargs):
    return 

