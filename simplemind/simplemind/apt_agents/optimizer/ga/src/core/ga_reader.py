import yaml 
import numpy as np
import importlib
import logging




def get_ga_param(conf_ga=None, vec_size=None, pop_size=None, gene=None, no_gene=False):
    # :param ga_param: A dictionary with the following keys:
        # vec_size: number of bits per gene
        # pop_size: numper of individuals per population
        # t_size: tournament size
        # p_mut_per_bit: probably of mutation per bit
        # cxpb: crossover probability
        # mutpb: mutation probability
        # max_gen: maximum evolution generation
        # creator_individual: individual from deap.creator
        # seed: [OPTIONAL] random seed
        # stats: [OPTIONAL] value of type deap.tools.Statistics
        # halloffame: [OPTIONAL] value of type deap.tools.HallOfFame
    
    # 
    from deap import creator
    from deap import base
    from deap import tools
    
    
    if conf_ga is not None:
        with open(conf_ga, 'r') as f:
            ga_param = yaml.load(f, Loader=yaml.FullLoader)
        ga_param["evolve_genes"] = ga_param.get("evolve_genes", True)
        ga_param["init_genes"] = ga_param.get("init_genes", ())
        if ga_param["init_genes"] is None: ga_param["init_genes"] = ()
        ga_param["init_process_n"] = ga_param.get("init_process_n", 0)
        ga_param["vec_size"] = ga_param.get("chrom_length", ga_param.get("vec_size"))   # looks for "chrom_length" and if not then just use what's in "vec_size"
        if ga_param.get("skip") is not None:
            skip = dict()
            for k in ga_param["skip"]:
                skip[k] = True
            ga_param["skip"] = skip 
        ga_param["evolve_genes"] = ga_param.get("evolve_genes", True)
    else:
        ga_param = {}
        ga_param["seed"] = 42
        ga_param["pop_size"] = 10
        ga_param["t_size"] = 3
        ga_param["p_mut_per_bit"] = .01
        ga_param["cxpb"] = .3
        ga_param["mutpb"] = 1.0
        ga_param["max_gen"] = 10
        ga_param["init_chroms"] = []
        ga_param["chrom_hex"] = False
        ga_param["evolve_genes"] = True
    if gene is not None:
        ga_param["init_chroms"] = [gene,]
        ga_param["evolve_genes"] = False
    if no_gene:
        ga_param["init_chroms"] = [None,]
        ga_param["evolve_genes"] = False

    # creator.create("FitnessMax", base.Fitness, weights=(1.0, 1.0))
    creator.create("FitnessMax", base.Fitness, weights=(1.0,))
    creator.create("Individual", list, fitness=creator.FitnessMax)
    ga_param["creator_individual"] = creator.Individual

    if ga_param.get("include_stats", True):
        stats = tools.Statistics(key=lambda ind: ind.fitness.values)
        stats.register("avg", np.mean)
        stats.register("std", np.std)
        stats.register("min", np.min)
        stats.register("max", np.max)
        ga_param["stats"] = stats
    if vec_size is not None:
        ga_param["vec_size"] = vec_size
    if pop_size is not None:
        ga_param["pop_size"] = pop_size
        
    if ga_param.get("import") is not None:
        ga_param.update(_load_task_specific_functions(ga_param["import"]))
    ga_param.update(function_setup(ga_param))
    
    return ga_param


def function_setup(ga_param):
    # if ga_param.get("Evaluator") is not None:
    #     evaluator = ga_param["Evaluator"]()
    #     ga_param["evaluate_task"] = evaluator.evaluate_task
    if ga_param.get("Compiler") is not None:
        compiler = ga_param["Compiler"]()
        ga_param["compile_function"] = compiler.compile_function
    
    ### potentially something here about Canary dataset
     
    return ga_param

def load_task_config(task_config_path,):
    with open(task_config_path, 'r') as f:
        task_config = yaml.load(f, Loader=yaml.FullLoader)
    tasks = []
    for task_id, task_info in task_config["tasks"].items():
        if task_info.get("import") is not None:
            loaded_funcs = _load_task_specific_functions(task_info["import"])
            task_obj = loaded_funcs["task"](config=task_info, task=loaded_funcs["task_func"], checker=loaded_funcs["task_checker"](), param_gen_func=loaded_funcs["task_param_generator"])
            tasks.append(task_obj)

    return tasks, task_config


# def load_task_specific_functions(ga_param):
#     loaded_funcs = dict()
#     if ga_param.get("import") is not None:
#         for filepath, funcs in ga_param["import"].items():
#             print(filepath)
#             module = importlib.import_module(filepath)
#             for tar_func, orig_func in funcs.items():
#                 loaded_funcs[tar_func] = getattr(module,  orig_func)
#     return loaded_funcs
def _load_task_specific_functions(import_param):
    log = logging.getLogger("opt.ga")
    loaded_funcs = dict()
    for filepath, funcs in import_param.items():
        log.debug("Loaded %s as function",filepath)
        module = importlib.import_module(filepath)
        for tar_func, orig_func in funcs.items():
            loaded_funcs[tar_func] = getattr(module,  orig_func)
    return loaded_funcs

