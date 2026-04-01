
set(COMPONENT_NAME "navigation")
set(LOCAL_PROJ_NAME "${PROJ_NAME}_${COMPONENT_NAME}")

set(EXPORT_FILENAME_LIST
    "helios_navigation_export.h"
    "helios_navigation_typedef.h"   
    "tracker.h"
    "tracker_config.h"
    "tracker_tool_config.h"
    "tracker_tracking_data.h"
    "tool_tracking_data.h"
    "device_administrator.h"
    "drill_config.h"
    "drill_config_repository.h"
    "drill_sprite.h"    
    "handpiece_sprite.h"
)

set(SRC_FILENAME_LIST
    "tracker.cpp"    
    "device_administrator.cpp"
    "handpiece_sprite.cpp"
    "drill_sprite.cpp"
    "drill_config_repository.cpp"
)

set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL) 
if(Qt5_FOUND)
    message(STATUS "Found Qt version: ${Qt5_VERSION}")
else()
    message(FATAL_ERROR "Configure Error: accquire Qt5 package failed!")
endif()

add_library(${LOCAL_PROJ_NAME} SHARED ${SRC_FILENAME_LIST} ${EXPORT_FILENAME_LIST})

target_compile_definitions(${LOCAL_PROJ_NAME} PRIVATE 
    HELIOS_NAVIGATION_EXPORTS
)

set_proj_utf8_charset(${LOCAL_PROJ_NAME})

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE 
    ${VTK_LIBRARIES} 
    "${PROJ_NAME}_basic"
    "${PROJ_NAME}_core"
    "${PROJ_NAME}_sprites"
    Qt5::Core 
    Qt5::OpenGL
)

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE
    $<$<CONFIG:Debug>:AIMTracker-d ARMDTracker-d>
    $<$<CONFIG:RelWithDebInfo>:AIMTracker-rd ARMDTracker-rd>
    $<$<CONFIG:Release>:AIMTracker ARMDTracker>
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
    EXPORT helios-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install(FILES
    $<TARGET_PDB_FILE:${LOCAL_PROJ_NAME}>
    DESTINATION bin
    CONFIGURATIONS Debug RelWithDebInfo
)

install(FILES ${EXPORT_FILENAME_LIST}
    DESTINATION "include/helios/${COMPONENT_NAME}"
)