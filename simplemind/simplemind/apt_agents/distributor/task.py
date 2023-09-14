import yaml, os, glob, time
from simplemind.apt_agents.distributor.condor.src.qia.scriptmaker import get_import_statement
from functools import partial

### Temporary ###
# from simplemind.apt_agents.optimizer.ga.src.evaluate.cxr_trachea import compile_function, evaluate_task

import importlib
import logging


def load_task_config(task_config_path,):
    with open(task_config_path, 'r') as f:
        task_config = yaml.load(f, Loader=yaml.FullLoader)
    tasks = []
    # for task_id, task_info in task_config["tasks"].items():
    for task_info in task_config["tasks"]:
        task_id = task_info["id"]
        if task_info.get("import") is not None:
            loaded_funcs = _load_task_specific_functions(task_info["import"])
            task_obj = loaded_funcs["task"](config=task_info, task=loaded_funcs["task_func"], checker=loaded_funcs["task_checker"](), param_gen_func=loaded_funcs["task_param_generator"])
            tasks.append(task_obj)

    return tasks, task_config

def _load_task_specific_functions(import_param):
    loaded_funcs = dict()
    for filepath, funcs in import_param.items():
        module = importlib.import_module(filepath)
        for tar_func, orig_func in funcs.items():
            loaded_funcs[tar_func] = getattr(module,  orig_func)
    return loaded_funcs


class TaskManager():
    def __init__(self, task_config_path=None, model=None, results_path=None, extra=None):
        # task_config = dict()
        # if task_config_path is not None:
        #     with open(task_config_path, 'r') as f:
        #         task_config = yaml.load(f, Loader=yaml.FullLoader)
        #     if task_config.get("skip") is not None:
        #         skip = dict()
        #         for k in task_config["skip"]:
        #             skip[k] = True
        #         task_config["skip"] = skip 

        self.task_config = self._load_tasks(task_config_path)
        self.model = model
        self.results_path = results_path
        self.extra = extra

    def _load_tasks(self, task_config_path):
        # if task_config_path is not None:
        tasks, task_config = load_task_config(task_config_path)
        self.tasks = tasks
        # else:
            # self.tasks = [ SMRunnerTask(config=task_config, task=sm_runner, checker=SM_Checker(), param_gen_func=gen_smrunner_param), ]
        #                 EvalTask(config=task_config, task=evaluate_task, checker=Evaluator_Checker(), param_gen_func=gen_evaluate_param), 
        #                 CompileTask(config=task_config, task=compile_function, checker=Compiler_Checker(), param_gen_func=gen_compile_param)]
        return task_config
    
    # def load_tasks(self, task_config_path):
    #     with open(task_config_path, 'r') as f:
    #         tasks_config = yaml.load(f, Loader=yaml.FullLoader)
        
    #     for task in tasks_config["tasks"]:
    #         task_obj = load_class()


    ### TODO: implement in BaseTasks
    def tailor_tasks(self, engine, optimizer, distributor, ):
        for task in self.tasks:
            task.tailor(engine, optimizer, distributor)
        return

    ### make sure to tailor task before setting up
    def setup_tasks(self,):
        for task in self.tasks:
            task.setup(task.config, self.model, self.results_path,  extra=self.extra)
        return        
    
    def get_tasks(self):
        return self.tasks

    def get_dependencies(self,):
        # prepcode = [
        #     get_import_statement(segment_func),
        #     get_import_statement(eval_func),
        # ]
        prepcode = [get_import_statement(task) for task in self.tasks]
        return prepcode

######## Task executor wrapper #####################

