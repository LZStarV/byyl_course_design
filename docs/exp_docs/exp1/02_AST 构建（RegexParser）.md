# 02 AST 构建（RegexParser）

## 数据结构

表 1 解析相关结构与中间变量
对象或变量名称 功能 存储结构
ASTNode 抽象语法树节点（类型/值/子节点） 结构体
ASTNode::Type 节点类型枚举（Concat/Union/Star/Plus/Question/CharSet/Symbol/Ref） 枚举
QVector<ASTNode*> children 子节点列表 容器
QString value 节点承载的字符或集合 字符串
ParsedToken 单个 Token 的规则与 AST 结构体
ParsedFile 整体产物（宏/Token/字母表） 结构体
Alphabet 统一字母表集合 自定义类型
RegexParser::parseExpr/parseConcat/parseFactor/parseAtom 递归下降子过程 函数
parseCharset 字符类解析（区间展开与集合承载） 函数
expand/simplify 宏引用展开与模式规约 函数
collectAlphabet(ASTNode*, Alphabet&, macros) 从 AST 收集字母表 函数

## 算法实现过程

1. 输入归一化与入口：以规则文件为输入，逐 Token 文本进入 `RegexParser::parse`，内部初始化 `ParsedFile` 并准备统一 `Alphabet`。
2. 递归下降解析（优先级）：按“原子→后缀→连接→选择”的优先级，自顶向下调用 `parseExpr/parseConcat/parseFactor/parseAtom`，相邻原子隐式折叠为 `Concat`，`|` 以最低优先级在 `parseExpr` 中折叠为 `Union`。
3. 转义处理与字符类：`\\` 转义字符按字面处理；`parseCharset` 对 `[ranges]` 展开为 `CharSet`，集合内容以项目实现中的字符串形式承载至 `ASTNode.value`。
4. 后缀一元节点：在读取原子后，如紧随 `*`/`+`/`?`，将其包裹为 `Star/Plus/Question` 节点，保持与后续自动机构造的语义一致。
5. 引用与宏展开：对命名引用 `Ref`（如 `_NAME123`）进行占位解析，随后在 `expand` 阶段替换为目标 AST，使复用定义在树结构中得到正确展开。
6. 模式规约与简化：对特定组合（如 `Star(Union(...))`）在 `simplify` 中进行规约，减少后续构造中的冗余结构。
7. 字母表收集：调用 `collectAlphabet(ast, alpha, macros)`，将树中出现的字符或集合统一加入 `Alphabet`，为后续自动机与生成器使用做准备。
8. 汇总输出：将 `ParsedToken{rule, ast}` 追加到 `ParsedFile.tokens`，并返回包含宏、Token 与字母表的 `ParsedFile`。

下图展示一棵包含连接、选择与后缀运算的典型 AST：
<!--【示意图】此处需要添加 'AST 示例图'：展示包含连接/选择/后缀的一棵表达式树。 -->

- 常用辅助构造：
  - 通过 `makeConcat/makeCharSet/makeSymbol` 等辅助函数快速构建节点。

## 各种情况讲解

- 空子表达式：`()` 产生空，需避免无效节点；
- 嵌套括号：递归解析并维护深度；
- 关键字大小写不敏感：示例 `makeKeywordCI` 按字符逐个构建大小写类并连接。

## 关键代码片段（含注释）

```
// 解析入口：将规则文件解析为 AST，并汇总为 ParsedFile
class RegexParser { public: static ParsedFile parse(const RegexFile& file); };

// 构造一个符号节点（单字符）
static ASTNode* makeSymbol(QChar ch)
{
    auto n = new ASTNode();
    n->type  = ASTNode::Symbol;
    n->value = QString(ch);
    return n;
}

// 构造一个连接节点（左子树接右子树）
static ASTNode* makeConcat(ASTNode* a, ASTNode* b)
{
    auto n = new ASTNode();
    n->type     = ASTNode::Concat;
    n->children = {a, b};
    return n;
}

// 构造一个字符集节点（由多个字符组成的集合）
static ASTNode* makeCharSet(const QString& chars)
{
    auto n = new ASTNode();
    n->type  = ASTNode::CharSet;
    n->value = chars; // 项目实现：以字符串形式承载集合内容（字符类）
    return n;
}

// 解析一个原子：字符/字符类/括号子式/引用
static ASTNode* parseAtom(/* 输入流与状态 */)
{
    // ... 从输入读取一个原子并返回对应 ASTNode（字符、字符类、括号、Ref）
}

// 折叠隐式连接：相邻原子按 Concat 组合
static ASTNode* parseConcat(/* 上下文 */)
{
    // ... 读取多个相邻原子，逐步用 makeConcat 进行折叠
}
```
