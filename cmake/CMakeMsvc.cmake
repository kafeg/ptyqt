macro(configure_msvc_runtime triplet)
    # Default to statically-linked runtime.
    if(${triplet} MATCHES "static")
        message(STATUS "${triplet} -> forcing use of statically-linked runtime.")
        set(_SEARCH "/MD")
        set(_REPLACE "/MT")
        set(MSVC_RUNTIME "static")
    else()
        message(STATUS "${triplet} -> forcing use of dynamically-linked runtime.")
        set(_SEARCH "/MT")
        set(_REPLACE "/MD")
    endif()

    # Set compiler options.
    set(variables CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO)

    foreach(variable ${variables})
        if(${variable} MATCHES "${_SEARCH}")
            string(REGEX REPLACE "${_SEARCH}" "${_REPLACE}" ${variable} "${${variable}}")
        endif()
    endforeach()

    # Set default values if flags are empty
    if ("${CMAKE_CXX_FLAGS_DEBUG}" STREQUAL "")
        message( "Set default value to var CMAKE_CXX_FLAGS_DEBUG" )
        set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1")
    endif()

    if ("${CMAKE_CXX_FLAGS_MINSIZEREL}" STREQUAL "")
        message( "Set default value to var CMAKE_CXX_FLAGS_MINSIZEREL" )
        set(CMAKE_CXX_FLAGS_MINSIZEREL "/MT /O1 /Ob1 /DNDEBUG")
    endif()

    if ("${CMAKE_CXX_FLAGS_RELEASE}" STREQUAL "")
        message( "Set default value to var CMAKE_CXX_FLAGS_RELEASE" )
        set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /DNDEBUG")
    endif()

    if ("${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" STREQUAL "")
        message( "Set default value to var CMAKE_CXX_FLAGS_RELWITHDEBINFO" )
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1 /DNDEBUG")
    endif()

    dump_list(variables)
endmacro()

macro(dump_all)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endmacro()

macro(dump_list _lst)
    foreach (_variableName ${${_lst}})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endmacro()
