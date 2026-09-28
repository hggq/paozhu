#!/usr/bin/env bash
#
# mini_project.sh - Trim the full Paozhu project into a clean scaffold
# Follows "Clean Project Initialization" four-step flow
#
# Usage:
#   ./mini_project.sh              Trim to clean scaffold (framework core + testhello)
#   ./mini_project.sh --benchmark  Trim then overlay TechEmpower benchmark files
#
set -euo pipefail

# ---- Argument parsing ----
BENCHMARK_MODE=0
if [ "${1:-}" = "--benchmark" ]; then
    BENCHMARK_MODE=1
fi

# ---- Locate project root (script directory) ----
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Benchmark source directory
BENCHMARK_SRC="$SCRIPT_DIR/Benchmark/paozhu_benchmark"

echo "=============================================="
echo " Paozhu Clean Project Initialization"
echo " Project root: $SCRIPT_DIR"
if [ "$BENCHMARK_MODE" -eq 1 ]; then
    echo " Mode: TechEmpower Benchmark"
    echo " Benchmark src: $BENCHMARK_SRC"
fi
echo "=============================================="

# ---- Safety check: must be in the paozhu project root ----
if [ ! -f "CMakeLists.txt" ] || [ ! -d "vendor/httpserver" ]; then
    echo "ERROR: Not in paozhu project root (missing CMakeLists.txt or vendor/httpserver)"
    exit 1
fi

# ============================================================
# Step 1 — Clean common/ registration files
#   Keep #include "httppeer.h" and type-def required includes,
#   empty out all registration function bodies (signatures kept)
# ============================================================
echo ""
echo "[Step 1/4] Cleaning common/ registration files..."

cat > common/autocontrolmethod.hpp << 'EOF'
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

namespace http
{ 
     
    void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
    {
    }
    
    void _initauto_co_control_httpmethodregto(std::map<std::string, regmethold_co_t> &methodcallback)
    {
    }
    
    void _initauto_domain_httpmethodregto(std::map<std::string, std::map<std::string, regmethold_t>> &domain_methodcallback)
    {
    }
    
    void _initauto_co_domain_httpmethodregto(std::map<std::string, std::map<std::string, regmethold_co_t>> &domain_methodcallback)
    {
    }
        
}    
#endif
EOF

cat > common/autorestfulpaths.hpp << 'EOF'
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPRESTFUL_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPRESTFUL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

namespace http
{
  void _initauto_control_httprestful_paths(std::map<std::string, std::vector<std::string>>  &restfulmethod)
  {
  }
    
    void _initauto_domain_httprestful_paths(std::map<std::string,std::map<std::string, std::vector<std::string>>>  &restfulmethod)
    {
        std::map<std::string, std::vector<std::string>> temp_path;
        std::map<std::string,std::map<std::string, std::vector<std::string>>>::iterator domain_iterator;  

        domain_iterator=restfulmethod.begin();
        temp_path.clear();
    }
    
}

#endif
EOF

cat > common/reghttpmethod.hpp << 'EOF'
#ifndef __HTTP_REGHTTPMETHOD_HPP
#define __HTTP_REGHTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif// defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h"
namespace http
{
void _inithttpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
{
}

}// namespace http
#endif
EOF

cat > common/reghttpmethod_pre.hpp << 'EOF'
#ifndef __HTTP_REGHTTPMETHOD_PRE_HPP
#define __HTTP_REGHTTPMETHOD_PRE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h"
namespace http
{
  void _inithttpmethodregto_pre(std::map<std::string, regmethold_t> &methodcallback)
  {
  }

}
#endif
EOF

# sockets_method_reg.hpp — keep http_socket.h (defines HTTP_SOCKET_REG)
cat > common/sockets_method_reg.hpp << 'EOF'
#pragma once
#include <map>
#include "httppeer.h"
#include "http_socket.h"

namespace http
{
void _inithttpsocketmethodregto(HTTP_SOCKET_REG &methodcallback)
{
}

}// namespace http
EOF

# websockets_method_reg.hpp — keep websockets_callback.h (defines WEBSOCKET_REG)
cat > common/websockets_method_reg.hpp << 'EOF'
#pragma once
#include <map>
#include "httppeer.h"
#include "websockets_callback.h"

namespace http
{
void _initwebsocketmethodregto(WEBSOCKET_REG &methodcallback)
{
}

}// namespace http
EOF

echo "  OK cleaned 6 registration files (autocontrolmethod / autorestfulpaths /"
echo "    reghttpmethod / reghttpmethod_pre / sockets_method_reg / websockets_method_reg)"

# ============================================================
# Step 2 — Remove ORM layer
#   models/*  schema/*  orm/*  (directories themselves kept)
# ============================================================
echo ""
echo "[Step 2/4] Removing ORM layer (models/ schema/ orm/)..."

