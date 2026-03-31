
set(COMPONENT_NAME "basic")
set(LOCAL_PROJ_NAME "${PROJ_NAME}_${COMPONENT_NAME}")

set(EXPORT_FILENAME_LIST
    "mirfak_basic_export.h"
    "mirfak_basic_typedef.h"
    "xy.hpp"    
    "xyz.hpp"    
    "langext.h"
    "str_util.h"
    "signal.hpp"
    "line.h"
    "plane.h"
    "mobject.h"
    "color.h"
    "sys_util.h"
    "bound_plane.h"
    "image_data_util.h"
    "coord_conv.h"
    "log_service.h"
)

set(SOURCE_FILENAMES
    "str_util.cpp"
    "line.cpp"
    "plane.cpp"
    "color.cpp"
    "sys_util.cpp"
    "bound_plane.cpp"
    "image_data_util.cpp"
    "coord_conv.cpp"
    "log_service.cpp"
)

add_library(${LOCAL_PROJ_NAME} SHARED ${SOURCE_FILENAMES} ${EXPORT_FILENAME_LIST})

target_compile_definitions(${LOCAL_PROJ_NAME} PRIVATE 
    MIRFAK_BASIC_EXPORTS
    _WIN32
)

set_proj_utf8_charset(${LOCAL_PROJ_NAME})

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE 
    ${VTK_LIBRARIES} 
)

set_target_properties(${LOCAL_PROJ_NAME} PROPERTIES
    DEBUG_POSTFIX -d
    RELWITHDEBINFO_POSTFIX -rd
)

set_target_properties(${LOCAL_PROJ_NAME} PROPERTIES  
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}   
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} 
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} 
)  

install(TARGETS ${LOCAL_PROJ_NAME}
    EXPORT mirfak-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install(FILES ${EXPORT_FILENAME_LIST}
    DESTINATION "include/mirfak/${COMPONENT_NAME}"
)

install(FILES
    $<TARGET_PDB_FILE:${LOCAL_PROJ_NAME}>
    DESTINATION bin
    CONFIGURATIONS Debug RelWithDebInfo
)
