import os
import threading
import sys
from simplemind.apt_agents.distributor.condor.src.creator import new
from simplemind.apt_agents.distributor.condor.src.utils import Cluster, iter_log, set_dag_job_hierarchy
import time, logging

import simplemind.apt_agents.distributor.condor.src.qia.scriptmaker as sm
from simplemind.apt_agents.distributor.condor.src.qia.temp import get_temp_dir
from simplemind.apt_agents.distributor.condor.src.qia.threadpool import ThreadPool, _AsynResultContainer
from simplemind.apt_agents.distributor.condor.src.qia.exceptions import NonZeroExitCode, TimedOut, ErrorValue, NotReady


class _AsynResult:
    def __init__(self, workpath):
        self.workpath = workpath
        self._value = None
        self._event = threading.Event()
        self._event.clear()
        
    def set(self, val):
        self._value = val
        self._event.set()
        
    def get(self, timeout=None):
        if self._event.wait(timeout):
            return self._value
        else:
            raise TimedOut

def _delete_file(file):
    while os.path.exists(file):
        try:
            os.remove(file)
        except:
            time.sleep(0.1)

## Simple object to make result consistent with map() operation ##
class DAGSubResult:
    def __init__(self, workpath):
        self.obj = sm.ScriptObject(workpath)
    def get(self):
        return self.obj.value
        
