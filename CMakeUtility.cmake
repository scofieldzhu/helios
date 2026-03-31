#choose difference libraries to link to different build type
#It's available for all CMake generate tool.
function(link_libraries_on_build_type TARGET DBG_LIB_LIST REL_LIB_LIST)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(${TARGET} ${DBG_LIB_LIST})
    else()
        target_link_libraries(${TARGET} ${REL_LIB_LIST})
    endif()
endfunction(link_libraries_on_build_type)

#set different C++ language standard to different build tool automatically.
function(auto_choose_cxx_standard)
    if(MSVC)
        if(MSVC_VERSION GREATER_EQUAL 1920)             
            set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE) 
            message(STATUS "Configuring for Visual Studio 2019 or newer, set CMAKE_CXX_STANDARD to 20.")
        elseif(MSVC_VERSION GREATER_EQUAL 1910 AND MSVC_VERSION LESS 1920)
            message(STATUS "Configuring for Visual Studio 2017, set CMAKE_CXX_STANDARD to 17.")
            set(CMAKE_CXX_STANDARD 17 PARENT_SCOPE)
        elseif(MSVC_VERSION GREATER_EQUAL 1900 AND MSVC_VERSION LESS 1910)
            message(STATUS "Configuring for Visual Studio 2015, set CMAKE_CXX_STANDARD to 14.")
            set(CMAKE_CXX_STANDARD 14 PARENT_SCOPE)
        else()
            message(FATAL_ERROR "MSVC version must be at least 19.0!")
        endif()
    else()
        message(STATUS "Configuring for Non-VS tool, set CMAKE_CXX_STANDARD to 17.")
        set(CMAKE_CXX_STANDARD 17 PARENT_SCOPE) #changed to 20 in future
    endif()
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)
endfunction(auto_choose_cxx_standard)

function(print_current_dir_files)
    file(GLOB_RECURSE XSOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.*")
    foreach(item IN LISTS XSOURCE_FILES)
        message(STATUS "\"${item}\"")
    endforeach()    
endfunction(print_current_dir_files)

function(set_proj_utf8_charset PROJ_NAME)
    if(MSVC)
        target_compile_options(${PROJ_NAME} PRIVATE /utf-8)
    else()
        target_compile_options(${PROJ_NAME} PRIVATE -finput-charset=UTF-8)
    endif()
endfunction(set_proj_utf8_charset)


