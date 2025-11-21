# syntax=docker/dockerfile:1
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        git \
        libssl-dev \
        ninja-build \
        python3 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . /workspace

# Fetch third-party crypto libraries (LibOQS & GmSSL)
RUN rm -rf libs/liboqs libs/GmSSL && \
    cd libs && \
    git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git && \
    git clone --depth 1 https://github.com/guanzhi/GmSSL.git

# Build dependencies and the UCI project via the provided script
RUN chmod +x build.sh && \
    ./build.sh

# Run the test suite to ensure everything passes
RUN cd build && ctest --output-on-failure

CMD ["/bin/bash"]
