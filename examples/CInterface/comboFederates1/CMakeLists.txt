

include_directories ("${PROJECT_SOURCE_DIR}/src/helics/shared_api_library")
add_executable(comboFed_c comboFed.c)

target_link_libraries(comboFed_c helicsSharedLib )

set_target_properties (comboFed_c PROPERTIES
    FOLDER examples
)

if (!MSVC)
target_compile_options(comboFed_c PRIVATE -ansi -Wall -pedantic -Werror)
endif()

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh
#               ${CMAKE_CURRENT_BINARY_DIR}/run.sh
#               COPYONLY)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET comboFed_c POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:comboFed_c>/")                 # <--this is out-file path
endforeach(keyfile)

add_custom_command(TARGET comboFed_c POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
        "$<TARGET_FILE_DIR:comboFed_c>/")                 # <--this is out-file path


