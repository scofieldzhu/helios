
set(COMPONENT_NAME "approcess")
set(LOCAL_PROJ_NAME "${PROJ_NAME}_${COMPONENT_NAME}")

set(EXPORT_FILENAME_LIST
    "helios_${COMPONENT_NAME}_export.h"    
    "helios_${COMPONENT_NAME}_typedef.h"
    "process_view.h"
    "process_view_model.h"
    "process_session.h"
    "process_session_context.h"
    "render_service.h"
    "process_factory_interface.h"
)

set(SRC_FILENAME_LIST    
    "process_view.cpp"
    "process_view_model.cpp"
    "process_session.cpp"
)

set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets) 
message(STATUS "Found Qt version: ${Qt5_VERSION}")

add_library(${LOCAL_PROJ_NAME} SHARED ${SRC_FILENAME_LIST} ${EXPORT_FILENAME_LIST})

target_compile_definitions(${LOCAL_PROJ_NAME} PRIVATE 
    HELIOS_APPROCESS_EXPORTS
)

target_compile_definitions(${LOCAL_PROJ_NAME} PUBLIC
    $<$<CONFIG:Debug>:BUILD_TYPE_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:BUILD_TYPE_RELWITHDEBINFO>
    $<$<CONFIG:Release>:BUILD_TYPE_RELEASE>
)

set_proj_utf8_charset(${LOCAL_PROJ_NAME})

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE 
    ${VTK_LIBRARIES} 
    "${PROJ_NAME}_basic"
    "${PROJ_NAME}_core"
    "${PROJ_NAME}_sprites"
    "${PROJ_NAME}_appbase"
    Qt5::Core 
    Qt5::Widgets
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