FROM centos:7

ADD . /opt/c_extension
WORKDIR /opt/c_extension

RUN set -xe \
  && yum install -y gcc \
                    make \
                    protobuf \
                    protobuf-c \
                    protobuf-c-compiler \
                    protobuf-c-devel \
                    python-devel \
                    python-setuptools \
                    gdb \
                    zlib-devel

RUN ulimit -c unlimited

RUN protoc-c --c_out=. deviceapps.proto

RUN python setup.py build_ext --inplace && python setup.py test -q && /bin/bash
