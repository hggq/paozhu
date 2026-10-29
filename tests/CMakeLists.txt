# tests/CMakeLists.txt
# 由根 CMake 的 -DENABLE_TESTS=ON 开启后才会进入本目录
# Entered only when -DENABLE_TESTS=ON is passed to the root CMake.
#
# 约定：tests/*.cpp 与 tests/orm/*.cpp 自动生成同名可执行文件，新增文件无需改本文件。
# 特殊依赖按文件名识别：文件名含 sqlite 的需要 -DENABLE_SQLITE=ON。
# Convention: every tests/*.cpp and tests/orm/*.cpp becomes an executable of the same name;
# no edit needed for new files. Special deps are keyed by filename: "sqlite" needs -DENABLE_SQLITE=ON.
# 名字含 pool_thread 的是例外：它需要整个框架的链接面与宏集，暂时不生成目标（理由见下面循环里那段）。
# "pool_thread" is the exception: it needs the whole framework to link and to be compiled with, so no
# target is generated for it yet — see the note inside the loop below.
# tests/orm/ 的子目录（各轮探针自带构建脚本）不参与这里的自动发现。
# Sub-directories under tests/orm/ (probes with their own build scripts) stay out of discovery.

# 与 paozhu_cli 保持一致的头文件搜索路径
# Include paths mirror the paozhu_cli target.
set(TEST_INCLUDE_DIRS
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/vendor/httpserver/include"
  "${CMAKE_SOURCE_DIR}/include"
  "${CMAKE_SOURCE_DIR}/orm"
  "${CMAKE_SOURCE_DIR}/models"
)

