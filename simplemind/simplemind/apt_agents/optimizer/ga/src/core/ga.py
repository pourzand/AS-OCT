import os
import random

try:
    from deap import base
    from deap import creator
    from deap import tools
    from deap import algorithms
except:
    pass


from functools import partial
from simplemind.apt_agents.optimizer.ga.src.utils import binlist_to_hexstr

import numpy as np

from shutil import rmtree
import glob

import logging
GA_LOG = logging.getLogger("opt.ga")



##################################
# Reporting stuff
# Record the diversity before pruning (after the tournament selection)
# Record diversity after adding the diverse immigrants and computing the fitness


def Hamming_dist(ind1, ind2):
    diffs = 0
    for bit1, bit2 in zip(str(ind1), str(ind2)):
        if not bit1==bit2:
            diffs+=1
    return np.sqrt(diffs)

def compute_diversity(population, alpha=1):
    diversity = []
    _diversity = []
    n = len(population)
    for i, ind1 in enumerate(population):
        try:
            if ind1.fitness.values:
                fitness = ind1.fitness.values[0]
            else:
                fitness = ind1.fitness.parent_value[0]
        except:
            fitness = 1.0
        ind_div = 0
        for k, ind2 in enumerate(population):
            if i == k:
                continue
            dist = Hamming_dist(ind1, ind2)
            ind_div += dist
        div = ind_div/(n-1)
        f_norm_div = ind_div/(n-1) + alpha * fitness  # fitness normalized diversity
        # print(i, f_norm_div, fitness, binlist_to_hexstr(ind1))
        _diversity.append([f_norm_div, div, fitness, " ", binlist_to_hexstr(ind1)])
        diversity.append(f_norm_div)
    overall_diversity = np.mean(diversity)
    sorted_diversity = sorted(zip(diversity, population), reverse=True, key=lambda t: t[0])
    _diversity = sorted(_diversity, reverse=True, key=lambda t: t[2])
    for i, v in enumerate(sorted_diversity):
        d, ind = v
        for item in _diversity:
            if binlist_to_hexstr(ind)==item[4]:
                item[3]=i+1
                break

    return overall_diversity, sorted_diversity, _diversity



def record_diversity(pop, pruned_pop=None):
    overall_diversity, _, _diversity = compute_diversity(pop)
    # new_pop = diversity_enforced_pruning(gen["population"], 8.5, min_n_ind=5)
    if pruned_pop is not None:
        _new_pop = [binlist_to_hexstr(x) for x in pruned_pop]
        for x in _diversity:
            if x[-1] in _new_pop:
                x.append("")
            else:
                x.append("x")

    return {"overall_diversity": overall_diversity, "diversity_chart": _diversity}

######## For compact mode ##########

### Not currently supported ###

def delete_seg(chr_dir):
    if isinstance(chr_dir, list):
        chr_dir = chr_dir[0]
    cleaned_up_done = os.path.join(chr_dir, "cleaned_up.txt")
    if os.path.exists(cleaned_up_done) or not os.path.exists(chr_dir):
        return
    # /scratch/wasil/data/node_cad/ga_2_experiment/island_4/3A1BA6EB5FC69295ADB84F4EB254B38A75D3187E2/LIDC-IDRI-0035/seg
    ## alternative ##
    
    # seg_dirs = [os.path.join(chr_dir, seg_dir, "seg") for seg_dir in os.listdir(chr_dir)]
    seg_dirs = [os.path.join(chr_dir, seg_dir, "seg") for seg_dir in glob.glob(os.path.join(chr_dir, "*",""))]
    print(seg_dirs[0])
    print(">",end="")
    
    for seg_dir in seg_dirs:
        if not os.path.exists(seg_dir):
            continue
        try:
            rmtree(seg_dir)
            print("*", end="")
        except:
            print("-", end="")
            pass
    with open(cleaned_up_done, 'w') as f:
        f.write("")

