cmake_minimum_required(VERSION 3.20)

# ---- Policies ----
cmake_policy(SET CMP0048 NEW)
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()

# ---- Project ----
project(Paozhu_web_framework
  VERSION 1.0.0
  LANGUAGES CXX
)

# ---- 设置默认构建类型为 Debug ----
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
  message(STATUS "No build type specified, defaulting to Debug")
endif()

# ---- Options ----
option(ENABLE_VCPKG "Use vcpkg toolchain" OFF)
option(ENABLE_BOOST "Enable Boost support" OFF)
option(ENABLE_GD "Enable GD graphics and QR" OFF)
option(ENABLE_BENCHMARK "Enable benchmarks" OFF)

# ---- Vcpkg toolchain ----
if(ENABLE_VCPKG)
  if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
      set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
          CACHE STRING "Vcpkg toolchain file")
    else()
      message(FATAL_ERROR "ENABLE_VCPKG is ON but VCPKG_ROOT environment variable is not set")
    endif()
  endif()

  macro(auto_enable_vcpkg_feature opt feature)
    if(${opt})
      if(NOT ";${VCPKG_MANIFEST_FEATURES};" MATCHES ";${feature};")
        message(STATUS "Auto append vcpkg feature: ${feature}")
        list(APPEND VCPKG_MANIFEST_FEATURES ${feature})
      endif()
    endif()
  endmacro()

  auto_enable_vcpkg_feature(ENABLE_GD "gd")
  auto_enable_vcpkg_feature(ENABLE_BOOST "boost")
endif()

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Common compiler options (interface library) ----
add_library(project_warnings INTERFACE)
if(MSVC)
  target_compile_options(project_warnings INTERFACE /W4 /utf-8 /EHsc)
else()
  target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-gnu-anonymous-struct>
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-nested-anon-types>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-misleading-indentation>
    $<$<CXX_COMPILER_ID:GNU>:-flarge-source-files>
  )
endif()

# ---- Output directories ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/temp")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/log")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cache")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/orm")

