##################################################################
# Please run this bash file inside the following docker container
##################################################################
# Define the environment variables
export version=%s
export base_dir=%s
export sn_path=%s
export case_csv=$base_dir/think_$version/apt_train_list.csv
export results_path=$base_dir/result_$version
export configurations_dir=$base_dir/configurations_$version
export distributor_config=$configurations_dir/distributor.yml
export task_config=$configurations_dir/task.yml
export logging_dir=$base_dir/log

# Run summary for relationships in SN
export log_file=$base_dir/log_sm_summary_relation_$version.log
python -c "from simplemind import tools; tools.summary_sn_relationships_graph(sn_path='${sn_path}',\
            summary_path='./sn_summary_relationship_graph.png')" 2>&1 | tee ${log_file}

# Run summary for chromosome in SN
export log_file=$base_dir/log_sm_summary_chrom_$version.log
python -c "from simplemind import tools; tools.summary_sn_chromosomes(sn_path='${sn_path}',\
            binary_chromosome=None, summary_path='./sn_summary_chromosome.txt')" 2>&1 | tee ${log_file}

# Run SM apt assess
export log_file=$base_dir/log_assess_$version.log
python -c "from simplemind import assess; assess.assess('${case_csv}', '${sn_path}', '${results_path}',\
        distributor_config='${distributor_config}', task_config='${task_config}', log_path='${logging_dir}',)" 2>&1 | tee ${log_file}
