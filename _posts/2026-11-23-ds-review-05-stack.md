---
layout: post
title: 数据结构复习（五）：顺序栈、链栈与行编辑
date: 2026-11-23 00:00:00 +0800
categories:
- 数据结构复习
category_path: 课程学习/数据结构/测验复习
description: 数据结构复习（五）：顺序栈、链栈与行编辑：结合实际文件、步骤和练习解析。
keywords: 数据结构, C++, 顺序表, 链表, 栈, 队列, 复习
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 对照 correct-sqstack.c 的栈顶约定

top=-1 表示空栈，top 指向当前栈顶元素，所以元素数 top+1，满栈 top==Max-1。另一种实现让 top 指向下一空位，空栈 top=0，满栈 top==Max。两种都正确，但判空、入栈和出栈表达式不能混用。


课程文件：`单链表通讯录作业/118和1115作业/correct-sqstack.c`（原代码节选）。

{% raw %}
````cpp
bool Push(SqStack *S, int e){
    if (S->top + 1 >= S->Max)
        return false;

    S->data[++S->top] = e;
    return true;
}

bool Pop(SqStack *S){
    if (S->top == -1)
        return false;

    S->top--;
    return true;
}
````
{% endraw %}


`data[++top]=e` 先增加栈顶再写入，第一次写下标 0。Pop 只减少 top，没有返回弹出的值；它适合行编辑中“删除末尾字符”的调用。如果题目要求取值，应额外输出 data[top]，并在减少 top 前完成读取。

## 行编辑的手算

原程序普通字符入栈，# 删除最后一个字符，@ 清空当前行，换行时按栈底到栈顶输出。对 ab#cd@xy#z：依次 a、ab、a、ac、acd、空、x、xy、x、xz，所以最终行为 xz。这里没有用反复 Pop 输出，因为那会倒序。

原文件 getchar 的结果保存在 int，能够区分字符值与 EOF。它只在读到换行时输出，若 EOF 前最后一行没有换行，该缓冲内容没有被输出。固定 100 容量，Push 失败未提示；InitStack 也应考虑分配失败。这些是当前代码的边界，不在本次擅自修改原作业。

## 链栈把插删放在哪一端

1113-linkstack.c 使用动态分配的头结点，Push 在头结点后头插，Pop 删除首数据结点，两者都 O(1)。它的 DestroyStack 从头指针开始释放，包含动态头结点；不要与通讯录的局部 head 混淆。

原 Pop 和 GetTop 本身不判空，依赖调用方前提。合法表达式演示可以满足部分前提，但非法表达式可能使操作数栈不足。面向通用输入时应增加失败返回或异常，而不是直接解引用空指针。

## 出栈序列判断

输入按 1、2、3、4 顺序入栈，输出可与入栈穿插。3、2、4、1 可行：入 1、2、3，出 3、2，入 4，出 4、1。3、1、2、4 不可行：弹出 3 后栈顶是 2，它挡住 1，不能跳过 2。

只有“全部先入栈，再全部出栈”才必须是完全逆序。允许穿插时存在多种合法输出。每次想输出某元素，先看它是不是当前栈顶；不是则只能继续按输入顺序入栈，不能任意调整。

## 独立示例入口

第六篇给出 stack_queue.cpp 完整程序，前半部分实现容量 4 的顺序栈，并验证空栈、满栈、逆序弹出及行编辑。行编辑辅助函数使用 string 保存字符，便于单独验证状态，与原程序的固定容量和换行 IO 边界分别说明。

练习：top=-1，push 10、push 20、pop、push 30，最终 top=1、栈内 [10,30]。若改用下一空位约定，最终 top=2；数值不同不代表元素不同。

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