### Not currently functional !!! ###
def clean(hof, population_history, name_input_ref_pair_lookup, local_workers=10):
    # if we have population history,
    #   iterate through each chromosome
    #   translate into hex
    #   append to base_dir
    #   check to see if "cleaned_up.txt" exists
    #   if not, then delete the seg file (shutil)
    #   
    hof_hex = [binlist_to_hexstr(chrom) for chrom in hof]
    # print("Hall of Fame:")
    # [print(chrom) for chrom in hof_hex]
    if population_history is not None:
        delete_dirs = []
        for chrom in population_history:
            chr_hex = binlist_to_hexstr(chrom)
            # print(chr_hex)
            if not chr_hex in hof_hex:
                print(".", end="")
                # print("Cleaning directory...")
                for _, case in name_input_ref_pair_lookup.items():
                    # delete_seg(os.path.join(case[0]["results_dir"], chr_hex))
                    delete_dirs.append(os.path.join(case[0]["results_dir"], chr_hex))
        delete_dirs = list(set(delete_dirs))
        parallel_args = [ [ x, ] for x in delete_dirs ]
        print("Cleaning up in parallel:")
        print("Parallel Workers:", local_workers)
        # map_result = parallelize(delete_seg, parallel_args, parallel_scheme="threading", custom_pool_size=local_workers) 
        # [x for x in map_result]    # to execute if need be
        
        # no error catching here because it's more efficient to just clean it the next time around
    return 

###############################



def restricted_tournament(population, n_worst, hfc_tournsize, survivor_pop, tourn_func=None):
    if tourn_func is None:
        tourn_func = tools.selTournament
    sorted_inds = sorted(population, reverse=False, key=lambda t:float(t.fitness.values[0]))
    GA_LOG.debug("Worst Last")
    GA_LOG.debug("{} {}".format(sorted_inds[0].fitness.values, sorted_inds[-1].fitness.values))
    r_tn_inds = sorted_inds[:n_worst]
    return tourn_func(r_tn_inds, k=survivor_pop, tournsize=hfc_tournsize)

def tournament(population, tourn_func, hfc_tourn_func=None):
    overall_pop = tourn_func(population)
    record = {"overall_tourn":{
                                "before": population,
                                "after": list(overall_pop),
            }}
    if hfc_tourn_func is not None:
        hfc_pop = hfc_tourn_func(population)
        # [print(x.fitness.values, x.id) for x in hfc_pop]
        # print("HFC")
        overall_pop.extend(hfc_pop)
        #record
        record["hfc_tourn"] = {   "before": population,
                                    "after": hfc_pop,
                                    }

    record["tourn"] = {     "before": population,
                            "after": overall_pop,
                            "diversity": compute_diversity(population, alpha=0) if len(population) > 1 else 0
                                }
    # [print(x.fitness.values, x.id) for x in overall_pop]
    return overall_pop, record

def mutate(population, tb, mutpb):
    mutated_population = []
    mutated_indices = []
    # print("population fitness", population[0].fitness)
    for i, ind in enumerate(population):
        parent_value = ind.fitness.values
        ind.fitness.parent_value = parent_value
        id = ind.id
        if random.random() + mutpb > 1:
            mut_ind = tb.mutate(creator.Individual(ind))[0]
            mut_ind.fitness.parent_value = parent_value
            mut_ind.id = id
            if list(mut_ind)==list(ind):
                # print("the same.")
                mut_ind.fitness.values = parent_value
            else:
                mutated_indices.append(i)
            mutated_population.append(mut_ind)
            # print("".join([str(x) for x in ind]))
            # print("".join([str(x) for x in mut_ind]))
            # print("".join([str(x) for x in copy]))
            #
            # print(mut_ind.fitness.values)
            # print(mut_ind.fitness.parent_value)
            # print("After^^")
            # input()
    record = {"mutation":  {"before": population,
                           "after": mutated_population,
                           "mutated_indices": mutated_indices,
                           } }
    return mutated_population, record

# parent_value[0] will always have the initial value before going through mutate operator
# fitness.values[0] will just be the initial value for now

