macro(proj_install)
    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib    
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )

    # install(DIRECTORY ${CMAKE_INSTALL_PREFIX}
    #     DESTINATION bin
    #     CONFIGURATIONS Release
    #     FILES_MATCHING 
    #     PATTERN "*.dll"
    #     PATTERN "*-d.dll" EXCLUDE
    #     PATTERN "*-rd.dll" EXCLUDE
    # )

    # install(DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
    #     DESTINATION bin
    #     FILES_MATCHING PATTERN "*.dll" PATTERN "*.pdb"
    # )

    # install(DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
    #     DESTINATION lib
    #     FILES_MATCHING PATTERN "*.lib"
    # )

    # set(EXPORT_FILES "")
    # foreach(FILE_PATH ${SOURCE_FILES})
    #     get_filename_component(FILE_EXT "${FILE_PATH}" EXT)
    #     if(("${FILE_EXT}" STREQUAL ".h") OR ("${FILE_EXT}" STREQUAL ".hxx"))
    #         list(APPEND EXPORT_FILES ${FILE_PATH})
    #     endif()
    # endforeach()
    # install(FILES ${EXPORT_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
    
endmacro(proj_install)
