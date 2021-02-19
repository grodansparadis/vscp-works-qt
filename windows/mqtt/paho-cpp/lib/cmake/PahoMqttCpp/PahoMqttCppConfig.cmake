# save build-time options
set(PAHO_BUILD_STATIC ON)
set(PAHO_BUILD_SHARED OFF)
set(PAHO_WITH_SSL ON)

include(CMakeFindDependencyMacro)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
find_dependency(PahoMqttC REQUIRED)
list(REMOVE_AT CMAKE_MODULE_PATH -1)
find_dependency(Threads REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/PahoMqttCppTargets.cmake")
