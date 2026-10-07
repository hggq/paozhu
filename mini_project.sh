#!/usr/bin/env bash
#
# mini_project.sh - Trim the full Paozhu project into a clean scaffold
# Follows "Clean Project Initialization" five-step flow
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

# Normal mode: 5 steps; benchmark mode appends the techempower overlay -> 6.
TOTAL_STEPS=5
if [ "$BENCHMARK_MODE" -eq 1 ]; then
    TOTAL_STEPS=6
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
#   Keep the includes that define the registration table types and the
#   type-def required includes, empty out all registration function bodies
#   (signatures kept)
# ============================================================
echo ""
echo "[Step 1/${TOTAL_STEPS}] Cleaning common/ registration files..."

cat > common/autocontrolmethod.hpp << 'EOF'
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h"

namespace http
{
    // The build regenerates this file from controller/src (the paozhu_codegen target
    // runs paozhu_pre before compiling the server), so an empty body is enough to keep
    // an unbuilt tree compiling: server.cpp calls _initauto_all_httputils() with no
    // arguments. The old per-map _initauto_*_httpmethodregto() set is gone.
    void _initauto_all_httputils()
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
// No-arg since the v6 router rewrite: registrations go through the reg_raw /
// REG_SYNC_SYNC macros straight into the router, and server.cpp calls it with no
// arguments. The old std::map<std::string, regmethold_t> & parameter is gone --
// keeping that spelling here breaks the build with "too few arguments to function".
inline void _inithttpmethodregto()
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
// Same rewrite as reghttpmethod.hpp: server.cpp calls this with no arguments.
inline void _inithttpmethodregto_pre()
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

# mqtt_method_reg.hpp — inbound MQTT handlers. Included UNCONDITIONALLY by
# vendor/httpserver/src/mqtt_reg.cpp and server.cpp, so the signature and the
# table-type include must stay; only the body is emptied.
cat > common/mqtt_method_reg.hpp << 'EOF'
#pragma once
#include "mqtt_reg.h"
#include "mqtt_session.h"

namespace http
{
// Business mqtt_api subclasses live in mqtt/ and are included + emplaced here
// (reg_key is the Client ID prefix).
inline void _initmqttmethodregto(MQTT_REG &reg)
{
}

}// namespace http
EOF

# The four resident outbound-client registries below are each double-guarded:
# server.cpp includes them inside #ifdef ENABLE_*_CLIENT, and the file body is
# wrapped in the same #ifdef. Keep the guard + the table-type include so the
# name stays available in both build tiers; only the emplace lines are removed.

# redis_regmethod.hpp — keep redis_subpub_reg.h (defines REDIS_SUBPUB_REG)
cat > common/redis_regmethod.hpp << 'EOF'
#pragma once

#ifdef ENABLE_REDIS_CLIENT
// Business pz::redis::redis_subpub_client subclasses live in redis/ and are
// included + emplaced here; server.cpp spawns one resident coroutine per entry.
#include "redis_subpub_reg.h"

namespace http
{

inline void _initredissubpubregto(pz::redis::REDIS_SUBPUB_REG &reg)
{
}

} // namespace http

#endif // ENABLE_REDIS_CLIENT
EOF

# ws_client_regmethod.hpp — keep ws_subpub_reg.h (defines WS_SUBPUB_REG)
cat > common/ws_client_regmethod.hpp << 'EOF'
#pragma once

#ifdef ENABLE_WEBSOCKETS_CLIENT
// Business http::ws_subpub_client subclasses live in websockets/ and are
// included + emplaced here; server.cpp spawns one resident coroutine per entry.
#include "ws_subpub_reg.h"

namespace http
{

inline void _initwssubpubregto(http::WS_SUBPUB_REG &reg)
{
}

} // namespace http

#endif // ENABLE_WEBSOCKETS_CLIENT
EOF

# sock_client_regmethod.hpp — keep sock_subpub_reg.h (defines SOCK_SUBPUB_REG)
cat > common/sock_client_regmethod.hpp << 'EOF'
#pragma once

#ifdef ENABLE_SOCKETS_CLIENT
// Business http::sock_subpub_client subclasses live in sockets/ and are
// included + emplaced here; server.cpp spawns one resident coroutine per entry.
#include "sock_subpub_reg.h"

namespace http
{

inline void _initsockssubpubregto(http::SOCK_SUBPUB_REG &reg)
{
}

} // namespace http

#endif // ENABLE_SOCKETS_CLIENT
EOF

# mqtt_client_regmethod.hpp — keep mqtt_subpub_reg.h (defines MQTT_SUBPUB_REG)
cat > common/mqtt_client_regmethod.hpp << 'EOF'
#pragma once

#ifdef ENABLE_MQTT_CLIENT
// Business http::mqtt_subpub_client subclasses live in mqtt/ and are included +
// emplaced here; server.cpp spawns one resident coroutine per entry (outbound
// to an external broker).
#include "mqtt_subpub_reg.h"

namespace http
{

inline void _initmqttsubpubregto(http::MQTT_SUBPUB_REG &reg)
{
}

} // namespace http

#endif // ENABLE_MQTT_CLIENT
EOF

echo "  OK cleaned 11 registration files:"
echo "    http:    autocontrolmethod / autorestfulpaths / reghttpmethod /"
echo "             reghttpmethod_pre"
echo "    inbound: sockets_method_reg / websockets_method_reg / mqtt_method_reg"
echo "    resident: redis_regmethod / ws_client_regmethod / sock_client_regmethod /"
echo "              mqtt_client_regmethod"

# ============================================================
# Step 2 — Remove resident-client and business demo code
#   redis/  mqtt/  sockets/  websockets/  are header-only demo clients
#   (echo_* / my_test_* / *_websockets.hpp). Their only consumers are the
#   registration files emptied in Step 1, and CMake adds them as include
#   directories only — nothing is compiled out of them — so the content goes
#   and the directories stay for business clients.
#
#   libs/webpay/ is the order/pay-channel layer bound to the [cms] tables; it
#   is globbed into the build by file(GLOB_RECURSE reflect_list ... libs/*.cpp)
#   (CMake) and add_files("libs/**.cpp") (xmake), so leaving it in a scaffold
#   that has no generated ORM makes the build fail on the first business model.
#   The other libs/ directories (img / markdown / pinyin / ipdata / types /
#   department) are framework utilities with no ORM dependency and are kept.
# ============================================================
echo ""
echo "[Step 2/${TOTAL_STEPS}] Removing demo code (redis/ mqtt/ sockets/ websockets/ libs/webpay/)..."

for demo_dir in redis mqtt sockets websockets; do
    rm -rf "$demo_dir"/* 2>/dev/null || true
    mkdir -p "$demo_dir"
done

rm -rf libs/webpay/* 2>/dev/null || true
mkdir -p libs/webpay

echo "  OK redis/ mqtt/ sockets/ websockets/ libs/webpay/ emptied (directories kept)"

# ============================================================
# Step 3 — Remove ORM layer
#   models/*  schema/*  orm/*  (directories themselves kept)
#
#   One file is kept: an empty orm/orm.h. Business code does
#   #include "orm.h"  (the ORM unified entry) and CMake only puts orm/ on the
#   include path because it exists, so deleting it too makes every remaining
#   consumer — including code the scaffold author writes later — fail with
#   "orm.h: 没有那个文件或目录" instead of compiling until a model is actually
#   used. `paozhu_cli orm <tag>` overwrites this placeholder with the generated
#   per-tag model includes.
# ============================================================
echo ""
echo "[Step 3/${TOTAL_STEPS}] Removing ORM layer (models/ schema/ orm/)..."

rm -rf models/*
rm -rf schema/*
rm -rf orm/*
mkdir -p orm

cat > orm/orm.h << 'EOF'
// ORM unified entry — placeholder kept by mini_project.sh.
// Run ./bin/paozhu_cli orm <tag> to generate the models, this file is then
// rewritten with one #include per generated model of every tag.
EOF

echo "  OK models/ schema/ orm/ emptied (empty orm/orm.h kept)"

# ============================================================
# Step 4 — Clean view files (keep registration skeleton)
#   4.1 Remove viewsrc/view/ and view/
#   4.2 Empty the namespace view body inside viewsrc/include/viewsrc.h
#   4.3 Empty function bodies in viewsrc/include/regviewmethod.hpp
# ============================================================
echo ""
echo "[Step 4/${TOTAL_STEPS}] Cleaning view files..."

# 4.1 Remove view sources and templates
rm -rf viewsrc/view/
rm -rf view/

# 4.2 Empty the internal namespace in viewsrc.h
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

# 4.3 Empty the function body in regviewmethod.hpp
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

# 4.4 Clean www/default, keep only index.html (content: Hello World! Paozhu)
rm -rf www/default/*
echo "Hello World! Paozhu" > www/default/index.html
echo "  OK www/default cleaned, only index.html kept"

# ============================================================
# Step 5 — Clean controller
#   Normal mode: keep testhello.cpp + serverwatch.cpp
#   Benchmark mode: delete all (techempower overlaid next)
#
#   serverwatch.cpp is kept in normal mode because router looks up the handler
#   frametasks_timeloop BY NAME (call_sync_regfun / find_sitecontent): without
#   that file no interval task is ever registered and the only trace is a
#   "frametasks_timeloop not registered" debug log. It also carries the
#   /paozhu_status and /paozhu_routes self-check pages.
# ============================================================
echo ""
if [ "$BENCHMARK_MODE" -eq 1 ]; then
    echo "[Step 5/${TOTAL_STEPS}] Cleaning controller (benchmark mode: delete all)..."
    rm -rf controller/include/*
    find controller/src -type f -delete
    find controller/src -mindepth 1 -type d -empty -delete 2>/dev/null || true
    echo "  OK controller emptied (techempower to be overlaid)"
else
    echo "[Step 5/${TOTAL_STEPS}] Cleaning controller (keep testhello.cpp + serverwatch.cpp)..."
    rm -rf controller/include/*
    find controller/src -type f ! -name 'testhello.cpp' ! -name 'serverwatch.cpp' -delete
    find controller/src -mindepth 1 -type d -empty -delete 2>/dev/null || true
    echo "  OK controller: testhello.cpp + serverwatch.cpp kept"
fi

# ============================================================
# Step 6 — Benchmark mode: overlay TechEmpower benchmark files
#   Copy business files from Benchmark/paozhu_benchmark/
#   Do NOT overwrite framework files (CMakeLists.txt / vendor / startup / etc.)
# ============================================================
if [ "$BENCHMARK_MODE" -eq 1 ]; then
    echo ""
    echo "[Step 6/${TOTAL_STEPS}] Overlaying TechEmpower benchmark files..."

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
    echo ""
    echo "Resident outbound clients: 0 registered."
    echo "  The client classes themselves are framework code and still build —"
    echo "  CMakeLists.txt defaults ENABLE_WEBSOCKETS/SOCKETS/MQTT_CLIENT to ON,"
    echo "  so startup prints \"[ws_subpub] init: registered 0 clients\" and the like."
    echo "  To add one: put the subclass in redis/ mqtt/ sockets/ websockets/ and"
    echo "  emplace it in the matching common/*_regmethod.hpp skeleton."
fi
echo ""