def crossover(population, cx_op, cxpb, cx_limit=1):
    # print(">>>>>")
    # [print(x.id) for x in population]
    #for recording purposes:
    orig_pop = population
    crossed_over_indices = []
    cx_indices = {i:0 for i in range(len(population))}

    for i in range(len(population)):
        # print(i)
        j = i+1
        if j == len(population):
            j = 0
        if random.random() + cxpb > 1:
            if cx_indices[i]+1 > cx_limit or cx_indices[j]+1 > cx_limit:
                continue
            cx_indices[i] += 1
            cx_indices[j] += 1
            GA_LOG.debug("{} {}".format(cx_indices[i], cx_indices[j]))
            ind1 = orig_pop[i]  #originally taken from "population" but want to avoid chain crossings in one generation
            ind2 = orig_pop[j]  #originally taken from "population" but want to avoid chain crossings in one generation
            # print(ind1.fitness.parent_value[0], ind2.fitness.parent_value[0])
            # print(ind1.id, ind2.id)
            combined_ids = list(ind1.id)
            combined_ids.extend(ind2.id)
            # print(combined_ids)
            combined_ids = list(set(combined_ids))
            # print(combined_ids)
            # print(ind1.id.extend(ind2.id))
            # print(set(ind1.id.extend(ind2.id)))
            # combined_ids = list(set(ind1.id.extend(ind2.id)))
            # combined_ids = list( set(ind1.id).update(ind2.id))
            averaged_fitness = (ind1.fitness.parent_value[0] + ind2.fitness.parent_value[0] )/2
            ind1, ind2 = cx_op(creator.Individual(ind1), creator.Individual(ind2))
            ind1.fitness.parent_value = ind2.fitness.parent_value = (averaged_fitness, )
            ind1.id = ind2.id = combined_ids
            population[i] = ind1
            population[j] = ind2
            crossed_over_indices.extend([i, j])
            # print(ind1.fitness.values, ind2.fitness.values)
            # print(ind1.fitness.parent_value, ind2.fitness.parent_value)
            # input()
    record = {"crossover":  {"before": orig_pop,
                           "after": population,
                           "crossed_over_indices": set(crossed_over_indices),
                           } }

    return population, record



def diversity_enforced_pruning(population, min_diversity, min_n_ind=5, alpha=1):
    overall_diversity, diversity, _ = compute_diversity(population, alpha=alpha)
    n_ind = len(diversity)
    # [print(x) for x in diversity]
    # [print(x) for x in _]
    GA_LOG.debug("Minimum diversity: %s", str(min_diversity))
    GA_LOG.debug("%s (initial)", str(overall_diversity))
    while overall_diversity <= min_diversity and n_ind > min_n_ind:
        overall_diversity, diversity, _ = compute_diversity(population[:n_ind-1], alpha=alpha)
        GA_LOG.debug("Overall diversity: %s",str(overall_diversity))
        n_ind -=1
    return [ind for _, ind in diversity]


def gen_ind(bit_dist):
    ind_shape = bit_dist.shape
    # print(bit_dist)
    # print("Seed",np.random.get_state())
    ind = np.round(np.random.rand(ind_shape[0]) + (-1 * (bit_dist.astype('float64') - .5))).astype("int8")
    return ind

def _homogenize_length(population_history):
    max_len = np.max([len(ind) for ind in population_history])
    for ind in population_history:
        while len(ind) < max_len:
            ind.append(0)
    return population_history

### just a wrapper for default random immigrants
def default_immigrants(n_immigrants, default_pop_func, id_counter):    
    immigrants = default_pop_func()[:n_immigrants]
    for immigrant in immigrants:
        id_counter+=1
        immigrant.id = [id_counter, ]
    return immigrants
    
def diversified_immigrants(n_immigrants, population_history, id_counter):
    immigrants = []
    population_history = _homogenize_length(population_history)
    GA_LOG.debug("Population history %s", str(len(population_history)))
    GA_LOG.debug("Id Counter: %s", str(id_counter))
    all_inds = np.array([np.array([int(i) for i in ind]) for ind in population_history])
    n_inds = all_inds.shape[0]
    inds_sum = np.sum(all_inds, axis=0)

    for i in range(n_immigrants):
        inds_mean = inds_sum / n_inds
        population_match = True
        while population_match:
            imm_npy = gen_ind(inds_mean)
            bin_str = [int(i) for i in imm_npy]
            diverse_immigrant = creator.Individual(bin_str)
            population_match = diverse_immigrant in population_history
        inds_sum += imm_npy
        n_inds += 1
        id_counter+=1
        diverse_immigrant.id = [id_counter, ]
        immigrants.append(diverse_immigrant)
        population_history.append(diverse_immigrant)

    return immigrants