# ORM 连接层抽成 OBJECT 库只编译一次：PGCONN_SRCS 由根 CMakeLists.txt 提供
# Compile the shared ORM connection layer once as an OBJECT library.
add_library(paozhu_test_orm OBJECT ${PGCONN_SRCS})
target_include_directories(paozhu_test_orm PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(paozhu_test_orm PUBLIC project_warnings)

# asio：非 vcpkg 模式下是根 CMake 创建的接口库，vcpkg 模式下是 asio::asio
# asio: local interface target without vcpkg, imported asio::asio with vcpkg.
if(TARGET asio)
  target_link_libraries(paozhu_test_orm PUBLIC asio)
elseif(TARGET asio::asio)
  target_link_libraries(paozhu_test_orm PUBLIC asio::asio)
endif()

target_link_libraries(paozhu_test_orm PUBLIC OpenSSL::SSL OpenSSL::Crypto)

if(UNIX)
  target_link_libraries(paozhu_test_orm PUBLIC Threads::Threads)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(paozhu_test_orm PUBLIC m dl)
  endif()
elseif(WIN32)
  target_compile_definitions(paozhu_test_orm PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0601)
  target_link_libraries(paozhu_test_orm PUBLIC ws2_32)
endif()

# SQLite 连接层（条件创建）：sqlite_conn.cpp 在开关外只是 stub，
# 且 stub 必须在 ENABLE_SQLITE 宏下整体排除，否则与真实实现重复符号
# SQLite layer (conditional): without the switch sqlite_conn.cpp is a stub, which must be
# compiled out under ENABLE_SQLITE or it clashes with the real implementation.
if(ENABLE_SQLITE)
  target_compile_definitions(paozhu_test_orm PUBLIC ENABLE_SQLITE)

  add_library(paozhu_test_sqlite OBJECT ${SQLITE_CONN_SRCS})
  target_compile_definitions(paozhu_test_sqlite PUBLIC ENABLE_SQLITE)
  target_link_libraries(paozhu_test_sqlite PUBLIC paozhu_test_orm)
  if(TARGET SQLite::SQLite3)
    target_link_libraries(paozhu_test_sqlite PUBLIC SQLite::SQLite3)
  else()
    target_link_libraries(paozhu_test_sqlite PUBLIC ${SQLITE3_LIBRARY})
    target_include_directories(paozhu_test_sqlite PUBLIC ${SQLITE3_INCLUDE_DIR})
  endif()
endif()

# 所有测试目标共用的库：开启 ENABLE_SQLITE 后 stub 被整体排除，
# 即使非 sqlite 用例也必须带上真实 sqlite 对象库，否则缺符号
# Libs every test links: with ENABLE_SQLITE the stub is compiled out, so even non-sqlite
# tests must carry the real sqlite objects or they fail to link.
set(paozhu_test_common_libs paozhu_test_orm)
if(ENABLE_SQLITE)
  list(APPEND paozhu_test_common_libs paozhu_test_sqlite)
endif()

# 自动发现 tests/ 与 tests/orm/ 下所有 .cpp（CONFIGURE_DEPENDS：新增/删除文件自动重新配置）
# 只收这两层的直属文件，orm/ 里的探针子目录由各自脚本构建，不进 CMake 目标
# Auto-discover .cpp in tests/ and tests/orm/ (CONFIGURE_DEPENDS picks up added/removed files).
# Only these two levels: probe sub-directories under tests/orm/ are built by their own scripts.
file(GLOB test_cpp_files CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/orm/*.cpp"
)

foreach(test_src IN LISTS test_cpp_files)
  get_filename_component(test_name "${test_src}" NAME_WE)

  # 文件名含 sqlite 的用例依赖 ENABLE_SQLITE，关闭时跳过并提示
  # sqlite-named tests need ENABLE_SQLITE; skip with a notice when it is off.
  if(test_name MATCHES "sqlite" AND NOT ENABLE_SQLITE)
    message(STATUS "ENABLE_TESTS=ON: ${test_name} skipped (need -DENABLE_SQLITE=ON)")
    continue()
  endif()

  # 线程切换用例要链接整个框架：它调 get_server_app()（定义在 server.cpp），而 server.cpp 又调
  # 控制器/视图的注册入口，闭合面不是往目标里加两三个 .cpp 能补上的；而且它必须和框架用同一套
  # ENABLE_* 宏编译——httpserver 与 ThreadPool 里有这些宏保护的成员，宏集不一致会读到错位的成员，
  # 用例假红。测试目标只带 ORM 连接层，所以先不进自动发现；等框架源码收成一份共享对象库再挂回来。
  # The pool-thread test links get_server_app() (server.cpp), which in turn pulls the controller and
  # view registration entry points — the closure is the whole app, not a few extra sources. It also
  # has to be compiled with the same ENABLE_* defines the framework uses: httpserver and ThreadPool
  # keep members behind those guards, so a mismatched define set reads misaligned members and the
  # cases fail for the wrong reason. Kept out of discovery until the framework ships a shared object
  # library the tests can link.
  if(test_name MATCHES "pool_thread")
    message(STATUS "ENABLE_TESTS=ON: ${test_name} skipped (needs the whole framework + its ENABLE_* defines)")
    continue()
  endif()

  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name} PRIVATE ${paozhu_test_common_libs})

  # http2_ring 用例把发送环实现直接编进测试目标（server.cpp 不参与测试构建）
  # http2_ring tests compile the ring implementation in directly (server.cpp stays out).
  if(test_name MATCHES "http2_ring")
    target_sources(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/http2_ring_queue.cpp")
  endif()

  # jwt 用例把 http::jwt 实现编进测试目标：jwt.cpp 不在 paozhu_test_orm 的源集里，
  # 不加这一段则 encode/verify/decode_without_verify 三个符号找不到
  # jwt tests compile the http::jwt implementation in; without it the three jwt symbols stay undefined.
  if(test_name MATCHES "jwt")
    target_sources(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/jwt.cpp")
  endif()

  # 文件名安全单测：把 func.cpp + datetime.cpp 编进去，覆盖 safe_filename / str2safefile / get_filename
  # Filename-safety tests: compile func.cpp + datetime.cpp (rand_string is in datetime.cpp,
  # pulled in by temp_name_uniq_tail inside func.cpp).
  if(test_name MATCHES "filename_safety")
    target_sources(${test_name} PRIVATE
      "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/func.cpp"
      "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/datetime.cpp"
    )
  endif()

  # POST 表单 obj_val 探针：obj_val 实现就在 request.cpp，而 request.cpp 已经编进
  # paozhu_test_orm（PGCONN_SRCS 的一部分），这里不要再重复编一次，否则会与
  # paozhu_test_orm 里的 request.cpp.o 产生重复符号（194 个 duplicate symbols）。
  # The POST-form probe needs obj_val, which already lives in request.cpp — and that
  # file is already compiled into paozhu_test_orm, so adding it again here just
  # duplicates every obj_val symbol against paozhu_test_orm's request.cpp.o.

  # --- test_pzexcel_p2: needs pzexcel + pzzip sources and ENABLE_OFFICE + zlib ---
  if(test_name MATCHES "pzexcel_p2")
    target_compile_definitions(${test_name} PRIVATE ENABLE_OFFICE)
    target_include_directories(${test_name} PRIVATE
      "${CMAKE_SOURCE_DIR}/vendor/pzexcel/include"
      "${CMAKE_SOURCE_DIR}/vendor/pzzip/include"
    )
    target_sources(${test_name} PRIVATE
      "${CMAKE_SOURCE_DIR}/vendor/pzexcel/src/pzexcel.cpp"
      "${CMAKE_SOURCE_DIR}/vendor/pzzip/src/pzzip.cpp"
    )
    target_link_libraries(${test_name} PRIVATE ZLIB::ZLIB)
  endif()

  # --- test_pzzip: standalone pzzip unit test (sources + zlib) ---
  if(test_name MATCHES "pzzip")
    target_include_directories(${test_name} PRIVATE
      "${CMAKE_SOURCE_DIR}/vendor/pzzip/include"
    )
    target_sources(${test_name} PRIVATE
      "${CMAKE_SOURCE_DIR}/vendor/pzzip/src/pzzip.cpp"
    )
    target_link_libraries(${test_name} PRIVATE ZLIB::ZLIB)
  endif()
endforeach()

# ---------------------------------------------------------------------------
# oauth_store_1028 探针：位于 tests/orm/<子目录>，不在上面的自动发现 GLOB 内，
# 这里单独建目标并补上它依赖的业务源（store/config/jwt/func）。
# The oauth_store_1028 probe lives in a sub-directory outside the discovery GLOB;
# build it here with the extra business sources it depends on.
# ---------------------------------------------------------------------------
add_executable(oauth_store_probe "${CMAKE_CURRENT_SOURCE_DIR}/orm/oauth_store_1028/oauth_store_probe.cpp")
target_link_libraries(oauth_store_probe PRIVATE ${paozhu_test_common_libs})
target_sources(oauth_store_probe PRIVATE
  "${CMAKE_SOURCE_DIR}/libs/oauth/oauth_store.cpp"
  "${CMAKE_SOURCE_DIR}/libs/oauth/oauth_config.cpp"
  "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/jwt.cpp"
  "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/func.cpp"
  "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/datetime.cpp"
  "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/urlcode.cpp"
  "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/serverconfig.cpp"
  "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/server_localvar.cpp"
  "${CMAKE_SOURCE_DIR}/models/cms/OauthClient.cpp"
  "${CMAKE_SOURCE_DIR}/models/cms/OauthCode.cpp"
  "${CMAKE_SOURCE_DIR}/models/cms/OauthRefresh.cpp"
)
