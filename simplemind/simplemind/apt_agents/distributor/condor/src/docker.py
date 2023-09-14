
from simplemind.apt_agents.distributor.condor.src.qia.temp import get_temp_dir
from simplemind.apt_agents.distributor.condor.src.qia.threadpool import ThreadPool
from simplemind.apt_agents.distributor.condor.src.qia.exceptions import NonZeroExitCode, TimedOut, ErrorValue, NotReady
from simplemind.apt_agents.distributor.condor.src.qia.executecmd import execute_command
import simplemind.apt_agents.distributor.condor.src.qia.scriptmaker as sm
import time, os


DEFAULT_DOCKER_IMAGE = "registry.cvib.ucla.edu/sm_release:ga_072522_2"


# DOCKER_INIT_CMD = "docker run --mount type=bind,source=/apps,target=/apps --mount type=bind,source=/cvib2,target=/cvib2 --mount type=bind,source=/scratch,target=/scratch --mount type=bind,source=/scratch2,target=/scratch2 --mount type=bind,source=/radraid,target=/radraid -it"



### module for semi-intellectually finding open GPU core to use in Docker ###
### if nvidia_smi is missing, then run:
###                            pip install nvidia-ml-py3 
# https://stackoverflow.com/questions/59567226/how-to-programmatically-determine-available-gpu-memory-with-tensorflow
# https://github.com/nicolargo/nvidia-ml-py3
def select_free_gpu(min_memory_gb, n_gpu=1):
    import nvidia_smi
    nvidia_smi.nvmlInit()
    n_gpu_collected = 0
    open_gpu_cores = []
    for i in range(nvidia_smi.nvmlDeviceGetCount()):
        # if i > 3:
        #     continue
        handle = nvidia_smi.nvmlDeviceGetHandleByIndex(i)
        info = nvidia_smi.nvmlDeviceGetMemoryInfo(handle)

        memory_gb = info.free/1024/1024
        print(i, memory_gb, min_memory_gb)
        if memory_gb > min_memory_gb:
            n_gpu_collected+=1
            open_gpu_cores.append(i)
            if n_gpu_collected == n_gpu:
                nvidia_smi.nvmlShutdown()
                return open_gpu_cores

        print("Total memory:", info.total)
        print("Free memory:", info.free)
        print("Used memory:", info.used)

    nvidia_smi.nvmlShutdown()

    return None

def build_docker_command(docker_image, script, drives=None):
    drive_mounts = ""
    if drives is not None:    # convert from dictionary to str
        drive_mounts = " ".join([f"-v {k}:{v}" for k, v in drives.items()])

    UID = os.getuid()
    GID = os.getgid()
    base_command = f"docker run -it -u {UID}:{GID}" ### TODO: eventually instantiate this by config for user
    
    docker_command = " ".join([base_command, drive_mounts, docker_image, "python", script])
    return docker_command