# each job is encapsulated in this object
### log_path here is not the general log_path, but the job-specific (ie case specific) log path used for outputting the Condor/distributor log files
class _ScriptJob:
    def __init__(self, workpath, prepcode, func, args, kwargs, creator, compute_env=None, watcher_dir=None, log_path=None):
        self.workpath = workpath
        self.creator = creator
        self.process = None
        self.log_path = log_path
        # self.condor_venv = condor_venv    #MWW 07272020
        # self.os_env = os_env    #MWW 07272020
        self.compute_env = compute_env
        self._submitted = False
        self.log = logging.getLogger("dist.condordag._scriptjob")
        ### Addressing the case that the cluster files are existing before and we ant to keep track of them again
        if self._exists(func, args, kwargs):
            self.obj = sm.ScriptObject(self.workpath)

            # what is this doing
            # probably recording which cluster it's in, so we can keep track of the job 
            ### Warning: this is actually not working well. TODO: fix this
            if os.path.exists(os.path.join(self.workpath, "cluster")):
                self.log.warning("Caution... probably does not work as anticipated")
                with open(os.path.join(self.workpath, "cluster"), "r") as f:
                    cluster = Cluster(int(f.read()), watcher_dir=watcher_dir) ### this will lead to error because Cluster() accepts the instantiated watcher
                for p in cluster.get_processes():
                    self.process = p
                    break
        else:
            _delete_file(os.path.join(self.workpath, sm.OUTPUT_FILENAME))
            _delete_file(os.path.join(self.workpath, sm.ERROR_FILENAME))
            _delete_file(os.path.join(self.workpath, "cluster"))
            self.obj = sm.make(self.workpath, prepcode, func, *args, **kwargs)
        
    def _exists(self, func, args, kwargs):
        if not (os.path.exists(os.path.join(self.workpath, sm.SCRIPT_FILENAME) or not os.path.exists(os.path.join(self.workpath, sm.INPUT_FILENAME)))):
            return False
        if os.path.exists(os.path.join(self.workpath, sm.SCRIPT_FILENAME)):
            try:
                if isinstance(sm.retrieve(self.workpath), ErrorValue):
                    return False
            except EOFError:
                return False
        return sm.is_same(self.workpath, func, *args, **kwargs)
        
    def done(self):
        outfile = os.path.join(self.workpath, sm.OUTPUT_FILENAME)
        if os.path.exists(outfile) and os.stat(outfile).st_size>0:
            return True
        return False
        
    def submit(self):   #TODO: implement GPU options here (how many GPUs, minimum GPU memory) for normal condor usage too 
        if self._submitted:
            return
        self._submitted = True
        if self.done():
            return
        if self.process is not None:
            if self.process.is_active():
                return
            self.remove()
        condor_script = os.path.join(self.workpath, "script.condor")
        job = self.creator(condor_script, self.obj.script , compute_env=self.compute_env, log_path=self.log_path) #MWW 07272020
        job["transfer_input_files"].append(os.path.join(os.path.dirname(self.obj.script), sm.INPUT_FILENAME))
        job["transfer_input_files"].append(os.path.join(os.path.dirname(self.obj.script), sm.SCRIPT_FILENAME))
        job["transfer_output_files"].append(sm.OUTPUT_FILENAME)
        cluster = job.submit()
        # self.log.debug(cluster)
        self.process = cluster.get_processes()[0]
        # self.log.debug(os.path.join(self.workpath, "cluster"))

        with open(os.path.join(self.workpath, "cluster"), "w") as f:
            f.write(str(cluster.id))
        
    def wait(self, timeout=None, retrynum=None):   #MWW 07012020 change timeout to None
        if self.done():
            self.log.debug("Output is finished preparing. Done waiting.")
            return
        if self.process is None:
            raise ValueError("No process to wait on!")
        retry_count = 0
        while True:
            try:
                self.log.debug("WAITING ATTEMPT >>> timeout is {}".format(timeout))
                self.process.wait(timeout)
                break
            except TimedOut:
                self.log.warning("Timed out in _ScriptJob")
                break
                # pass
            except NonZeroExitCode:
                ### This should happen if there is a problem with the GA Watcher sub-job.
                self.log.error("Fatal error from job or sub-job.")
                raise
            except:
                self.log.error("Unexpected error: {}".format( sys.exc_info()[0]))
                raise

        condor_log_file = self.process.log
        self.log.debug("Parsing through the Condor log to see what happened. If NonZeroExitCode found, then returning NonZeroExitCode to be handled.")
        if condor_log_file is None:
            condor_log_file = os.path.join(self.workpath, "log", "%s.%s.log" % (self.process.id, self.process.pid))
            if not os.path.exists(condor_log_file):
                self.log.warning("Condor log file does not exist! Cannot parse.") ### TODO: Do I need to handle this in some way?
                return
        info = None
        for i in iter_log(condor_log_file, id=self.process.id, pid=self.process.pid):
            info = i
        if info["title"]=="Job terminated.":
            if info["content"][0].find("return value 0")>=0:
               pass
            else:
                self.log.info("WAITING JOB TERMINATED, NonZeroExitCode raised.")
                raise NonZeroExitCode #should this be raised? if so, it should be caught somewhere
        self.log.debug("WAITING FINISHED")


    def remove(self):
        if self.process is not None:
            self.process.remove()
            self.process = None
        
    def retrieve(self):
        return self.obj.value
        
    def create(self, gpus=0, gpu_memory=0):
        if self._submitted:
            return
        # self._submitted = True
        if self.done():
            return
        if self.process is not None:
            if self.process.is_active():
                return
            self.log.info("Removing due to inactivity, in 'create' method.")
            self.remove()
        condor_script = os.path.join(self.workpath, "script.condor")
        job = self.creator(condor_script, self.obj.script , compute_env=self.compute_env, log_path=self.log_path) #MWW 07272020
        job["transfer_input_files"].append(os.path.join(os.path.dirname(self.obj.script), sm.INPUT_FILENAME))
        job["transfer_input_files"].append(os.path.join(os.path.dirname(self.obj.script), sm.SCRIPT_FILENAME))
        job["transfer_output_files"].append(sm.OUTPUT_FILENAME)

        if gpus>0:
            job["request_gpus"] = gpus
        else:
            job["request_gpus"] = 0
        if gpu_memory>0:    #assume GPU memory is given in GB, so convert it to MB
            gpu_memory*= 1000
            ### requirements will now be in format of:
            # Requirements = ((Machine == "REDLRADADM14958.ad.medctr.ucla.edu") || (Machine  == "REDLRADADM23710.ad.medctr.ucla.edu") ) && (GPUMEM >= 46000)
            job["requirements"] = "(%s) && (%s)"%(job["requirements"], "GPUMEM >= %s"%(str(gpu_memory)) )
        else:
            if gpus> 0:
                self.log.info("No GPU memory requirements")
        return job.create()


    ### Not ready to use ###
    def find_cluster(self):
        ## what if i don't have the cluster file
        if os.path.exists(os.path.join(self.workpath, "cluster")):
            with open(os.path.join(self.workpath, "cluster"), "r") as f:
                cluster = Cluster(int(f.read()))
            for p in cluster.get_processes():
                self.process = p
                break


        
