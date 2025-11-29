cmake_minimum_required(VERSION 3.21)
project(byyl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

set(APP_SOURCES
    byyl_course_design_1/main.cpp
    byyl_course_design_1/mainwindow.cpp
    byyl_course_design_1/mainwindow.h
    byyl_course_design_1/mainwindow.ui
    src/core/model/Alphabet.h
    src/core/model/Automata.h
    src/core/regex/RegexLexer.h
    src/core/regex/RegexLexer.cpp
    src/core/regex/RegexParser.h
    src/core/regex/RegexParser.cpp
    src/core/automata/Thompson.h
    src/core/automata/Thompson.cpp
    src/core/automata/SubsetConstruction.h
    src/core/automata/SubsetConstruction.cpp
    src/core/automata/Hopcroft.h
    src/core/automata/Hopcroft.cpp
    src/core/generator/CodeGenerator.h
    src/core/generator/CodeGenerator.cpp
    src/core/Engine.h
    src/core/Engine.cpp
)

qt_add_executable(byyl
    ${APP_SOURCES}
)

target_link_libraries(byyl PRIVATE Qt6::Widgets)

set_target_properties(byyl PROPERTIES
    MACOSX_BUNDLE TRUE
)

enable_testing()
add_executable(GuiTest tests/auto_test_ui.cpp)
target_sources(GuiTest PRIVATE
    byyl_course_design_1/mainwindow.cpp
    byyl_course_design_1/mainwindow.h
    byyl_course_design_1/mainwindow.ui
    src/core/regex/RegexLexer.cpp
    src/core/regex/RegexParser.cpp
    src/core/automata/Thompson.cpp
    src/core/automata/SubsetConstruction.cpp
    src/core/automata/Hopcroft.cpp
    src/core/generator/CodeGenerator.cpp
    src/core/Engine.cpp
)
target_link_libraries(GuiTest PRIVATE Qt6::Widgets Qt6::Test)
add_test(NAME GuiTest COMMAND GuiTest)

add_executable(CliRegexTest tests/cli_regex_test.cpp)
target_sources(CliRegexTest PRIVATE
    src/core/regex/RegexLexer.cpp
    src/core/regex/RegexParser.cpp
    src/core/automata/Thompson.cpp
    src/core/automata/SubsetConstruction.cpp
    src/core/automata/Hopcroft.cpp
    src/core/generator/CodeGenerator.cpp
    src/core/Engine.cpp
)
target_link_libraries(CliRegexTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME CliRegexTest COMMAND CliRegexTest)
