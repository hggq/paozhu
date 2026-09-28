# Paozhu Production Dockerfile
# 构建上下文: 项目根目录 (docker compose 自动设置)
# 基于 Benchmark/paozhu_benchmark (已清理 + TechEmpower 基准就绪)
#
# 用法:
#   docker compose -f docker/prod/docker-compose.yml up -d
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# 核心依赖: 编译工具 + zlib
# 不安装 MySQL 客户端库 (基准不需要)
RUN apt update -yqq && apt install -yqq \
        build-essential \
        cmake \
        git \
        zip \
        unzip \
        zlib1g-dev \
        libzip-dev \
    && apt-get clean

# 复制已清理好的基准项目 (CMakeLists.txt 默认 ENABLE_BENCHMARK=ON, 入口 main_docker.cpp)
COPY Benchmark/paozhu_benchmark /paozhu

WORKDIR /paozhu

# 解压 asio
RUN unzip -q asio.zip

# Release 构建
RUN cmake . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

# 服务端口
EXPOSE 8888

# 前台运行 (PID 1 = 服务进程, 信号可正常处理)
CMD ["./bin/paozhu"]
