cmake_minimum_required(VERSION 2.8)

project(da4qi4)

file(GLOB_RECURSE HEADER_FILES  *.h *.hpp)
add_custom_target(Include SOURCES ${HEADER_FILES})

link_directories(/home/zhuangyan/software/boost_1_68_0/stage/lib)
link_libraries(boost_system boost_filesystem pthread)

include_directories(.)
include_directories(/home/zhuangyan/software/boost_1_68_0)

aux_source_directory(. TOP_SRC_FILES)
aux_source_directory(./def DEF_SRC_FILES)
aux_source_directory(./utilities UTILITIES_SRC_FILES)
aux_source_directory(./http-parser HTTP_PARSER_SRC_FILES)
aux_source_directory(./multipart-parser MULTIPART_PARSER_SRC_FILES)

add_definitions()

add_executable(${PROJECT_NAME} 
                ${TOP_SRC_FILES}
                ${HTTP_PARSER_SRC_FILES} 
                ${MULTIPART_PARSER_SRC_FILES}
                ${DEF_SRC_FILES}
                ${UTILITIES_SRC_FILES}
                )
