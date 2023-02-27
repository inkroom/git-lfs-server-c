cmake_minimum_required(VERSION 3.14)


set(LIBRARY_N cos)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

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


# header_directories(INCLUDES /usr/local/include/cos_c_sdk h)

find_library(APR_LIBRARY apr-1 PATHS /usr/local/apr/lib/)
find_library(APR_UTIL_LIBRARY aprutil-1 PATHS /usr/local/apr/lib/)
find_library(MINIXML_LIBRARY mxml)
find_library(CURL_LIBRARY curl)

include_directories (${APR_INCLUDE_DIR})
include_directories (${APR_UTIL_INCLUDE_DIR})
include_directories (${MINIXML_INCLUDE_DIR})
include_directories (${CURL_INCLUDE_DIR})
# include_directories("include/curl")
# message("url ${CURL_INCLUDE_DIRS}")
include_directories ("/usr/local/include/cos_c_sdk")

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


# add_subdirectory(../facil.io)
# if(USE_STATIC_LINK)
# message("静态")
# add_library(${LIBRARY_N} STATIC cos.c)
# elseif()
add_library(${LIBRARY_N} cos.c)

# endif()
target_include_directories(${LIBRARY_N} PUBLIC /usr/local/include/cos_c_sdk)

target_link_libraries(${LIBRARY_N} facil.io) ## 父模块有这个依赖，子模块直接引入即可
# target_link_libraries(${PROJECT_N} PRIVATE  cos_c_sdk::cos_c_sdk)
target_link_libraries(${LIBRARY_N} ${COS_LIBRARY})
target_link_libraries(${LIBRARY_N} ${APR_UTIL_LIBRARY})
target_link_libraries(${LIBRARY_N} ${APR_LIBRARY})
target_link_libraries(${LIBRARY_N} ${MINIXML_LIBRARY})

# target_link_libraries(${PROJECT_N} curl)
target_link_libraries(${LIBRARY_N} ${CURL_LIBRARY})
if(USE_STATIC_LINK)
find_library(THREAD_LIBRARY pthread)

message("link 依赖 ${THREAD_LIBRARY}")
# target_link_libraries(${LIBRARY_N}  idn ssl crypto dl)
    target_link_libraries(${LIBRARY_N} ${IDN_LIBRARY} ${SSL_LIBRARY} ${C_LIBRARY} ${DL_LIBRARY} ${THREAD_LIBRARY})
endif()
