import os
import math
import csv, yaml



def _load_default_metrics():
    default_metric_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'default_metrics.yml')
    print(default_metric_path)
    with open(default_metric_path, 'r') as f:
        default_metrics = yaml.load(f, Loader=yaml.FullLoader)
    return default_metrics


def binlist_to_hexstr(input):
    """Returns a hexdecimal string from a given list of 0s and 1s.
    Below is an example:
        Command: binlist_to_hexstr([1,1,0,0,1,0,1,1,0])
        Expected ouput: 3D0
    """
    return (('%%0%sX' % math.ceil(len(input) / 4)) % int("".join((str(i) for i in reversed(input))), 2))[::-1]




def graph(pts, save_file="", color_top10=True):
    import matplotlib.pyplot as plt
    sorted_pts = sorted(pts, key=lambda t: t[3])
    x = [int(i[2]) for i in sorted_pts]
    y = [float(i[3]) for i in sorted_pts]
    bottom10_x = x[:10]
    bottom10_y = y[:10]
    top10_x = x[-10:]
    top10_y = y[-10:]

    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_title('Fitness Over Generations')
    ax.set_xlabel('Generation')
    ax.set_ylabel('Fitness')

    # ax = fig.add_axes([0,0,1,1])

    ax.scatter(x, y, c="black")
    if color_top10:
        ax.scatter(top10_x, top10_y, c="green")
        ax.scatter(bottom10_x, bottom10_y, c="red")

    fig.show()
    if save_file:
        fig.savefig(save_file)





def load_config(config_path):
    with open(config_path, 'r') as f:
        config = yaml.load(f, Loader=yaml.FullLoader)

    metrics = {}

    if config.get("default_metrics") is not None:
        default_metrics = _load_default_metrics()
        for sol_elem, task in config["default_metrics"].items():
            if default_metrics.get(task) is not None:
                metrics[sol_elem] = default_metrics[task]
            else:
                print(task, "is not supported as a default metric.")
                # metrics[sol_elem] = []
        
    
    if config.get("metrics") is not None:
        for sol_elem, task_metrics in config["metrics"].items():
            if metrics.get(sol_elem) is not None:
                metrics[sol_elem].extend(task_metrics)
                metrics[sol_elem] = list(set(metrics[sol_elem]))
            else:
                metrics[sol_elem] = task_metrics
    return metrics


