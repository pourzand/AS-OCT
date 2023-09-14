************************************************
How to build docker
************************************************
Building your own docker within your QIA directory

Steps:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1. Build docker 

    Within your pulled QIA directory (change "my_docker_image_name" to your own docker image name please):

    .. code-block:: console

        cd Docker
        docker build -t my_docker_image_name .


2. Push to registry

    .. code-block:: console

        docker login registry.cvib.ucla.edu
        docker tag my_docker_image_name:latest registry.cvib.ucla.edu/my_docker_image_name:latest
        docker push registry.cvib.ucla.edu/my_docker_image_name:latest

3. Optionally, on another GPU:

    .. code-block:: console

        docker login registry.cvib.ucla.edu
        docker pull registry.cvib.ucla.edu/my_docker_image_name:latest
        docker tag registry.cvib.ucla.edu/my_docker_image_name:latest my_docker_image_name:latest
    
