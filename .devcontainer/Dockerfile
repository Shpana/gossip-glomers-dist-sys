FROM ubuntu:latest AS workspace

RUN apt-get update && apt-get install -y \
    git \
    git-lfs \
    make \
    cmake \
    clang-format \
    clang-tidy

# Installing folly
RUN apt-get update && apt-get install -y \
    libgoogle-glog-dev \
    libgflags-dev \
    libboost-all-dev \
    libdouble-conversion-dev \
    libssl-dev \
    libevent-dev \
    libiberty-dev \
    libfmt-dev \
    liblz4-dev \
    liblzma-dev \
    libsnappy-dev \
    libzstd-dev \
    zlib1g-dev \
    libunwind-dev \
    libdwarf-dev \
    libfast-float-dev \
    libsodium-dev

RUN git clone https://github.com/axboe/liburing.git /usr/lib/uring
RUN cd /usr/lib/uring && \
    ./configure --cc=/usr/bin/gcc --cxx=/usr/bin/g++ && \
    make && make install 

RUN git clone https://github.com/facebook/folly /usr/lib/folly
RUN cd /usr/lib/folly/build && \
    cmake \
        -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
        -DCMAKE_C_COMPILER=/usr/bin/gcc \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_CXX_STANDARD_REQUIRED=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && \
    make all && make install
