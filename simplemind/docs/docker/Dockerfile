FROM sphinxdoc/sphinx-latexpdf
# FROM sphinxdoc/sphinx

ARG MYPATH=/usr/local
ARG MYLIBPATH=/usr/lib

RUN apt-get update && apt-get install -y --no-install-recommends \
        autotools-dev \
        build-essential \
        ca-certificates \
        cmake \
        git \
        wget \
        curl \
        vim
RUN rm -rf /var/lib/apt/lists/*

# install miniconda.
# create and activate python virtual env with desired version
RUN wget --quiet --no-check-certificate https://repo.continuum.io/miniconda/Miniconda3-4.6.14-Linux-x86_64.sh --no-check-certificate -O ~/miniconda.sh && \
    /bin/bash ~/miniconda.sh -b -p /opt/conda
RUN /opt/conda/bin/conda create -n py python=3.7.2
RUN echo "source /opt/conda/bin/activate py" > ~/.bashrc
ENV PATH /opt/conda/envs/py/bin:$PATH
RUN /bin/bash -c "source /opt/conda/bin/activate py"

RUN /bin/bash -c "source /opt/conda/bin/activate py && conda install cython numpy -y && pip install scikit-build && pip install matplotlib"
RUN /bin/bash -c "source /opt/conda/bin/activate py && conda install -c conda-forge jupyterlab -y"
RUN /bin/bash -c "source /opt/conda/bin/activate py && conda install -c conda-forge nbsphinx -y"

RUN pip install sphinx-rtd-theme numpydoc sphinx-copybutton
# RUN pip install ipywidgets matplotlib medpy opencv-python plotly tabulate
# RUN pip install tensorflow pandas scikit-image pydicom

# ARG UNAME=testuser
# ARG UID=1000
# ARG GID=1000
# RUN groupadd -g $GID -o $UNAME
# RUN useradd -l -m -u $UID -g $GID -o -s /bin/bash $UNAME && \
#     usermod -aG sudo $UNAME
# RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
# USER $UNAME

# CMD /bin/bash