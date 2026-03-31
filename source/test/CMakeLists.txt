
set(LOCAL_PROJ_NAME "${PROJ_NAME}_Demo")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL) 
if(Qt5_FOUND)
    message(STATUS "Found Qt version: ${Qt5_VERSION}")
else()
    message(FATAL_ERROR "Configure Error: accquire Qt5 package failed!")
endif()

# 定义 UI 文件
set(NP_UI_FILES
    main_widget.ui
)

# 使用 qt5_wrap_ui 处理 UI 文件
qt5_wrap_ui(NP_UI_HEADERS ${NP_UI_FILES})

set(SOURCE_FILENAMES
    "test.cpp"
    "main_widget.h"
    "main_widget.cpp"
    "slice_overlay_toolbar.h"
    "slice_overlay_toolbar.cpp"
    "ortho_toolbar.h"
    "ortho_toolbar.cpp"
    "raycasting_config_load.h"
    "raycasting_config_load.cpp"
    "raycasting_conf.h"
    "vtkOrientationMarkerWidget2.h"
    "vtkOrientationMarkerWidget2.cxx"
)

set(SOURCE_FILES)
foreach(item IN LISTS SOURCE_FILENAMES)  
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${item}")
endforeach() 

add_executable(${LOCAL_PROJ_NAME} ${SOURCE_FILES} ${NP_UI_HEADERS})

target_include_directories(${LOCAL_PROJ_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/source/mirfak/core
    ${PROJECT_SOURCE_DIR}/source/mirfak/sprites
    ${PROJECT_SOURCE_DIR}/source/interfaces
)

target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE
    ${VTK_LIBRARIES} 
    "${PROJ_NAME}_basic"
    "${PROJ_NAME}_core"
    "${PROJ_NAME}_sprites"
    "${PROJ_NAME}_appbase"
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL
)

# target_compile_definitions(${LOCAL_PROJ_NAME} PRIVATE
#     $<$<CONFIG:Debug>:BUILD_TYPE_DEBUG=1>
#     $<$<CONFIG:RelWithDebInfo>:BUILD_TYPE_RELWITHDEBINFO=1>
#     $<$<CONFIG:Release>:BUILD_TYPE_RELEASE=1>
# )

set_proj_utf8_charset(${LOCAL_PROJ_NAME})

set_target_properties(${LOCAL_PROJ_NAME} PROPERTIES
    DEBUG_POSTFIX -d
    RELWITHDEBINFO_POSTFIX -rd
)

if(MSVC)    
    set(DEBUG_ENV "PATH=%PATH%;${TRD_DIR}/bin;${TRD_DIR}/bin/debug;${TRD_DIR}/VTK/bin/;${TRD_DIR}/DICOM/bin")
    set(RELWITHINFO_ENV "PATH=%PATH%;${TRD_DIR}/bin;${TRD_DIR}/bin/relwithdebinfo;${TRD_DIR}/VTK/bin;${TRD_DIR}/DICOM/bin")
    set(RELEASE_ENV "PATH=%PATH%;${TRD_DIR}/bin;${TRD_DIR}/bin/release;${TRD_DIR}/VTK/bin;${TRD_DIR}/DICOM/bin")
    set_target_properties(${LOCAL_PROJ_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT
        "$<$<CONFIG:Debug>:${DEBUG_ENV}>$<$<CONFIG:RelWithDebInfo>:${RELWITHINFO_ENV}>$<$<CONFIG:Release>:${RELEASE_ENV}>"
    )
    set_target_properties(${LOCAL_PROJ_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif(MSVC)

set_target_properties(${LOCAL_PROJ_NAME} PROPERTIES  
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} # 用于可执行文件  
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}  # 用于共享库  
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}  # 用于静态库  
)  

add_custom_command(TARGET ${LOCAL_PROJ_NAME} PRE_BUILD
    COMMAND lupdate -recursive ${PROJECT_SOURCE_DIR}/source/test -ts 
            ${PROJECT_SOURCE_DIR}/res/conf/lang/${LOCAL_PROJ_NAME}_zh_CN.ts
            ${PROJECT_SOURCE_DIR}/res/conf/lang/${LOCAL_PROJ_NAME}_en.ts
    COMMAND lrelease ${PROJECT_SOURCE_DIR}/res/conf/lang/${LOCAL_PROJ_NAME}_zh_CN.ts
            -qm ${PROJECT_SOURCE_DIR}/res/conf/lang/${LOCAL_PROJ_NAME}_zh_CN.qm
    COMMAND lrelease ${PROJECT_SOURCE_DIR}/res/conf/lang/${LOCAL_PROJ_NAME}_en.ts
            -qm ${PROJECT_SOURCE_DIR}/res/conf/lang/${LOCAL_PROJ_NAME}_en.qm
)

