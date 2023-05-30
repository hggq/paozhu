CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(Paozhu_web_framework)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(mode "CMAKE_BUILD_TYPE")
 
set(ENABLE_BOOST OFF CACHE BOOL "choose ON to enable")
set(ENABLE_GD OFF CACHE BOOL "choose ON to enable")

set(BOOST_OPEN "  ")
set(GD_OPEN " ")

if(ENABLE_BOOST STREQUAL "ON")  
  message("ENABLE_BOOST")
  set(BOOST_OPEN " -DENABLE_BOOST ")
endif()

if(ENABLE_GD STREQUAL "ON")  
    message("ENABLE_GD")
    set(GD_OPEN " -DENABLE_GD ")
endif()

if(${mode} AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))  
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
    message("Debug mode:${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS " -Wall -Wextra -pthread  -g -fsanitize=address  -DASIO_STANDALONE -DDEBUG ${BOOST_OPEN} ${GD_OPEN} -I/usr/local/include -I/usr/local/mysql/include   -I/usr/include -I/usr/include/mysql " )


elseif(${mode} AND (CMAKE_BUILD_TYPE STREQUAL "Release")) 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -O3")
    message("Release mode:${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS "  -pthread  -DASIO_STANDALONE ${BOOST_OPEN} ${GD_OPEN}  -I/usr/local/include -I/usr/local/mysql/include -I/usr/include -I/usr/include/mysql  " )
    file(MAKE_DIRECTORY /usr/local/etc/paozhu)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf/ DESTINATION /usr/local/etc/paozhu/)
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
    message("Debug mode")
    set(CMAKE_CXX_FLAGS " -Wall -Wextra -pthread  -g -fsanitize=address  -DASIO_STANDALONE -DDEBUG ${BOOST_OPEN} ${GD_OPEN} -I/usr/local/include -I/usr/local/mysql/include   -I/usr/include  -I/usr/include/mysql  " )
  
endif()



file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/temp)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/log)
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/bin/paozhu_empty)
#EXECUTE_PROCESS(COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/bin/paozhu_empty)
set(PAOZHU_PRE paozhu_pre)
add_executable(${PAOZHU_PRE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli/autopickmethod.cpp ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/src/md5.cpp)
add_executable(paozhu_empty ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli/autoemptymethod.cpp)



file(GLOB_RECURSE orm_list ${CMAKE_CURRENT_SOURCE_DIR}/orm/*.cpp)
file(GLOB_RECURSE source_list ${CMAKE_CURRENT_SOURCE_DIR}/models/*.cpp)
file(GLOB_RECURSE FRAMEWORK_CPP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpserver/*.cpp)
file(GLOB_RECURSE common_list ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp)
file(GLOB_RECURSE controller_list ${CMAKE_CURRENT_SOURCE_DIR}/controller/src/*.cpp)
file(GLOB_RECURSE viewsrc_list ${CMAKE_CURRENT_SOURCE_DIR}/viewsrc/view/*.cpp)
file(GLOB_RECURSE reflect_list ${CMAKE_CURRENT_SOURCE_DIR}/libs/*.cpp)
file(GLOB_RECURSE src_list ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

foreach(cppfile IN LISTS controller_list)
  string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/controller/src/" "" cppfilename ${cppfile})
  string(REGEX REPLACE ".cpp" "" cppbasename ${cppfilename})
#  message(${cppbasename})
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/controller/include/${cppbasename}.h")
  else()
      file(TOUCH "${CMAKE_CURRENT_SOURCE_DIR}/controller/include/${cppbasename}.h")
  endif()
endforeach()

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
include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(paozhu_cli ${CMAKE_CURRENT_SOURCE_DIR}/vendor/httpcli/http_cli.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)


if(${mode} AND (CMAKE_BUILD_TYPE STREQUAL "Release")) 
  add_executable(paozhu ${CMAKE_CURRENT_SOURCE_DIR}/test/testdaemon.cpp ${common_list} ${viewsrc_list} ${FRAMEWORK_CPP_PATH} ${orm_list} ${reflect_list} ${src_list} ${source_list} ${controller_list})
else()
  add_executable(paozhu ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp ${common_list} ${viewsrc_list} ${FRAMEWORK_CPP_PATH} ${orm_list} ${reflect_list} ${src_list} ${source_list} ${controller_list})
endif()



add_custom_command(
    TARGET paozhu_empty paozhu
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin/paozhu_pre
    PRE_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    COMMAND echo "-- controls method --"
    COMMAND ${PAOZHU_PRE} ${CMAKE_CURRENT_SOURCE_DIR}/
    COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/bin/paozhu_empty
    )

if(USE_STANDALONE_ASIO)
if(CMAKE_SYSTEM_NAME MATCHES "Darwin") 
set(ASIO_PATH "/usr/local/opt/asio/include" "/usr/local/include")
else()
set(ASIO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/asio "/usr/include")
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


if(ENABLE_BOOST STREQUAL "ON")  
message("---ENABLE_BOOST-----")
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
find_package_handle_standard_args(mysql REQUIRED_VARS MYSQL_ROOT_DIR)

FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
  /usr/local/include/mysql
  /usr/include/mysql
  /usr/local/mysql/include
)
SET(MYSQL_NAMES mysqlclient)
FIND_LIBRARY(MYSQL_LIBRARY
  NAMES ${MYSQL_NAMES}
  PATHS /usr/lib /usr/local/lib /usr/local/mysql/lib
  PATH_SUFFIXES mysql
)

IF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND TRUE)
  SET( MYSQL_LIBRARIES ${MYSQL_LIBRARY} )
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND FALSE)
  SET( MYSQL_LIBRARIES )
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)

IF (MYSQL_FOUND)
  IF (NOT MYSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Found MySQL: ${MYSQL_LIBRARY}")
  ENDIF (NOT MYSQL_FIND_QUIETLY)
ELSE (MYSQL_FOUND)
  IF (MYSQL_FIND_REQUIRED)
    MESSAGE(STATUS "Looked for MySQL libraries named ${MYSQL_NAMES}.")
    MESSAGE(FATAL_ERROR "Could NOT find MySQL library")
  ENDIF (MYSQL_FIND_REQUIRED)
ENDIF (MYSQL_FOUND)

target_include_directories(paozhu PUBLIC ${MYSQL_INCLUDE_DIR})
target_link_libraries(paozhu ${MYSQL_LIBRARY})

target_include_directories(paozhu_cli PUBLIC ${MYSQL_INCLUDE_DIR})
target_link_libraries(paozhu_cli ${MYSQL_LIBRARY})



if(ENABLE_GD STREQUAL "ON")
message("---ENABLE_GD-----")

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
#end ENABLE_GD
endif()

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


message("Compile framework mode")

target_link_libraries(paozhu  m  dl)

