# Documentation for SM

Documentation generated by sphinx, ReadtheDocs theme and numpydoc extentions.
Here is some explanation about how to use the documentation tools: https://docs.google.com/document/d/1eoL1l8rIrL3IYZ8TSYZep4EtAZ6z-NRt6xWU2wka_9U/edit?usp=sharing


# How to build the HTML

Run the docker container and build the HTML

```{bash}
export library_home=/cvib2/apps/personal/youngwonchoi/lib
export document_home=./
docker run -it -u $(id -u):$(id -g) -v $document_home:/workdir -v $library_home:/medqia -v /cvib2:/cvib2 -v /scratch:/scratch -v /radraid:/radraid -w /workdir --privileged registry.rip.ucla.edu/sphinxdoc_nb_numpydoc:latest

make html
```
