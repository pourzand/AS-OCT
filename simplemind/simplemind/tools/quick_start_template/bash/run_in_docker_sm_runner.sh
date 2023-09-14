##################################################################
# Please run this bash file inside the following docker container
##################################################################
# Define the environment variables
export version=%s
export base_dir=%s
export sn_path=%s

# Run SM runner on first case in ga train list
export image_path=%s
export single_case_result=$base_dir/sm_runner
export working_directory=$base_dir/think_$version
export user_resource_directory=$base_dir/configurations_$version/resource_local
export log_file=$base_dir/log_sm-runner_$version.log
python -c "from simplemind import sm; sm.runner('${image_path}', '${sn_path}', '${single_case_result}',\
        working_directory='${working_directory}', user_resource_directory='${user_resource_directory}', force_overwrite=True,)" 2>&1 | tee ${log_file}

export cmd="from simplemind import sm; sm.runner('${image_path}', '${sn_path}', '${single_case_result}',\
        working_directory='${working_directory}', user_resource_directory='${user_resource_directory}', force_overwrite=True,)"
echo "$cmd"
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c '${cmd}' 2>&1 | tee ${log_file}"
