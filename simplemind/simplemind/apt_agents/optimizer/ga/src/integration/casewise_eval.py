

import __qia__
import qia.common.miu as miu

from qia.ga.ga_reader import load_input_ref, _gen_input, _gen_output, _gen_ref, get_ga_param, BaseResultsLoader as results_loader

from qia.ga.evaluate_cxr_trachea import evaluate_task
import os, yaml


## Runs MIU 
def ga_miu(chrom_str, param_input, outpath, log_file=None,):
    if chrom_str is not None and not chrom_str.strip('0'):
        chrom_str = None

    if not os.path.exists(os.path.join(outpath, "solution_info.txt")) or param_input.get("rerun", False):
        extra = []
        if chrom_str is not None:
            extra.append("-c")
            extra.append(chrom_str)
        if param_input.get("roi_dir") is not None:    #MWW 09162020
            extra.append("-r")
            extra.append(param_input["roi_dir"])
        if param_input.get("resource_dir") is not None:    #MWW 09102021
            extra.append("-u")
            extra.append(param_input["resource_dir"])
        if param_input.get("working_dir") is not None:    #MWW 09162020
            os.makedirs(param_input["working_dir"], exist_ok=True)
            extra.append("-d")
            extra.append(param_input["working_dir"])
        if param_input.get("skip") is not None:
            if param_input["skip"].get("screenshots", False):
                extra.append("-i")
            if param_input["skip"].get("tensorboard", False):
                extra.append("-t")
        if param_input.get("rerun", False):
            extra.append("-f")
        if log_file is not None:
            os.makedirs(os.path.dirname(log_file), exist_ok=True)
        miu.segment(
            param_input["image_file"], 
            outpath=outpath, 
            extra=extra if extra else None, 
            model=param_input["model"], 
            log_file=log_file,
            compact=True,
        )

    return outpath


output_base_dir = ""
chr_str = None
case_csv = ""
model = ""
skip = None
one_case_only = True

# ga_param = get_ga_param(conf_ga=args.ga_conf, gene=args.chrom, no_gene=args.no_chrom)
input_func = _gen_input(model_file=model, skip=skip)

### load your own _gen_ref if you have
case_inputs = load_input_ref(case_csv, input_func, _gen_ref)
for case in case_inputs:

    param_input, ref_input = case
    dir_str = chr_str if chr_str is not None else "DEFAULT"
    seg_dir = os.path.join(output_base_dir, dir_str, param_input["id"], "seg")
    log_file = os.path.join(output_base_dir, dir_str, param_input["id"], "log", "seg_log.txt")
    ga_miu(chr_str, param_input, seg_dir, log_file=log_file)
    eval_dir = os.path.join(output_base_dir, dir_str, param_input["id"], "eval")
    eval_output = evaluate_task(seg_dir, ref_input, eval_dir, dataset=param_input.get("dataset"), id=param_input.get("id"), skip_images=skip.get("eval_screenshots"))
    eval_file = os.path.join(output_base_dir, dir_str, param_input["id"], "eval_res.yml")
    with open(eval_file, 'w') as f:
        f.write(yaml.dump(eval_output))

    if one_case_only:
        break