rm -rf models/*
rm -rf schema/*
rm -rf orm/*

echo "  OK models/ schema/ orm/ emptied"

# ============================================================
# Step 3 — Clean view files (keep registration skeleton)
#   3.1 Remove viewsrc/view/ and view/
#   3.2 Empty the namespace view body inside viewsrc/include/viewsrc.h
#   3.3 Empty function bodies in viewsrc/include/regviewmethod.hpp
# ============================================================
echo ""
echo "[Step 3/4] Cleaning view files..."

# 3.1 Remove view sources and templates
rm -rf viewsrc/view/
rm -rf view/

# 3.2 Empty the internal namespace in viewsrc.h
cat > viewsrc/include/viewsrc.h << 'EOF'
#ifndef __HTTP_VIEWSRC_ALL_METHOD_H
#define __HTTP_VIEWSRC_ALL_METHOD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"

namespace http { 
namespace view { 

}

}
#endif
EOF

# 3.3 Empty the function body in regviewmethod.hpp
cat > viewsrc/include/regviewmethod.hpp << 'EOF'
#ifndef __HTTP_REG_VIEW_METHOD_HPP
#define __HTTP_REG_VIEW_METHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"
#include "viewmethold_reg.h"
#include "viewsrc.h"

namespace http
{
  void _initview_method_regto(VIEW_REG  &_viewmetholdreg)
  {
  } 
}
#endif
EOF

echo "  OK viewsrc/view/ and view/ removed"
echo "  OK viewsrc.h / regviewmethod.hpp registration content emptied"

# 3.4 Clean www/default, keep only index.html (content: Hello World! Paozhu)
rm -rf www/default/*
echo "Hello World! Paozhu" > www/default/index.html
echo "  OK www/default cleaned, only index.html kept"

# ============================================================
# Step 4 — Clean controller
#   Normal mode: keep only testhello.cpp
#   Benchmark mode: delete all (techempower overlaid next)
# ============================================================
echo ""
if [ "$BENCHMARK_MODE" -eq 1 ]; then
    echo "[Step 4/5] Cleaning controller (benchmark mode: delete all)..."
    rm -rf controller/include/*
    find controller/src -type f -delete
    find controller/src -mindepth 1 -type d -empty -delete 2>/dev/null || true
    echo "  OK controller emptied (techempower to be overlaid)"
else
    echo "[Step 4/4] Cleaning controller (keep testhello.cpp)..."
    rm -rf controller/include/*
    find controller/src -type f ! -name 'testhello.cpp' -delete
    find controller/src -mindepth 1 -type d -empty -delete 2>/dev/null || true
    echo "  OK controller: only testhello.cpp kept"
fi

# ============================================================
# Step 5 — Benchmark mode: overlay TechEmpower benchmark files
#   Copy business files from Benchmark/paozhu_benchmark/
#   Do NOT overwrite framework files (CMakeLists.txt / vendor / startup / etc.)
# ============================================================
if [ "$BENCHMARK_MODE" -eq 1 ]; then
    echo ""
    echo "[Step 5/5] Overlaying TechEmpower benchmark files..."

    if [ ! -d "$BENCHMARK_SRC" ]; then
        echo "ERROR: Benchmark source dir not found: $BENCHMARK_SRC"
        exit 1
    fi

    # controller: techempower
    mkdir -p controller/include controller/src
    cp "$BENCHMARK_SRC/controller/include/techempower.h" controller/include/
    cp "$BENCHMARK_SRC/controller/src/techempower.cpp" controller/src/
    echo "  OK controller: techempower"

    # models: World, Fortune
    rm -rf models/*
    mkdir -p models/include
    cp -r "$BENCHMARK_SRC/models/." models/
    echo "  OK models: World, Fortune"

    # orm: world_base, fortune_base
    rm -rf orm/*
    cp -r "$BENCHMARK_SRC/orm/." orm/
    echo "  OK orm: world_base, fortune_base"

    # libs/types: techempower_json
    rm -f libs/types/techempower_json.h libs/types/techempower_json_jsonreflect.cpp
    cp "$BENCHMARK_SRC/libs/types/techempower_json.h" libs/types/
    cp "$BENCHMARK_SRC/libs/types/techempower_json_jsonreflect.cpp" libs/types/
    echo "  OK libs/types: techempower_json"

    # view: fortunes.html
    rm -rf view/*
    mkdir -p view/techempower
    cp "$BENCHMARK_SRC/view/techempower/fortunes.html" view/techempower/
    echo "  OK view: fortunes.html"

    # viewsrc: viewsrc.h, regviewmethod.hpp, fortunes.cpp
    rm -rf viewsrc/*
    cp -r "$BENCHMARK_SRC/viewsrc/." viewsrc/
    echo "  OK viewsrc: viewsrc.h / regviewmethod.hpp / fortunes.cpp"

    # common registration files: techempower versions of autocontrolmethod / json_reflect_headers
    cp "$BENCHMARK_SRC/common/autocontrolmethod.hpp" common/
    cp "$BENCHMARK_SRC/common/json_reflect_headers.h" common/
    echo "  OK common: autocontrolmethod.hpp / json_reflect_headers.h"

    # conf: server.conf (port 8888), orm.conf (tfb-database)
    cp "$BENCHMARK_SRC/conf/server.conf" conf/
    cp "$BENCHMARK_SRC/conf/orm.conf" conf/
    echo "  OK conf: server.conf (8888) / orm.conf (tfb-database)"
fi

# ============================================================
# Done
# ============================================================
echo ""
echo "=============================================="
echo " Cleanup complete!"
echo "=============================================="
echo ""
if [ "$BENCHMARK_MODE" -eq 1 ]; then
    echo "Mode: TechEmpower Benchmark"
    echo "Entry: startup/main_docker.cpp (CMakeLists.txt default ENABLE_BENCHMARK=ON)"
    echo "Port: 8888"
    echo ""
    echo "Build:"
    echo "  cmake . -B build -DCMAKE_BUILD_TYPE=Release"
    echo "  cmake --build build -j\$(nproc)"
    echo ""
    echo "Benchmark endpoints: /json /plaintext /db /queries /fortunes /updates"
else
    echo "Mode: Clean scaffold (framework core + testhello)"
    echo ""
    echo "Build:"
    echo "  cd build && cmake .. && make -j\$(sysctl -n hw.ncpu)"
    echo ""
    echo "Start and visit http://127.0.0.1/hello to verify"
fi
echo ""
