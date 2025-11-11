# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/cudasdr_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/cudasdr_autogen.dir/ParseCache.txt"
  "cudasdr_autogen"
  )
endif()