def ga_summary(config_path, sn_path, checkpoint, results_dir, summary_dir) :

    results_file = os.path.join(results_dir, "%s", "final.yml")
    optimization_png = os.path.join(summary_dir, "optimization_visualization.png")
    optimization_summary = os.path.join(summary_dir, "optimization.csv")
       
    ### needed for unpickling GA evolution ###
    import pickle
    from deap import base
    from deap import creator
    from deap import tools
    from deap import algorithms

    creator.create("FitnessMax", base.Fitness, weights=(1.0, 1.0))
    creator.create("Individual", list, fitness=creator.FitnessMax)
    creator_individual = creator.Individual

    with open(checkpoint, "rb") as f:
        cp = pickle.load(f)
    gens = cp["logbook"][:]
    points = []

    history_csv_contents = []

    binary_chr_list = []
    gen_n = 0
    header = ["fitness",]

    metrics = load_config(config_path)

    import logging 
    log = logging.getLogger()
    # formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    formatter = logging.Formatter('[%(asctime)s|%(levelname)-5s|%(filename)-19s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    log.setLevel(logging.DEBUG)

    from simplemind.goal import semantic_network
    sn = semantic_network.BaseSemanticNetwork(sn_path, log)


    header.extend(["hex_chr","binary_chr", "gen_n"])
    header_metrics = []
    for gen in gens:
        gen_n = gen["gen"]
        pop_n = gen["evals"]

        for ind in gen["population"]:
            fitness = ind.fitness.values[0]
            chromosome = ind
            hex_id = binlist_to_hexstr(chromosome)
            points.append([hex_id, "".join([str(x) for x in chromosome]), gen_n, fitness])
            binary_chr="".join([str(x) for x in chromosome])
            binary_chr_list.append((binary_chr, fitness))
            print(binary_chr)
            
            csv_contents = dict(hex_chr=hex_id, binary_chr=binary_chr,fitness=fitness, gen_n=gen_n,)
            print(results_file%hex_id)
            if os.path.exists(results_file%hex_id):
                with open(results_file%hex_id, 'r') as f:
                    contents = yaml.load(f, Loader=yaml.Loader)
                for sol_elem, task_metrics in metrics.items():
                    for metric in task_metrics:
                        metric_id = "_".join((metric, sol_elem))
                        try:
                            csv_contents[metric_id]  = contents[sol_elem][metric]
                            header_metrics.append(metric_id)
                        except:
                            csv_contents[metric_id]  = None
            sn_dict = sn.get_parameters_from_chromosome(binary_chr = binary_chr,)
            for param_id, param in sn_dict.items():
                csv_contents[param_id] = param["value"]
                
                
            history_csv_contents.append(csv_contents)
    header.extend(list(set(header_metrics)))
    print(len(history_csv_contents))
    #### Adding chromosomes from latest unfinished generation
    print(">>> UNFINISHED CHROMOSOMES <<< ")
    gen_n += 1
    fitness = None
    for i, ind in enumerate(cp["population"]):
        chromosome = ind
        hex_id = binlist_to_hexstr(chromosome)
        print(i, hex_id)
    print("")
    """
    if False:   # disabling this
    # for ind in cp["population"]:
        chromosome = ind
        hex_id = binlist_to_hexstr(chromosome)
        # n_id = ind.id
        binary_chr="".join([str(x) for x in chromosome])
        print(results_file%hex_id)
        if os.path.exists(results_file%hex_id):
            with open(results_file%hex_id, 'r') as f:
                contents = yaml.load(f, Loader=yaml.Loader)
            fitness = contents["fitness"]
            if task=="nodule":
                fpr = contents["fp_mean"]
                csv_contents = dict(hex_chr=hex_id, binary_chr=binary_chr,fitness=fitness, gen_n=gen_n, sensitivity=sensitivity, fpr=fpr)
            elif task=="cxr":
                sensitivity = contents["TraCh"]["sensitivity"]
                dce_score_mean = contents["TraCh"]["dce_score_mean"]
                dice_coefficient_mean = contents["TraCh"]["dice_coefficient_mean"]
                csv_contents = dict(hex_chr=hex_id, binary_chr=binary_chr,fitness=fitness, gen_n=gen_n, sensitivity=sensitivity, dce_score_mean=dce_score_mean, dice_coefficient_mean=dice_coefficient_mean)
            for params in model_reader.output_parameters(binary_chr):
                k = "_".join((params["node_id"], params["bit_id"]))
                csv_contents[k] = params["value"]
                # print(params)
            history_csv_contents.append(csv_contents)
    print(len(history_csv_contents))
    """

    graph(points, optimization_png)

    supplementary_header = list(sn_dict.keys())
    header.extend(supplementary_header)

    ### Sorting history by fitness by default ###
    history_csv_contents = sorted(history_csv_contents, reverse=True, key=lambda t:float(t["fitness"]))

    with open(optimization_summary, 'w', newline='') as csvfile:
        fieldnames = header
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames, extrasaction='ignore',)

        writer.writeheader()
        for row in history_csv_contents:
            # print(row)
            writer.writerow(row)

    ### diversity can be done in a separate file? ###
    # from simplemind.apt_agents.tools.ga.diversity import compute_diversity
    # overall_diversity, sorted_diversity, _diversity = compute_diversity(binary_chr_list)
    # [print(v) for v in _diversity]
    # [print(v) for v in sorted_diversity]

    print(optimization_summary)
    print(optimization_png)




if __name__=="__main__":
    from argparse import ArgumentParser

    parser = ArgumentParser(description="Evaluate a single gene")
    parser.add_argument("config_path", help="path to status config")
    parser.add_argument("sn_path", help="custom model file")
    parser.add_argument("checkpoint",  help="custom model file")
    parser.add_argument("results_dir",  help="general results directory")
    parser.add_argument("summary_dir",  help="where summary files will be saved")
    args = parser.parse_args()

    ga_summary(args.config_path, args.sn_path, args.checkpoint, args.results_dir, args.summary_dir)