cmake_minimum_required(VERSION 3.21)
project(byyl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

set(CORE_SOURCES
    src/syntax/Grammar.h
    src/syntax/Grammar.cpp
    src/syntax/GrammarParser.h
    src/syntax/GrammarParser.cpp
    src/syntax/LL1.h
    src/syntax/LL1.cpp
    src/syntax/LR0.h
    src/syntax/LR0.cpp
    src/syntax/AST.h
    src/syntax/SyntaxParser.cpp
    src/syntax/DotGenerator.cpp
    src/generator/SyntaxCodeGenerator.cpp
    src/generator/SyntaxCodeGenerator.h
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
    src/visual/DotExporter.h
    src/visual/DotExporter.cpp
    src/Engine.h
    src/Engine.cpp
    src/config/Config.h
    src/config/Config.cpp
)

set(APP_SOURCES
    app/main.cpp
    app/mainwindow.cpp
    app/mainwindow.h
    app/mainwindow.ui
    app/components/ToastWidget/ToastWidget.h
    app/components/ToastWidget/ToastWidget.cpp
    app/components/ToastManager/ToastManager.h
    app/components/ToastManager/ToastManager.cpp
    app/components/SettingsDialog/SettingsDialog.h
    app/components/SettingsDialog/SettingsDialog.cpp
    app/components/ImagePreviewDialog/ImagePreviewDialog.h
    app/components/ImagePreviewDialog/ImagePreviewDialog.cpp
    app/pages/home/HomePage.h
    app/pages/home/HomePage.cpp
    app/pages/exp1/Exp1Page.h
    app/pages/exp1/Exp1Page.cpp
    app/pages/exp2/Exp2Page.h
    app/pages/exp2/Exp2Page.cpp
    app/experiments/exp1/helpers/AutomataTableHelper.h
    app/experiments/exp1/helpers/AutomataTableHelper.cpp
    app/experiments/exp2/helpers/SyntaxTableHelper.h
    app/experiments/exp2/helpers/SyntaxTableHelper.cpp
    app/experiments/exp2/tabs/grammar/GrammarEditorTab.h
    app/experiments/exp2/tabs/grammar/GrammarEditorTab.cpp
    app/experiments/exp2/tabs/grammar/FirstFollowTab.h
    app/experiments/exp2/tabs/grammar/FirstFollowTab.cpp
    app/experiments/exp2/tabs/syntax/SyntaxTreeTab.h
    app/experiments/exp2/tabs/syntax/SyntaxTreeTab.cpp
    app/experiments/exp2/tabs/syntax/SyntaxCodeViewTab.h
    app/experiments/exp2/tabs/syntax/SyntaxCodeViewTab.cpp
    app/experiments/exp1/tabs/regex/RegexEditorTab.h
    app/experiments/exp1/tabs/regex/RegexEditorTab.cpp
    app/experiments/exp1/tabs/automata/NFAViewTab.h
    app/experiments/exp1/tabs/automata/NFAViewTab.cpp
    app/experiments/exp1/tabs/automata/DFAViewTab.h
    app/experiments/exp1/tabs/automata/DFAViewTab.cpp
    app/experiments/exp1/tabs/automata/MinDFAViewTab.h
    app/experiments/exp1/tabs/automata/MinDFAViewTab.cpp
    app/experiments/exp1/tabs/code/CodeViewTab.h
    app/experiments/exp1/tabs/code/CodeViewTab.cpp
    app/experiments/exp1/tabs/test/TestValidationTab.h
    app/experiments/exp1/tabs/test/TestValidationTab.cpp
    app/controllers/SyntaxController/SyntaxController.h
    app/controllers/SyntaxController/SyntaxController.cpp
    app/controllers/TestValidationController/TestValidationController.h
    app/controllers/TestValidationController/TestValidationController.cpp
    app/controllers/RegexController/RegexController.h
    app/controllers/RegexController/RegexController.cpp
    app/controllers/AutomataController/AutomataController.h
    app/controllers/AutomataController/AutomataController.cpp
    app/controllers/SettingsController/SettingsController.h
    app/controllers/SettingsController/SettingsController.cpp
    app/controllers/GeneratorController/GeneratorController.h
    app/controllers/GeneratorController/GeneratorController.cpp
    app/controllers/CodeViewController/CodeViewController.h
    app/controllers/CodeViewController/CodeViewController.cpp
    app/services/NotificationService/NotificationService.h
    app/services/NotificationService/NotificationService.cpp
    app/services/FileService/FileService.h
    app/services/FileService/FileService.cpp
    app/services/DotService/DotService.h
    app/services/DotService/DotService.cpp
)

add_library(byyl_core STATIC ${CORE_SOURCES})
target_include_directories(byyl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(byyl_core PRIVATE Qt6::Core)

qt_add_executable(byyl
    ${APP_SOURCES}
)

target_link_libraries(byyl PRIVATE Qt6::Widgets byyl_core)

if(APPLE)
    set_target_properties(byyl PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

enable_testing()
add_executable(GuiTest tests/ui/auto_test_ui.cpp)
target_sources(GuiTest PRIVATE
    app/mainwindow.cpp
    app/mainwindow.h
    app/mainwindow.ui
    app/controllers/SyntaxController/SyntaxController.h
    app/controllers/SyntaxController/SyntaxController.cpp
    app/controllers/AutomataController/AutomataController.h
    app/controllers/AutomataController/AutomataController.cpp
    app/controllers/SettingsController/SettingsController.cpp
    app/controllers/GeneratorController/GeneratorController.cpp
    app/controllers/CodeViewController/CodeViewController.cpp
    app/controllers/TestValidationController/TestValidationController.h
    app/controllers/TestValidationController/TestValidationController.cpp
    app/services/NotificationService/NotificationService.h
    app/services/NotificationService/NotificationService.cpp
    app/services/DotService/DotService.cpp
    app/services/FileService/FileService.cpp
    app/components/ToastWidget/ToastWidget.cpp
    app/components/ToastManager/ToastManager.cpp
    app/components/SettingsDialog/SettingsDialog.cpp
    app/components/ImagePreviewDialog/ImagePreviewDialog.cpp
    app/pages/home/HomePage.cpp
    app/pages/exp1/Exp1Page.cpp
    app/pages/exp2/Exp2Page.cpp
    app/experiments/exp1/helpers/AutomataTableHelper.cpp
    app/experiments/exp2/helpers/SyntaxTableHelper.cpp
    app/experiments/exp2/tabs/grammar/GrammarEditorTab.cpp
    app/experiments/exp2/tabs/grammar/FirstFollowTab.cpp
    app/experiments/exp2/tabs/syntax/SyntaxTreeTab.cpp
    app/experiments/exp2/tabs/syntax/SyntaxCodeViewTab.cpp
    app/experiments/exp1/tabs/regex/RegexEditorTab.cpp
    app/experiments/exp1/tabs/automata/NFAViewTab.cpp
    app/experiments/exp1/tabs/automata/DFAViewTab.cpp
    app/experiments/exp1/tabs/automata/MinDFAViewTab.cpp
    app/experiments/exp1/tabs/code/CodeViewTab.cpp
    app/experiments/exp1/tabs/test/TestValidationTab.cpp
    src/syntax/Grammar.cpp
    src/syntax/GrammarParser.cpp
    src/syntax/LL1.cpp
    src/syntax/LR0.cpp
    src/syntax/SyntaxParser.cpp
    src/syntax/DotGenerator.cpp
    src/generator/SyntaxCodeGenerator.cpp
    src/visual/DotExporter.cpp
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.cpp
    src/Engine.cpp
    src/syntax/Grammar.cpp
    src/syntax/GrammarParser.cpp
    src/syntax/LL1.cpp
    src/config/Config.cpp
    app/controllers/RegexController/RegexController.cpp
)
target_link_libraries(GuiTest PRIVATE Qt6::Widgets Qt6::Test)
add_test(NAME 界面测试 COMMAND GuiTest)

add_executable(GrammarParserTest tests/syntax/grammar_parser_test.cpp)
target_sources(GrammarParserTest PRIVATE
    src/syntax/GrammarParser.cpp
    src/syntax/Grammar.cpp
)
target_link_libraries(GrammarParserTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 文法解析测试 COMMAND GrammarParserTest)

add_executable(LL1Test tests/syntax/ll1_test.cpp)
target_sources(LL1Test PRIVATE
    src/syntax/LL1.cpp
    src/syntax/GrammarParser.cpp
    src/syntax/Grammar.cpp
)
target_link_libraries(LL1Test PRIVATE Qt6::Core Qt6::Test)
add_test(NAME LL1算法测试 COMMAND LL1Test)


add_executable(CliRegexTest tests/cli/cli_regex_test.cpp)
target_sources(CliRegexTest PRIVATE
    tests/cli/cli_regex_test.h
    src/visual/DotExporter.cpp
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.cpp
    src/Engine.cpp
    src/syntax/Grammar.cpp
    src/syntax/GrammarParser.cpp
    src/syntax/LL1.cpp
    src/config/Config.cpp
)
target_link_libraries(CliRegexTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 命令行正则测试 COMMAND CliRegexTest)

add_executable(CodegenTest tests/codegen/codegen_compile_run_test.cpp)
target_sources(CodegenTest PRIVATE
    tests/codegen/codegen_compile_run_test.h
    src/visual/DotExporter.cpp
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.cpp
    src/Engine.cpp
    src/syntax/Grammar.cpp
    src/syntax/GrammarParser.cpp
    src/syntax/LL1.cpp
    src/config/Config.cpp
)
target_link_libraries(CodegenTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 代码生成器测试 COMMAND CodegenTest)

add_executable(ConfigWeightTest tests/config/config_weight_test.cpp)
target_sources(ConfigWeightTest PRIVATE
    tests/config/config_weight_test.h
    src/visual/DotExporter.cpp
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/generator/CodeGenerator.cpp
    src/Engine.cpp
    src/syntax/Grammar.cpp
    src/syntax/GrammarParser.cpp
    src/syntax/LL1.cpp
    src/config/Config.cpp
)
target_link_libraries(ConfigWeightTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 权重配置测试 COMMAND ConfigWeightTest)

add_executable(GenDirConfigTest tests/config/gen_dir_config_test.cpp)
target_sources(GenDirConfigTest PRIVATE
    tests/config/gen_dir_config_test.h
    src/config/Config.cpp
)
target_link_libraries(GenDirConfigTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 生成目录配置测试 COMMAND GenDirConfigTest)

add_executable(ConfigGuiPersistTest tests/config/config_gui_persist_test.cpp)
target_sources(ConfigGuiPersistTest PRIVATE
    tests/config/config_gui_persist_test.h
    src/config/Config.cpp
)
target_link_libraries(ConfigGuiPersistTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME 配置持久化测试 COMMAND ConfigGuiPersistTest)

add_executable(DotExportTest tests/automata/dot_export_test.cpp)
target_sources(DotExportTest PRIVATE
    tests/automata/dot_export_test.h
    src/visual/DotExporter.cpp
    src/regex/RegexLexer.cpp
    src/regex/RegexParser.cpp
    src/automata/Thompson.cpp
    src/automata/SubsetConstruction.cpp
    src/automata/Hopcroft.cpp
    src/Engine.cpp
    src/syntax/Grammar.cpp
    src/syntax/GrammarParser.cpp
    src/syntax/LL1.cpp
    src/generator/CodeGenerator.cpp
    src/config/Config.cpp
)
target_link_libraries(DotExportTest PRIVATE Qt6::Core Qt6::Test)
add_test(NAME DOT导出测试 COMMAND DotExportTest)
