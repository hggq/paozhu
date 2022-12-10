CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(Paozhu_web_framework)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
set(CMAKE_CXX_FLAGS "  -pthread   -DASIO_STANDALONE -DDEBUG -I/usr/local/include  -I/usr/include -I/usr/include/mysql-cppconn-8 " )
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(USE_STANDALONE_ASIO ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "SOURCE dir ${CMAKE_CURRENT_SOURCE_DIR}")


set(CMAKE_BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_CACHEFILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/controller)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/viewsrc/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/websockets/include)

file(GLOB_RECURSE orm_list ${CMAKE_CURRENT_SOURCE_DIR}/orm/*.cpp)
file(GLOB_RECURSE source_list ${CMAKE_CURRENT_SOURCE_DIR}/models/*.cpp)
file(GLOB_RECURSE FRAMEWORK_CPP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/*.cpp)
file(GLOB_RECURSE common_list ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp)
file(GLOB_RECURSE controller_list ${CMAKE_CURRENT_SOURCE_DIR}/controller/src/*.cpp)
file(GLOB_RECURSE viewsrc_list ${CMAKE_CURRENT_SOURCE_DIR}/viewsrc/view/*.cpp)


function(include_sub_directories_recursively root_dir)
    if (IS_DIRECTORY ${root_dir})               # 当前路径是一个目录吗，是的话就加入到包含目录
        #        if (${root_dir} MATCHES "include")
        message("include dir: " ${root_dir})
        include_directories(${root_dir})
        #        endif()
    endif()

    file(GLOB ALL_SUB RELATIVE ${root_dir} ${root_dir}/*) # 获得当前目录下的所有文件，让如ALL_SUB列表中

    foreach(sub ${ALL_SUB})
        if (IS_DIRECTORY ${root_dir}/${sub})
            include_sub_directories_recursively(${root_dir}/${sub}) # 对子目录递归调用，包含
        endif()
    endforeach()
endfunction()

include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver)
include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR}/orm)
include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR}/models)
include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR}/controller/include)

 
add_executable(paozhu_cli ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli/http_cli.cpp)
add_executable(paozhu ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp ${common_list} ${viewsrc_list} ${FRAMEWORK_CPP_PATH} ${orm_list} ${source_list} ${controller_list})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)


if(USE_STANDALONE_ASIO)
if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(ASIO_PATH "/usr/local/opt/asio/include" "/usr/local/include")
else()
set(ASIO_PATH "/usr/include")
endif()
  
    target_compile_definitions(paozhu INTERFACE ASIO_STANDALONE)
    find_path(ASIO_PATH asio.hpp)
    message(state " Standalone Asio found: " ${ASIO_PATH})
    if(NOT ASIO_PATH)
        message(FATAL_ERROR "Standalone Asio not found")
    else()
        target_include_directories(paozhu INTERFACE ${ASIO_PATH})
    endif()

    include_directories(${ASIO_PATH})

endif()

find_package(Boost REQUIRED
             COMPONENTS system filesystem)
if(Boost_FOUND)
    include_directories("${Boost_INCLUDE_DIRS}/boost")

    MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
    MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARY_DIRS}")
    MESSAGE( STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}")
    link_directories(${Boost_LIBRARY_DIRS})
    target_link_libraries (paozhu ${Boost_LIBRARIES})
endif()        



find_package(OpenSSL REQUIRED)
 
if(OPENSSL_FOUND)
 
  message(STATUS "OPENSSL_VERSION = ${OPENSSL_VERSION}")
  message(STATUS "OPENSSL_SSL_LIBRARY = ${OPENSSL_SSL_LIBRARY}")
  message(STATUS "OPENSSL_CRYPTO_LIBRARY = ${OPENSSL_CRYPTO_LIBRARY}")
  message(STATUS "OPENSSL_INCLUDE_DIR = ${OPENSSL_INCLUDE_DIR}")  
  INCLUDE_DIRECTORIES("${OPENSSL_INCLUDE_DIR}")
  target_link_libraries (paozhu ${OPENSSL_SSL_LIBRARY})  
  target_link_libraries (paozhu ${OPENSSL_CRYPTO_LIBRARY})  
endif()


find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  message(STATUS "Zlib Found! ${ZLIB_LIBRARIES}")
  include_directories(${ZLIB_INCLUDE_DIR})
  set(zlib_library ${ZLIB_LIBRARIES})
  target_link_libraries(paozhu z)
endif()

find_path(MYSQL_ROOT_DIR mysql)
MESSAGE( STATUS "MYSQL_ROOT_DIR = ${MYSQL_ROOT_DIR} ")

find_path(MYSQL_CONCPP_DIR mysql-connector-c++)
MESSAGE( STATUS "MYSQL_CONCPP_DIR = ${MYSQL_CONCPP_DIR} ")

set(find_name mysqlcppconn8)
if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(find_dir "/usr/local/opt/mysql-connector-c++/lib")
else()
set(find_dir "/usr/lib/x86_64-linux-gnu")
endif()

MESSAGE( STATUS "mysqlcppconn8 = ${find_dir} ")
find_library(CONCPP_LIB
  NAMES mysqlcppconn8
  PATHS "${find_dir}"
  NO_DEFAULT_PATH
)
MESSAGE( STATUS "mysqlcppconn8 = ${CONCPP_LIB} ")
find_library(CONCPPSSL_LIB
  NAMES ssl
  PATHS "${find_dir}"
  NO_DEFAULT_PATH
)
MESSAGE( STATUS "mysqlcppconn8 = ${CONCPPSSL_LIB} ")
find_library(CONCPPCRYPTO_LIB
  NAMES crypto
  PATHS "${find_dir}"
  NO_DEFAULT_PATH
)
MESSAGE( STATUS "mysqlcppconn8 = ${CONCPPCRYPTO_LIB} ")

if(NOT CONCPP_LIB)
  message(FATAL_ERROR
    "Could not find mysql Connector/C++ libraries at: ${find_dir} , Please intall mysql Connector/C++, Visit  https://dev.mysql.com/downloads/connector/cpp/ "
  )
endif()
target_link_libraries(paozhu ${CONCPP_LIB})
target_link_libraries(paozhu_cli ${CONCPP_LIB})
message(STATUS "Using connector lib at: ${CONCPP_LIB}")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(GD_ROOT_DIR "/usr/local/opt/gd/lib")
else()
set(GD_ROOT_DIR "/usr/lib/x86_64-linux-gnu")
endif()


set(find_gdname gd)

find_library(GD_LIB_DIR
  NAMES ${find_gdname}
  PATHS "${GD_ROOT_DIR}"
  NO_DEFAULT_PATH
)

 
 
if(NOT GD_LIB_DIR)
message(FATAL_ERROR
"GD Graphics Library NOT FOUND! please install . "
)
endif()

message(STATUS "GD Graphics Library  at: ${GD_LIB_DIR}")



if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(QR_ROOT_DIR "/usr/local/opt/qrencode/lib")
else()
set(QR_ROOT_DIR "/usr/lib/x86_64-linux-gnu")
endif()



set(find_qrname qrencode)

find_library(QR_LIB_DIR
  NAMES ${find_qrname}
  PATHS "${QR_ROOT_DIR}"
  NO_DEFAULT_PATH
)

 
 
if(NOT QR_LIB_DIR)
message(FATAL_ERROR
"qrencode Library  NOT FOUND! please install . "
)
endif()

message(STATUS "qrencode at: ${QR_LIB_DIR}")

INCLUDE_DIRECTORIES("${GD_ROOT_DIR}/include")
INCLUDE_DIRECTORIES("${QR_ROOT_DIR}/include")
link_directories("${QR_ROOT_DIR}/lib")
link_directories("${GD_ROOT_DIR}/lib")
target_link_libraries(paozhu ${GD_LIB_DIR})
target_link_libraries(paozhu ${QR_LIB_DIR})

 
if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(PNG_ROOT_DIR "/usr/local/opt/libpng/lib")
else()
set(PNG_ROOT_DIR "/usr/lib/x86_64-linux-gnu")
endif()


find_library(PNG_LIB_DIR
  NAMES png
  PATHS "${PNG_ROOT_DIR}"
  NO_DEFAULT_PATH
)
target_link_libraries(paozhu ${PNG_LIB_DIR})

if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(FREETYPE_ROOT_DIR "/usr/local/opt/freetype/lib")
else()
set(FREETYPE_ROOT_DIR "/usr/lib/x86_64-linux-gnu")
endif()

find_library(FREETYPE_LIB_DIR
  NAMES freetype
  PATHS "${FREETYPE_ROOT_DIR}"
  NO_DEFAULT_PATH
)
target_link_libraries(paozhu ${FREETYPE_LIB_DIR})


if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(BROTLI_ROOT_DIR "/usr/local/opt/brotli/lib")
else()
set(BROTLI_ROOT_DIR "/usr/lib/x86_64-linux-gnu")
endif()

#find_package(Brotli COMPONENTS encoder decoder common REQUIRED)

set(find_brname brotlienc)
set(find_brdename brotlidec)
find_library(BR_LIB_DIR
  NAMES ${find_brname}
  PATHS "${BROTLI_ROOT_DIR}"
  NO_DEFAULT_PATH
)
find_library(BRDEC_LIB_DIR
  NAMES ${find_brdename}
  PATHS "${BROTLI_ROOT_DIR}"
  NO_DEFAULT_PATH
)
INCLUDE_DIRECTORIES("${BROTLI_ROOT_DIR}/include")
link_directories("${BROTLI_ROOT_DIR}/lib")

if(NOT BR_LIB_DIR)
message(FATAL_ERROR
"Brotli Library  NOT FOUND! please install . "
)
endif()

message(STATUS "Brotli at: ${BR_LIB_DIR}")
target_link_libraries(paozhu ${BR_LIB_DIR})
 

if(NOT BRDEC_LIB_DIR)
message(FATAL_ERROR
"Brotli Library  NOT FOUND! please install . "
)
endif()

message(STATUS "Brotli at: ${BRDEC_LIB_DIR}")
target_link_libraries(paozhu ${BRDEC_LIB_DIR})


target_link_libraries(paozhu  m  dl)



