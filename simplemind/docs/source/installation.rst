.. highlight:: shell

######################################
Installation
######################################


.. Stable release
.. ^^^^^^^^^^^^^^^^^^^^^^

.. Someday...


************************************************
Using official Docker image (recommanded)
************************************************

    .. code-block:: bash

        # pull the latest Docker image
        docker pull smaiteam/sm-develop

        # test installation
        docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir smaiteam/sm-develop:latest bash -c "python -c 'import simplemind; print(simplemind.__version__)'"

************************************************
Build the Docker image from sources
************************************************

    .. code-block:: bash

        # clone git repo
        git clone https://gitlab.com/sm-ai-team/simplemind.git
        cd simplemind

        # build Docker image
        docker build -t sm_develop:latest .

        # run docker container
        docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -w /workdir sm_develop:latest bash

        # test installation
        python -c "import simplemind; print(simplemind.__version__)"
