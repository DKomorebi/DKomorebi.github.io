---
layout: post
title: 数据结构复习（七）：中缀转后缀与双栈手算
date: 2026-11-23 00:00:00 +0800
categories:
- 数据结构复习
category_path: 课程学习/数据结构/测验复习
description: 数据结构复习（七）：中缀转后缀与双栈手算：结合实际文件、步骤和练习解析。
keywords: 数据结构, C++, 顺序表, 链表, 栈, 队列, 复习
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 原作业支持的输入范围

1113-linkstack.c 把字符 1 到 9 视为操作数，处理 + - * / 的优先级，示例没有括号、空格、多位数或一元负号。它不是完整算术解析器。Precedence 将其他字符也落入默认优先级，因此扩展输入前必须先识别合法 token，不能直接给原程序输入 12 或括号并期待正确。

## 转换时用的是操作符栈

操作数直接加入输出；操作符到来时，弹出优先级更高或同级且应先计算的栈顶操作符，再压入当前操作符。对本篇全为左结合的 + - * /，比较使用 >=。扫描结束后按栈顶顺序排空。

| 扫描 1+2*3 | 操作符栈（底→顶） | 输出 |
| --- | --- | --- |
| 1 | 空 | 1 |
| + | + | 1 |
| 2 | + | 12 |
| * | + * | 12 |
| 3 | + * | 123 |
| 结束 | 空 | 123*+ |

8/4/2 的第二个 / 应先弹出第一个 /，得到 84/2/，结果为 1。若误用严格大于，可能形成 842//，把原式算成 8/(4/2)=4。右结合运算符若未来加入，不能继续无条件套 >=。

## 求值时换成操作数栈

扫描 123*+：压 1、2、3；遇 * 先弹右操作数 b=3，再弹左操作数 a=2，算 a*b=6 压回；遇 + 算 1+6=7。减法和除法特别依赖顺序，不能写 b-a 或 b/a。

1-2-3 的后缀为 12-3-，结果 -4。后缀表达式可以没有括号，但合法性仍需检查：运算前至少两个操作数，结束后必须恰好一个结果，除数不能为零。

## 补充版示例的范围

下面新增的独立教学程序支持单个数字 0…9、空格、括号和四则运算，不支持多位数或一元运算。使用 int 整数运算，例如 9/2 得 4；假定所有运算结果处于 int 可表示范围，本程序不做溢出检测，不能称为通用安全计算器。


下面是独立教学示例的完整代码，采用 C++11。下载后可单独编译。

{% raw %}
````cpp
#include <cassert>
#include <cstdio>
#include <stack>
#include <string>
#include <stdexcept>
// Teaching domain: single digits, + - * /, parentheses, spaces.
// No unary operators or multi-digit numbers; arithmetic must fit int.
int priority(char c) {return c=='*'||c=='/' ? 2 : 1;}
bool op(char c) {return c=='+'||c=='-'||c=='*'||c=='/';}
std::string postfix(const std::string& input) {
    std::stack<char> s;std::string out;bool needOperand=true;
    for(char c:input) {
        if(c==' ')continue;
        if(c>='0'&&c<='9') {
            if(!needOperand)throw std::runtime_error("adjacent operands");
            out+=c;needOperand=false;
        } else if(c=='(') {
            if(!needOperand)throw std::runtime_error("missing operator");
            s.push(c);
        } else if(c==')') {
            if(needOperand)throw std::runtime_error("missing operand");
            while(!s.empty()&&s.top()!='('){out+=s.top();s.pop();}
            if(s.empty())throw std::runtime_error("unmatched parenthesis");
            s.pop();needOperand=false;
        } else if(op(c)) {
            if(needOperand)throw std::runtime_error("missing operand");
            while(!s.empty()&&s.top()!='('&&priority(s.top())>=priority(c)) {
                out+=s.top();s.pop();
            }
            s.push(c);needOperand=true;
        } else throw std::runtime_error("invalid character");
    }
    if(needOperand)throw std::runtime_error("incomplete expression");
    while(!s.empty()) {
        if(s.top()=='(')throw std::runtime_error("unmatched parenthesis");
        out+=s.top();s.pop();
    }
    return out;
}
int evaluate(const std::string& text) {
    std::stack<int> s;
    for(char c:text) {
        if(c>='0'&&c<='9'){s.push(c-'0');continue;}
        if(!op(c)||s.size()<2)throw std::runtime_error("invalid postfix");
        int b=s.top();s.pop();int a=s.top();s.pop();
        if(c=='/'&&b==0)throw std::runtime_error("division by zero");
        s.push(c=='+'?a+b:c=='-'?a-b:c=='*'?a*b:a/b);
    }
    if(s.size()!=1)throw std::runtime_error("invalid result count");
    return s.top();
}
int main() {
    assert(postfix("1+2*3")=="123*+");
    assert(evaluate(postfix("1+2*3"))==7);
    assert(evaluate(postfix("8/4/2"))==1);
    assert(evaluate(postfix("1-2-3"))==-4);
    assert(evaluate(postfix("(1+2)*3"))==9);
    assert(evaluate(postfix("0+9/2"))==4);
    for(const char* bad:{"", "1+", "(1+2", "1+2)", "12+3", "-1", "1**2"}) {
        bool caught=false;
        try { (void)postfix(bad); } catch(const std::runtime_error&) {caught=true;}
        assert(caught);
    }
    bool caught=false;
    try {(void)evaluate(postfix("1/(2-2)"));}
    catch(const std::runtime_error&){caught=true;}
    assert(caught);
    std::puts("expressions: PASS");
}
````
{% endraw %}

[下载 expressions.cpp](/assets/code/ds-review-2026/expressions.cpp)


## 括号与复杂度

左括号作为边界入栈，右括号弹出直到匹配左括号，并丢弃这一对括号。结束还剩左括号或遇右括号找不到对应项都应报错。每个符号最多入栈、出栈各一次，所以转换时间 O(n)、辅助空间 O(n)，不能因为 while 在 for 内就机械判成 O(n²)。

练习先手算：(1+2)*3 → 12+3* → 9；9-4/2 → 942/- → 7；2*3+4*5 → 23*45*+ → 26。再运行样例对照，错了先标记当时的栈顶，不要只改最终答案。

## 系列导航

- [数据结构测验复习导读：按现有作业复习到栈和队列]({% post_url 2026-11-23-ds-review-00-guide %})
- [数据结构复习（一）：逻辑结构、复杂度与 C/C++ 指针]({% post_url 2026-11-23-ds-review-01-foundations %})
- [数据结构复习（二）：从顺序表通讯录理解插入和删除]({% post_url 2026-11-23-ds-review-02-sequence %})
- [数据结构复习（三）：单链表前驱、删除与逆置]({% post_url 2026-11-23-ds-review-03-linked %})
- [数据结构复习（四）：循环链表、双向链表与指针题]({% post_url 2026-11-23-ds-review-04-other-lists %})
- [数据结构复习（五）：顺序栈、链栈与行编辑]({% post_url 2026-11-23-ds-review-05-stack %})
- [数据结构复习（六）：循环队列回绕、判满与链队列]({% post_url 2026-11-23-ds-review-06-queue %})
- [数据结构复习（七）：中缀转后缀与双栈手算]({% post_url 2026-11-23-ds-review-07-expressions %})
- [数据结构复习（八）：模拟练习、答案与考前检查]({% post_url 2026-11-23-ds-review-08-practice %})
