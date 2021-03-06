project(limesdrinput)

set(limesdrinput_SOURCES
	limesdrinput.cpp
	limesdrinputplugin.cpp
    limesdrinputsettings.cpp
    limesdrinputwebapiadapter.cpp
	limesdrinputthread.cpp
)

set(limesdrinput_HEADERS
	limesdrinput.h
	limesdrinputplugin.h
    limesdrinputsettings.h
    limesdrinputwebapiadapter.h
	limesdrinputthread.h
)

include_directories(
        ${CMAKE_SOURCE_DIR}/swagger/sdrangel/code/qt5/client
	${CMAKE_SOURCE_DIR}/devices
	${LIMESUITE_INCLUDE_DIR}
)

if(NOT SERVER_MODE)
    set(limesdrinput_SOURCES
        ${limesdrinput_SOURCES}
        limesdrinputgui.cpp

        limesdrinputgui.ui
    )
    set(limesdrinput_HEADERS
        ${limesdrinput_HEADERS}
        limesdrinputgui.h
    )

    set(TARGET_NAME inputlimesdr)
    set(TARGET_LIB "Qt5::Widgets")
    set(TARGET_LIB_GUI "sdrgui")
    set(INSTALL_FOLDER ${INSTALL_PLUGINS_DIR})
else()
    set(TARGET_NAME inputlimesdrsrv)
    set(TARGET_LIB "")
    set(TARGET_LIB_GUI "")
    set(INSTALL_FOLDER ${INSTALL_PLUGINSSRV_DIR})
endif()

add_library(${TARGET_NAME} SHARED
	${limesdrinput_SOURCES}
)

if(LIMESUITE_EXTERNAL)
    add_dependencies(${TARGET_NAME} limesuite)
endif()

target_link_libraries(${TARGET_NAME}
        Qt5::Core
        ${TARGET_LIB}
	sdrbase
	${TARGET_LIB_GUI}
        swagger
        ${LIMESUITE_LIBRARY}
        limesdrdevice
)

install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_FOLDER})
