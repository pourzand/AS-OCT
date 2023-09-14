##################################################
# Define all our apt-installed dependencies into args
##################################################
# We're going to need to reuse these lists to build our final runner
# image; all apt-installed dependencies need to be defined here for
# aggressive docker caching.

# This approach may seem strange, but it's intentional.
# We want to be able to modify our dependency lists and not trigger a
# rebuild of every c++ library, but we also need to keep track of all
# these dependencies, so that we can install them in the final runtime
# image
ARG DEPS_CORE='wget \
	autotools-dev \
	build-essential \
	ca-certificates \
	cmake \
	git \
	wget \
	curl \
	vim'
ARG DEPS_DCMTK='libtiff5-dev \
	libpng-dev \
	libjpeg-dev \
	libgif-dev \
	libxml2-dev \
	libssl-dev \
	zlib1g-dev \
	libwrap0-dev \
	libssl-dev'

ARG DEPS_BOOST='libbz2-dev'
ARG DEPS_GDCM='g++'
ARG DEPS_RUNTIME='graphviz \
	graphviz-dev'

ARG DEPS_ALL="$DEPS_CORE $DEPS_DCMTK $DEPS_BOOST $DEPS_BOOST $DEPS_GDCM $DEPS_RUNTIME"

ARG MYPATH=/usr/local
ARG MYLIBPATH=/usr/lib

##################################################
# Contruct our base cpp builder image
##################################################
FROM nvidia/cuda:10.0-cudnn7-devel-ubuntu18.04 as cpp-builder

ARG DEPS_CORE

# Fetch the core dependencies for our C++ builder image
RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends $DEPS_CORE

##################################################
# Construct our base python image (used for boost-builder and final runtime)
##################################################
FROM cpp-builder as conda-base

# prefetch source
RUN wget --quiet --no-check-certificate https://repo.continuum.io/miniconda/Miniconda3-4.6.14-Linux-x86_64.sh --no-check-certificate -O ~/miniconda.sh && \
	/bin/bash ~/miniconda.sh -b -p /opt/conda

# env config
RUN /opt/conda/bin/conda create -n py python=3.7.2
RUN echo "source /opt/conda/bin/activate py" > ~/.bashrc
ENV PATH /opt/conda/envs/py/bin:$PATH
RUN /bin/bash -c "source /opt/conda/bin/activate py"


RUN /opt/conda/bin/conda install --debug --override-channels -c main -c conda-forge jupyterlab
RUN /opt/conda/bin/conda install --debug --override-channels -c main -c conda-forge cython numpy -y

#RUN /opt/conda/bin/conda install -c conda-forge jupyterlab
#RUN conda install cython numpy -y

##################################################
# Build zlib
##################################################
FROM cpp-builder AS zlib-builder

ARG MYPATH

# prefetch sources
WORKDIR /opt/sources
RUN wget --quiet https://github.com/madler/zlib/archive/refs/tags/v1.2.12.tar.gz -O zlib.tar.gz \
	&& tar xfz zlib.tar.gz

# compile
WORKDIR /opt/sources/zlib-1.2.12
RUN ./configure --prefix=$MYPATH && make -j"$(nproc)" #&& make install -j"$(nproc)"

##################################################
# Build VTK
##################################################
FROM cpp-builder AS vtk-builder

ARG MYPATH

# Prefetch sources
WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate http://www.vtk.org/files/release/7.0/VTK-7.0.0.tar.gz -O vtk.tar.gz && \
	tar xfz vtk.tar.gz

# compile
WORKDIR /opt/sources/VTK-7.0.0
RUN sed -i 's/\[345\]/[34567]/g' CMake/vtkCompilerExtras.cmake
RUN sed -i 's/\[345\]/[34567]/g' CMake/GenerateExportHeader.cmake
RUN mkdir build
WORKDIR /opt/sources/VTK-7.0.0/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=$MYPATH -DCMAKE_BUILD_TYPE=Release \
    -DVTK_Group_Rendering:BOOL=OFF -DVTK_Group_StandAlone:BOOL=ON -DVTK_RENDERING_BACKEND=None
RUN make -j"$(nproc)" #&& make install -j"$(nproc)"

##################################################
# Build ITK
##################################################
FROM cpp-builder AS itk-builder

ARG MYPATH

# prefetch sources
WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate https://github.com/InsightSoftwareConsortium/ITK/releases/download/v5.0.0/InsightToolkit-5.0.0.tar.gz -O itk.tar.gz && \
    tar xfz itk.tar.gz

# copy over and install compiled vtk libraries
RUN mkdir -p /opt/sources/VTK-7.0.0
COPY --from=vtk-builder /opt/sources/VTK-7.0.0 /opt/sources/VTK-7.0.0
WORKDIR /opt/sources/VTK-7.0.0/build
RUN make install -j"$(nproc)"