def execute_tasks(param):
    log = logging.getLogger("dist.task")
    tasks, tasks_param = param
    if not isinstance(tasks, list): ### temporary way to handle single_task inputs
        tasks = [tasks,]
        tasks_param = [tasks_param,]
    # overall_start = time.time()
    previous_output = None
    for i, param_input, task in zip(range(len(tasks)), tasks_param, tasks):
        task.checker.update_output_info(param_input)
        if task.checker.task_finished():
            try:
                output = task.checker.load_output()      ### ensure the output can be loaded
                continue
            except:
                log.warning("Task finished but Task output could not be loaded. Rerunning.")
        reset_checkers(tasks, i)   
        # time_elapsed = time.time() - starttime
        output = task.task(param_input, previous_output)
        # time_elapsed = time.time() - starttime
        if not task.checker.task_finished(output):
            reset_checkers(tasks, 0)   
            output, finished = task.checker.rerun_tasks(i, tasks_param, tasks, previous_output)

            if not finished:
                raise("Task", i, task.__name__, "failed to complete.")
        previous_output = output
    # time_elapsed = time.time() - starttime
    return output

def reset_checkers(tasks, i):
    for i_checker, task in enumerate(tasks):
        if i_checker > i:
            task.checker.set_unfinished()
    return



############### INPUT GENERATORS #################


## Defines input parameters for smrunner
def gen_smrunner_param(parameter_set, case, model_file, results_dir, watcher_base_dir=None, roi_dir=None, resource_dir=None,  working_dir=None, cpu_preprocessing_only=False, skip=None, force_overwrite=True):
    case_dict = case
    parameter_set_id = parameter_set
    # if (parameter_set is not None and not parameter_set.strip('0')) or parameter_set is None:
    if parameter_set is None:
        parameter_set = None
        parameter_set_id = "DEFAULT"

    watcher_dir = None
    if watcher_base_dir is not None:
        watcher_dir = os.path.join(watcher_base_dir, parameter_set_id)

    skip_params = dict()
    if skip is not None:
        if skip.get("train_screenshots", False):
            skip_params["skip_png_training"] = True
        if skip.get("pred_screenshots", False):
            skip_params["skip_png_prediction"] = True
        if skip.get("tensorboard", False):
            skip_params["skip_tensorboard"] = True
    
    param_input = {
        "id": case_dict["id"],                          #what is this for LCS -- probably the recon_id + the reconstruction params
        "image_path": case_dict["image_file"],
        "sn_entry_path": model_file,
        "output_dir": os.path.join(results_dir, parameter_set_id, case_dict["id"], "sm_runner"),               #case directory for results
        "roi_dir": roi_dir,                  #case directory for premade input rois  # MWW 09162020
        "working_directory": working_dir,          #working directory for preprocessing  # MWW 01302021
        "watcher": watcher_dir,          #working directory for preprocessing  # MWW 01302021
        "user_resource_directory": resource_dir,    #
        "cpu_preprocessing_only": cpu_preprocessing_only,           #whether we're skipping processing of anything
        "force_overwrite": force_overwrite,
        "chromosome": parameter_set,
        "log_file": os.path.join(results_dir, parameter_set_id, case_dict["id"], "log", "sm_runner.log")
    }

    param_input.update(skip_params)
    return param_input


from simplemind.apt_agents.optimizer.utils import _gen_ref

##### Needed for GA 3.0
def gen_evaluate_param(chrom, case, base_results_dir="", skip_images=False):
    chrom_id = chrom
    if chrom is None:
        chrom_id = "DEFAULT"
    
    reference_path = _gen_ref(case)
    result_path = os.path.join(base_results_dir, chrom_id, case["id"], "sm_runner")
    outpath = os.path.join(base_results_dir, chrom_id, case["id"], "eval")
    
    return (result_path, reference_path, outpath, case["dataset"], case["id"], skip_images)

def gen_compile_param(chrom, cases=None, base_results_dir="", report_templates=None, rerun=False, canary=None):
    # cases, outpath, rerun, canary = param
    chrom_id = chrom
    if chrom is None:
        chrom_id = "DEFAULT"
    outpath = os.path.join(base_results_dir, chrom_id)

    
    return (cases, outpath, rerun, report_templates, canary)



