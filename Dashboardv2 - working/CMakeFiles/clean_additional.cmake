# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles\\Dashboard_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Dashboard_autogen.dir\\ParseCache.txt"
  "Dashboard_autogen"
  )
endif()
