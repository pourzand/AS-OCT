from simplemind.apt_agents.distributor.utils import MapFunc
from functools import partial
import yaml, os
from simplemind.apt_agents.distributor.utils import argshash_workpath_generator, ga_hashfunc, log_hashfunc

from simplemind.apt_agents.distributor.condor.src.pool import CondorPoolDAG, ThreadPool
from simplemind.apt_agents.distributor.condor.src.docker import DockerPool

from simplemind.apt_agents.distributor.utils import condor_creator
import logging

class BaseDistributor():
    __name__ = "BaseDistributor"
    def __init__(self, distributor_config=None, distributor_config_path=None, log_path=None):
        self.distributor_config = distributor_config
        if distributor_config_path is not None:
            self.load_distributor_config(distributor_config_path)
        self.dependencies = None 
        self.log_path = log_path
        self.log = logging.getLogger("dist")
        self.set_pool()
        return

    ### rough equivalent to pooling_args
    ### TODO: Finish this
    def load_distributor_config(self, distributor_config_path):
        with open(distributor_config_path, 'r') as f:
            self.distributor_config = yaml.load(f, Loader=yaml.FullLoader) 
        return

    def set_job_mapper(self,):
        self.job_mapper = MapFunc(map=map)
    
    ## Override ##
    def set_pool(self,):
        return

    def prep_jobs(self, jobs):
        ### Flattening for sequential jobs ###
        flattened_jobs = []
        for parameter_set_jobs in jobs:
            for taskwise_jobs in parameter_set_jobs:
                readied_taskwise_jobs = [(task, task_param) for task, task_param, info in taskwise_jobs]
                flattened_jobs.extend(readied_taskwise_jobs)
            
            # for taskwise_jobs in parameter_set_jobs: 
                # for casewise_jobs in taskwise_jobs:
                    # tasks, task_params = parameter_set_jobs
                # [flattened_jobs.extend(task_jobs) for task_jobs in taskwise_jobs]
        return flattened_jobs

    def _get_pool_size(self, jobs):
        pool_size =  len(jobs) if len(jobs) < self.distributor_config.get("parallel_n", 5) else self.distributor_config.get("parallel_n", 5)
        return pool_size

    def execute_jobs(self, func, jobs):
        map_results = []
        self.set_job_mapper()
        self.log.debug("Mapping jobs...")

        jobs = self.prep_jobs(jobs)

        map_result = self.job_mapper.map(func, jobs)
        self.log.debug("Successfully mapped jobs...")
        try:
            if self.job_mapper is not None:
                self.log.debug("Attempting join...")
                self.job_mapper.join()
                self.log.debug("Join Done...")
                self.job_mapper.close() 
                self.log.debug("Closed Done.")
        except:
            self.log.debug("Join failed")

        map_results.append([x for x in map_result])
        return map_results
    ### requires TaskManager to plug into the distributor
    def set_dependencies(self, dependencies):
        self.dependencies = dependencies
        return

### Handling of args translated into parallel 
class ThreadDistributor(BaseDistributor):
    __name__ = "ThreadDistributor"
    # def __init__(self,):
    #     self.flatten = False
    #     return super().__init__()
    def set_pool(self,):
        self.pool = ThreadPool

    def _get_pool_size(self, jobs):
        pool_size =  len(jobs) if len(jobs) < self.distributor_config.get("parallel_n", 5) else  self.distributor_config.get("parallel_n", 5)
        return pool_size

    def set_job_mapper(self, pool=None):
        if pool is None:
            pool = self.pool
        self.job_mapper = MapFunc( map=pool.map,
                            map_join=pool.join,
                            map_close=pool.close,)

    def execute_jobs(self, func, jobs):
        map_results = []
        pool_size = self._get_pool_size(jobs)
        with self.pool(pool_size) as pool:
            self.distributor_config["pool"] = pool
            self.set_job_mapper(pool)
            self.log.debug("Mapping jobs...")
            jobs = self.prep_jobs(jobs)
            map_result = None
            self.log.debug("DEBUG: job length...")
            self.log.debug(len(jobs))
            map_result = self.job_mapper.map(func, jobs)
            self.log.debug("Successfully mapped jobs...")
            try:
                if self.job_mapper.map is not None:
                    self.log.debug("Attempting join...")
                    self.job_mapper.join()
                    self.log.debug("Join Done...")
                    self.job_mapper.close() 
                    self.log.debug("Closed Done.")
            except:
                self.log.debug("Join failed")

        map_results.append([x for x in map_result])
        return map_results




# os.makedirs(computing_management["condor_workpath"], exist_ok=True)

## Runs each job sequentially if pool_size == 1, (within each chromosome, then by chromosome)
# for parameter_set in parameter_sets:
#   for job in flattened_jobs:
#       #run single job
#
## Runs `pool_size` jobs in parallel if pool_size > 1, (within each chromosome, then by chromosome)
# for parameter_set in parameter_sets:
#   for job in flattened_jobs:
#       #run `pool_size` number of parallel jobs


