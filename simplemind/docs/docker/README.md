# Docker build, test and push

## Build a docker image

```{bash}
docker build -t sphinxdoc_nb_numpydoc:latest .
```

## Test a docker image

Run the docker container and build the HTML

```{bash}
export library_home=/cvib2/apps/personal/youngwonchoi/lib
export document_home=../doc/
docker run -it -u $(id -u):$(id -g) -v $document_home:/workdir -v $library_home:/medqia -v /cvib2:/cvib2 -v /scratch:/scratch -v /radraid:/radraid -w /workdir --privileged sphinxdoc_nb_numpydoc:latest bash

make html
```

## Push the docker container

```{bash}
docker tag sphinxdoc_nb_numpydoc:latest registry.rip.ucla.edu/sphinxdoc_nb_numpydoc:latest
docker push registry.rip.ucla.edu/sphinxdoc_nb_numpydoc:latest
```