#submits the actual job
# need to have this submitted more often than just 3 times
def _async_script_job_wrapper(asyn_result, job, timeout, retrynum=5, dag=False, sub_pool_size=15, watcher_dir=None, log_path=None, compute_env=None):
    log = logging.getLogger("dist.condordag.asyncscriptjobwrapper")
    try:    #MWW 07282020
        if dag:
            job = job.submit_dag(max_jobs=sub_pool_size, watcher_dir=watcher_dir, log_path=log_path, compute_env=compute_env)
        else:
            job.submit()    # (1) Submit job
        log.debug("Job Submitted: {}".format( job))
        time.sleep(15)      # MWW 11042021, to allow time for Condor dag to be submitted and ready before being checked
    except NonZeroExitCode:
        log.debug("NonZeroExitCode from job.submit() in pool.py ... Retrying ...")
        i = 0
        done = False
        while i < 5:
            try:
                log.debug("Retrying condor job submission... Try {}".format(i))
                # wait for 5 seconds
                time.sleep(5)
                job.submit()
                done = True
                break
            except:
                pass
            i+=1
        if not done:
            job.remove()
            asyn_result.set(ErrorValue(sys.exc_info())) #asyn result is carried on
            log.debug("Removed after job.submit()")
            log.debug(ErrorValue(sys.exc_info()))
            return True #unsure if this should return True or False

    try:
        interval_seconds = 15
        log.debug("Timeout: {} Retrynum {} interval seconds {}".format(timeout, retrynum, interval_seconds))
        job.wait(timeout, interval_seconds=interval_seconds) # (2) Check status (and wait if not finished)

        # retrieve just tries to find the open file

        if not dag:
            log.debug("Retrieving results from `output`")
            log.debug(asyn_result)
            asyn_result.set(job.retrieve()) # (3) Retrieves results from output file
            log.debug("Done retrieving results.")
        else:
            # retrieve the job somehow
            # potentially making a wrapper object for Process
            # but also don't really need this
            pass
            # asyn_result.set(job.retrieve())
    except TimedOut:
        log.warning("Timed out in async_job_wrapper in pool.py, after either job.wait or job.retrieve() ")
        if not dag:
            asyn_result.set(TimedOut)
        else:
            raise(TimedOut)
    except NotReady:
        i = 0
        done = False
        while i < 5:
            try:
                log.debug("Retrying retrieval... Try ", i)
                # wait for 5 seconds
                time.sleep(5)
                if not dag:
                    asyn_result.set(job.retrieve())
                done = True
                break
            except:
                pass
            i+=1
        if not done:
            job.remove()
            asyn_result.set(ErrorValue(sys.exc_info())) #asyn result is carried on
            log.debug(ErrorValue(sys.exc_info()))
            #should this return False? or does it even mattter?
    return True

        
class CondorPool:
    def __init__(self, poolsize=None, workernum=None, error_callback=None, timeout=None, retrynum=1, log_path=None):
        self.log = logging.getLogger("dist.condor")
        workernum = poolsize
        self.log.debug("CONDORPOOL: poolsize: {} workernum {}".format(poolsize, workernum))
        self._pool = ThreadPool(parallelnum=workernum, poolsize=poolsize, error_callback=error_callback)
        self._timeout = timeout
        self._retrynum = retrynum
        self.log_path = log_path
    def __del__(self):
        self.close()

    def set_timeout(self, timeout):
        self._timeout = timeout

    # done individually for each job, within asyn result container
    # initiated via pool put (_pool is a threadpool)
    # _async_script_job_wrapper function is in charge of the actual submit + watching the job + removing when finished
    def apply(self, func, args=None, kwargs=None, timeout=None, prepcode=None, workpath=None, creator=None, compute_env=None, cur_log_path=None):
        if args is None:
            args = []
        if kwargs is None:
            kwargs = {}
        if workpath is None:
            workpath = get_temp_dir()
        job = _ScriptJob(workpath, prepcode, func, args, kwargs, creator, compute_env=compute_env, log_path=cur_log_path) #MWW 07272020
        ret = _AsynResult(workpath)
        self.log.debug("Applying in condorpool async_script_job_wrapper")
        # this happens for all jobs in the beginning
        self._pool.put(_async_script_job_wrapper, args=(ret, job, self._timeout, self._retrynum), timeout=timeout)
        return ret

    # map will get an async_map output (just a list of the functions + inputs + depenednecies)
    #   which will be iterated through in the asyn result container
    #       --> async result container will call "apply"
    def map(self, func, seq, prepcode=None, workpath=None, creator=None, workpath_generator=None, compute_env=None):
        async_res = self.async_map(func, seq, prepcode, workpath, creator, workpath_generator, compute_env=compute_env)
        return _AsynResultContainer(async_res)
    
    #preps functions into list to be called sequentially
    # will do for all jobs in the input sent to map (in seq)
    def async_map(self, func, seq, prepcode=None, workpath=None, creator=None, workpath_generator=None, compute_env=None ):
        is_temp = False
        if workpath is None and workpath_generator is None:
            is_temp = True
            workpath = get_temp_dir()
        count = 0
        ret = []

        for s in seq:
            if workpath_generator is None:
                cur_wp = cur_logpath = os.path.join(workpath, "job"+str(count))
                if is_temp:
                    cur_wp = workpath.alias(cur_wp)
            else:
                cur_wp, cur_logpath = workpath_generator(func, s) 
                #### TODO: make this workpath
            ret.append(self.apply(func, args=(s,), prepcode=prepcode, workpath=cur_wp, log_path=cur_logpath, creator=creator, compute_env=compute_env, cur_log_path=cur_logpath))
            #this is being limited by the thread count
            count += 1
            # self.log("async_map job #{}".format(count))
        return ret
                
    def join(self, timeout=None):
        return self._pool.join(timeout)
        
    def close(self):
        self._pool.close()
        
    def __enter__(self):
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False