# ---- Export compile commands ----
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Helper: recursive include directories (kept for compatibility) ----
# 用于系统/第三方库目录（忽略内部警告）
function(include_system_dirs_recursively root_dir)
  if(IS_DIRECTORY ${root_dir})
    message(STATUS "include dir (system): ${root_dir}")
    include_directories(SYSTEM ${root_dir})
  endif()
  file(GLOB_RECURSE subdirs LIST_DIRECTORIES true RELATIVE ${root_dir} ${root_dir}/*)
  foreach(sub IN LISTS subdirs)
    if(IS_DIRECTORY ${root_dir}/${sub})
      include_system_dirs_recursively(${root_dir}/${sub})
    endif()
  endforeach()
endfunction()

# 用于项目业务代码（保持严格警告）
function(include_project_dirs_recursively root_dir)
  if(IS_DIRECTORY ${root_dir})
    message(STATUS "include dir (project): ${root_dir}")
    include_directories(${root_dir})   # 不加 SYSTEM
  endif()
  file(GLOB_RECURSE subdirs LIST_DIRECTORIES true RELATIVE ${root_dir} ${root_dir}/*)
  foreach(sub IN LISTS subdirs)
    if(IS_DIRECTORY ${root_dir}/${sub})
      include_project_dirs_recursively(${root_dir}/${sub})
    endif()
  endforeach()
endfunction()

# ---- Source files (with CONFIGURE_DEPENDS for automatic update) ----
file(GLOB_RECURSE orm_list        CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/orm/*.cpp")
file(GLOB_RECURSE source_list     CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/models/*.cpp")
file(GLOB_RECURSE framework_cpps  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/*.cpp")
file(GLOB_RECURSE common_list     CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp")
file(GLOB_RECURSE controller_list CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/controller/src/*.cpp")
file(GLOB_RECURSE viewsrc_list    CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/viewsrc/view/*.cpp")
file(GLOB_RECURSE reflect_list    CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/libs/*.cpp")
file(GLOB_RECURSE src_list        CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Auto-generate missing header stubs for controllers
foreach(cppfile IN LISTS controller_list)
  string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/controller/src/" "" rel_path "${cppfile}")
  string(FIND "${rel_path}" "/" pos)
  if(pos GREATER_EQUAL 0)
    string(REGEX MATCHALL "([A-Za-z0-9._-]+)/" dirs "${rel_path}")
    set(fullpath "${CMAKE_CURRENT_SOURCE_DIR}/controller/include/")
    foreach(dir IN LISTS dirs)
      string(REPLACE "/" "" dirname "${dir}")
      string(APPEND fullpath "${dirname}")
      if(NOT IS_DIRECTORY "${fullpath}")
        file(MAKE_DIRECTORY "${fullpath}")
      endif()
      string(APPEND fullpath "/")
    endforeach()
  endif()
  string(REGEX REPLACE ".cpp$" "" basename "${rel_path}")
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/controller/include/${basename}.h")
    file(TOUCH "${CMAKE_CURRENT_SOURCE_DIR}/controller/include/${basename}.h")
  endif()
endforeach()


# CLI tool
add_executable(paozhu_cli
  "${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli/http_cli.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/src/mysql_conn.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/src/mysql_conn_pool.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/src/clientdatacache.cpp"
)

# Main target
set(paozhu_srcs
  ${common_list}
  ${viewsrc_list}
  ${framework_cpps}
  ${orm_list}
  ${reflect_list}
  ${src_list}
  ${source_list}
  ${controller_list}
)

add_executable(paozhu
  $<IF:$<CONFIG:Release>,
    ${CMAKE_CURRENT_SOURCE_DIR}/startup/main_daemon.cpp,
    ${CMAKE_CURRENT_SOURCE_DIR}/startup/main_dev.cpp>
  ${paozhu_srcs}
)

# ---- Compile definitions & common options ----
target_compile_definitions(paozhu PRIVATE ASIO_STANDALONE)
if(ENABLE_BENCHMARK)
  target_compile_definitions(paozhu PRIVATE BENCHMARK)
endif()

target_link_libraries(paozhu PRIVATE project_warnings)
target_link_libraries(paozhu_cli PRIVATE project_warnings)

# Debug options
target_compile_options(paozhu PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:-g -fsanitize=address -fno-omit-frame-pointer>
  $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/fsanitize=address>
)
target_link_options(paozhu PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:-fsanitize=address>
)

target_compile_definitions(paozhu PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)

# ---- Include directories (recursive legacy includes) ----
include_system_dirs_recursively("${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver")
include_system_dirs_recursively("${CMAKE_CURRENT_SOURCE_DIR}/orm")
include_system_dirs_recursively("${CMAKE_CURRENT_SOURCE_DIR}/models")

include_project_dirs_recursively("${CMAKE_CURRENT_SOURCE_DIR}/controller/include")
include_project_dirs_recursively("${CMAKE_CURRENT_SOURCE_DIR}/libs")
include_project_dirs_recursively("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_project_dirs_recursively("${CMAKE_CURRENT_SOURCE_DIR}/common")

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bin/paozhu_pre)
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/bin/paozhu_pre)
endif()
set(PAOZHU_PRE paozhu_pre)
add_executable(${PAOZHU_PRE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli/autopickmethod.cpp ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/src/md5.cpp)


# Non-recursive include directories from original script
target_include_directories(paozhu PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/viewsrc/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/viewsrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/websockets"
  "${CMAKE_CURRENT_SOURCE_DIR}/sockets"
)

# Additional global includes from original script
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/controller")

# ===== 代码生成：paozhu_pre（每次构建都运行） =====
add_custom_target(paozhu_codegen ALL
    COMMAND $<TARGET_FILE:paozhu_pre> "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS paozhu_pre
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Running paozhu_pre to generate controller stubs..."
)

# ===== 让 paozhu 主目标等待代码生成完毕 =====
add_dependencies(paozhu paozhu_codegen)

# ---- Platform specific settings ----
if(WIN32)
  target_compile_definitions(paozhu PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0601)
  target_link_libraries(paozhu PRIVATE ws2_32 psapi)
  target_link_libraries(paozhu_cli PRIVATE ws2_32)
endif()

# ---- ASAN runtime DLL auto-copy for MSVC Debug (64-bit only) ----
if(MSVC AND WIN32)
  # Get the directory of cl.exe (compiler), ASAN DLL typically lives next to it
  get_filename_component(CL_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

  # Try to find the ASAN dynamic runtime (64-bit)
  find_file(ASAN_DLL_SOURCE
    NAMES clang_rt.asan_dynamic-x86_64.dll
    PATHS "${CL_DIR}"
    NO_DEFAULT_PATH
  )

  if(NOT ASAN_DLL_SOURCE)
    set(ASAN_DLL_SOURCE "" CACHE FILEPATH "Full path to clang_rt.asan_dynamic-x86_64.dll (set manually if needed)")
    if(NOT ASAN_DLL_SOURCE)
      message(WARNING
        "ASAN runtime DLL not found automatically.\n"
        "  Please locate clang_rt.asan_dynamic-x86_64.dll (typically in your Visual Studio installation,\n"
        "  e.g., VC/Tools/MSVC/<version>/bin/Hostx64/x64/) and set ASAN_DLL_SOURCE to its full path:\n"
        "    cmake -DASAN_DLL_SOURCE=\"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.40.33807/bin/Hostx64/x64/clang_rt.asan_dynamic-x86_64.dll\" ..\n"
        "  The DLL will then be automatically copied to the executable directory (e.g., bin/Debug/)."
      )
    endif()
  endif()

  # Only copy in Debug mode (ASAN is usually enabled there)
  if(ASAN_DLL_SOURCE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET paozhu POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${ASAN_DLL_SOURCE}"
              "$<TARGET_FILE_DIR:paozhu>"
      COMMENT "Copying ASAN runtime DLL to output directory"
    )
  endif()
endif()

# ---- Dependency handling ----
if(ENABLE_VCPKG)
  # Using vcpkg
  target_compile_definitions(paozhu PRIVATE ENABLE_VCPKG)
  find_package(asio CONFIG REQUIRED)
  target_link_libraries(paozhu PRIVATE asio::asio)
  target_link_libraries(paozhu_cli PRIVATE asio::asio)

  find_package(OpenSSL REQUIRED)
  target_link_libraries(paozhu PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  target_link_libraries(paozhu_cli PRIVATE OpenSSL::SSL OpenSSL::Crypto)

  find_package(ZLIB REQUIRED)
  target_link_libraries(paozhu PRIVATE ZLIB::ZLIB)

  find_package(unofficial-brotli CONFIG REQUIRED)
  target_link_libraries(paozhu PRIVATE unofficial::brotli::brotlidec unofficial::brotli::brotlienc)

  if(ENABLE_BOOST)
    target_compile_definitions(paozhu PRIVATE ENABLE_BOOST)
    set(Boost_NO_WARN_NEW_VERSIONS ON)
    find_package(Boost REQUIRED COMPONENTS filesystem coroutine)
    target_link_libraries(paozhu PRIVATE Boost::boost Boost::filesystem Boost::coroutine)
  endif()

  if(ENABLE_GD)
    find_package(PkgConfig)
    pkg_check_modules(LIBGD REQUIRED IMPORTED_TARGET gdlib)
    target_link_libraries(paozhu PRIVATE PkgConfig::LIBGD)

    find_path(QRENCODE_INCLUDE_DIR NAMES qrencode.h)
    find_library(QRENCODE_LIBRARY_RELEASE qrencode)
    target_include_directories(paozhu PRIVATE ${QRENCODE_INCLUDE_DIR})
    target_link_libraries(paozhu PRIVATE ${QRENCODE_LIBRARY_RELEASE})

    find_package(PNG REQUIRED)
    target_link_libraries(paozhu PRIVATE PNG::PNG)

    find_package(Freetype REQUIRED)
    target_link_libraries(paozhu PRIVATE Freetype::Freetype)

    if(WIN32)
      # Copy qrencode.dll to bin
      if(MSVC)
        set(qrencode_dll "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/qrencode.dll")
      elseif(MINGW)
        set(qrencode_dll "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/libqrencode.dll")
      endif()
      add_custom_command(TARGET paozhu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${qrencode_dll}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
      )
    endif()
  endif()
else()
  # Not using vcpkg: system libraries
  # On Windows, we highly recommend using vcpkg; exit with a helpful message if not.
  if(WIN32)
    message(FATAL_ERROR
      "On Windows, ENABLE_VCPKG must be ON. "
      "Please install vcpkg and set VCPKG_ROOT, or enable ENABLE_VCPKG."
    )
  endif()

  # ASIO
  add_library(asio INTERFACE)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
  if(APPLE)
    find_path(ASIO_INCLUDE_DIR asio.hpp
      PATHS /usr/local/opt/asio/include /usr/local/include /opt/homebrew/opt/asio
            "${CMAKE_CURRENT_SOURCE_DIR}/asio"
    )
  else()
    find_path(ASIO_INCLUDE_DIR asio.hpp
      PATHS "${CMAKE_CURRENT_SOURCE_DIR}/asio" /usr/include
    )
  endif()
  if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "Standalone Asio not found")
  endif()
  target_include_directories(asio INTERFACE ${ASIO_INCLUDE_DIR})
  target_link_libraries(paozhu PRIVATE asio)
  target_link_libraries(paozhu_cli PRIVATE asio)

  # OpenSSL
  find_package(OpenSSL REQUIRED)
  target_link_libraries(paozhu PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  target_link_libraries(paozhu_cli PRIVATE OpenSSL::SSL OpenSSL::Crypto)

  # ZLIB
  find_package(ZLIB REQUIRED)
  target_link_libraries(paozhu PRIVATE ZLIB::ZLIB)

  # Boost
  if(ENABLE_BOOST)
    target_compile_definitions(paozhu PRIVATE ENABLE_BOOST)
    find_package(Boost REQUIRED COMPONENTS system filesystem)
    target_link_libraries(paozhu PRIVATE Boost::boost Boost::system Boost::filesystem)
  endif()

  # GD, qrencode, PNG, Freetype
  if(ENABLE_GD)
    target_compile_definitions(paozhu PRIVATE ENABLE_GD)

    # ---- GD (library + header) ----
    find_library(GD_LIBRARY gd
      PATHS /usr/local/opt/gd/lib /opt/homebrew/opt/gd/lib
            /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib
    )
    if(NOT GD_LIBRARY)
      message(FATAL_ERROR "GD library not found")
    endif()
    find_path(GD_INCLUDE_DIR gd.h
      PATHS /usr/local/opt/gd/include /opt/homebrew/opt/gd/include
            /usr/include /usr/local/include
    )
    if(NOT GD_INCLUDE_DIR)
      message(FATAL_ERROR "GD header not found")
    endif()
    target_link_libraries(paozhu PRIVATE ${GD_LIBRARY})
    target_include_directories(paozhu PRIVATE ${GD_INCLUDE_DIR})
    message(STATUS "Found GD: ${GD_LIBRARY} (include: ${GD_INCLUDE_DIR})")

    # ---- qrencode (library + header) ----
    find_library(QRENCODE_LIBRARY qrencode
      PATHS /usr/local/opt/qrencode/lib /opt/homebrew/opt/qrencode/lib
            /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib
    )
    if(NOT QRENCODE_LIBRARY)
      message(FATAL_ERROR "qrencode library not found")
    endif()
    find_path(QRENCODE_INCLUDE_DIR qrencode.h
      PATHS /usr/local/opt/qrencode/include /opt/homebrew/opt/qrencode/include
            /usr/include /usr/local/include
    )
    if(NOT QRENCODE_INCLUDE_DIR)
      message(FATAL_ERROR "qrencode header not found")
    endif()
    target_link_libraries(paozhu PRIVATE ${QRENCODE_LIBRARY})
    target_include_directories(paozhu PRIVATE ${QRENCODE_INCLUDE_DIR})
    message(STATUS "Found qrencode: ${QRENCODE_LIBRARY} (include: ${QRENCODE_INCLUDE_DIR})")

    # ---- PNG and Freetype via CMake modules ----
    find_package(PNG REQUIRED)
    target_link_libraries(paozhu PRIVATE PNG::PNG)

    find_package(Freetype REQUIRED)
    target_link_libraries(paozhu PRIVATE Freetype::Freetype)
  endif()

  # Brotli
  find_library(BROTLI_ENC brotlienc
    PATHS /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib
          /usr/local/opt/brotli/lib /opt/homebrew/opt/brotli/lib
  )
  find_library(BROTLI_DEC brotlidec
    PATHS /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib
          /usr/local/opt/brotli/lib /opt/homebrew/opt/brotli/lib
  )
  if(NOT BROTLI_ENC OR NOT BROTLI_DEC)
    message(FATAL_ERROR "Brotli libraries not found")
  endif()
  message(STATUS "Found Brotli encoder: ${BROTLI_ENC}")
  message(STATUS "Found Brotli decoder: ${BROTLI_DEC}")
  target_link_libraries(paozhu PRIVATE ${BROTLI_ENC} ${BROTLI_DEC})
  if(APPLE)
    foreach(prefix /usr/local/opt /opt/homebrew/opt)
      if(IS_DIRECTORY "${prefix}/brotli/include")
        target_include_directories(paozhu PRIVATE "${prefix}/brotli/include")
      endif()
    endforeach()
  endif()
endif()

# ---- Final common libraries ----
if(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(paozhu PRIVATE Threads::Threads)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(paozhu PRIVATE m dl)
  endif()
  target_link_libraries(paozhu_cli PRIVATE Threads::Threads)
endif()

# ---- Copy config files (Unix only) ----
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND UNIX)
  file(MAKE_DIRECTORY /usr/local/etc/paozhu)
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/conf/" DESTINATION /usr/local/etc/paozhu/)
  message(STATUS "Configuration files copied to /usr/local/etc/paozhu")
endif()