class BaseChecker():
    def __init__(self, output_info=None):
        self.log = logging.getLogger("dist.task")
        if output_info is not None:
            self.update_output_info(output_info)


    def update_output_info(self, output_info):
        self.output_info=output_info

        self.results_file = self.output_info.get("results_file")


    ### what if it's reprocessing task #1 and task #2 needs updated?
    def task_finished(self, task_output=None):
        ### if we didn't just process it (ie task_output is None), check if results_file is there
        if task_output is None:
            if os.path.exists(self.results_file):
                return True
            else:
                return False

        ### if we did just process it, check if the process actually finished (that task_output is a dictionary that has 'finished')
        if task_output.get("finished", False):
            self._create_done_file(task_output)
            return True
        return False
        
    def load_output(self):
        with open(self.results_file, 'r') as f:
            output = yaml.load(f, Loader=yaml.FullLoader)
        return output

    def rerun_tasks(self, i, tasks_param, tasks, previous_output):
        finished = False
        try:
            ### Wait 15 seconds.. try it again..
            time.sleep(15)
            output = tasks[i].task(tasks_param[i], previous_output)
            finished = True
        except Exception as e: 
            self.log.warning("Exception in running Task {} {}: {}".format(i, task[i].__name__, e))
            ### Wait 15 seconds.. try it again..
            time.sleep(15)
            self.log.warning("Running previous steps again:")
            for j, param_input, task in zip(range(len(tasks)), tasks_param, tasks):
                output = task.task(param_input, previous_output)
                if j==i:
                    finished = True
                    break 
        return output, finished
    
    def _create_done_file(self, task_output):
        if not os.path.exists(self.results_file):
            with open(self.results_file, "w") as f:
                f.write(yaml.dump(task_output))

        return 

    def set_unfinished(self,):
        if os.path.exists(self.results_file): os.remove(self.results_file)
        return


class Evaluator_Checker(BaseChecker):
    def update_output_info(self, output_info):
        result_path, reference_path, outpath, dataset, case_id, skip_images = output_info
        self.output_info=output_info
        # self.results_file = self.output_info.get("eval_file")
        # if self.results_file is None:
        self.results_file = os.path.join(os.path.dirname(result_path), "eval_res.yml")


class Compiler_Checker(BaseChecker):
    def update_output_info(self, output_info):
        cases, outpath, rerun, report_templates, canary = output_info

        # self.results_file = self.output_info.get("compile_res")
        # if self.results_file is None:
        self.results_file = os.path.join(outpath, "final.yml")

class SM_Checker(BaseChecker):            
    def update_output_info(self, output_info):
        self.output_info=output_info
        output_dir = output_info["output_dir"]
        self.output_info['sm_img'] = os.path.join(output_dir, "dicom.seri") ## Placeholder
        self.output_info['sm_done'] = os.path.join(output_dir, "file_list.txt")
        self.output_info['sm_error_logs'] = [os.path.join(output_dir, "error_log_*.log"), ] 

        ### placeholder
        if self.output_info.get("sm_res") is None:
            self.output_info['sm_res'] = os.path.join(os.path.dirname(output_dir), "sm_res.yml")

    def task_finished(self, task_output=None):
        ### if we didn't just process it (ie task_output is None), check if "sm_res.yml" is there
        if task_output is None:
            if os.path.exists(self.output_info["sm_res"]):
                return True
            else:
                return False

        ### if we did just process it, check if the process actually finished (3 different checkpoints)
        if not os.path.exists(self.output_info['sm_img']):
            img_path = os.path.join(os.path.split(self.output_info['sm_img'])[0], "source_image.txt")
            with open(img_path, 'r') as f:
                image_path = f.read()
            self.output_info['sm_img'] = image_path

        self.log.debug("sm_img: {}".format(self.output_info.get("sm_img", "")))
        self.log.debug("sm_done: {}".format( self.output_info.get("sm_done", "")))
        no_error = True 
        error_logs = self.output_info.get("sm_error_logs")
        if error_logs is not None:
            for error_log in error_logs:
                self.log.debug("Looking for... {}".format(error_log))
                if "*" in error_log:
                    detected_errors = glob.glob(error_log)
                    if detected_errors: 
                        no_error = False
                else:
                    if os.path.exists(error_log):
                        no_error = False 
                self.log.debug("No errors? {}".format( no_error))
        finished = os.path.exists(self.output_info.get("sm_img", "")) and os.path.exists(self.output_info.get("sm_done", "")) and no_error

        ### if these are finished, then you can also make the done file
        if finished:
            task_output = dict(result_path=self.output_info["output_dir"])
            self._create_done_file(task_output)
        return finished

    def load_output(self):
        with open(self.output_info["sm_res"],'r') as f:
            output = yaml.load(f, Loader=yaml.FullLoader)
        return output

    def rerun_tasks(self, i, tasks_param, tasks, previous_output):
        finished = False
        try:
            ### Wait 15 seconds.. try it again..
            time.sleep(15)
            output = tasks[i].task(tasks_param[i], previous_output)
            finished = True
        except Exception as e: 
            self.log.warning("Exception in running Task {} {}: {}".format(i, task[i].__name__, ":", e))
            ### Wait 15 seconds.. try it again..
            time.sleep(15)
            self.log.warning("Running previous steps again:")
            for j, param_input, task in zip(range(len(tasks)), tasks_param, tasks):
                output = task.task(param_input, previous_output)
                if j==i:
                    finished = True
                    break 
        return output, finished

    ## Should be dictionary with "result_path" key
    def _create_done_file(self, task_output):
        if not os.path.exists(self.output_info["sm_res"]):
            with open(self.output_info["sm_res"], "w") as f:
                f.write(yaml.dump(task_output))

        return 

    def set_unfinished(self,):
        if os.path.exists(self.output_info["sm_res"]): os.remove(self.output_info["sm_res"])
        if os.path.exists(self.output_info["sm_done"]): os.remove(self.output_info["sm_done"])
        return