def repop(population, n_pop, repop_func):
    orig_pop = list(population)
    n_immigrants = n_pop - len(population)
    immigrants = repop_func(n_immigrants)


    # print("n_immigrants:", n_immigrants)
    # print("before:", population)
    population.extend(immigrants)
    # print("after:", population)
    record = {"immigrants":{
                                "before": orig_pop,
                                "after": population,
                                "diversity": compute_diversity(population),
                                "n_immigrants":n_immigrants,
            }}

    # print("after immigrants")
    # input(record)
    return population, record

# (A)
def dep(population, dep_func):
    reduced_population = dep_func(population)

    record = {"dep":{
                                "before": population,
                                "after": list(reduced_population),
                                "chart": record_diversity(population, pruned_pop=reduced_population),
            }}

    # population, record_fill_pop  = fill_pop(reduced_population, len(population), repop_func,)
    # n_immigrants = len(population) - len(reduced_population)
    # record.update(record_fill_pop)
    return reduced_population, record

    # immigrants = repop_func(n_immigrants)[:n_immigrants]    # takes the last n_immigrants in case repop_func is the default random population() func that just generates n_population number
    # reduced_population.extend(immigrants)
    # # overall_diversity, _, _diversity = compute_diversity(reduced_population)
    # record = {"immigrants":{
    #                             "before": population,
    #                             "after": reduced_population,
    #                             "diversity": compute_diversity(population),
    #                             "n_immigrants":n_immigrants,
    #         }}

    # return reduced_population, record


## Perform GA operations ##
# 1. Overall tournament
# 2. HFC Tournament
# 3. Mutation + Crossover
# 4. Diversity-enforced pruning
# 5. Diversified Immigrants
# 6. Fitness evaluation

