# Define the environment variables
export version=%s
export base_dir=%s
export sn_path=%s
export case_csv=$base_dir/think_$version/apt_train_list.csv
export results_path=$base_dir/result_$version
export configurations_dir=$base_dir/configurations_$version
export optimizer_config=$configurations_dir/optimizer.yml
export distributor_config=$configurations_dir/distributor.yml
export task_config=$configurations_dir/task.yml
export checkpoint=$base_dir/apt_optimizer_checkpoint_$version/pilot.pk

# Run summary for relationships in SN
export log_file=$base_dir/log_sm_summary_relation_$version.log
export summary_sn_relationships_cmd="from simplemind import tools;\
tools.summary_sn_relationships_graph(sn_path=\"${sn_path}\",\
summary_path=\"./sn_summary_relationship_graph.png\")"
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c '${summary_sn_relationships_cmd}' 2>&1 | tee ${log_file}"

# Run summary for chromosome in SN
export log_file=$base_dir/log_sm_summary_chrom_$version.log
export summary_sn_chrom_cmd="from simplemind import tools;\
tools.summary_sn_chromosomes(sn_path=\"${sn_path}\",\
binary_chromosome=None,\
summary_path=\"./sn_summary_chromosome.txt\")"
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c '${summary_sn_chrom_cmd}' 2>&1 | tee ${log_file}"

# Run SM apt optimizer
export log_file=$base_dir/log_optimize_$version.log
export cmd="from simplemind import apt; apt.optimizer(\"${case_csv}\", \"${sn_path}\", \"${results_path}\", \
optimizer_config=\"${optimizer_config}\", \
distributor_config=\"${distributor_config}\", \
task_config=\"${task_config}\", \
checkpoint=\"${checkpoint}\")"
echo "$cmd"
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c '${cmd}' 2>&1 | tee ${log_file}"
