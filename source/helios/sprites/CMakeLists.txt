
set(COMPONENT_NAME "sprites")
set(LOCAL_PROJ_NAME "${PROJ_NAME}_${COMPONENT_NAME}")

set(EXPORT_FILENAME_LIST
    "helios_sprites_export.h"
    "point_cloud_sprite.h"
    "ortho_planes_sprite.h"
    "backdrop_sprite.h"
    "plane_sprite.h"
    "plane_cutter_sprite.h"
    "plane_intersection_line_sprite.h"
    "plane_intersection_line_editor.h"
    "polygon_sprite.h"
    "polygon_editor.h"
    "sphere_sprite.h"
    "line_sprite.h"
    "surface_volume_sprite.h"
    "abstract_volume_sprite.h"
    "composite_volume_sprite.h"
    "label_sprite.h"
    "surgical_path_sprite.h"
    "surgical_path_editor.h"
    "polydata_cutter_sprite.h"
    "tube_sprite.h"
    "point_move_tool_sprite.h"
    "point_move_tool_editor.h"
    "torus_sprite.h"
    "arrow_sprite.h"
    "wlw_editor.h"
    "helios_sprites_typedef.h"
)

set(SRC_FILENAME_LIST
    "polygon_editor.cpp"
    "label_sprite.cpp"
    "surface_volume_sprite.cpp"
    "abstract_volume_sprite.cpp"
    "composite_volume_sprite.cpp"
    "surgical_path_sprite.cpp"
    "surgical_path_editor.cpp"
    "polydata_cutter_sprite.cpp"
    "point_move_tool_sprite.cpp"
    "point_move_tool_editor.cpp"    
    "tube_sprite.cpp"
    "torus_sprite.cpp"
    "arrow_sprite.cpp"
    "sphere_sprite.cpp"
    "line_sprite.cpp"
    "polygon_sprite.cpp"
    "plane_intersection_line_editor.cpp"
    "plane_intersection_line_sprite.cpp"
    "plane_cutter_sprite.cpp"
    "plane_sprite.cpp"
    "backdrop_sprite.cpp"
    "ortho_planes_sprite.cpp"
    "point_cloud_sprite.cpp"
    "wlw_editor.cpp"
)

add_library(${LOCAL_PROJ_NAME} SHARED ${EXPORT_FILENAME_LIST} ${SRC_FILENAME_LIST})

target_compile_definitions(${LOCAL_PROJ_NAME} PRIVATE 
    HELIOS_SPRITES_EXPORTS
)

set_proj_utf8_charset(${LOCAL_PROJ_NAME})

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE 
    ${VTK_LIBRARIES} 
    "${PROJ_NAME}_basic"
    "${PROJ_NAME}_core"
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