project(multipart_parser)

aux_source_directory(. MMPART_PARSER_SRC_FILES)

include_directories(.)

if(_DAQI_TARGET_TYPE_ STREQUAL "SHARED_LIB")    
    add_compile_options(-fPIC)
endif()

add_library(${PROJECT_NAME} STATIC ${MMPART_PARSER_SRC_FILES})        
set_target_properties (${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
