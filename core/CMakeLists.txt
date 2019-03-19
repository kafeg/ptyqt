project(ptyqt-core VERSION 0.1 LANGUAGES CXX)

set(SOURCE_FILES
    ptyqt.h
    ptyqt.cpp
    iptyprocess.h
)

if (MSVC)
    set(SOURCE_FILES
        ${SOURCE_FILES}
        winptyprocess.h
        winptyprocess.cpp
        #conptynamedpipeprocess.h
        #conptynamedpipeprocess.cpp
        conptyanonymouspipeprocess.h
        conptyanonymouspipeprocess.cpp
        conpty_shared.h
        )
else()
    set(SOURCE_FILES
        ${SOURCE_FILES}
        unixptyprocess.cpp
        unixptyprocess.h
        )
endif()

if("${BUILD_TYPE}" STREQUAL "STATIC")
    add_library( ptyqt STATIC ${SOURCE_FILES} )
else()
    add_library( ptyqt SHARED ${SOURCE_FILES} )
endif()

target_link_libraries(ptyqt Qt5::Core)

if (MSVC)
    target_link_libraries( ptyqt Qt5::Network ${WINPTY_LIBS})
    add_dependencies(ptyqt winpty)
endif()

install(TARGETS ptyqt DESTINATION ${PTYQT_INSTALL_LIB_DIR})
install(FILES ptyqt.h iptyprocess.h DESTINATION ${PTYQT_INSTALL_INCLUDE_DIR})
