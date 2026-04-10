# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CloudStorage_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CloudStorage_autogen.dir\\ParseCache.txt"
  "CloudStorage_autogen"
  )
endif()
