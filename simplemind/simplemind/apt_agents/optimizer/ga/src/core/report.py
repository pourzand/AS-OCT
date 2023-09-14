import logging

GA_REPORT_LOG = logging.getLogger("opt.ga.report")
try:
    import jinja2
except:
    GA_REPORT_LOG.warning("jinja requirement missing")
import os    


def _write_jinja(data, outfile, template_file):
    # print(template_file)
    # print(os.path.dirname(template_file))
    env = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=os.path.dirname(template_file)))
    basepath = os.path.dirname(os.path.abspath(outfile))
    env.globals["to_html"] = lambda x: os.path.relpath(x, basepath).replace("\\", "/")
    template = env.get_template(os.path.basename(template_file))
    with open(outfile, "w") as f:
        f.write(template.render(data))
    
def generate_report(result_dictionary, final_result, outpath, template, case_wise_contents=None, sub_template=None, canary=False):
    """Generates report for a given gene.
    :param result_dictionary: A dictionary containing the case-wise evaluation results. >>> k,res_dict in result_dictionary.items()
    :param final_result: A dictionary with overall results. 
    :param outpath: String containing the path to the folder where generated reports will be stored.
    :param template: Template file for the report at the gene level.
    :param case_wise_contents: List holding case-wise dictionaries to make sub_report. 
    :param sub_template: Template file for the detailed report at the case level. If none is provided then sub reports will be skipped.
    :param canary: If this is a canary set.
    """
    gene = os.path.basename(outpath)
    try:
        gene_binary = str(bin(eval("0xF" + gene[::-1]))).replace("0b1111", "")
    except:
        gene_binary = "N/A"

    keys = sorted(result_dictionary.keys())
    report_html = os.path.join(outpath, "report.html") if not canary else os.path.join(outpath, "canary_report.html")
    _write_jinja({
        "gene": gene,
        "gene_binary":  gene_binary,   #hex strings were saved backwards for readability
        "keys": keys,
        "final_result": final_result,
        "result_dictionary": result_dictionary,
    }, report_html, template)

        
    if sub_template is not None and case_wise_contents is not None:
        sub_report_path = os.path.join(outpath, "sub_report")
        if not os.path.exists(sub_report_path):
            os.makedirs(sub_report_path)
        for k in keys:
            GA_REPORT_LOG.debug(k)
            _write_jinja(case_wise_contents[k], os.path.join(sub_report_path, "%s.html" % k), sub_template)



#### Depracated ??? Previously only used for CXR ###
def generate_comprehensive_report(result_dictionary, final_result, case_wise_contents, outpath, template=None):
    """Generates report for a given gene.
    :param result_dictionary: A dictionary containing the evaluation results.
    :param outpath: String containing the path to the folder where generated reports will be stored.
    :param template: Template file for the report at the gene level. Will use default is none is provided.
    :param sub_template: Template file for the detailed report at the case level. Will use default is none is provided.
    """
    keys = sorted(result_dictionary.keys())
    _write_jinja({
        "gene": os.path.basename(outpath),
        "keys": keys,
        "final_result": final_result,
        "result_dictionary": result_dictionary,
    }, os.path.join(outpath, "comprehensive_report.html"), template)