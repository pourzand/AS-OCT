************************************************
How to set your jupyter notebook server
************************************************
How to launch your own jupyter server, and use the jupyter notebook under your permission.

Here is some steps to set your jupyter notebook:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You need to do these steps only when your jupyter notebook server has been down (e.g. because of server reboots)

you need to change the contents within ``{...}`` to your own

1. Recommend to open a screen

    .. code-block:: console

        screen -S jupyter

2. Go to the place that you want to make it as your jupyter home

    .. code-block:: console
    
        cd {/radraid/apps/personal/youngwon}

3. Choose the port that you want to use (e.g. other people are not using at that time).
    I picked ``7001`` in this example

4. Run a docker container
    
    .. hint::  If you have not build the docker image yet, please see the **Using Docker** section from `_Installation`.

    .. code-block:: console
    
        docker run -it -u $(id -u):$(id -g) -p {7001}:{7001} --privileged -v $PWD:/workdir -v ~/.local:/.local -v /cvib2:/cvib2 -v /scratch:/scratch -v /radraid:/radraid -w /workdir sm_release:latest bash

The volume mounting arguments in `{-v ....}` is optional, but you should add any storage that you wanted to use within the docker container.

1. Launch a jupyter server

    .. code-block:: console
    
        jupyter notebook --port={7001} --no-browser --ip=0.0.0.0

    
    If you see any error related to permission something in this step, first try this outside the docker container
    
    .. code-block:: console
    
        chmod -R 777 ~/.local

    If this one is not helpful, ask the person who has a sudo permission in your computing server.

2. Detach the screen without kill the jupyter server.

    .. code-block:: console
        
        ctrl+a+d


Open the jupyter home
^^^^^^^^^^^^^^^^^^^^^^^^^^

Now it is ready to open your jupyter home.

If you launch your jupyter server in the computing server ``REDLRADADM23710`` (lambda1 server from inhouse)), and your port was ``7001``, open

http://REDLRADADM23710:7001

and check the log from jupyter server (in your docker container) to catch the token.

You need to copy the token in the log from jupyter server in your docker container.


Tips
^^^^^

When you need to check the token again,

1. Attach the screen

    .. code-block:: console
        
        screen -r jupyter

2. Find out your token in there. 

    If log is too long, press ``ctrl+c``. Then, it will show you token and ask you to quit your jupyter server.

    Press ``n`` if you don't want to really kill the jupyter notebook.


.. _Installation: ./installation.rst