"""
If you want to change an input... param_input will be from ``load_input_ref`` func

Currently just ignores previous_output as a param -- later on might be useful
"""
def sm_runner(param_input, previous_output=None):
    from simplemind import sm
    if not os.path.exists(os.path.join(param_input["output_dir"], "solution_info.txt")) or param_input.get("force_overwrite", False):
        if param_input["log_file"] is not None:
            os.makedirs(os.path.dirname(param_input["log_file"]), exist_ok=True)

        sm_params = ["image_path", "sn_entry_path", "output_dir", 
            "working_directory", "user_resource_directory", 
            "force_overwrite",
            "chromosome", "watcher", 
            "skip_tensorboard", 
            "skip_png_training", "skip_png_prediction", 
            "verbose"]

        sm_input = {k:v for k, v in param_input.items() if k in sm_params}    ## removes unnecessary params from sm_input
        sm.runner(**sm_input
        )

    return dict(output_dir=param_input["output_dir"])


def sm_runner_setup(model_file, results_dir, working_dir=None, roi_dir=None, skip=None, watcher=False, resource_dir=None, cpu_preprocessing_only=False, force_overwrite=True, gen_param=gen_smrunner_param,):
    watcher_base_dir = None
    if watcher and working_dir is not None:
        watcher_base_dir = os.path.join(working_dir, "watcher")

    gen_smrunner_param_loaded = partial(gen_smrunner_param, model_file=model_file, results_dir=results_dir, resource_dir=resource_dir, watcher_base_dir=watcher_base_dir, roi_dir=roi_dir, working_dir=working_dir, cpu_preprocessing_only=cpu_preprocessing_only, force_overwrite=force_overwrite )
    return gen_smrunner_param_loaded




"""

tasks:
    - SMRunnerTask:
        casewise: True
        working_dir: None
        # skip: 
        #     train_screenshots: False
        #     pred_screenshots: False
        #     tensorboard: False
        # cpu_preprocessing_only: False
        # roi_dir: None
        # force_over÷write: True
    - EvalTask:
        casewise: True
        skip:
            eval_screenshots: False

    - CompileTask:
        casewise: False

import:
    simplemind.apt_agents.task.evaluate.cxr_trachea: 
        EvalTask: EvalTask
        evaluate_task: evaluate_task
        EvalChecker: EvalChecker
        gen_eval_param: gen_eval_param

    simplemind.apt_agents.task.sm_runner:
        SMRunnerTask: SMRunnerTask
        ### do i need the below?
        sm_runner: sm_runner
        SM_Checker: SM_Checker
        gen_smrunner_param: gen_smrunner_param

    simplemind.apt_agents.task.evaluate.cxr_trachea: 
        CompileTask: CompileTask
        compile_function: compile_function
        CompileChecker: CompileChecker
        gen_compile_param: gen_compile_param


"""


