# 测试报告

## 环境
- macOS 15.7.2，Qt 6.9.3，CMake 4.2.0，Ninja 1.13.2

## 单元与 UI 自动化
- 目标：`GuiTest`
- 结果：`ctest --test-dir build-macos -V` 全部通过

## 集成测试（TINY）
- 正则：`resources/tiny.regex`
- 源：`resources/sample.tny`
- 步骤：在 GUI 中加载正则，完成转换，生成代码；将源粘贴至测试面板并运行
- 结果：生成编码序列，空白与注释被跳过

## 日志
- 启用：`export QT_LOGGING_RULES="*.debug=true"`
