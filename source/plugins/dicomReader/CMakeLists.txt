
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(LOCAL_PLUGIN_NAME "DICOMReader")

find_package(DICOM REQUIRED CONFIG)

if(TARGET VTK::DICOM)
    set_target_properties(VTK::DICOM PROPERTIES
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL LinguistTools)

set(NP_UI_FILES
    dicomReaderDlg.ui
)

qt5_wrap_ui(NP_UI_HEADERS ${NP_UI_FILES})

set(QM_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_en.qm"
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_zh_CN.qm"
)

add_library(${LOCAL_PLUGIN_NAME} SHARED
    dicomReaderDlg.h
    dicomReaderDlg.cpp
    dicomReader.h
    dicomReader.cpp
    dicomReader.json
    ${NP_UI_HEADERS}
    ${QM_FILES}
    resource.qrc
)

target_link_libraries(${LOCAL_PLUGIN_NAME} PRIVATE 
    ${VTK_LIBRARIES}
    VTK::DICOM
    Qt5::Core 
    Qt5::Gui 
    Qt5::Widgets 
    Qt5::OpenGL
)

target_include_directories(${LOCAL_PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/source/interfaces
)

set_proj_utf8_charset(${LOCAL_PLUGIN_NAME})

add_custom_command(TARGET ${LOCAL_PLUGIN_NAME} PRE_BUILD
    COMMAND lupdate -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts 
            ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_zh_CN.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_en.ts
    COMMAND lrelease ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_zh_CN.ts
            -qm ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_zh_CN.qm
    COMMAND lrelease ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_en.ts
            -qm ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LOCAL_PLUGIN_NAME}_en.qm
)

set_target_properties(${LOCAL_PLUGIN_NAME} PROPERTIES
    DEBUG_POSTFIX -d
    RELWITHDEBINFO_POSTFIX -rd
)

set_target_properties(${LOCAL_PLUGIN_NAME} PROPERTIES  
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins" # 用于可执行文件  
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"  # 用于共享库  
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"  # 用于静态库  
)  

set(HEADER_FILES
    "${CMAKE_SOURCE_DIR}/source/interfaces/IViewer.h"
    "${CMAKE_SOURCE_DIR}/source/interfaces/IViewerEventSink.h"
    "${CMAKE_SOURCE_DIR}/source/interfaces/IDICOMReader.h"
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
