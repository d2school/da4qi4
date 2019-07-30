cmake_minimum_required(VERSION 2.8)

#find_program(CCACHE_PROGRAM ccache)
#if(CCACHE_PROGRAM)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
#    message(STATUS "use CCACHE program : ${CCACHE_PROGRAM}.")
#endif()

project(da4qi4)

#set(_DAQI_TARGET_TYPE_ "DEMO_SERVER")
#set(_DAQI_TARGET_TYPE_ "STATIC_LIB")
#set(_DAQI_TARGET_TYPE_ "SHARED_LIB")

find_library(LIBBOOSTSYSTEM boost_system /usr/local/lib /usr/lib NO_DEFAULT_PATH)
find_library(LIBBOOSTFILESYSTEM boost_filesystem /usr/local/lib /usr/lib NO_DEFAULT_PATH)
find_library(LIBICONV iconv /usr/local/lib)

link_libraries(${LIBBOOSTSYSTEM} ${LIBBOOSTFILESYSTEM} ${LIBICONV} pthread)

include_directories(.)
include_directories(include/)
include_directories(nlohmann_json/include/)

set(JSON_MultipleHeaders ON  CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

aux_source_directory(./src/redis-client DAQI_SRC_FILES)
aux_source_directory(./src/utilities DAQI_SRC_FILES)
aux_source_directory(./src/def DAQI_SRC_FILES)
aux_source_directory(./src/intercepters DAQI_SRC_FILES)
aux_source_directory(./src/client DAQI_SRC_FILES)
aux_source_directory(./src DAQI_SRC_FILES)

aux_source_directory(./http-parser DAQI_SRC_FILES)
aux_source_directory(./multipart-parser DAQI_SRC_FILES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")

link_libraries(nlohmann_json)

if(_DAQI_TARGET_TYPE_ STREQUAL "DEMO_SERVER")
    message(STATUS "~BUILD DAQI AS DEMO SERVER~")    
    link_libraries(ssl)
    link_libraries(crypto)
    add_definitions(-D_BUILD_DAQI_DEMO_SERVER_=1)
    add_executable(${PROJECT_NAME} ${DAQI_SRC_FILES})  
elseif(_DAQI_TARGET_TYPE_ STREQUAL "STATIC_LIB")    
    message(STATUS "~BUILD DAQI AS STATIC LIB~")    
    add_library(${PROJECT_NAME} STATIC ${DAQI_SRC_FILES})  
    set_target_properties (${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
elseif(_DAQI_TARGET_TYPE_ STREQUAL "SHARED_LIB")    
    message(STATUS "~BUILD DAQI AS SHARED LIB~")    
    add_library(${PROJECT_NAME} SHARED ${DAQI_SRC_FILES})        
    set_target_properties (${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
endif()

function(echo_all_cmake_variable_values)
  get_cmake_property(vs VARIABLES)
  foreach(v ${vs})
    message(STATUS “${v}=’${${v}}'”)
  endforeach(v)
  message(STATUS “”)
endfunction()

#echo_all_cmake_variable_values()
