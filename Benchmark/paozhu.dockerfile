# paozhu TechEmpower benchmark dockerfile
# Build context: Benchmark/
# Usage: docker build -f paozhu.dockerfile -t paozhu_bench .
FROM ubuntu:24.04

# Suppress interactive apt prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install only core deps: build toolchain + zlib + libzip (for asio.zip extraction)
# Skip MySQL client libs — not needed for the benchmark
RUN apt update -yqq && apt install -yqq \
        build-essential \
        cmake \
        git \
        zip \
        unzip \
        zlib1g-dev \
        libzip-dev \
    && apt-get clean

# Copy the cleaned benchmark project tree
COPY paozhu_benchmark /paozhu

WORKDIR /paozhu

# Extract asio
RUN unzip -q asio.zip

# Release build (CMakeLists.txt defaults ENABLE_BENCHMARK=ON, entry main_docker.cpp)
RUN cmake . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

# TechEmpower benchmark port
EXPOSE 8888

# Run in foreground (PID 1 = service process, signals handled cleanly)
CMD ["./bin/paozhu"]
