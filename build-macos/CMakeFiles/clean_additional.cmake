# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles/CliRegexTest_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/CliRegexTest_autogen.dir/ParseCache.txt"
  "CMakeFiles/GuiTest_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/GuiTest_autogen.dir/ParseCache.txt"
  "CMakeFiles/byyl_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/byyl_autogen.dir/ParseCache.txt"
  "CliRegexTest_autogen"
  "GuiTest_autogen"
  "byyl_autogen"
  )
endif()
