cmake_minimum_required(VERSION 3.10)
add_definitions(-DUNICODE -D_UNICODE)
project(task_progressor VERSION 1.0)

#设置qt模块
# set(CMAKE_PREFIX_PATH $ENV{QTDIR580_64})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets) 

#设置日志头文件和库
# include_directories(${TRD_DIR}/include/visual ../)

set(TASK_HEADERS 
    task_progressor_view.h
    movie_label.h
    ${CMAKE_SOURCE_DIR}/source/mirfak/appbase/task_progressor_ipc_data.h
)

set(TASK_SOURCES 
    main.cpp 
    task_progressor_view.cpp 
    movie_label.cpp 
)
#qt5_wrap_ui(TASK_UI_HEADERS task_progressor.ui) 

set(TASK_QRCFILES 
    res/task_progressor.qrc
)
SET(ALL_FILES ${TASK_HEADERS} ${TASK_SOURCES} ${TASK_QRCFILES}) 

add_executable(${PROJECT_NAME} ${ALL_FILES}) 

set_target_properties(${PROJECT_NAME} PROPERTIES  
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} # 用于可执行文件  
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}  # 用于共享库  
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}  # 用于静态库  
)  
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/source/mirfak/appbase
)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)

# target_link_libraries(${PROJECT_NAME}
#     $<$<CONFIG:Debug>:debug/glog>
#     $<$<CONFIG:RelWithDebInfo>:glog>
#     $<$<CONFIG:Release>:glog >
# )

set(CMAKE_INSTALL_BINDIR bin)

install(TARGETS ${PROJECT_NAME}
    # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${INSTALL_SUFFIX}
    # LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${INSTALL_SUFFIX}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.exe" DESTINATION ${CMAKE_INSTALL_BINDIR})