class DockerPool:
    def __init__(self, poolsize=None, workernum=None, error_callback=None, timeout=None, retrynum=1, single_job=False):
        if poolsize == 1:
            single_job = True
        self.return_func = None
        if not single_job:
            workernum = poolsize
            print("DOCKERPOOL: poolsize:", poolsize, "workernum", workernum)
            try:
                self._pool = ThreadPool(parallelnum=workernum, poolsize=poolsize, error_callback=error_callback)
            except:
                i = 0
                finished = False
                while i < retrynum or finished: 
                    i+=1
                    print("ThreadPool init failed, trying again...")
                    time.sleep(5)

                    self._pool = ThreadPool(parallelnum=workernum, poolsize=poolsize, error_callback=error_callback)
                    finished = True
            self._timeout = timeout
            self._retrynum = retrynum
        else:
            print("DOCKERPOOL: Single-job non-pool version")
        self.single_job = single_job

    def set_timeout(self, timeout):
        self._timeout = timeout
    def set_return_func(self, return_func): 
        self.return_func = return_func
        # This is a return function to pair up with map, so that the value returned is not always the status (0) 
        # Theoretically would a function that would wrap the "exe_cmd" in map and then have an extra line that runs _get_evaluation, and returns that
        return 

    ### `creator`` is vestigial
    def map(self, func, seq, prepcode=None, workpath=None, creator=None, workpath_generator=None, compute_env=None, gpu_requirement=None):
        
        if compute_env is None:
            compute_env = dict()
        docker_image = compute_env.get("image", DEFAULT_DOCKER_IMAGE)

        is_temp = False
        if workpath is None and workpath_generator is None:
            is_temp = True
            workpath = get_temp_dir()

        count = 0
        ret = []

        ### assume it's flattened
        # for s in seq:
        # print(len(seq))
        # print(len(seq[0]))
        # for task, param, info in seq:   ## assume 1 task per job for now
        #     print(task)
        #     input(param)
        for task, param, info in seq:   ## assume 1 task per job for now

            input_prepcode = prepcode 
            if workpath_generator is None:
                if workpath is None: raise("Workpath for job is not specified.")    #TODO: better backup plan for this
                cur_wp = os.path.join(workpath, "job"+str(count))
                if is_temp:
                    cur_wp = workpath.alias(cur_wp)
            else:
                cur_wp = workpath_generator(task.task, info)
            # if isinstance(param, dict):
            #     param = dict(param)
            args=((task, param),)

            # input(gpu_requirement)
            if gpu_requirement is not None:  
                if gpu_requirement.get("n_gpus", 0) > 0 and gpu_requirement.get("gpu_memory_filter", 0) > 0:
                    free_gpu_cores = select_free_gpu(float(gpu_requirement["gpu_memory_filter"]), n_gpu=int(gpu_requirement["n_gpus"]))
                    if free_gpu_cores is None:
                        ### TODO: Improve this so GPU cores can be used in parallel, and also to wait if GPU core is not available
                        raise("No local GPU cores are available and/or sufficient to run your process locally.")
                    if prepcode is not None:
                        if isinstance(prepcode, str):
                            input_prepcode = [prepcode, ]
                        else: 
                            input_prepcode = list(prepcode)
                    else:
                        input_prepcode = []
                    input_prepcode.append('os.environ["CUDA_DEVICE_ORDER"]="PCI_BUS_ID"')
                    input_prepcode.append('os.environ["CUDA_VISIBLE_DEVICES"]="%s"'%(",".join([str(gpu) for gpu in free_gpu_cores])))
            # print(input_prepcode)
            # input()
            kwargs ={}
            obj = sm.make(cur_wp, input_prepcode, func, *args, **kwargs)
            exe_cmd = build_docker_command(docker_image, obj.script, drives=compute_env.get("drives") )
            ### by default, running this command will return value of 0 if there is no error
            ### so, if you would like to extract the results, you can specify a return function

            if self.return_func is None:
                exe_cmd = exe_cmd.split(" ")
            else:
                exe_cmd = [exe_cmd, args] 
            ret.append(exe_cmd)

            #this is being limited by the thread count
            count += 1
            print("Docker threading job #", count)
            # input(" ".join(exe_cmd))
            print(" ".join(exe_cmd))
            ### TODO: check if these are unique .... 
            # print(param)
            # print("=================")
            # input()
        
        if self.single_job:
            print("Mapping.>>>>>>>", len(ret))
            # map(print, printer)
            if self.return_func is not None:
                return map(self.return_func, ret)
            return map(execute_command, ret)
        if self.return_func is not None:
            return self._pool.map(self.return_func, ret)
        return self._pool.map(execute_command, ret)
    def __del__(self):
        self.close()
    def join(self, timeout=None):
        if not self.single_job:
            return self._pool.join(timeout)
        
    def close(self):
        if not self.single_job:
            self._pool.close()
        
    def __enter__(self):
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        if not self.single_job:
            self.close()
        return False
    # def _map(self, func, seq, prepcode=None, workpath=None, creator=None, workpath_generator=None, condor_venv="", os_env="ldap"):
    #     async_res = self.async_map(func, seq, prepcode, workpath, creator, workpath_generator, condor_venv=condor_venv, os_env=os_env)
    #     return _AsynResultContainer(async_res)
    # def apply(self, func, args=None, kwargs=None, timeout=None, prepcode=None, workpath=None, creator=None, condor_venv="", os_env="ldap"):
    #     job = _ScriptJob(workpath, prepcode, func, args, kwargs, creator, condor_venv=condor_venv, os_env=os_env) #MWW 07272020

    # def async_map(self, func, seq, prepcode=None, workpath=None, creator=None, workpath_generator=None, condor_venv="", os_env="ldap" ):
    #     if creator is None:
    #         creator = default_creator
    #     is_temp = False
    #     if workpath is None and workpath_generator is None:
    #         is_temp = True
    #         workpath = get_temp_dir()
    #     count = 0
    #     ret = []

    #     for s in seq:
    #         if workpath_generator is None:
    #             cur_wp = os.path.join(workpath, "job"+str(count))
    #             if is_temp:
    #                 cur_wp = workpath.alias(cur_wp)
    #         else:
    #             cur_wp = workpath_generator(func, s)
    #         ret.append(self.apply(func, args=(s,), prepcode=prepcode, workpath=cur_wp, creator=creator, condor_venv=condor_venv, os_env=os_env))
    #         #this is being limited by the thread count
    #         count += 1
    #         print("async_map job #", count)
    #     return ret