install(TARGETS ${LOCAL_PROJ_NAME}
    RUNTIME DESTINATION demo    
    CONFIGURATIONS Release
)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/res"
    DESTINATION "demo"
    CONFIGURATIONS Release
)

# set(QTCMD "
#     file(COPY \"${TRD_DIR}/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" 
#         FILES_MATCHING 
#         PATTERN \"*.dll\" 
#         PATTERN \"debug\" EXCLUDE
#         PATTERN \"relwithdebinfo\" EXCLUDE
#         PATTERN \"release\" EXCLUDE
#         PATTERN \"atlas\" EXCLUDE
#         PATTERN \"model\" EXCLUDE
#         PATTERN \"qt5\" EXCLUDE
#         )  
#     if(\$<CONFIG:Debug>)
#         execute_process(COMMAND \"windeployqt.exe\" --no-translations \"${CMAKE_INSTALL_PREFIX}/demo/${LOCAL_PROJ_NAME}-d.exe\")
#         file(COPY \"${TRD_DIR}/VTK/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3-d.dll\")       
#         file(COPY \"${TRD_DIR}/DICOM/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3d.dll\")       
#         file(COPY \"${TRD_DIR}/bin/debug/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*.dll\")  
#     endif()
#     if(\$<CONFIG:RelWithDebInfo>)
#         execute_process(COMMAND \"windeployqt.exe\" --no-translations \"${CMAKE_INSTALL_PREFIX}/demo/${LOCAL_PROJ_NAME}-rd.exe\")        
#         file(COPY \"${TRD_DIR}/VTK/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3.dll\") 
#         file(COPY \"${TRD_DIR}/DICOM/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3.dll\")       
#         file(COPY \"${TRD_DIR}/bin/relwithdebinfo/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*.dll\")  
#     endif()
#     if(\$<CONFIG:Release>)
#         execute_process(COMMAND \"windeployqt.exe\" --no-translations \"${CMAKE_INSTALL_PREFIX}/demo/${LOCAL_PROJ_NAME}.exe\")
#         file(COPY \"${TRD_DIR}/VTK/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3.dll\") 
#         file(COPY \"${TRD_DIR}/DICOM/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3.dll\")       
#         file(COPY \"${TRD_DIR}/bin/release/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*.dll\")        
#     endif()
#     file(COPY \"${CMAKE_INSTALL_PREFIX}/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\")                
# ")

set(QTCMD "
    if(\$<CONFIG:Release>)
        file(COPY \"${TRD_DIR}/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" 
            FILES_MATCHING 
            PATTERN \"*.dll\" 
            PATTERN \"debug\" EXCLUDE
            PATTERN \"relwithdebinfo\" EXCLUDE
            PATTERN \"release\" EXCLUDE
            PATTERN \"atlas\" EXCLUDE
            PATTERN \"model\" EXCLUDE
            PATTERN \"qt5\" EXCLUDE
            ) 
        execute_process(COMMAND \"windeployqt.exe\" --no-translations \"${CMAKE_INSTALL_PREFIX}/demo/${LOCAL_PROJ_NAME}.exe\")
        file(COPY \"${TRD_DIR}/VTK/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3.dll\") 
        file(COPY \"${TRD_DIR}/DICOM/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*-9.3.dll\")       
        file(COPY \"${TRD_DIR}/bin/release/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\" FILES_MATCHING PATTERN \"*.dll\")        
        file(COPY \"${CMAKE_INSTALL_PREFIX}/bin/\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/demo\"
            FILES_MATCHING
            PATTERN \"*.dll\"
            PATTERN \"*.pdb\" EXCLUDE
            PATTERN \"*-rd.dll\" EXCLUDE
            PATTERN \"*-d.dll\" EXCLUDE
        )                
    endif()
")
STRING(REGEX REPLACE "\\\\" "/" QTCMD ${QTCMD} )
install(CODE ${QTCMD})