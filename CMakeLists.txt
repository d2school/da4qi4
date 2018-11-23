cmake_minimum_required(VERSION 2.8)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
    message(STATUS "use CCACHE program : ${CCACHE_PROGRAM}.")
endif()

project(da4qi4)

find_library(LIBBOOSTSYSTEM boost_system /usr/local/lib /usr/lib NO_DEFAULT_PATH)
find_library(LIBBOOSTFILESYSTEM boost_filesystem /usr/local/lib /usr/lib NO_DEFAULT_PATH)
find_library(LIBMYSQLPP mysqlpp /usr/local/lib /usr/lib NO_DEFAULT_PATH)

link_libraries(${LIBBOOSTSYSTEM} ${LIBBOOSTFILESYSTEM} mysqlclient pthread crypto)

include_directories(.)

include_directories(./nlohmann_json/include/)
set(JSON_MultipleHeaders ON  CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

aux_source_directory(./def SRC_FILES)
aux_source_directory(./http-parser SRC_FILES)
aux_source_directory(./intercepters SRC_FILES)
aux_source_directory(./multipart-parser SRC_FILES)
aux_source_directory(./redis-client SRC_FILES)
aux_source_directory(./utilities SRC_FILES)
aux_source_directory(./tools/aliyun_sms SRC_FILES)

aux_source_directory(. SRC_FILES)

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

add_executable(${PROJECT_NAME} ${SRC_FILES})
