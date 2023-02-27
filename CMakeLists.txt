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


file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.c)

message("src ${SRCS}")

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


add_subdirectory(cos)

add_subdirectory(facil.io)
message(STATUS ${SRCS})
message(" cos ${COS_LIBRARY} ")
add_executable(${PROJECT_N} ${SRCS})

target_include_directories(${PROJECT_N} PUBLIC include )
target_link_libraries(${PROJECT_N} PUBLIC cos)
target_link_libraries(${PROJECT_N} PUBLIC facil.io)