## Class for taking care of evolving generation by generation 
# TODO: Document how to implement new evolutionary operator
# (A) Make the function
#       - return population, record
# (B) Add to self.log
class Evolver():
    #update ga_param to have population_history
    def __init__(self, toolbox, ga_param=None, record=False, log=None):
        self.tb = toolbox
        self.di = False
        self.record = record
        self.op_names = []
        #default, if nothing is activated:
        if ga_param is None:
            n_worst = 0
            ga_param = { "hfc": False,
                        "dep": False,
                        "di": False,
                            }

            # default tournament
            p_mut_per_bit = .01
            overall_t_size = 10
            tournsize = 3
            self.tb.register("mate", tools.cxTwoPoint)
            self.tb.register("mutate", tools.mutFlipBit, indpb=p_mut_per_bit)
            self.tb.register("overall_tourn", tools.selTournament, tournsize=tournsize, k=overall_t_size)
            self.ga_param = {}
        else:
            self.ga_param = ga_param
            self.tb_ops = []
            self.register_operations(ga_param["operations"])

        if self.record:
            # self.overall_tourn_record = {"before":[], "after":[]}
            # self.hfc_tourn_record = {"before":[], "after":[]}
            # self.mutation_record = {"before":[], "after":[]}
            # self.crossover_record = {"before":[], "after":[]}
            if log is not None:
                self.log = log
            else:
                ### (B)
                self.log = {
                    "tourn": [],
                    "overall_tourn": [],
                    "hfc_tourn": [],
                    "mutation": [],
                    "crossover": [],
                    "dep": [],
                    "immigrants": [],
                }

    def evolve(self, population):
        # print("repop",self.tb.repop)
        if len(population) > 1:
            overall_diversity, _, _ = compute_diversity(population, alpha=1)
            GA_LOG.info("Overall diversity before evolution: %s",str(overall_diversity))
        [GA_LOG.debug("{} {}".format(x.fitness.values, x.id)) for x in population]
        for i, op in enumerate(self.tb_ops):
            GA_LOG.info(self.op_names[i])
            output = op(population)
            # print(type(output[0]), type(output[1]))
            if len(output) == 2 and isinstance(output[1], dict):
                population, record = output
                self.log_activity(record)
            else:
                population = output
                self.log_activity({self.op_names[i]: population})
            if len(population) > 1:
                overall_diversity, _, _ = compute_diversity(population, alpha=1)
                GA_LOG.debug("After op: %s",str(overall_diversity))
            [GA_LOG.debug("{} {}".format(x.fitness.values, x.id)) for x in population]
            # print("After op:",population[0].fitness.values)
            # [print("".join([str(i) for i in x])) for x in population]
        self.tb.population_history.extend(population)
        return population

    #record comes in format of {"key": info, "key2":info2}
    def log_activity(self, record):
        if record.get("immigrants") is not None:
            # print("Updating counter from immigrants")
            self.tb.id_counter+=record["immigrants"]["n_immigrants"]
            # input(self.tb.id_counter)  #REACTIVATE
        if self.record:
            for key, info in record.items():
                if self.log.get(key) is not None:
                    self.log[key].append(info)

    def register_operations(self, operations):
        # - "overall_tourn"
        # - "hfc_tourn"
        # - "mutation"
        # - "crossover"
        # - "dep"
        # - "repop"
        
        ### (C)
        for op in operations:
            if op == "tourn":
                hfc_params = self.ga_param.get("hfc")
                self.add_tourn(self.ga_param["overall_t_size"], self.ga_param["overall_survivor_pop"], hfc_params=hfc_params)
                self.op_names.append(op)
            elif op == "shuffle":
                self.add_shuffle()
                self.op_names.append(op)
            elif op == "mutation": 
                self.add_mut(self.ga_param["p_mut_per_bit"], self.ga_param["mutpb"])
                self.op_names.append(op)
            elif op == "crossover":
                self.add_cx(self.ga_param["cxpb"],)
                self.op_names.append(op)
            elif op == "dep":
                self.add_dep(self.ga_param["dep"]["min_diversity"],
                            min_n_ind=self.ga_param["dep"]["min_n_ind"],
                            alpha=self.ga_param["dep"]["alpha"],
                            # di = self.ga_param["di"],
                            # population_history = self.tb.population_history,
                            # id_counter = self.tb.id_counter,

                             )
                self.op_names.append(op)
            elif op == "repop":
                self.add_repop(di=self.ga_param["di"],
                            population_history=self.tb.population_history,
                            id_counter=self.tb.id_counter,
                             )
                self.op_names.append(op)

    def add_tourn(self, overall_t_size, overall_survivor_pop, hfc_params=None):
        self.tb.register("overall_tourn", tools.selTournament, tournsize=overall_t_size, k=overall_survivor_pop)
        if hfc_params is not None:
            self.tb.register("restricted_tourn", restricted_tournament, n_worst=hfc_params["n_worst"], hfc_tournsize=hfc_params["t_size"],
                             survivor_pop=hfc_params["survivor_pop"], tourn_func=tools.selTournament)
        else:
            self.tb.restricted_tourn = None     #dirty
        self.tb.register("tournament", tournament, tourn_func=self.tb.overall_tourn, hfc_tourn_func=self.tb.restricted_tourn)
        self.tb_ops.append(partial(self.tb.tournament))

    def add_shuffle(self):
        self.tb_ops.append(partial(sorted, key=lambda k: random.random()))

    def add_mut(self, p_mut_per_bit, mutpb):
        self.tb.register("mutate", tools.mutFlipBit, indpb=p_mut_per_bit)
        mut = partial(mutate, tb=self.tb, mutpb=mutpb)
        self.tb_ops.append(mut)

    def add_cx(self, cxpb):
        # ORDER MATTERS, SO NEED TO SHUFFLE BEFORE PUTTING IN HERE
        self.tb.register("mate", tools.cxTwoPoint)
        cx = partial(crossover, cx_op=self.tb.mate, cxpb=cxpb)
        self.tb_ops.append(cx)

    def add_mut_cxr(self, p_mut_per_bit, cxpb, mutpb):
        # ORDER MATTERS, SO NEED TO SHUFFLE BEFORE PUTTING IN HERE
        self.tb.register("mate", tools.cxTwoPoint)
        self.tb.register("mutate", tools.mutFlipBit, indpb=p_mut_per_bit)
        mut_cxr = partial(algorithms.varAnd, toolbox=self.tb, cxpb=cxpb, mutpb=mutpb)
        self.tb_ops.append(mut_cxr)

    ### (D)
    def add_dep(self, min_diversity, min_n_ind=5, alpha=0):
        self.tb.register("dep", diversity_enforced_pruning, min_diversity=min_diversity, min_n_ind=min_n_ind, alpha=alpha)
        # if di:
        #     self.di = True
        #     self.tb.register("repop", diversified_immigrants, population_history=population_history, id_counter=id_counter)
        # else:
        #     self.di = False
        #     self.tb.register("repop", self.tb.population(), id_counter=id_counter)
        self.tb.register("dep", dep, dep_func=self.tb.dep)
        self.tb_ops.append(partial(self.tb.dep))

    def add_repop(self, di=False, population_history=None, id_counter=-1):
        if di:
            self.di = True
            self.tb.register("repop_method", diversified_immigrants, population_history=population_history, id_counter=id_counter)
        else:
            self.di = False
            self.tb.register("repop_method", default_immigrants, default_pop_func=self.tb.population, id_counter=id_counter)
        self.tb.register("repop", repop, n_pop=self.ga_param["overall_pop_size"], repop_func=self.tb.repop_method)
        self.tb_ops.append(partial(self.tb.repop))


    def update_tb(self, population_history=None, id_counter=None):
        if population_history is not None:
            self.tb.population_history = population_history
        if id_counter is not None:
            self.tb.id_counter = id_counter

        GA_LOG.debug("Updated TB id counter %s", str(id_counter))
        GA_LOG.debug("Updated TB id counter %s", str(self.tb.id_counter))

        # try:
        #     dep_ind = self.op_names.index("dep")
        # except:
        #     dep_ind = None
        # if dep_ind is not None:
        #     self.tb.unregister("dep_repop")
        #     self.tb.unregister("repop")
        #     if self.di:
        #         self.tb.register("repop", diversified_immigrants, population_history=self.tb.population_history, id_counter=int(self.tb.id_counter))
        #     else:
        #         #update also for default generation
        #         self.tb.register("repop", population_history=self.tb.population_history, id_counter=int(self.tb.id_counter))
        #     self.tb.register("dep_repop", dep_repop, dep_func=self.tb.dep, repop_func=self.tb.repop)
        #     self.tb_ops[dep_ind] = self.tb.dep_repop

    def clean(self, halloffame):
        self.tb.clean(halloffame, self.tb.population_history)

