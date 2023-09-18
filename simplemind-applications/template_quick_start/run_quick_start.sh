export conf_quick_start=quick_start_template.yml
export cmd="from simplemind import tools; tools.quick_start(\"${conf_quick_start}\")"
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c '${cmd}'"