# compile itk
WORKDIR /opt/sources/InsightToolkit-5.0.0
RUN mkdir build
WORKDIR /opt/sources/InsightToolkit-5.0.0/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=$MYPATH -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_DOXYGEN=OFF -DBUILD_EXAMPLES=OFF \
    -DBUILD_SHARED_LIBS=OFF -DITK_DYNAMIC_LOADING=OFF -DBUILD_TESTING=OFF \
    -DCMAKE_BACKWARDS_COMPATIBILITY=3.1 -DITK_USE_KWSTYLE=OFF \
    -DITK_BUILD_ALL_MODULES=ON -DModule_ITKVtkGlue=ON -DITK_USE_REVIEW=ON 
RUN make -j"$(nproc)" #&& make install -j"$(nproc)"

##################################################
# Build DCMTK
##################################################
FROM cpp-builder as dcmtk-builder

ARG MYPATH
ARG DEPS_DCMTK

# prefetch
WORKDIR /opt/sources
RUN wget --quiet https://github.com/DCMTK/dcmtk/archive/DCMTK-3.6.4.tar.gz -O dcmtk.tar.gz && \
	tar xfz dcmtk.tar.gz

# fetch our additional dependencies
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends $DEPS_DCMTK

# compile
WORKDIR /opt/sources/dcmtk-DCMTK-3.6.4
RUN mkdir build
WORKDIR /opt/sources/dcmtk-DCMTK-3.6.4/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=$MYPATH -DCMAKE_LIBRARY_PATH=$MYLIBPATH -DDCMTK_FORCE_FPIC_ON_UNIX:BOOL=ON
RUN make -j"$(nproc)" #&& make install -j"$(nproc)"

##################################################
# Build GDCM
##################################################
FROM cpp-builder as gdcm-builder

ARG DEPS_GDCM

# prefetch
WORKDIR /opt/sources
#RUN curl -LO https://github.com/malaterre/GDCM/archive/v3.0.4.tar.gz && \
#	tar -zxvf v3.0.4.tar.gz
RUN wget --quiet https://github.com/malaterre/GDCM/archive/v3.0.4.tar.gz && \
	tar -zxvf v3.0.4.tar.gz

# additional dependencies
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends $DEPS_GDCM

# compile
RUN mkdir /opt/sources/GDCM-3.0.4/build
WORKDIR /opt/sources/GDCM-3.0.4/build
RUN cmake -D CMAKE_INSTALL_RPATH=/local/gdcm/lib -D CMAKE_INSTALL_PREFIX=/local/gdcm -D GDCM_BUILD_APPLICATIONS=1 -D GDCM_BUILD_SHARED_LIBS=1 ..
RUN make -j"$(nproc)" #&& RUN make -j 16 install

##################################################
# Build Boost
##################################################
FROM conda-base AS boost-builder

ARG MYPATH
ARG DEPS_BOOST

# prefetch source
WORKDIR /opt/sources
RUN wget --quiet http://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz -O boost.tar.gz && \
	tar xfz boost.tar.gz

# amend the base dependencies
RUN apt-get update && apt-get install -y --fix-missing --no-install-recommends $DEPS_BOOST

# compile
WORKDIR /opt/sources/boost_1_60_0
ENV CPLUS_INCLUDE_PATH /opt/conda/envs/py/include/python3.7m/
RUN ./bootstrap.sh --prefix=$MYPATH --with-python=python3.7 --with-libraries=python,filesystem,system,test,iostreams
# PATCH for python 3.7 compliance - see https://github.com/boostorg/python/commit/660487c43fde76f3e64f1cb2e644500da92fe582 for detail
RUN wget --quiet https://raw.githubusercontent.com/boostorg/python/660487c43fde76f3e64f1cb2e644500da92fe582/src/converter/builtin_converters.cpp
RUN mv libs/python/src/converter/builtin_converters.cpp libs/python/src/converter/builtin_converters.BAK
RUN mv builtin_converters.cpp libs/python/src/converter/builtin_converters.cpp
RUN mkdir build
RUN ./b2 --prefix=build/ -j"$(nproc)" install

##################################################
# Assemble the final image
##################################################
FROM conda-base AS runtime

ARG MYPATH
ARG DEPS_ALL

# amend the image to ensure that we have all system deps we need (some will already be satisfied)
RUN apt-get update && apt-get install -y --fix-missing --no-install-recommends $DEPS_ALL

RUN pip install ipywidgets plotly tabulate pygraphviz PyQt5 scikit-build

# prefetch everything we need (do this FIRST so we don't re-trigger the download every time we change SM source code)
## elastix
WORKDIR /opt/sources
RUN mkdir -p /opt/sources/elastix
RUN wget --quiet --no-check-certificate https://github.com/SuperElastix/elastix/releases/download/4.9.0/elastix-4.9.0-linux.tar.bz2 -O elastix.tar.bz2 && \
	tar xjf elastix.tar.bz2 -C elastix