try:    #formerly to make it condor compatible
    class ImprovedToolbox(base.Toolbox):
        def __init__(self):
            base.Toolbox.__init__(self)
            self.id_counter = -1
            self.population_history = []
        def get_id(self):
            self.id_counter+=1
            return [self.id_counter, ]
        def add_ind(self, ind):
            self.population_history.append(ind)
            return
except:
    pass
def new_toolbox(creator_individual, eval_pop_func, ga_param, cleaner):
    """Helper function that returns a deap.base.Toolbox object.
    :param creator_individual: The type of the individual created via deap.creator.create function. Below is an example:
        from deap import creator
        creator.create("FitnessMax", base.Fitness, weights=(1.0,))
        creator.create("Individual", list, fitness=creator.FitnessMax)
    :param eval_pop_func: A function to evaluate a given population.
    :param vec_size: Number of bits in a single individual.
    :param pop_size: Number of individual in a population.
    :param t_size: Tournament size (number of selected individuals per selection).
    :param p_mut_per_bit: Probability of mutation at the bit level.
    """
    toolbox = ImprovedToolbox()
    toolbox.register("attr_bit", random.randint, 0, 1)
    toolbox.register("individual", tools.initRepeat, creator_individual, toolbox.attr_bit, ga_param["vec_size"])
    toolbox.register("population", tools.initRepeat, list, toolbox.individual, ga_param["overall_pop_size"])
    toolbox.register("evaluate", eval_pop_func)
    toolbox.register("clean", cleaner) 
        
    return toolbox