class DockerDistributor(ThreadDistributor):
    def __init__(self, distributor_config=None, distributor_config_path=None, log_path=None):
        self.workpath_generator = None
        return super().__init__(distributor_config=distributor_config, distributor_config_path=distributor_config_path,log_path=log_path)
    ### TODO: this is only needed if you want to return a specific value
    def set_return_func(self, return_func):
        self.pool.set_return_func(return_func) 
    def set_pool(self,):
        self.pool = DockerPool

    def set_job_mapper(self, pool=None):
        if pool is None:
            pool = self.pool
        pool.set_timeout(self.distributor_config.get("timeout"))
        poolmap = partial(pool.map, workpath_generator=self.workpath_generator, compute_env=self.distributor_config.get("docker"), prepcode=self.dependencies, gpu_requirement=self.distributor_config.get("gpu_requirement"))
        self.job_mapper = MapFunc( map=poolmap,
                            map_join=pool.join,
                            map_close=pool.close,
                            map_is_alive = None,
                            )

    def execute_jobs(self, func, jobs):
        if self.workpath_generator is None:
            self.set_workpath_generator()
        return super().execute_jobs(func, jobs)



    def set_workpath_generator(self):
        self.workpath_generator = partial(argshash_workpath_generator, wp_hashfunc=ga_hashfunc, dist_rootpath=self.distributor_config["condor_workpath"], log_rootpath=os.path.join(self.log_path, "dist"), logpath_hashfunc=log_hashfunc)



## Runs each chromosome sequentially if pool_size == 1. Runs the jobs within the chromosome in parallel by sub_pool_size.
# for parameter_set in parameter_sets:
#   for task_params in task_divided_params:     ## currently there is a list of n_task, each of which is a case_wise list of `n_cases` long (i.e. runs case-wise across each task)
#       # task_params is the list of case-wise jobs for each task
#       # this needs to be turned into a cascade of jobs
#
## Runs `pool_size` jobs in parallel if pool_size > 1, (within each chromosome, then by chromosome)
# for parameter_set in parameter_sets:
#   for job in flattened_jobs:
#       #run `pool_size` number of parallel jobs

class CondorDAGDistributor(ThreadDistributor):
    __name__ = "CondorDAGDistributor"
    def __init__(self,distributor_config=None, distributor_config_path=None, log_path=None):
        self.workpath_generator = None
        return super().__init__(distributor_config=distributor_config, distributor_config_path=distributor_config_path, log_path=log_path)
        
    def set_pool(self,):
        # self.pool = partial(CondorPoolDAG, sub_pool_size=self.distributor_config["parallel_sub_n"], error_path=self.distributor_config["error_path"], )
        ### Error path will now be stored in "log_path" provided by __init__
        ### TODO: remove the above first line.
        self.pool = partial(CondorPoolDAG,poolsize=self.distributor_config["parallel_n"],sub_pool_size=self.distributor_config["parallel_sub_n"], log_path=self.log_path, )

    def prep_jobs(self, jobs):
        return jobs

    def set_job_mapper(self, pool=None):
        if pool is None:
            pool = self.pool
        poolmap = partial(pool.map, workpath_generator=self.workpath_generator, creator=condor_creator, compute_env=self.distributor_config.get("docker"), prepcode=self.dependencies, gpu_requirement=self.distributor_config.get("gpu_requirement"))
        self.job_mapper = MapFunc( map=poolmap,
                            map_join=pool.join,
                            map_close=pool.close,
                            map_is_alive = pool._pool._pool_join_thread.is_alive,   ## only difference with dockerdist
                            )

    def execute_jobs(self, func, jobs):
        if self.workpath_generator is None:
            self.set_workpath_generator()
        map_results = []
        with self.pool() as pool:
            self.distributor_config["pool"] = pool
            self.set_job_mapper(pool)
            pool.set_timeout(self.distributor_config.get("timeout"))    ### probably more efficient elsewhere
            self.log.debug("Mapping jobs...")
            map_result = None
            map_result = self.job_mapper.map(func, jobs)   ## only differences ###TODO: investigate why this is needed
            self.log.debug("Sucessfully mapped jobs...")
            try:
                if self.job_mapper.map is not None:
                    self.log.debug("Attempting join...")
                    self.job_mapper.join()
                    self.log.debug("Join 1 Done...")
                    self.job_mapper.close()
                    self.log.debug("Closed 1 Done.")
            except:
                self.log.debug("Join failed")

        map_results = map_result    ## only differences

        return map_results

    


    
    def set_workpath_generator(self):
        self.workpath_generator = partial(argshash_workpath_generator, wp_hashfunc=ga_hashfunc, dist_rootpath=self.distributor_config["condor_workpath"], log_rootpath=os.path.join(self.log_path, "dist"), logpath_hashfunc=log_hashfunc)