"""
Wasil removed the cmd line alternatives for cleanliness. 
"""
class CondorPoolDAG:
    # Pool size will be the number of parallel chromosomes
    def __init__(self, poolsize=None, sub_pool_size=15, workernum=None, error_callback=None, timeout=None, retrynum=1, log_path=None):
        workernum = poolsize
        self._pool = ThreadPool(parallelnum=workernum, poolsize=poolsize, error_callback=error_callback)
        self._timeout = timeout
        self._retrynum = retrynum
        self.sub_pool_size = sub_pool_size
        self.log_path = log_path
        self.log = logging.getLogger("dist.condordag")
        self.log.debug("CONDORPOOLDAG: poolsize: {} workernum {} sub_poolsize {}".format(poolsize, workernum, sub_pool_size))
    def __del__(self):
        self.close()

    def set_timeout(self, timeout):
        self._timeout = timeout

    # done individually for each job, within asyn result container
    # initiated via pool put (_pool is a threadpool)
    # _async_script_job_wrapper function is in charge of the actual submit + watching the job + removing when finished
    # TODO: either implement timeout feature, or take it out
    def apply(self, func, args=None, kwargs=None, timeout=None, prepcode=None, workpath=None, creator=None, compute_env=None, gpus=0, gpu_memory=0, watcher_dir=None, cur_log_path=None):
        if args is None:
            args = []
        if kwargs is None:
            kwargs = {}
        if workpath is None:
            workpath = get_temp_dir()

        ### Add the option to include GPU requirement in here
        job = _ScriptJob(workpath, prepcode, func, args, kwargs, creator, compute_env=compute_env, watcher_dir=watcher_dir, log_path=cur_log_path) #MWW 07272020
        # self.log.debug("Creating job but not running:")
        return job.create(gpus=gpus, gpu_memory=gpu_memory)

    # map will get an async_map output (just a list of the functions + inputs + depenednecies)
    #   which will be iterated through in the asyn result container
    #       --> async result container will call "apply"
    def map(self, func, seq, prepcode=None, workpath=None, creator=None, workpath_generator=None, compute_env=None, gpu_requirement=None ):
        async_res = self.async_map(func, seq, prepcode, workpath, creator, workpath_generator, compute_env=compute_env, gpu_requirement=gpu_requirement)
        return async_res
        
    #preps functions into list to be called sequentially
    # will do for all jobs in the input sent to map (in seq)
    def async_map(self, func, seq, prepcode=None, workpath=None, creator=None, workpath_generator=None, compute_env=None, gpu_requirement=None ):
        # seq needs to be a list of lists
        #   outer list: list of chromosomes that will each be under one async
        #   inner list: list of jobs under one chromosome
        is_temp = False
        if workpath is None and workpath_generator is None:
            is_temp = True
            workpath = get_temp_dir()
        count = 0
        ret = []


        max_sequential_jobs = 3                            ### hard coded sequential jobs
        for chrom in seq:
            watcher_dir = None       
            chr_jobs = []
            chr_ret = []
            temp_c = 0
            hierarchy=-1
            for tasks in chrom:
                sequential_job_counter = 0
                hierarchy+=1
                for task, param, info in tasks: ### for now assume there's only one task per job -- TODO: make generalizable in case more than one task exists per job
                    # hierarchy+= 1
                # for s in chrom["case_infos"]:
                    gpus = 0
                    gpu_memory = 0
                    if workpath_generator is None:
                        cur_wp = cur_logpath = os.path.join(workpath, "job"+str(count))
                        if is_temp:
                            cur_wp = workpath.alias(cur_wp)
                    else:
                        cur_wp, cur_logpath = workpath_generator(task.task, info)    ## might need to take task.task's __name__ into `info` 


                    ### SM Runner specific processing... maybe make generalizable later ###
                    if watcher_dir is None:
                        try:
                            watcher_dir = param.get("watcher")
                            self.log.debug("Successfully set watcher directory.")
                        except:
                            pass
                    ### TODO: have this defined somewhere else
                    if self.log_path is None:
                        if watcher_dir is not None:
                            self.log_path = os.path.join(os.path.dirname(os.path.dirname(watcher_dir)), "error")

                        else:
                            try:
                                self.log_path = os.path.join(param["working_dir"], "error")
                            except:
                                self.log.warning("Warning: Error path is not defined.")
                    #######################################################################

                    ### Handling GPU requirements
                    """
                    gpu_requirement:
                        n_gpus: 1
                        gpu_memory_filter: 20000
                        n_jobs: 3     # set None if you want it to be all jobs that a GPU is required
                    """
                    if gpu_requirement is not None and info.get("gpu_required", False):  
                        max_sequential_jobs = gpu_requirement.get("n_jobs", max_sequential_jobs)

                        if sequential_job_counter < gpu_requirement.get("n_jobs", 100000000000000000000000000000000000000000000000):
                            gpus = gpu_requirement.get("n_gpus", 0)
                            gpu_memory = gpu_requirement.get("gpu_memory_filter", 0)
                            sequential_job_counter+=1
                        if sequential_job_counter <= max_sequential_jobs:
                    ##TODO: double check that this works whenever sequential is not necessary
                            hierarchy+= 1
                    #######################################
                    else:
                        ### TODO: Make it so that all tasks are parallel unless specified by something passed in (task/info/compute env)
                        if sequential_job_counter <= max_sequential_jobs:
                            hierarchy+= 1



                    ### NOTE: this is where the GPU information gets fed per case in
                    ## In the future, `func` can be replaced by the relevant function actually
                    condor_script = self.apply(func, args=((task,param),), prepcode=prepcode, workpath=cur_wp, creator=creator, compute_env=compute_env, gpus=gpus, gpu_memory=gpu_memory, watcher_dir=watcher_dir, cur_log_path=cur_logpath)
                    job = dict()
                    job["job_path"] = condor_script
                    job["job_id"] = os.path.basename(cur_wp)
                    job["hierarchy"] = hierarchy
                    chr_ret.append(DAGSubResult(os.path.dirname(condor_script)))
                        
                    chr_jobs.append(job)

                    count += 1
                    temp_c+= 1
                    # self.log.debug("async_map job #{}".format(count))
                    # input("")
            # self.log.debug(chr_jobs)
            self.log.debug("Created {} Condor job submission files.".format(temp_c))
            ret.append(_AsynResultContainer(chr_ret))

            # just using latest info file
            dag_path, _ = workpath_generator(None, info, dag=True)
            job = self.dag_apply(chr_jobs, dag_path)
            
            self._pool.put(_async_script_job_wrapper, args=(None, job, self._timeout, self._retrynum, True, self.sub_pool_size, watcher_dir, self.log_path, compute_env), timeout=None)



        return ret
    

    ### TODO: another way to create dag job hierarchy

    # (1) make contents
    # (2) dag workpath
    # (3) watching the process
    # where are the ids coming from? 
    # how about hierarchies

    """
    Assumes that there's always a 0 hierarchy
    And that the hierarchy goes in numerical order from 0
    """
    def dag_apply(self, jobs, dag_path):
        
       # (1) get the contents 
        contents = set_dag_job_hierarchy(jobs)
        
        # (2) write the dagpath 
        os.makedirs(os.path.dirname(dag_path), exist_ok=True)
        
        with open(dag_path, 'w') as f:
            f.write(contents)

        # just make process here 
        job = new(dag_path, None) #MWW 07272020

        return job
    def join(self, timeout=None):
        return self._pool.join(timeout)
        
    def close(self):
        self._pool.close()
        
    def __enter__(self):
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False