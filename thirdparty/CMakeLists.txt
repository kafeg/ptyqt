if (MSVC)

    include(ExternalProject)

    if("${WINPTY_ROOT_DIR}" STREQUAL "")

        set(WINPTY_URL https://github.com/rprichard/winpty/releases/download/0.4.3/winpty-0.4.3-msvc2015.zip)

        message("Download WinPty " ${WINPTY_URL})

        ExternalProject_Add(
            winpty
            URL ${WINPTY_URL}
            CONFIGURE_COMMAND ""
            PREFIX winpty
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            #INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/glm ${CMAKE_BINARY_DIR}/installed/glm
            LOG_DOWNLOAD ON
            LOG_INSTALL ON
            )

        set(WINPTY_ROOT_DIR ${PROJECT_BINARY_DIR}/thirdparty/winpty/src/winpty)
        set(WINPTY_ROOT_DIR "${WINPTY_ROOT_DIR}" PARENT_SCOPE)

    else()
        message("Use user-defined WinPty")
    endif()

    set(WINPTY_SUB_ARCH ${TARGET_ARCH})
    if ("${WINPTY_SUB_ARCH}" STREQUAL "x86")
        set(WINPTY_SUB_ARCH ia32)
    endif()

    #message("WINPTY_SUB_ARCH " ${WINPTY_SUB_ARCH})
    #message("TARGET_ARCH " ${TARGET_ARCH})

    #can be linked only to DLL, because we compile in MSVC but WinPty builded with CygWin
    if("${BUILD_TYPE}" STREQUAL "STATIC")
        set(WINPTY_LIBS ${WINPTY_ROOT_DIR}/${WINPTY_SUB_ARCH}/lib/winpty.lib PARENT_SCOPE)
    else()
        set(WINPTY_LIBS ${WINPTY_ROOT_DIR}/${WINPTY_SUB_ARCH}/bin/winpty.dll PARENT_SCOPE)
    endif()

    #winpty.dll and winpty-agent.exe must be near your project executable
    set(WINPTY_DIST_FILES ${WINPTY_ROOT_DIR}/${WINPTY_SUB_ARCH}/bin/winpty.dll
        ${WINPTY_ROOT_DIR}/${WINPTY_SUB_ARCH}/bin/winpty-agent.exe
        PARENT_SCOPE)
    #message("WINPTY_DIST_FILES" ${WINPTY_DIST_FILES})
endif()
