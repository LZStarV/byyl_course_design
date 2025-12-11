# 03 Thompson 方法转 NFA

## 数据结构

表 1 关键存储结构与中间变量
对象或变量名称 功能 存储结构
NFA / NFAState / NFAEdge 自动机、状态与边 结构体
Alphabet 字母表集合 自定义类型
ASTNode* ast 当前递归处理的节点 指针
QPair<int,int> p / a / b 片段头尾编号对（first/second） QPair<int,int>
NFAEdge e 构造边的临时变量 结构体
int nextId 合并片段时的下一可用状态编号 int
int mappedStart 合并片段时的起始态映射 int
int globalStart 合并所有 Token 的统一起始态 int
bool epsilon 是否 ε 转移 bool
QString symbol 边上的符号（单字符或标签） QString

## 算法实现过程

1. 初始化与输入准备：从正则表达式的 AST 根节点开始，创建空的 `NFA nfa` 与字母表 `Alphabet alpha`。这一步不做任何边或状态的实际连接，仅作为后续构造的容器与符号登记处。
2. 原子片段构造（两节点一边）：遇到单字符时，先调用 `newFrag(nfa)` 生成一对“头/尾”状态；随后在两者之间添加一条非 ε 的字符边，并将该字符登记到 `alpha`。这一模式是所有复杂结构的基本骨架，便于在组合时进行 ε 拼接。
3. 连接（Concat，顺序拼接）：对左、右子式分别递归生成片段 `a` 与 `b`。随后用一条 ε 边把 `a` 的尾连接到 `b` 的头，并取消 `a.second` 的接受标记（因为整体接受由 `b` 的尾决定）。结果片段的头来自 `a`，尾来自 `b`。
4. 选择（Union，分叉与汇合）：为两个分支分别生成片段后，再新建一对头尾 `p`。从新头以两条 ε 边分别指向两个分支的头；从两个分支的尾以 ε 边指向新尾，并清除分支尾的接受标记。这样得到一个“Y 形”结构，在图上体现为分叉进入与汇合退出。
5. 闭包（Star，零次或多次）：生成分支片段 `a` 后，新建 `p={head,tail}` 作为闭包的外层。通过四条 ε 边实现“可跳过”和“可循环”：新头到分支头、新头到新尾、分支尾到新尾、分支尾回到分支头；并取消分支尾的接受标记，使整体接受由新尾统一管理。
6. 正闭包（Plus，至少一次）：可视作“一次连接 + 闭包”。先确保至少经过一次分支（等价于一次 `Concat`），再附加闭包的循环与跳过结构，使得匹配次数下限为 1。
7. 可选（Question，是否出现一次）：等价于 `Union(ε, X)`。通过为 `ε` 与目标子式各开一条分支，实现“出现一次或不出现”的语义，最终由新尾作为统一的接受出口。
8. 字符类（CharSet，多边并行）：在同一对 `p={head,tail}` 上，为类内每个字符各添加一条非 ε 边；如有需要也可以合并标签的方式归并到一个符号集合。所有出现的字符或标签都登记到 `alpha`。
9. 引用（Ref，宏与子式替换）：当节点为命名引用时，先替换为其目标 AST，再按上述规则参与构造。这一步保证复用与层次化定义在自动机构造中被正确展开。
10. 多 Token 合并（统一入口）：当需要将多个 Token 的 NFA 放入同一个图中时，使用偏移与映射（维护 `nextId` 与 `mappedStart`）逐个追加，并以统一的 `globalStart` 通过 ε 边引入各片段，形成“星型”总 NFA，便于后续进行子集构造（DFA 化）。
11. 结果输出与登记：返回包含起始与接受态标记的 `nfa`，此时字母表 `alpha` 已同步包含所有在构造过程中出现的符号，为后续状态表生成与代码导出做准备。

## 运算符操作逻辑（逐项说明）

1) 单字符/空串转移（含 ε）
- 新建起始与结束两个状态；按字符或 ε 建立一条边。
- `<!--【NFA 图】此处需要添加 '单字符/ε 转移图'：两节点一边，标注符号或 ε。-->`

2) 连接（Concat）
- 将前一片段的所有接受态用 ε 指向后一片段的起始态；新的起始为前片段头，新的接受为后片段尾。
- `<!--【NFA 图】此处需要添加 '连接运算 NFA 图'：片段拼接示意，ε 边连接。-->`

3) 选择（Union）
- 新建头，ε 指向两个分支头；新建尾，两个分支尾 ε 指向新尾；
- `<!--【NFA 图】此处需要添加 '选择运算 NFA 图'：分叉与汇合。-->`

4) 闭包（Star）
- 新建头与尾：头 ε→ 分支头、头 ε→ 尾；分支尾 ε→ 分支头（循环）、分支尾 ε→ 尾（退出）。
- `<!--【NFA 图】此处需要添加 '闭包运算 NFA 图'：自环与出口。-->`

5) 正闭包（Plus）
- 等价于“一次连接 + 闭包”：先构造一次分支，再接 `Star` 结构。
- `<!--【NFA 图】此处需要添加 '正闭包 NFA 图'：至少一次进入。-->`

6) 可选（Question）
- 等价 `Union(ε, X)`：在 X 与 ε 之间二选一。
- `<!--【NFA 图】此处需要添加 '可选运算 NFA 图'：ε 分支与常规分支。-->`

