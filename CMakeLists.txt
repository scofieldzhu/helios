cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(PROJ_NAME "helios")

project(${PROJ_NAME} VERSION 1.0)

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)  

message(STATUS "IS_MULTI_CONFIG = ${IS_MULTI_CONFIG}")

if(NOT IS_MULTI_CONFIG)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type")  
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo)  
  endif()
  message(STATUS "CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE}!")
else()
  set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo CACHE STRING "Available build configurations" FORCE)
endif()

include(CMakeUtility.cmake)
auto_choose_cxx_standard()

set(CMAKE_PREFIX_PATH "${TRD_DIR}" ${CMAKE_PREFIX_PATH})  
find_package(VTK 9.3.0 REQUIRED)
message(STATUS "Found VTK version: ${VTK_VERSION}")   

foreach(ITEM IN LISTS VTK_LIBRARIES)
    set_target_properties(${ITEM} PROPERTIES
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
endforeach()

# provide spdlog package for all projects
find_package(spdlog CONFIG REQUIRED)
message(STATUS "Found spdlog version: ${spdlog_VERSION} include dir:${spdlog_INCLUDE_DIRS}")   

if(TARGET spdlog::spdlog)
    set_target_properties(spdlog::spdlog PROPERTIES
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
endif()

link_libraries(
  spdlog::spdlog
)

add_compile_definitions(
    $<$<NOT:$<CONFIG:Release>>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
    $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

include_directories(
  ${TRD_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

link_directories(
  ${TRD_DIR}/lib
)

# disable specific warnnings for all projects
if(MSVC)
  add_compile_options(/wd4244 /wd4251 /wd4275)
else()
  add_compile_options(-Wno-4244 -Wno-4251 -Wno-4275)
endif()

add_subdirectory(source/helios/basic ${CMAKE_BINARY_DIR}/basic)
add_subdirectory(source/helios/core ${CMAKE_BINARY_DIR}/core)
add_subdirectory(source/helios/sprites ${CMAKE_BINARY_DIR}/sprites)
#add_subdirectory(source/helios/navigation ${CMAKE_BINARY_DIR}/navigation)
add_subdirectory(source/helios/appbase ${CMAKE_BINARY_DIR}/appbase)
add_subdirectory(source/helios/approcess ${CMAKE_BINARY_DIR}/approcess)
add_subdirectory(source/plugins/dicomReader ${CMAKE_BINARY_DIR}/plugins/dicomReader)
add_subdirectory(source/plugins/pathDesigner ${CMAKE_BINARY_DIR}/plugins/pathDesigner)
#add_subdirectory(source/plugins/trackerToolManager ${CMAKE_BINARY_DIR}/plugins/trackerToolManager)
add_subdirectory(source/test ${CMAKE_BINARY_DIR}/test)
add_subdirectory(source/task_progressor ${CMAKE_BINARY_DIR}/task_progressor)


# make install 
file(GLOB HELIOS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/helios/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/helios/*.hpp"
)

install(FILES ${HELIOS_HEADERS}
    DESTINATION "include/helios"
)

# install targets
include(CMakePackageConfigHelpers)

# 生成配置版本文件
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/helios-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 配置主配置文件
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/helios-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/helios-config.cmake
    INSTALL_DESTINATION lib/cmake/helios
)

# 安装导出目标文件
install(EXPORT helios-targets
    FILE helios-targets.cmake
    NAMESPACE helios::
    DESTINATION lib/cmake/helios
)

# 安装配置文件
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/helios-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/helios-config-version.cmake
    DESTINATION lib/cmake/helios
)