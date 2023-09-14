import yaml, logging

class Optimizer():
    __name__ = "Base_Optimizer"
    def __init__(self, optimizer_config, dataset=None, model=None, checkpoint=None, canary_subset=None):
        self.log = logging.getLogger("opt")
        self.checkpoint = checkpoint
        self.history = []
        self.iteration = 0
        self.max_iterations = 10
        self.pop_size = 10          ### Rename
        self.bit_length = 10        ### Rename
        self.evolved_gen=False      ### Rename
        self.load_config(optimizer_config)
        return

    def load_config(self, optimizer_config):
        with open(optimizer_config, 'r') as f:
            config = yaml.load(f)

    def load_checkpoint(self,):
        return

    def set_engine(self, engine):
        self.engine = engine

    def optimize(self,):
        while self.continue_optimization():
            parameter_sets = self.get_current_population()

            self.engine.prepare_jobs(parameter_sets)
            self.engine.execute_jobs()
            self.evolved_gen=False

            parameter_sets_performances = self.engine.get_performances()   # dictionary: {[encoded_parameter_set]: [performance_metric],}

            self.step_forward(parameter_sets_performances)
        self.log.info("Finished evolution!")
    ### ***Overload this***
    def continue_optimization(self,):
        return self.iteration < self.max_iterations

    def get_current_population(self, ):
        return self.current_population


    ### ***Overload this***
    ### Default implementation is random selection of binary encoding of the parameter sets
    def get_next_population(self, parameter_sets_performances):
        history_dict = self.get_collapsed_history()
        previous_parameter_sets = list(history_dict.keys())

        max_combinations = 2**self.bit_length

        i = 0 
        attempted_combinations = set()
        next_population = []
        while i < self.pop_size:
            matched_previous = True
            while matched_previous:
                bin_str = None
                if len(attempted_combinations) == max_combinations:
                    self.log.debug("No more combinations to try...")
                    break
                bin_str = "".join(list(np.round(np.random.rand(10)).astype("int8").astype("str")))
                attempted_combinations.add(bin_str)
                matched_previous = bin_str in previous_parameter_sets
            if bin_str is not None:
                next_population.append(bin_str)
            i+=1


        return next_population 

    def get_collapsed_history(self):
        ### flattens history to one dictionary
        history_dict = {}
        [history_dict.update(parameter_sets_performances) for parameter_sets_performances in self.history]
        return history_dict

    def step_forward(self, parameter_sets_performances):
        self.history.append(parameter_sets_performances)
        # evolved_gen = False ## include in GA
        self.save_progress()
        self.iteration+=1
        self.current_population = self.get_next_population(parameter_sets_performances)
        self.save_progress()
        

    def save_progress(self,):
        return 

############################# GENETIC ALGORITHM OPTIMIZER ##########################

"""
offspring_population = {}
"""
"""
    Missing params:
        toolbox
        evolver

    From ga_param to be declared under self:
        seed
        chrom_hex
        chrom_init
        halloffame
        logbook
        evolved_gen
        offspring_population
        previous_log
        stats
        

"""



from simplemind.apt_agents.optimizer.ga.src.core.ga import Evolver, clean, new_toolbox
from simplemind.apt_agents.optimizer.ga.src.utils import binlist_to_hexstr
from simplemind.apt_agents.optimizer.ga.src.core.ga_reader import get_ga_param 
from simplemind.apt_agents.distributor.utils import filler_func
import random, numpy as np, pickle, os

try:
    from deap import tools
    from deap import creator
except: 
    pass


