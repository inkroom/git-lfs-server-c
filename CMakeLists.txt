cmake_minimum_required(VERSION 3.14)

option(USE_STATIC_LINK "use static compile" OFF)
if(USE_STATIC_LINK)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc ")
endif()
set(PROJECT_N lfs)
project(${PROJECT_N} VERSION 1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 最好只在debug下生成这个，或者 -DCMAKE_EXPORT_COMPILE_COMMANDS=on


if(USE_STATIC_LINK)
    if (WIN32 OR MSVC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    elseif (UNIX)
        # 仅查找静态库，强制后缀为 .a
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

    #     # 如果只是优先查找静态库，保证 .a 后缀在前面即可，把默认的后缀加上
    #     # set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    endif()
endif()

file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.c)

# a macro that gets all of the header containing directories.
MACRO(header_directories return_list includes_base_folder extention)
    FILE(GLOB_RECURSE new_list ${includes_base_folder}/*.${extention})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()
# a macro that gets all of the header containing directories.
header_directories(INCLUDES ${PROJECT_SOURCE_DIR}/include/ h)

# include(FetchContent)
# FetchContent_Declare(curl
#         GIT_REPOSITORY https://github.com/curl/curl.git
#         GIT_TAG 7.88.1)
# FetchContent_MakeAvailable(curl)

# add_subdirectory(cos-c-sdk-v5)


# find_package(libcos_c_sdk)



FIND_PROGRAM(APR_CONFIG_BIN NAMES apr-config apr-1-config PATHS /usr/bin /usr/local/bin /usr/local/apr/bin/)
FIND_PROGRAM(APU_CONFIG_BIN NAMES apu-config apu-1-config PATHS /usr/bin /usr/local/bin /usr/local/apr/bin/)

IF (APR_CONFIG_BIN)
    EXECUTE_PROCESS(
        COMMAND ${APR_CONFIG_BIN} --includedir
        OUTPUT_VARIABLE APR_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
        COMMAND ${APR_CONFIG_BIN} --cflags
        OUTPUT_VARIABLE APR_C_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
        COMMAND ${APR_CONFIG_BIN} --link-ld
        OUTPUT_VARIABLE APR_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
ELSE()
    MESSAGE(FATAL_ERROR "Could not find apr-config/apr-1-config")
ENDIF()

IF (APU_CONFIG_BIN)
    EXECUTE_PROCESS(
        COMMAND ${APU_CONFIG_BIN} --includedir
        OUTPUT_VARIABLE APR_UTIL_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
        COMMAND ${APU_CONFIG_BIN} --cflags
        OUTPUT_VARIABLE APU_C_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
        COMMAND ${APU_CONFIG_BIN} --link-ld
        OUTPUT_VARIABLE APU_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
ELSE()
    MESSAGE(FATAL_ERROR "Could not find apu-config/apu-1-config")
ENDIF()

#curl-config
FIND_PROGRAM(CURL_CONFIG_BIN NAMES curl-config)

IF (CURL_CONFIG_BIN)
    EXECUTE_PROCESS(
        COMMAND ${CURL_CONFIG_BIN} --libs
        OUTPUT_VARIABLE CURL_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
ELSE()
    MESSAGE(FATAL_ERROR "Could not find curl-config")
ENDIF()
# set(CURL_LIBRARY "-lcurl")
# find_package(CURL REQUIRED)

include_directories (${APR_INCLUDE_DIR})
include_directories (${APR_UTIL_INCLUDE_DIR})
include_directories (${MINIXML_INCLUDE_DIR})
include_directories (${CURL_INCLUDE_DIR})
# include_directories("include/curl")
# message("url ${CURL_INCLUDE_DIRS}")
include_directories ("/usr/local/include/cos_c_sdk")

find_library(APR_LIBRARY apr-1 PATHS /usr/local/apr/lib/)
find_library(APR_UTIL_LIBRARY aprutil-1 PATHS /usr/local/apr/lib/)
find_library(MINIXML_LIBRARY mxml)
find_library(CURL_LIBRARY curl)
if(USE_STATIC_LINK)
    find_library(COS_LIBRARY libcos_c_sdk_static.a PATHS /usr/local/lib/)

else()
    find_library(COS_LIBRARY cos_c_sdk PATHS /usr/local/lib/)
    message("not static ${COS_LIBRARY}")

endif()

if(USE_STATIC_LINK)

    # curl需要的依赖
    find_library(IDN_LIBRARY idn)
    find_library(SSL_LIBRARY ssl)
    find_library(C_LIBRARY crypto)
    find_library(DL_LIBRARY dl)
    message("dl ${DL_LIBRARY}")
    message("idn ${IDN_LIBRARY}")
    message("ssl ${SSL_LIBRARY}")
    message("c ${C_LIBRARY}")

endif()

add_subdirectory(facil.io)
message(STATUS ${SRCS})
message(" cos ${COS_LIBRARY} ")
add_executable(${PROJECT_N} ${SRCS})

target_include_directories(${PROJECT_N} PUBLIC include )
target_link_libraries(${PROJECT_N} facil.io)
# target_link_libraries(${PROJECT_N} PRIVATE  cos_c_sdk::cos_c_sdk)
target_link_libraries(${PROJECT_N} ${COS_LIBRARY})
target_link_libraries(${PROJECT_N} ${APR_UTIL_LIBRARY})
target_link_libraries(${PROJECT_N} ${APR_LIBRARY})
target_link_libraries(${PROJECT_N} ${MINIXML_LIBRARY})

# target_link_libraries(${PROJECT_N} curl)
target_link_libraries(${PROJECT_N} ${CURL_LIBRARY})
if(USE_STATIC_LINK)
# target_link_libraries(${PROJECT_N}  idn ssl crypto dl)
    target_link_libraries(${PROJECT_N}  ${IDN_LIBRARY} ${SSL_LIBRARY} ${C_LIBRARY} ${DL_LIBRARY})
endif()