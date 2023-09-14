# Define the environment variables
export tb_port=%s
export logdir=%s
export cmd="tensorboard --logdir=${logdir} --port=${tb_port}"

# Run the tensorboard
docker run -it -p $tb_port:$tb_port -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop bash -c "$cmd"
