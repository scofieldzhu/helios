
set(COMPONENT_NAME "appbase")
set(LOCAL_PROJ_NAME "${PROJ_NAME}_${COMPONENT_NAME}")

set(EXPORT_FILENAME_LIST
    "helios_${COMPONENT_NAME}_export.h"    
    "doc_package.h"
    "vtk_image_data_serializer.h"
    "vtk_poly_data_serializer.h"
    "default_image_data_serializer.h"   
    "default_poly_data_serializer.h"    
    "path_util.h"
    "helios_appbase_typedef.h"
    "log_misc.h"
    "session_id.h"
    "session.h"
    "session_context.h"
    "session_manager.h"
    "plugin_manager.h"
    "scene_layout.h"
    "scene_layout_manager.h"
    "resource_util.h"
    "dicom_meta_data.h"
    "meta_data_serializer.h"
    "progress_control_view.h"
    "task_progressor_controller.h"
)

set(SRC_FILENAME_LIST    
    "default_image_data_serializer.cpp"
    "default_poly_data_serializer.cpp"
    "file_zipper.h"
    "file_zipper.cpp"
    "file_unzipper.cpp"
    "file_unzipper.h"
    "path_util.cpp"
    "log_misc.cpp"
    "plugin_manager.cpp"    
    "session.cpp"
    "session_manager.cpp"
    "doc_package.cpp"
    "scene_layout_manager.cpp"  
    "resource_util.cpp"
    "task_progressor_ipc_data.h"
    "task_progressor_controller.cpp"
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
    HELIOS_APPBASE_EXPORTS
)

target_compile_definitions(${LOCAL_PROJ_NAME} PUBLIC
    $<$<CONFIG:Debug>:BUILD_TYPE_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:BUILD_TYPE_RELWITHDEBINFO>
    $<$<CONFIG:Release>:BUILD_TYPE_RELEASE>
)

set_proj_utf8_charset(${LOCAL_PROJ_NAME})

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE 
    zlibwapi
    ${VTK_LIBRARIES} 
    "${PROJ_NAME}_basic"
    "${PROJ_NAME}_core"
    "${PROJ_NAME}_sprites"
    Qt5::Core 
    Qt5::OpenGL
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

install(FILES ${EXPORT_FILENAME_LIST}
    DESTINATION "include/helios/${COMPONENT_NAME}"
)

install(FILES
    $<TARGET_PDB_FILE:${LOCAL_PROJ_NAME}>
    DESTINATION bin
    CONFIGURATIONS Debug RelWithDebInfo
)