### Must run this in conda env ### 
### conda activate deep_med ###
from qia.ga.evaluate_cxr_trachea import ResultsCompiler_CXR_Trachea
import os, glob , yaml

def load_result_dictionary(results_dir):
    result_dictionary = dict()
    search = os.path.join(results_dir, "*", "eval_res.yml")
    eval_res_files = glob.glob(search)
    for eval_res_file in eval_res_files:
        idx = os.path.basename(os.path.dirname(eval_res_file))
        with open(eval_res_file, 'r') as f:
            result_dictionary[idx] = yaml.load(f, Loader=yaml.FullLoader)
    return result_dictionary


results_dir = ""

result_dictionary = load_result_dictionary(results_dir)
results_compiler = ResultsCompiler_CXR_Trachea()


results_compiler.compile_function(result_dictionary, results_dir, rerun=False, canary=False)
