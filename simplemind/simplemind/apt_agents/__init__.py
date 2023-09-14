import csv, yaml, os
from simplemind.apt_agents.optimizer.ga.src.utils import binlist_to_hexstr
from simplemind.apt_agents.distributor.task import execute_tasks 

from simplemind.apt_agents.optimizer import Optimizer
from simplemind.apt_agents.distributor import BaseDistributor
#CondorDAGDistributor, DockerDistributor

from simplemind.apt_agents.distributor.task import _load_task_specific_functions
# from simplemind.apt_agents.distributor.condor.src.qia.logging_utils import generic_logger
import logging

def load(optimizer_config_path=None, distributor_config_path=None):
    optimizer = distributor = None
    if optimizer_config_path is not None:
        with open(optimizer_config_path, 'r') as f:
            optimizer_config = yaml.load(f, Loader=yaml.FullLoader)
        if optimizer_config.get("import") is not None:
            loaded_funcs = _load_task_specific_functions(optimizer_config["import"])
            optimizer = loaded_funcs.get("optimizer")
    if optimizer is None: optimizer = Optimizer

    if distributor_config_path is not None:
        with open(distributor_config_path, 'r') as f:
            distributor_config = yaml.load(f, Loader=yaml.FullLoader)
        if distributor_config.get("import") is not None:
            loaded_funcs = _load_task_specific_functions(distributor_config["import"])
            distributor = loaded_funcs.get("distributor")
    if distributor is None: distributor = BaseDistributor

    return optimizer, distributor

class Engine():
    def __init__(self, results_dir=None, job_distributor=None, log_path=None):
        
        self.results_dir = results_dir
        self.distributor = job_distributor
        self.log_path = log_path                # this may be needed to set up downstream logs
        
        ## have engine logger setup upon startup?
        # self._setup_logger(log_path)
        self.log = logging.getLogger("engine")
        return

        
    # def _setup_logger(self, log_path):
    #     engine_log = None
    #     if log_path is not None:
    #         engine_log = os.path.join(log_path, "engine.log")
    #     self.engine_logger = generic_logger(2, filepath=engine_log, name="engine")
    #     return

    ## each task should have the function, the script (template) to run, and the dependencies
    ## then each task should just take in a `parameter_set` (e.g. binary chromosome) 
    def set_tasks(self, tasks, dependencies=None):
        self.tasks = tasks
        self.dependencies = dependencies

    def load_dataset(self, cases):
        ### each case in `dataset` should have:
        #       id, dataset, image_file, reference
        with open(cases, newline='') as csvfile:
            reader = csv.DictReader(csvfile)
            self.cases = [dict(row) for row in reader]

        for task in self.tasks:
            task.set_case_list(self.cases)       
        return

    def get_args(self, parameter_set):
        args = []

        for task in self.tasks:
            taskwise_params = []
            casewise_params, casewise_infos = task.gen_params(parameter_set)
            ### each task_param is the params across a list of n patients
            ### or potentially population-wide
            taskwise_args = [(task, param, info) for param, info in zip(casewise_params, casewise_infos)]
            args.append(taskwise_args)

        return args

    ### based on chromosomes, prepare the job arguments (but don't run them yet)
    def prepare_jobs(self, parameter_sets):

        """
            Should we try to find/load the already-processed cases or have the jobs handle that?
            --> It is less efficient to have the jobs handle it but it is probably more streamlined.
            --> Only have the compile step decide that it's unfinished.
            
            How about already finished chromosomes? 
            --> Definitely more efficient to see if those are finished.
        """

        # canary_subset = None
        # if canary_obj is not None:
        #     canary_subset = canary_obj.canary_data

        # datasets = ( (canary_subset, "canary"), (name_input_ref_pair_lookup, "full") )
        self.parameter_sets = parameter_sets
        self.parameter_arguments = []
        for parameter_set in parameter_sets:
            try:
                encoded_parameter_set = binlist_to_hexstr(parameter_set)
            except:
                ### then it is already in hexstr or it is None
                encoded_parameter_set = parameter_set
            parameter_set_id = encoded_parameter_set
            if parameter_set_id is None:
                parameter_set_id = "DEFAULT"

            if self.finished(parameter_set_id):
                ### Store that it's already compiled, and load later
                continue    #check this

            args = self.get_args(encoded_parameter_set)   ### input param is a list of dictionaries/lists as a param for each task function
            
            # ### pass along objects
            # args = [self.tasks, tasks_param]
            self.parameter_arguments.append(args)
        return self.parameter_arguments ### in case it's wanted 

    def execute_jobs(self, parameter_arguments=None):
        if parameter_arguments is None:
            parameter_arguments = self.parameter_arguments
        # for cases_args in parameter_arguments:
        #     ## for each parameter_set ...
        #     if not cases_args: continue
        #     self.distributor.execute_jobs(execute_tasks, cases_args)

        self.distributor.execute_jobs(execute_tasks, parameter_arguments)

    def get_performances(self,):
        perf_dict = {}
        for parameter_set in self.parameter_sets:
            parameter_set_id = binlist_to_hexstr(parameter_set)
            parameter_set_performance_output = os.path.join(self.results_dir, parameter_set_id, "final.yml")
            if not os.path.exists(parameter_set_performance_output):
                return None
            with open(parameter_set_performance_output, 'r') as f:
                perf_metrics = yaml.load(f, Loader=yaml.FullLoader)
            fitness = perf_metrics["fitness"]
            perf_dict[parameter_set_id] = fitness
        # dictionary: {[encoded_parameter_set]: [performance_metric],}
        return perf_dict

    def finished(self, parameter_set_id):
        parameter_set_performance_output = os.path.join(self.results_dir, parameter_set_id, "final.yml")
        if os.path.exists(parameter_set_performance_output):
            return True
        return False
