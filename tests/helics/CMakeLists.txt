##############################################################################
#Copyright � 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################


#add a baseline library for underlying dependencies and flags for test executables
add_library(helics_test_base INTERFACE)
target_link_libraries(helics_test_base INTERFACE ${Boost_LIBRARIES_test})

target_include_directories(helics_test_base SYSTEM INTERFACE
  $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
)

#set_property(TARGET helics_test_base APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
#  $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
#  )
  
add_subdirectory(common)

add_subdirectory (core)
add_subdirectory(application_api)

if (BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
    add_subdirectory(shared_library)
endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
    add_subdirectory(apps)
endif()

