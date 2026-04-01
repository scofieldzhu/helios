
set(COMPONENT_NAME "core")
set(LOCAL_PROJ_NAME "${PROJ_NAME}_${COMPONENT_NAME}")

set(EXPORT_FILENAME_LIST
    "helios_core_export.h"
    "helios_core_typedef.h"
    "sprite.h"
    "plane_scene.h"
    "axial_plane_scene.h"
    "coronal_plane_scene.h"
    "sagittal_plane_scene.h"
    "hud_scene.h"
    "core_pre_decl.h"    
    "pick_info.h"
    "slice_plane_sprite.h"
    "sprite_picker.h"
    "sprite_cell_picker.h"
    "scene.h"
    "sprite_prop_picker.h"
    "sprite_group.hpp"
    "render_widget.h"
    "camera_interactor_style.h"    
    "action_event.h"
    "action_slot_mapper.h"
    "editor.h"
    "scene_repository.h"
)

set(SRC_FILENAME_LIST
    "sprite.cpp"
    "scene.cpp"
    "plane_scene.cpp"
    "axial_plane_scene.cpp"
    "coronal_plane_scene.cpp"
    "sagittal_plane_scene.cpp"        
    "hud_scene.cpp"
    "slice_plane_sprite.cpp"
    "sprite_cell_picker.cpp"    
    "sprite_prop_picker.cpp"
    "render_widget.cpp"
    "camera_interactor_style.cpp"
    "event_translator.h"
    "event_translator.cpp"
    "action_event.cpp"
    "action_slot_mapper.cpp"
    "editor.cpp"
    "scene_repository.cpp"
)

set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL) 
if(Qt5_FOUND)
    message(STATUS "Found Qt version: ${Qt5_VERSION}")
else()
    message(FATAL_ERROR "Configure Error: accquire Qt5 package failed!")
endif()

add_library(${LOCAL_PROJ_NAME} SHARED ${EXPORT_FILENAME_LIST} ${SRC_FILENAME_LIST})

target_compile_definitions(${LOCAL_PROJ_NAME} PRIVATE 
    HELIOS_CORE_EXPORTS
)

set_proj_utf8_charset(${LOCAL_PROJ_NAME})

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE 
    ${VTK_LIBRARIES} 
    "${PROJ_NAME}_basic"
    Qt5::Core Qt5::OpenGL
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