add_executable(valueFed valueFed.cpp)

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

target_link_libraries(valueFed helics-static)

set_target_properties (valueFed PROPERTIES
    FOLDER examples
)

add_custom_command(TARGET valueFed POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.sh"      # <--this is in-file
	"$<TARGET_FILE_DIR:valueFed>/")                 # <--this is out-file path

add_custom_command(TARGET valueFed POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.bat"      # <--this is in-file
	"$<TARGET_FILE_DIR:valueFed>/")                 # <--this is out-file path

