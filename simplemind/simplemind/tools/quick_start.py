"""Configuration tools

This script contains tools to configure a SimpleMind application.

The generated application follows this default structure:
{app_version}
ㄴ sn_{app_version}
ㄴ configurations_{app_version}
    ㄴ resource_local
    ㄴ resource_distributor
    ㄴ distributor.yml
    ㄴ optimizer.yml
    ㄴ task.yml
ㄴ apt_distributor_{app_version}
ㄴ apt_optimizer_checkpoint_{app_version}
ㄴ think_{app_version}
ㄴ result_{app_version}
ㄴ summary_{app_version}
ㄴ run_apt_assess.sh
ㄴ run_in_docker_apt_assess.sh
ㄴ run_apt_optimize.sh
ㄴ run_in_docker_apt_optimize.sh
ㄴ run_tb.sh
ㄴ run_summary_apt.sh
"""

from argparse import ArgumentParser
import os, re
import logging
import yaml, shutil
import pandas as pd

def quick_start(conf_quick_start):
    """Quick start tool
    A tool to configure a SimpleMind application from
    the semantic network (SN)

    Parameters
    ----------
    """

    log = logging.getLogger()
    # formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    formatter = logging.Formatter('[%(asctime)s|%(levelname)-5s|%(filename)-12s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    log.setLevel(logging.DEBUG)
    log.info('---------------------------------------------------------------')
    log.info('Quick-start a SimpleMind application')
    log.info('---------------------------------------------------------------')
    log.info('Generating a SM application from the configuration: %s' % conf_quick_start)
    log.info('---------------------------------------------------------------')
    
    with open(conf_quick_start, 'r') as f:
        app_info = yaml.load(f, Loader=yaml.FullLoader)
    template_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'quick_start_template')

    app_version = app_info['generate_application']['app_version_name']
    app_base = app_info['generate_application']['base_directory']
    abs_path_on = app_info['generate_application'].get('abs_path_on', False)
    app_base_relative = "."
    if abs_path_on: app_base_relative = os.path.join(app_base, app_version)
    # app_base = os.path.realpath(app_base)

    """Generate application and sub directories"""
    base_dir = os.path.join(app_base, app_version)
    os.makedirs(base_dir, exist_ok=True)

    conf_dir = os.path.join(base_dir, f'configurations_{app_version}') # configurations
    os.makedirs(base_dir, exist_ok=True)

    sn_dir = os.path.join(base_dir, f'knowledge_{app_version}') # semantic network
    apt_dist_dir = os.path.join(base_dir, f'apt_distributor_{app_version}') # apt.distributor byproduct
    apt_opt_checkpoint_dir = os.path.join(base_dir, f'apt_optimizer_checkpoint_{app_version}') # apt.optimizer byproduct (checkpoints)
    think_dir = os.path.join(base_dir, f'think_{app_version}') # think byproduct
    results_dir = os.path.join(base_dir, f'result_{app_version}') # result
    summary_dir = os.path.join(base_dir, f'summary_{app_version}') # summary during analysis

    resource_dist_dir = os.path.join(conf_dir, "resource_distributor") # resource conf for apt.distributor
    resource_local_dir = os.path.join(conf_dir, "resource_local") # resource conf for apt.local
    error_dir = os.path.join(base_dir, f'error_{app_version}')

    dirs_to_make = [sn_dir, apt_dist_dir, apt_opt_checkpoint_dir, think_dir,
                    results_dir, summary_dir, resource_dist_dir, resource_local_dir, 
                    error_dir]
    _ = [os.makedirs(dir_, exist_ok=True) for dir_ in dirs_to_make]

    """knowledge base"""
    sn_info = app_info['knowledge_base']
    org_sn_node_list = sn_info.get('node_list')
    if org_sn_node_list:
        sn_dst = os.path.join(sn_dir, os.path.basename(os.path.dirname(org_sn_node_list)))
        if not os.path.exists(sn_dst):
            shutil.copytree(os.path.dirname(org_sn_node_list), sn_dst)
            with open(os.path.join(sn_dst, "README.md"), 'w') as f:
                f.write("Original knowledge base: %s"% org_sn_node_list)

    """for the think byproduct files"""
    if sn_info.get('cnn_node'):
        for cnn_node, cnn_node_dict in sn_info['cnn_node'].items():
            cnn_node_dir = os.path.join(think_dir, cnn_node)
            os.makedirs(cnn_node_dir, exist_ok=True)
            if cnn_node_dict.get('custom_architecture'):
                custom_arch_dst = os.path.join(cnn_node_dir, os.path.basename(cnn_node_dict['custom_architecture']))
                shutil.copyfile(cnn_node_dict['custom_architecture'], custom_arch_dst)
    
    """for the data files"""
    data_info = app_info['data']
    if data_info.get('train'):
        if data_info['train'].get('cnn_train_list'):
            for cnn_node, cnn_data_path in data_info['train']['cnn_train_list'].items():
                os.makedirs(os.path.join(think_dir, cnn_node), exist_ok=True)
                cnn_train_list = os.path.join(think_dir, cnn_node, 'train_list.csv')
                shutil.copyfile(cnn_data_path, cnn_train_list)

    data_info = app_info['data']
    if data_info.get('train'):
        if data_info['train'].get('apt_train_list'):
            apt_train_list = os.path.join(think_dir, 'apt_train_list.csv')

            shutil.copyfile(data_info['train']['apt_train_list'], apt_train_list)
            ref_apt_train_df = pd.read_csv(apt_train_list)
            apt_train_df = pd.DataFrame()
            for k in ("id","dataset","image_file","reference"):
                if k in ref_apt_train_df:
                    apt_train_df[k] = ref_apt_train_df[k]
            
            if data_info.get('data_key') is not None and not 'dataset' in apt_train_df:
                apt_train_df['dataset'] = data_info['data_key']
            apt_train_df.to_csv(apt_train_list)
        else:
            log.warning('---------------------------------------------------------------')
            log.warning('APT Train list not copied because not defined. ')
            log.warning('---------------------------------------------------------------')
    """configuration files"""
    # resource configurations
    conf_info = app_info['configurations']
    if conf_info.get('computing_resource'):
        for resource_file in conf_info['computing_resource'].get('local', []):
            shutil.copyfile(resource_file, os.path.join(resource_local_dir, os.path.basename(resource_file)))
        for resource_file in conf_info['computing_resource'].get('distributor', []):
            shutil.copyfile(resource_file, os.path.join(resource_dist_dir, os.path.basename(resource_file)))

    """configuration files"""
    ### apt task conf
    if conf_info.get('task'): task_conf_path = conf_info['task']
    else: task_conf_path = os.path.join(template_dir, 'task.yml') # from default task.yml
    if conf_info.get('apt_optimizer'):
        evaluate_src = conf_info['apt_optimizer'].get('evaluate_src')
    # if conf_info.get('apt_optimizer'):
        compile_src = conf_info['apt_optimizer'].get('compile_src')
    else:
        evaluate_src = "'[FILL_IN_EVALUATOR]'"
        compile_src = "'[FILL_IN_COMPILER]'"
    with open(task_conf_path, 'r') as f:
        contents = f.read()
    contents = contents % (evaluate_src, compile_src)
    with open(os.path.join(conf_dir, "task.yml"), 'w') as f:
        f.write(contents)
    task_conf_path = os.path.join(conf_dir, "task.yml")

    with open(task_conf_path, 'r') as f:
        task_conf = yaml.load(f, Loader=yaml.FullLoader)
    for task_info in task_conf.get("tasks", []):
        if task_info["id"]=="SM_Runner":
            task_info["working_dir"] = f'{app_base_relative}/think_{app_version}'
            task_info["resource_dir"] = f'{app_base_relative}/configurations_{app_version}/resource_local'
        # elif task_info["id"]=="Evaluator":
            # evaluate_src = experiment_info.get("evaluate_src", "simplemind.apt_agents.optimizer.ga.src.evaluate.{}".format(experiment_info.get("dataset", "")))
        elif task_info["id"]=="Compiler":
            # compiler_src = experiment_info.get("evaluate_src", "simplemind.apt_agents.optimizer.ga.src.evaluate.{}".format(experiment_info.get("dataset", "")))
            if conf_info.get('apt_optimizer'):
                if conf_info['apt_optimizer'].get('report_template'):
                    task_info["report_templates"] = {}
                    task_info["report_templates"]["report"] = conf_info['apt_optimizer'].get("report_template")
                    if conf_info['apt_optimizer'].get('subreport_template'):
                        task_info["report_templates"]["subreport"] = conf_info['apt_optimizer'].get("subreport_template")
    with open(os.path.join(conf_dir, "task.yml"), 'w') as f:
        f.write(yaml.dump(task_conf))
    
    ### apt.optimizer conf
    if conf_info.get('apt_optimizer'):
        if conf_info['apt_optimizer'].get('configuration'):
            shutil.copyfile(conf_info['apt_optimizer']['configuration'], 
                        os.path.join(conf_dir, "optimizer.yml"))
        else:
            tmp_conf_optimizer_ga = os.path.join(template_dir, 'conf_optimizer_ga.yml')
            with open(tmp_conf_optimizer_ga, 'r') as f:
                contents = f.read()
            # contents = contents % (total_chrom) #TODO: update the total chrom length from sn
            with open(os.path.join(conf_dir, "optimizer.yml"), 'w') as f:
                f.write(contents)
    
    ### apt.distributor conf
    conf_dist_local = os.path.join(template_dir, 'conf_distributor_local.yml') # from default task.yml
    conf_dist_condor = os.path.join(template_dir, 'conf_distributor_condor.yml') # from default task.yml
    if conf_info.get('apt_distributor'):
        if conf_info['apt_distributor'].get('local'):
            conf_dist_local = conf_info['apt_distributor']['local']
        if conf_info['apt_distributor'].get('condor'):
            conf_dist_condor = conf_info['apt_distributor']['local']

    with open(conf_dist_local, 'r') as f:
        distributor_conf = yaml.load(f, Loader=yaml.FullLoader)
    distributor_conf["error_path"] = os.path.join(f'{app_base_relative}/error_{app_version}', "distributor")
    with open(os.path.join(conf_dir, "distributor.yml"), 'w') as f:
        f.write(yaml.dump(distributor_conf))

    with open(conf_dist_condor, 'r') as f:
        distributor_conf = yaml.load(f, Loader=yaml.FullLoader)
    distributor_conf["error_path"] = os.path.join(f'{app_base_relative}/error_{app_version}', "distributor")
    distributor_conf["condor_workpath"] = os.path.join(f'apt_distributor_{app_version}', "pilot")
    with open(os.path.join(conf_dir, "distributor_condor.yml"), 'w') as f:
        f.write(yaml.dump(distributor_conf))

    ### apt.summary conf
    if conf_info.get('apt_summary'):
        conf_apt_summary = conf_info['apt_summary']['configuration']
        with open(conf_apt_summary, 'r') as f:
            apt_summary_conf = yaml.load(f, Loader=yaml.FullLoader)
        with open(os.path.join(conf_dir, "summary_apt.yml"), 'w') as f:
            f.write(yaml.dump(apt_summary_conf))

    """bash files"""
    ### tensorboard conf
    tb_run_template = os.path.join(template_dir, 'bash', 'run_tb.sh')
    with open(tb_run_template, 'r') as f:
        contents = f.read()
    tb_port = conf_info['analysis'].get('tensorboard_port', None)
    contents = contents % (tb_port, f'think_{app_version}')
    with open(os.path.join(base_dir, "run_tb.sh"), 'w') as f:
        f.write(contents)

    sn_dst_node_list = os.path.join(f'knowledge_{app_version}', 
                os.path.basename(os.path.dirname(org_sn_node_list)), 
                os.path.basename(org_sn_node_list))
    ### apt assess
    run_assess_template = os.path.join(template_dir, 'bash', 'run_apt_assess.sh')
    with open(run_assess_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, app_base_relative, sn_dst_node_list)
    with open(os.path.join(base_dir, "run_apt_assess.sh"), 'w') as f:
        f.write(contents)
    
    ### apt assess inside docker
    run_in_docker_assess_template = os.path.join(template_dir, 'bash', 'run_in_docker_apt_assess.sh')
    with open(run_in_docker_assess_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, app_base_relative, sn_dst_node_list)
    with open(os.path.join(base_dir, "run_in_docker_apt_assess.sh"), 'w') as f:
        f.write(contents)

    ### apt optimize
    run_optimze_template = os.path.join(template_dir, 'bash', 'run_apt_optimize.sh')
    with open(run_optimze_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, app_base_relative, sn_dst_node_list)
    with open(os.path.join(base_dir, "run_apt_optimize.sh"), 'w') as f:
        f.write(contents)

    ### apt optimize inside docker
    run_in_docker_optimze_template = os.path.join(template_dir, 'bash', 'run_in_docker_apt_optimize.sh')
    with open(run_in_docker_optimze_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, app_base_relative, sn_dst_node_list)
    with open(os.path.join(base_dir, "run_in_docker_apt_optimize.sh"), 'w') as f:
        f.write(contents)

    ### sm runner
    #   figure out what the first case is, as a sample
    path_to_first_case = ""
    if data_info['train'].get('apt_train_list'):
        chunks = pd.read_csv(data_info['train'].get('apt_train_list'), chunksize=1)
        df = next(chunks)
        path_to_first_case = df["image_file"][0]

    run_sm_runner_template = os.path.join(template_dir, 'bash', 'run_sm_runner.sh')
    with open(run_sm_runner_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, app_base_relative, sn_dst_node_list, path_to_first_case)
    with open(os.path.join(base_dir, "run_sm_runner.sh"), 'w') as f:
        f.write(contents)

    ### sm runner inside docker
    run_in_docker_sm_runner_template = os.path.join(template_dir, 'bash', 'run_in_docker_sm_runner.sh')
    with open(run_in_docker_sm_runner_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, app_base_relative, sn_dst_node_list, path_to_first_case)
    with open(os.path.join(base_dir, "run_in_docker_sm_runner.sh"), 'w') as f:
        f.write(contents)

    ## Summarize blackboard
    analysis_info = conf_info['analysis']
    vis_node_list = []
    vis_reference_keys = []
    cal_node_list = []
    if analysis_info.get('blackboard'):
        if analysis_info['blackboard'].get('node_to_visualize'):
            for vis_node, ref_node in analysis_info['blackboard']['node_to_visualize'].items():
                vis_node_list.append(vis_node)
                if ref_node: vis_reference_keys.append(ref_node)
                else: vis_reference_keys.append('')
        if analysis_info['blackboard'].get('node_to_evaluate'):
            cal_node_list = analysis_info['blackboard']['node_to_evaluate']
    
    ### summary_bb
    run_summary_bb_template = os.path.join(template_dir, 'bash', 'run_summary_bb.sh')
    with open(run_summary_bb_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, '.', ','.join(vis_node_list), ','.join(vis_reference_keys), ','.join(cal_node_list))
    with open(os.path.join(base_dir, "run_summary_bb.sh"), 'w') as f:
        f.write(contents)

    ### summary_bb inside docker
    run_in_docker_summary_bb_template = os.path.join(template_dir, 'bash', 'run_in_docker_summary_bb.sh')
    with open(run_in_docker_summary_bb_template, 'r') as f:
        contents = f.read()
    contents = contents % (app_version, '.', ','.join(vis_node_list), ','.join(vis_reference_keys), ','.join(cal_node_list))
    with open(os.path.join(base_dir, "run_in_docker_summary_bb.sh"), 'w') as f:
        f.write(contents)

    ### summary_apt
    if conf_info.get('apt_summary'):
        run_summary_apt_template = os.path.join(template_dir, 'bash', 'run_summary_apt.sh')
        with open(run_summary_apt_template, 'r') as f:
            contents = f.read()
        contents = contents % (app_version, '.', sn_dst_node_list)
        with open(os.path.join(base_dir, "run_summary_apt.sh"), 'w') as f:
            f.write(contents)

    log.info('---------------------------------------------------------------')
    log.info(f'Generated SM application directory: {base_dir}')
    log.info('---------------------------------------------------------------')

if __name__=='__main__':
    parser = ArgumentParser(description='Toos to configure a SimpleMind application')
    parser.add_argument('--conf_quick_start', type=str, dest='conf_quick_start', 
                        help="Path to the conf_quick_start")
    args = parser.parse_args()
    quick_start(args.conf_quick_start)
