# source /scratch/wasil/env/etc/profile.d/conda.sh
# conda activate deep_med
#### uses yaml, deap that's only in deep_med
export version=%s
export base_dir=%s
export sn_path=%s
export checkpoint=$base_dir/apt_optimizer_checkpoint_$version/pilot.pk
export case_csv=$base_dir/think_$version/apt_train_list.csv
export results_path=$base_dir/result_$version
export configurations_dir=$base_dir/configurations_$version
export optimizer_config=$configurations_dir/optimizer.yml
export distributor_config=$configurations_dir/distributor.yml
export task_config=$configurations_dir/task.yml
export logging_dir=$base_dir/log

export log_file=log_optimize_condor_$version.log
python -c "from simplemind import apt; apt.optimizer('${case_csv}', '${sn_path}', '${results_path}',\
        checkpoint='${checkpoint}', \
        optimizer_config='${optimizer_config}', distributor_config='${distributor_config}', task_config='${task_config}', log_path='${logging_dir}',\
        )" 2>&1 | tee ${log_file}