7) 字符类（CharSet）
- 对字符类中的每个字符从同一头到同一尾建立一条边，或以合并标签表示。
- `<!--【NFA 图】此处需要添加 '字符类转移图'：多边并行。-->`

8) 引用（Ref）
- 将命名引用替换为目标 AST 或直接拼装为对应 NFA 片段。

### 合并与偏移

- 将多个 NFA 合并到一个图时，需对状态 id 进行偏移与映射，保证合并后的图编号唯一且边关系保持；
- 合并构造：为每个 Token 的 NFA 从统一起始态以 ε 引入，形成“星型”总 NFA，便于后续子集构造。

下图为“Thompson 构造流程图”占位：
<!--【流程图】此处需要添加 'Thompson 构造流程图'：递归分派与片段组合流程。 -->

## 关键代码片段（含注释）

```
// Thompson 构造的入口声明：从 AST 构建等价的 NFA
class Thompson { public: static NFA build(ASTNode* ast, Alphabet alpha); };

// 新建一个最小片段：仅包含“头/尾”两个状态，尾标记为接受
static QPair<int,int> newFrag(NFA& nfa) {
    int s = nfa.states.size() + 1;
    int t = s + 1;
    NFAState ss; ss.id = s;
    NFAState tt; tt.id = t; tt.accept = true;
    nfa.states.insert(s, ss);
    nfa.states.insert(t, tt);
    return {s, t};
}

// 按节点类型递归构造片段，并用 ε 边进行组合拼装
static QPair<int,int> buildFrom(NFA& nfa, ASTNode* ast) {
    if (!ast) return {0, 0};
    // 原子：单字符，头到尾一条非 ε 字符边
    if (ast->type == ASTNode::Symbol) {
        auto p = newFrag(nfa);
        NFAEdge e; e.to = p.second; e.epsilon = false; e.symbol = ast->value;
        nfa.states[p.first].edges.push_back(e);
        return p;
    }
    // 原子：字符类，同一头/尾为类中每个字符添加一条非 ε 边
    if (ast->type == ASTNode::CharSet) {
        auto p = newFrag(nfa);
        for (auto ch : ast->value) {
            NFAEdge e; e.to = p.second; e.epsilon = false; e.symbol = QString(ch);
            nfa.states[p.first].edges.push_back(e);
        }
        return p;
    }
    // 组合：连接，左尾 ε→ 右头；整体接受由右尾决定
    if (ast->type == ASTNode::Concat) {
        auto a = buildFrom(nfa, ast->children[0]);
        auto b = buildFrom(nfa, ast->children[1]);
        NFAEdge e; e.to = b.first; e.epsilon = true;
        nfa.states[a.second].edges.push_back(e);
        nfa.states[a.second].accept = false;
        return {a.first, b.second};
    }
    // 组合：选择，新头 ε→ 两分支头；分支尾 ε→ 新尾
    if (ast->type == ASTNode::Union) {
        auto a = buildFrom(nfa, ast->children[0]);
        auto b = buildFrom(nfa, ast->children[1]);
        auto p = newFrag(nfa);
        NFAEdge e1; e1.epsilon = true; e1.to = a.first; nfa.states[p.first].edges.push_back(e1);
        NFAEdge e2; e2.epsilon = true; e2.to = b.first; nfa.states[p.first].edges.push_back(e2);
        NFAEdge e3; e3.epsilon = true; e3.to = p.second; nfa.states[a.second].edges.push_back(e3);
        NFAEdge e4; e4.epsilon = true; e4.to = p.second; nfa.states[b.second].edges.push_back(e4);
        nfa.states[a.second].accept = false;
        nfa.states[b.second].accept = false;
        return p;
    }
    // 组合：闭包，新头可跳过到新尾；分支尾既可退出到新尾也可回到分支头
    if (ast->type == ASTNode::Star) {
        auto a = buildFrom(nfa, ast->children[0]);
        auto p = newFrag(nfa);
        NFAEdge e1; e1.epsilon = true; e1.to = a.first; nfa.states[p.first].edges.push_back(e1);
        NFAEdge e2; e2.epsilon = true; e2.to = p.second; nfa.states[p.first].edges.push_back(e2);
        NFAEdge e3; e3.epsilon = true; e3.to = p.second; nfa.states[a.second].edges.push_back(e3);
        NFAEdge e4; e4.epsilon = true; e4.to = a.first; nfa.states[a.second].edges.push_back(e4);
        nfa.states[a.second].accept = false;
        return p;
    }
    // ...（Plus：至少一次；Question：Union(ε, X)；Ref：替换为目标 AST 后复用上述逻辑）
    return newFrag(nfa);
}
```

## 单元测试

输入：
```
_tok100 = a | b
```

预测结果：
- 以 Thompson 构造得到的 NFA 包含 ε 边（选择分叉与汇合需要 ε 转移）；
- 至少一个接受态存在。

测试结果：
- 结构检查通过：检测到 ε 边与接受态计数 ≥ 1；
- GUI 截图：自动机核心单元测试（FACoreTest）运行截图（含 NFA 结构检查输出）。
<!--【GUI 截图】此处需要添加 'FACoreTest 运行截图'：显示 NFA 结构检查结果。 -->
下图展示“选择运算”的 NFA 结构示意：
<!--【NFA 图】此处需要添加 '选择运算 NFA 图'：显示新头 ε→ 两分支头，分支尾 ε→ 新尾。 -->
