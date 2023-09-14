export version=%s
export base_dir=%s
export node_list=%s
export reference_keys=%s
export cal_nodes=%s
export fig_mode='t' # c:Coronal, s:sagittal, a:axial, t:2D_slices
export eval_mode='w' # r: read, w: write or overwrite
export chromosome='DEFAULT'
export case_list=$base_dir/think_$version/apt_train_list.csv
export result_dir_list_path=$base_dir/result_$version/$chromosome
export summary_dir=$base_dir/summary_$version/png_$chromosome
export summary_html_name=evaluation_${version}_${chromosome}_${fig_mode}.html
export summary_performance_name=performance_${version}_${chromosome}.csv

export cmd="from simplemind import tools; tools.summary_blackboards(summary_dir=\"${summary_dir}\",\
case_list_path=\"${case_list}\", result_dir_list_path=\"${result_dir_list_path}\",\
summary_html_name=\"${summary_html_name}\", summary_performance_name=\"${summary_performance_name}\",\
node_list = \"${node_list}\", reference_keys = \"${reference_keys}\", cal_nodes = \"${cal_nodes}\", \
fig_mode = \"${fig_mode}\",  eval_mode = \"${eval_mode}\")"
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c '${cmd}'"
