FROM ubuntu:latest AS workspace

RUN apt update && apt install -y \
    wget \
    curl \
    zip \
    unzip \
    git \
    git-lfs \
    cmake \
    build-essential \
    python3 \
    python3-pip \
    python3-venv \
    clang-format \
    clang-tidy

# Install maelstrom
RUN apt update && apt install -y \
    openjdk-17-jdk \
    graphviz \
    gnuplot

RUN wget -P /tmp https://github.com/jepsen-io/maelstrom/releases/download/v0.2.4/maelstrom.tar.bz2 && \
    tar xvjf /tmp/maelstrom.tar.bz2 -C /opt/ && \
    ln -s /opt/maelstrom/maelstrom /usr/bin/maelstrom
