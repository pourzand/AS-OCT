"""ga entry point
"""


from argparse import ArgumentParser
from simplemind.apt_agents.distributor.task import execute_tasks ###







"""
Things that job distributor should take care of:

- case_list (?)
- sm model (???)
- ordered tasks (e.g. sm.runner, evaluate, compile, visualize)
- 

Variables:
    tasks
    checkers
    dataset
    tasks_param (or way to generate)
    parameter_sets
    parameter_set_ids
    results_dir # overall results dir
"""



        

### should know nothing about the jobs it's running
"""
    # pool_type
    distributor:
        path.to.distributor.module:
            Distributor: CondorDAGDistributor
    parallel_n
    parallel_sub_n
    parallel_misc_n
    gpu_requirement:
        n_gpus: 1
        gpu_memory_filter: 20000
        n_jobs: 3     # set None if you want it to be all jobs that a GPU is required
    compute_env: xxxx
    condor_workpath
    timeout
    
    
    error_path #???

    Needed variables:
        pool
        dist_args

        aggregate -- True if docker/threadpool
                     False if CondorPoolDag

    aggregate = True
    if parallel_scheme=="condor":
        # pool_func = CondorPoolDAG
        pool_func = partial(CondorPoolDAG, sub_pool_size=pooling_args["parallel_sub_dag_n"], error_path=pooling_args["error_path"])
        aggregate = False
    elif parallel_scheme=="docker":
        pool_func = DockerPool
        pooling_args["return_func"] = docker_return_func
    else:
        pool_func = ThreadPool


        if aggregate:
            pooling_args["parallel_sub_dag_n"] = parallel_job_limit
        else:
            pooling_args["parallel_dag_n"] = parallel_job_limit


"""

from simplemind.apt_agents.optimizer.ga.src.utils import binlist_to_hexstr
from simplemind.apt_agents.distributor.task import execute_tasks 

from simplemind.apt_agents import Engine, load
from simplemind.apt_agents.distributor.task import TaskManager


def optimizer(case_list, model, results_path, 
            optimizer_config=None, distributor_config=None, task_config=None, 
            checkpoint = None, log_path=None, verbose=2, canary_subset=None):
    """apt optimizer module

    Parameters
    ----------
    case_list: str
        a csv containing cases to be evaluated
    model: str
        custom model file
    results_path : str
        path where results will be held
    optimizer_config : str
        configuration for optimizer" (default=None)
    distributor_config : str
        configuration for distributor (default=None)
    task_config : str
        configuration for jobs (default=None)
    log_path : str
        optional path for error reporting (default=None)
    canary_subset : str
        series to initially test if gene has good enough performance (default=None)

    Returns
    -------
    
    """
    log_path = get_log_path(log_path)
    simplemind_logger_setup(verbose=verbose, log_path=log_path, entry_point="apt")

    ### Load up the tasks ###
    task_manager = TaskManager(task_config, model=model, results_path=results_path)

    optimizer_obj, distributor_obj = load(optimizer_config_path=optimizer_config, distributor_config_path=distributor_config)

    ### Initialize the job distributor ###
    # distributor = CondorDAGDistributor(distributor_config_path=args.distributor_config)
    distributor = distributor_obj(distributor_config_path=distributor_config, log_path=log_path)
    # distributor.load_distributor_config()
    distributor.set_dependencies(task_manager.get_dependencies())   ## technically unnecessary for regular map .. TODO: how to reconcile?

    ### Initialize the APT Optimizer ###
    apt_optimizer = optimizer_obj(optimizer_config, checkpoint=checkpoint, canary_subset=canary_subset)

    ### initialize the APT Engine, which creates and offers jobs to distributor, based on what APT Optimizer wants ###
    apt_engine = Engine(results_dir=results_path, log_path=log_path, job_distributor=distributor)

    ### Tailor some of the task settings/parameters to the engine, optimizer, and distributor ###
    task_manager.tailor_tasks(apt_engine, apt_optimizer, distributor)
    
    ### Finalize task setup, and give to APT Engine ###
    task_manager.setup_tasks()
    apt_engine.set_tasks(task_manager.get_tasks())
    apt_engine.load_dataset(case_list)

    ### Plug in engine into optimizer ### 
    apt_optimizer.set_engine(apt_engine)

    ### Run APT! ###
    apt_optimizer.optimize()

from simplemind.apt_agents.distributor.condor.src.qia.logging_utils import simplemind_logger_setup, get_log_path

if __name__=="__main__":
    parser = ArgumentParser(description="Automatic Parameter Tuning")
    parser.add_argument("case_list", help="a csv containing cases to be evaluated")
    parser.add_argument("model", action="store", help="custom model file")
    parser.add_argument("results_path", action="store", help="path where results will be held",)
    parser.add_argument("--optimizer_config", action="store", help="configuration for optimizer", default=None)
    parser.add_argument("--distributor_config", action="store", help="configuration for distributor", default=None)
    parser.add_argument("--task_config", action="store", help="configuration for jobs", default=None)
    parser.add_argument("--checkpoint", help="checkpoint", default=None)
    parser.add_argument("--log_path", action="store", help="optional path to directory for logging", default=None)
    parser.add_argument("--verbose", action="store", help="verbosity for log", default=2)   ### TODO: Change this default to 0
    parser.add_argument("--canary_subset", action="store", help="series to initially test if gene has good enough performance", default=None)
    args = parser.parse_args()

    # log_path = get_log_path(args.log_path)

    # simplemind_logger_setup(verbose=args.verbose, log_path=log_path, entry_point="apt")
    optimizer(args.case_list, args.model, args.results_path, 
            optimizer_config=args.optimizer_config, distributor_config=args.distributor_config, task_config=args.task_config, 
            checkpoint=args.checkpoint, log_path=args.log_path, verbose=args.verbose, canary_subset=args.canary_subset)
    
