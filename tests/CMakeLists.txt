# tests/CMakeLists.txt
# 由根 CMake 的 -DENABLE_TESTS=ON 开启后才会进入本目录
# Entered only when -DENABLE_TESTS=ON is passed to the root CMake.
#
# 约定：tests/ 下每个 *.cpp 自动生成同名可执行文件，新增文件无需改本文件。
# 特殊依赖按文件名识别：文件名含 sqlite 的需要 -DENABLE_SQLITE=ON。
# Convention: every tests/*.cpp becomes an executable of the same name; no edit needed
# for new files. Special deps are keyed by filename: "sqlite" needs -DENABLE_SQLITE=ON.

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

# 自动发现 tests/ 下所有 .cpp（CONFIGURE_DEPENDS：新增/删除文件自动重新配置）
# Auto-discover every .cpp here (CONFIGURE_DEPENDS picks up added/removed files).
file(GLOB test_cpp_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(test_src IN LISTS test_cpp_files)
  get_filename_component(test_name "${test_src}" NAME_WE)

  # 文件名含 sqlite 的用例依赖 ENABLE_SQLITE，关闭时跳过并提示
  # sqlite-named tests need ENABLE_SQLITE; skip with a notice when it is off.
  if(test_name MATCHES "sqlite" AND NOT ENABLE_SQLITE)
    message(STATUS "ENABLE_TESTS=ON: ${test_name} skipped (need -DENABLE_SQLITE=ON)")
    continue()
  endif()

  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name} PRIVATE ${paozhu_test_common_libs})

  # http2_ring 用例把发送环实现直接编进测试目标（server.cpp 不参与测试构建）
  # http2_ring tests compile the ring implementation in directly (server.cpp stays out).
  if(test_name MATCHES "http2_ring")
    target_sources(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/vendor/httpserver/src/http2_ring_queue.cpp")
  endif()
endforeach()
