project(llhttp)

aux_source_directory(. LLHTTP_SRC_FILES)
aux_source_directory(helper/ LLHTTP_SRC_FILES)

include_directories(.)
include_directories(helper/)

if(_DAQI_TARGET_TYPE_ STREQUAL "SHARED_LIB")    
    add_compile_options(-fPIC)
endif()

add_library(${PROJECT_NAME} STATIC ${LLHTTP_SRC_FILES})        
set_target_properties (${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)