## dicom dict
WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate https://raw.githubusercontent.com/InsightSoftwareConsortium/DCMTK/master/dcmdata/data/dicom.dic
RUN mv dicom.dic /usr/local/lib/dicom.dic

# copy and install our compiled dependencies into our runtime
COPY --from=zlib-builder /opt/sources/zlib-1.2.12 /opt/sources/zlib-1.2.12
WORKDIR /opt/sources/zlib-1.2.12
RUN make -j"$(nproc)" install

COPY --from=vtk-builder /opt/sources/VTK-7.0.0 /opt/sources/VTK-7.0.0
WORKDIR /opt/sources/VTK-7.0.0/build
RUN make install -j"$(nproc)"

COPY --from=itk-builder /opt/sources/InsightToolkit-5.0.0 /opt/sources/InsightToolkit-5.0.0
WORKDIR /opt/sources/InsightToolkit-5.0.0/build
RUN make install -j"$(nproc)"

COPY --from=gdcm-builder /opt/sources/GDCM-3.0.4 /opt/sources/GDCM-3.0.4
WORKDIR /opt/sources/GDCM-3.0.4/build
RUN make install -j"$(nproc)"

COPY --from=dcmtk-builder /opt/sources/dcmtk-DCMTK-3.6.4 /opt/sources/dcmtk-DCMTK-3.6.4
WORKDIR /opt/sources/dcmtk-DCMTK-3.6.4/build
RUN make install -j"$(nproc)"

COPY --from=boost-builder /opt/sources/boost_1_60_0/build /opt/sources/boost_1_60_0/build
WORKDIR /opt/sources/boost_1_60_0/build
RUN cp -rv include/boost /usr/local/include/
RUN cp -rv lib/* /usr/local/lib/

# ENVIRONMENT CONFIG
ENV ELASTIX_LIBRARYDIR /opt/sources/elastix/lib
ENV LD_LIBRARY_PATH $ELASTIX_LIBRARYDIR:$LD_LIBRARY_PATH
ENV ELASTIX_PATH /opt/sources/elastix/bin
ENV PATH $ELASTIX_PATH:$PATH

ENV DCMDICTPATH /usr/local/lib/dicom.dic

ENV MAIN_LIBRARYDIR /usr/local/lib
ENV LD_LIBRARY_PATH $MAIN_LIBRARYDIR:$LD_LIBRARY_PATH

ENV LOBESEG_BOOST_LIBRARYDIR /usr/local/boost-1.60.0/lib
ENV LOBESEG_MAIN_LIBRARYDIR /usr/local/lobseg/lib
ENV LOBESEG_ITK_LIBRARYDIR $LOBESEG_MAIN_LIBRARYDIR/InsightToolkit
ENV LOBESEG_LD_LIBRARY_PATH $LOBESEG_BOOST_LIBRARYDIR:$LOBESEG_MAIN_LIBRARYDIR:$LOBESEG_ITK_LIBRARYDIR:

ENV GDCM_PATH /local/gdcm/bin
ENV PATH $GDCM_PATH:$PATH

# Final GDCM setup?
# I'm not sure what this block does or if its necessary
#WORKDIR /
#RUN tar cvzf gdcm.tar.gz /local/gdcm
#RUN tar cvzf include.tar.gz /usr/local/include
#RUN tar cvzf lib.tar.gz /usr/local/lib

# Run some cleanup
RUN apt-get autoclean
RUN apt-get autoremove
RUN mv /opt/sources/elastix /opt/
RUN rm -rf /opt/sources/*
RUN mv /opt/elastix /opt/sources/

# Finally, install our SM stuff
WORKDIR /opt/sources
COPY ./requirements/ds_requirements.txt /tmp/ds_requirements.txt
COPY ./requirements/qia_requirements.txt /tmp/qia_requirements.txt

RUN pip install -r /tmp/ds_requirements.txt
RUN pip install -r /tmp/qia_requirements.txt

COPY ./requirements/deep_med_requirements.txt /tmp/deep_med_requirements.txt
RUN pip install -r /tmp/deep_med_requirements.txt

RUN ln -s /opt/conda/envs/py/bin/python /usr/local/bin/py

WORKDIR /app
COPY setup.py setup.py
COPY . .
RUN chmod 777 simplemind/think/bin/sm/script/*.ini
RUN chmod 777 simplemind/tools/quick_start_template/*.yml
RUN chmod 777 simplemind/tools/quick_start_template/bash/*.sh
RUN chmod 777 simplemind/apt_agents/optimizer/ga/config/*/*.tpl
RUN chmod 777 simplemind/apt_agents/tools/ga/default_metrics.yml
RUN python setup.py install

