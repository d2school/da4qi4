cmake_minimum_required(VERSION 3.9)

project(da4qi4)

#set(_DAQI_TARGET_TYPE_ "DEMO_SERVER")
#set(_DAQI_TARGET_TYPE_ "STATIC_LIB")
#set(_DAQI_TARGET_TYPE_ "SHARED_LIB")

include_directories(.)
include_directories(include/)
include_directories(nlohmann_json/include/)

find_package(Boost 1.65.1 REQUIRED COMPONENTS coroutine filesystem system)

if (Boost_FOUND)
    message(STATUS ${Boost_VERSION})
    message(STATUS ${Boost_INCLUDE_DIRS})
    message(STATUS ${Boost_LIBRARY_DIRS})
    
    add_definitions(-D BOOST_COROUTINES_NO_DEPRECATION_WARNING)

    if ((Boost_VERSION EQUAL 106600) OR (Boost_VERSION GREATER 106600))
        add_definitions(-D_USE_BOOST_VERSION_GE_1_66_=1)
    endif()

    add_definitions(-D_USE_BOOST_VERSION_=${Boost_VERSION})        
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_library(LIBICONV iconv /usr/local/lib)

link_libraries(${LIBICONV} pthread)

set(JSON_MultipleHeaders ON  CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

add_subdirectory(llhttp)
add_subdirectory(multipart-parser)

aux_source_directory(./src/redis-client DAQI_SRC_FILES)
aux_source_directory(./src/utilities DAQI_SRC_FILES)
aux_source_directory(./src/def DAQI_SRC_FILES)
aux_source_directory(./src/intercepters DAQI_SRC_FILES)
aux_source_directory(./src/client DAQI_SRC_FILES)
aux_source_directory(./src/net-detail DAQI_SRC_FILES)
aux_source_directory(./src/websocket DAQI_SRC_FILES)
aux_source_directory(./src DAQI_SRC_FILES)

aux_source_directory(./ DAQI_SRC_FILES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

link_libraries(multipart_parser)
link_libraries(llhttp)    
link_libraries(nlohmann_json)

if(_DAQI_TARGET_TYPE_ STREQUAL "DEMO_SERVER")
    message(STATUS "~BUILD DAQI AS DEMO SERVER~")    
    link_libraries(ssl)
    link_libraries(crypto)
    add_definitions(-D_BUILD_DAQI_DEMO_SERVER_=1)
    add_executable(${PROJECT_NAME} ${DAQI_SRC_FILES})  
    target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})
elseif(_DAQI_TARGET_TYPE_ STREQUAL "STATIC_LIB")    
    message(STATUS "~BUILD DAQI AS STATIC LIB~")    
    add_library(${PROJECT_NAME} STATIC ${DAQI_SRC_FILES})  
    set_target_properties (${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
    target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})
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