################# TASKS #############################
class BaseTask():
    __name__ = "BaseTask"
    def __init__(self, task, checker, param_gen_func, config=None):
        self.task = task
        self.checker = checker
        self.initialized_param_gen_func = self.param_gen_func = param_gen_func
        self.param_gen = []
        self.config = config
        self.cases = None
        self.casewise = True

    def set_case_list(self, cases):
        self.cases = cases

    def param_output_gen(self, results_dir, parameter_set, case):
        return os.path.join(results_dir, parameter_set, case["id"])

    ### input params
    def gen_params(self, parameter_set):
        params = []
        info = []
        if self.casewise:
            for case in self.cases:
                params.append(self.initialized_param_gen_func(parameter_set, case=case))
                info_dict = dict(case)
                info_dict["parameter_set"] = parameter_set
                info.append(info_dict)
        else:
            params.append(self.initialized_param_gen_func(parameter_set, cases=self.cases))
            info.append(dict(parameter_set=parameter_set))
        return params, info

    ### output params
    def gen_output_params(self, parameter_set):
        params = []
        info = []
        if self.casewise:
            for case in self.cases:
                params.append(self.initialized_param_gen_func(parameter_set, case=case))
                info_dict = dict(case)
                info_dict["parameter_set"] = parameter_set
                info.append(info_dict)
        else:
            params.append(self.param_gen_func(parameter_set))
            info.append(dict(parameter_set=parameter_set))
        return params, info

    def setup(self,*args, **kargs):
        return

    def tailor(self, *args, **kargs):
        return

class SMRunnerTask(BaseTask):
    __name__ = "SMRunnerTask"
    def __init__(self, config, task=sm_runner, checker=SM_Checker, param_gen_func=gen_smrunner_param):
        super().__init__(task, checker, param_gen_func, config)
        self.watcher_on = False

    def set_case_list(self, cases):
        super().set_case_list(cases)
        
        new_case_list = []
        for case in self.cases:
            case = dict(case)
            case["gpu_required"] = self.config.get("gpu_required", False)
            new_case_list.append(case)
        self.cases = new_case_list
        
    def setup(self, config, sm_model, results_dir, extra=None):
        self.initialized_param_gen_func = sm_runner_setup(sm_model, 
                                                        results_dir, 
                                                        working_dir=config.get("working_dir"), 
                                                        roi_dir=config.get("roi_dir"),
                                                        resource_dir=config.get("resource_dir"),
                                                        skip=config.get("skip"), 
                                                        watcher=self.watcher_on,
                                                        cpu_preprocessing_only=config.get("cpu_preprocessing_only", True), 
                                                        force_overwrite=config.get("force_overwrite", True), 
                                                        gen_param=self.param_gen_func,)
        return

    def tailor(self, engine, optimizer, distributor):
        if distributor is not None:
            if distributor.__name__=="CondorDAGDistributor":
                self.set_watcher(True)
        return

    def set_watcher(self, on):
        self.watcher_on = on
    
class EvalTask(BaseTask):
    __name__ = "EvalTask"
    def setup(self, config, sm_model, results_dir, extra=None):
        self.initialized_param_gen_func = partial(self.param_gen_func, base_results_dir=results_dir, skip_images=config.get("skip", dict()).get("eval_screenshots", False))


class CompileTask(BaseTask):
    __name__ = "CompileTask"
    def __init__(self, task, checker, param_gen_func, config=None):
        super().__init__(task, checker, param_gen_func, config)
        self.casewise=False

    def setup(self, config, sm_model, results_dir, extra=None):
        self.initialized_param_gen_func = partial(self.param_gen_func, base_results_dir=results_dir, report_templates=config.get("report_templates"), rerun=False, canary=None)