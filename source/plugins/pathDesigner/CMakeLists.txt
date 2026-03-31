
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(LOCAL_PLUGIN_NAME "PathDesigner")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL)

set(NP_UI_FILES
    pathDesignerDlg.ui
)

# 使用 qt5_wrap_ui 处理 UI 文件
qt5_wrap_ui(NP_UI_HEADERS ${NP_UI_FILES})

add_library(${LOCAL_PLUGIN_NAME} SHARED
    pathDesignerDlg.h
    pathDesignerDlg.cpp
    pathDesigner.h
    pathDesigner.cpp
    pathDesigner.json
    ${NP_UI_HEADERS}
)

target_link_libraries(${LOCAL_PLUGIN_NAME} PRIVATE 
    ${VTK_LIBRARIES}
    Qt5::Core 
    Qt5::Gui 
    Qt5::Widgets 
    Qt5::OpenGL
    "${PROJ_NAME}_basic"
    "${PROJ_NAME}_core"
    "${PROJ_NAME}_sprites"
)

# 插件与宿主共享接口头文件
target_include_directories(${LOCAL_PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/source/interfaces
)

set_target_properties(${LOCAL_PLUGIN_NAME} PROPERTIES
    DEBUG_POSTFIX -d
    RELWITHDEBINFO_POSTFIX -rd
)

set_proj_utf8_charset(${LOCAL_PLUGIN_NAME})

set_target_properties(${LOCAL_PLUGIN_NAME} PROPERTIES  
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins" # 用于可执行文件  
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"  # 用于共享库  
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"  # 用于静态库  
)  

set(HEADER_FILES
    "${CMAKE_SOURCE_DIR}/source/interfaces/IViewer.h"
    "${CMAKE_SOURCE_DIR}/source/interfaces/IViewerEventSink.h"
    "${CMAKE_SOURCE_DIR}/source/interfaces/IPathDesigner.h"
)

install(TARGETS ${LOCAL_PLUGIN_NAME}
    RUNTIME DESTINATION bin/plugins
    INCLUDES DESTINATION include/interfaces    
)

install(FILES ${HEADER_FILES}
    DESTINATION "include/interfaces"   
)

install(FILES "${CMAKE_SOURCE_DIR}/plugins/Debug/${LOCAL_PLUGIN_NAME}-d.pdb"
    DESTINATION "bin/plugins"
    CONFIGURATIONS Debug      
)

install(FILES "${CMAKE_SOURCE_DIR}/plugins/RelWithDebInfo/${LOCAL_PLUGIN_NAME}-rd.pdb"
    DESTINATION "bin/plugins"
    CONFIGURATIONS RelWithDebInfo      
)