## TODO: doesn't handle default chromosome well.
def update_chrom_fitness(parameter_set_performances, population):
    log = logging.getLogger("opt")
    log.debug("***** chrom fitness values and weights *****")
    [log.debug("Fitness: {}; Fitness Weights: {}".format(chrom.fitness.values, chrom.fitness.weights)) for chrom in population]
    for chrom in population:
        # if chrom is None:
        #     print("No-gene evaluation finished.")
        #     continue
        try:
            parameter_id = binlist_to_hexstr(chrom)
        except:
            parameter_id = chrom

        fitness = parameter_set_performances.get(parameter_id)
        # print("".join([str(x) for x in chrom]))
        # print(chrom.fitness.values)
        # print(chrom.fitness.weights)
        # input()
        if fitness is not None:
            chrom.fitness.values = (fitness,)


class GA_Optimizer(Optimizer):
    __name__ = "GA_Optimizer"
    def __init__(self, optimizer_config, dataset=None, model=None, checkpoint=None, canary_subset=None):
        super().__init__(optimizer_config, dataset=dataset, model=model, checkpoint=checkpoint, canary_subset=canary_subset)

        self.offspring_population = {}
        self.init_toolbox()
        self.halloffame=None
        self.evolved_gen=True
        ### load existing checkpoint / start new evolution ###
        if not self.load_checkpoint(): self._initialize_new_evolution()
        self.evolver = Evolver(self.toolbox, ga_param=self.ga_param, record=True, log=self.previous_log)
        self.log.info("Starting GA APT... {} max generations".format(self.max_iterations))
    def load_checkpoint(self):
        if self.checkpoint is not None and os.path.exists(self.checkpoint):
            self.log.info("Loading checkpoint")
            with open(self.checkpoint, "rb") as f:
                cp = pickle.load(f)
            self.current_population = cp["population"]
            self.iteration = cp["generation"]
            self.halloffame = cp["halloffame"]
            self.logbook = cp["logbook"]
            self.evolved_gen = cp.get("evolved_gen", False)
            self.offspring_population = cp.get("offspring_population", {}) # start of a new generation
            self.toolbox.population_history = cp.get("population_history", self.current_population)
            self.previous_log = cp.get("log", None)
            self.toolbox.id_counter = cp.get("id_counter", -1)
            
            self.log.info("Generation {} loaded.".format(self.iteration))
            ### TODO: implement later
            # if toolbox.canary is not None:
            #     print("Canary params :::: ",cp.get("canary_params"))
            #     toolbox.canary.update_params(cp.get("canary_params", {}))

            random.setstate(cp["rndstate"])
            return True
        else:
            return False

    def _initialize_new_evolution(self):
        self.log.info("Initializing new GA run")
        self.log.debug("seed: {}".format(self.seed))
        if self.seed is not None:
            random.seed(self.seed)
            np.random.seed(self.seed)
        self.current_population = self.toolbox.population()
        self.log.info("Initialized population size: {}".format( len(self.current_population)))
        if self.init_chroms is not None:
            for i, chrom in enumerate(self.init_chroms):
                if self.chrom_hex:
                    #hex strings are saved backwards for readability
                    bin_str = str(bin(eval("0xF"+chrom[::-1]))).replace("0b1111","")[::-1]
                else:
                    bin_str = str(chrom)
                bin_str = [int(i) for i in bin_str]
                self.current_population[i] = creator.Individual(bin_str)  #creator in this case is from `deap` package
                self.log.debug("".join([str(x) for x in self.current_population[i]]))
                self.log.debug(self.current_population[i].fitness.values)
                self.log.debug(self.current_population[i].fitness.weights)
            self.log.debug("Added chromosomes: {}".format(len(self.init_chroms)))
        self.log.info("After filling population with chrs: {} individuals".format(len(self.current_population)))
        for ind in self.current_population:  #attach an id to each individual
            ind.id = self.toolbox.get_id()
        self.iteration = 0
        if self.halloffame is None:
            self.halloffame = tools.HallOfFame(10)
        self.logbook = tools.Logbook()
        self.offspring_population = {0: self.current_population}
        self.toolbox.population_history = list(self.current_population)    #fixes onee problem but introduces another?
        self.previous_log = None

    def load_config(self, optimizer_config): 
        self.ga_param = get_ga_param(optimizer_config)
        self.seed = self.ga_param["seed"]
        self.chrom_hex = self.ga_param["chrom_hex"]
        self.init_chroms = self.ga_param["init_chroms"]
        self.stats = self.ga_param.get("stats")
    
        self.max_iterations = self.ga_param["max_gen"]
        self.pop_size = self.ga_param["overall_pop_size"]          ### Rename
        self.bit_length = self.ga_param["chrom_length"]            ### Rename


    def init_toolbox(self,):
        # if self.ga_param.get("compact", False):
        #     print("Compact mode ON.")
        #     cleaner = partial(clean, name_input_ref_pair_lookup=name_input_ref_pair_lookup, local_workers=computing_management["parallel"]["parallel_misc_n"])
        # else:
        #     cleaner = partial(filler_func)
        
        cleaner = filler_func ## placeholder
        canary_obj = None

        self.toolbox = new_toolbox(self.ga_param["creator_individual"], update_chrom_fitness, self.ga_param, cleaner)
        ### maybe replace eval_pop with something that sudo-runs ---> just uses the output from the distributor\
        #               -->> just mirror the same output 
        
        self.toolbox.canary = canary_obj

    def get_current_population(self):
        self.invalid_ind = [ind for ind in self.current_population if not ind.fitness.valid]
        return self.invalid_ind
        # return [binlist_to_hexstr(chrom) for chrom in self.invalid_ind]

    def step_forward(self, parameter_sets_performances):
        self.toolbox.evaluate(parameter_sets_performances, self.get_current_population())    # hack to just update fitness values instead of computing them

        ### only do once per population
        
        ### temporarily disabled
        # if self.stats is not None:
        #     record = self.stats.compile(self.current_population)
        #     self.logbook.record(gen=self.iteration, evals=len(self.invalid_ind), population=self.current_population, **record)
        # else:
        self.logbook.record(gen=self.iteration, evals=len(self.invalid_ind), population=self.current_population)

        self.halloffame.update(self.current_population)
        self.offspring_population[self.iteration] = self.current_population  #unsure if I need this

        self.log.debug("Begin cleaning..")
        self.evolver.clean(self.halloffame)   # if compact was not specified then cleaning is just a dummy function
        
        super().step_forward(parameter_sets_performances)

        return 

    def get_next_population(self, parameter_sets_performances):
        ### ignore parameter_sets_performances because it was already incorporated
        self.log.info("================= Starting new generation ================")
        self.log.info("Generation: {}".format( self.iteration))
        self.log.debug("{} {}".format(self.iteration, self.evolved_gen))
        if self.iteration!=0 and not self.evolved_gen:
            self.current_population = self.evolver.evolve(self.current_population)
            if self.toolbox.canary is not None: self.toolbox.canary.update_best_fitness(self.halloffame)   # what happens if initiating
            self.log.debug("after evolution id counter: {}", self.evolver.tb.id_counter)
            self.evolver.update_tb()
            self.evolved_gen = True
        return self.current_population

    def save_progress(self, ):

        #################

        if self.checkpoint is not None:
            if not os.path.exists(os.path.dirname(self.checkpoint)):
                os.makedirs(os.path.dirname(self.checkpoint), exist_ok=True)
            if self.evolver.tb.canary is not None: canary_params=self.evolver.tb.canary.export_params()
            else: canary_params=None
            cp = dict(
                population=self.current_population, generation=self.iteration, halloffame=self.halloffame,
                offspring_population=self.offspring_population,
                logbook=self.logbook, rndstate=random.getstate(), evolved_gen=self.evolved_gen,
                population_history=self.evolver.tb.population_history, log=self.evolver.log, id_counter=self.evolver.tb.id_counter,
                canary_params=canary_params,
            )
            with open(self.checkpoint, "wb") as cp_file:
                pickle.dump(cp, cp_file)

