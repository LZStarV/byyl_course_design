cmake_minimum_required(VERSION 3.21)
project(byyl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

set(APP_SOURCES
    app/main.cpp
    app/mainwindow.cpp
    app/mainwindow.h
    app/mainwindow.ui
    src/model/Alphabet.h
    src/model/Automata.h
    src/regex/RegexLexer.h
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.h
    src/regex/RegexParser.cpp
    src/automata/Thompson.h
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.h
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.h
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.h
    src/generator/CodeGenerator.cpp
    src/Engine.h
    src/Engine.cpp
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
    app/mainwindow.cpp
    app/mainwindow.h
    app/mainwindow.ui
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.cpp
    src/Engine.cpp
)
target_link_libraries(GuiTest PRIVATE Qt6::Widgets Qt6::Test)
add_test(NAME 界面测试 COMMAND GuiTest)

add_executable(CliRegexTest tests/cli_regex_test.cpp)
target_sources(CliRegexTest PRIVATE
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.cpp
    src/Engine.cpp
)
target_link_libraries(CliRegexTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 命令行正则测试 COMMAND CliRegexTest)

add_executable(CodegenTest tests/codegen_compile_run_test.cpp)
target_sources(CodegenTest PRIVATE
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.cpp
    src/Engine.cpp
)
target_link_libraries(CodegenTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 代码生成器测试 COMMAND CodegenTest)
