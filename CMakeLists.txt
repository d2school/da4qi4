cmake_minimum_required(VERSION 2.8)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
    message(STATUSS "use CCACHE program : ${CCACHE_PROGRAM}.")
endif()

project(da4qi4)

file(GLOB_RECURSE HEADER_FILES  *.h *.hpp)
add_custom_target(Include SOURCES ${HEADER_FILES})

find_library(LIBBOOSTSYSTEM boost_system /usr/local/lib /usr/lib NO_DEFAULT_PATH)
find_library(LIBBOOSTFILESYSTEM boost_filesystem /usr/local/lib /usr/lib NO_DEFAULT_PATH)

link_libraries(${LIBBOOSTSYSTEM} ${LIBBOOSTFILESYSTEM} mysqlclient pthread)

include_directories(.)
include_directories(./nlohmann_json/include/)

set(JSON_MultipleHeaders ON  CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

aux_source_directory(. TOP_SRC_FILES)
aux_source_directory(./intercepters TOP_SRC_FILES)

aux_source_directory(./def DEF_SRC_FILES)
aux_source_directory(./utilities UTILITIES_SRC_FILES)
aux_source_directory(./http-parser HTTP_PARSER_SRC_FILES)
aux_source_directory(./multipart-parser MULTIPART_PARSER_SRC_FILES)

add_definitions(-Dcimg_display=0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")

link_libraries(nlohmann_json)

function(echo_all_cmake_variable_values)
  get_cmake_property(vs VARIABLES)
  foreach(v ${vs})
    message(STATUS “${v}=’${${v}}'”)
  endforeach(v)
  message(STATUS “”)
endfunction()

#echo_all_cmake_variable_values()

add_executable(${PROJECT_NAME} 
                ${TOP_SRC_FILES}
                ${HTTP_PARSER_SRC_FILES} 
                ${MULTIPART_PARSER_SRC_FILES}
                ${DEF_SRC_FILES}
                ${UTILITIES_SRC_FILES}                
                )
