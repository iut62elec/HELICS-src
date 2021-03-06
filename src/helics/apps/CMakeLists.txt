##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
set(helics_apps_public_headers
	Player.hpp
	Recorder.hpp
	Echo.hpp
	Source.hpp
	Tracer.hpp
	helicsApp.hpp
	)

set(helics_apps_private_headers
	PrecHelper.hpp
	SignalGenerators.hpp
	)

set(helics_apps_library_files
	Player.cpp
	Recorder.cpp
	PrecHelper.cpp
	SignalGenerators.cpp
	Echo.cpp
	Source.cpp
	Tracer.cpp
	helicsApp.cpp
)

add_library(helics_apps-static STATIC ${helics_apps_library_files} ${helics_apps_public_headers} ${helics_apps_private_headers})

target_link_libraries(helics_apps-static PUBLIC helics-static)
target_include_directories(helics_apps-static SYSTEM PRIVATE ${Boost_INCLUDE_DIR})

target_include_directories(helics_apps-static SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/ThirdParty)

if (BUILD_PLAYER)
	add_executable(helics_player playerMain.cpp)
	target_link_libraries(helics_player helics_apps-static )
	set_target_properties (helics_player PROPERTIES FOLDER apps)
	target_include_directories(helics_player SYSTEM PRIVATE ${Boost_INCLUDE_DIR})
endif()


if (BUILD_RECORDER)
	add_executable(helics_recorder recorderMain.cpp)
	target_link_libraries(helics_recorder helics_apps-static)
	set_target_properties (helics_recorder PROPERTIES FOLDER apps)
	target_include_directories(helics_recorder SYSTEM PRIVATE ${Boost_INCLUDE_DIR})
endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
	add_executable(helics_app appMain.cpp)
	target_link_libraries(helics_app helics_apps-static)
	set_target_properties (helics_app PROPERTIES FOLDER apps)
	target_include_directories(helics_app SYSTEM PRIVATE ${Boost_INCLUDE_DIR})
endif()

INSTALL(TARGETS helics_apps-static EXPORT helics-targets
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	COMPONENT libs)

#install(FILES $<TARGET_PDB_FILE:${helics_apps-static}> DESTINATION ${CMAKE_INSTALL_LIBDIR} OPTIONAL COMPONENT libs)

INSTALL(FILES ${helics_apps_public_headers} 
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/apps COMPONENT headers)

if (BUILD_PLAYER)
INSTALL(TARGETS helics_player DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_player POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_player>/")                 # <--this is out-file path
endforeach(keyfile)

endif()

if (BUILD_RECORDER)
INSTALL(TARGETS helics_recorder DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
if (NOT BUILD_PLAYER)
foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_recorder POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_recorder>/")                 # <--this is out-file path
endforeach(keyfile)
endif()
endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
INSTALL(TARGETS helics_app DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
if (NOT BUILD_PLAYER AND NOT BUILD_RECORDER)
foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_app POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_app>/")                 # <--this is out-file path
endforeach(keyfile)
endif()
endif()

