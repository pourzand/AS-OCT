export version=%s
export base_dir=%s
export checkpoint=$base_dir/apt_optimizer_checkpoint_$version/pilot.pk
export sn_path=%s
export results_path=$base_dir/result_$version
export summary_dir=$base_dir/summary_$version
export configurations_dir=$base_dir/configurations_$version
export summary_config=$configurations_dir/summary_apt.yml


export cmd="from simplemind.apt_agents.tools.ga.summary import ga_summary; ga_summary(\"${summary_config}\", \"${sn_path}\", \"${checkpoint}\", \"${results_path}\", \"${summary_dir}\",)"
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c '${cmd}'"
