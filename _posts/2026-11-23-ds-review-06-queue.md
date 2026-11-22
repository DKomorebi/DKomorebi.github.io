---
layout: post
title: 数据结构复习（六）：循环队列回绕、判满与链队列
date: 2026-11-23 00:00:00 +0800
categories:
- 数据结构复习
category_path: 课程学习/数据结构/测验复习
description: 数据结构复习（六）：循环队列回绕、判满与链队列：结合实际文件、步骤和练习解析。
keywords: 数据结构, C++, 顺序表, 链表, 栈, 队列, 复习
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 为什么普通数组队列会假溢出

如果 rear 只向数组尾部增长，出队只是 front 增长，前面空出的空间不会复用。rear 到数组末端时，即使前面有空位也不能继续按原方式写入。循环队列用取模让索引回绕，复用这些空间。

## 先固定本篇的约定

front 指向队头元素，rear 指向下一次入队位置，数组长度 M，牺牲一个存储单元。空队 front==rear，满队 (rear+1)%M==front，长度 (rear-front+M)%M，最大有效元素数 M-1。

有些题目使用 count 或额外标志区分空满，能使用全部 M 个单元；那时公式要跟随其定义，不能把本篇判满式套在所有循环队列上。

## 手算一次回绕

M=5，初始 f=0,r=0。入 1、2、3、4 后 f=0,r=4，满；出 1、2 后 f=2,r=4；入 5 后 r=0，再入 6 后 r=1，此时 (1+1)%5==2 再次满。逻辑队列为 3、4、5、6，物理数组顺序已不等于逻辑出队顺序。

继续出四次，f 最终到 1，与 r 相等表示空。不要因为 front 大于 rear 就说长度为负，也不要将两个索引的大小当作空满条件。


下面是独立教学示例的完整代码，采用 C++11。下载后可单独编译。

{% raw %}
````cpp
#include <cassert>
#include <cstdio>
#include <string>
struct Stack {
    int a[4]{}; int top=-1;
    bool push(int x) { if(top==3)return false; a[++top]=x;return true; }
    bool pop(int& x) { if(top==-1)return false;x=a[top--];return true; }
};
struct Queue {
    // One slot is deliberately reserved: capacity is 4, not 5.
    int a[5]{}; int front=0,rear=0;
    int size()const {return (rear-front+5)%5;}
    bool push(int x) {
        if((rear+1)%5==front)return false;
        a[rear]=x;rear=(rear+1)%5;return true;
    }
    bool pop(int& x) {
        if(front==rear)return false;
        x=a[front];front=(front+1)%5;return true;
    }
};
std::string edit(const std::string& input) {
    std::string result;
    for(char c:input) {
        if(c=='#') {if(!result.empty())result.pop_back();}
        else if(c=='@')result.clear();
        else result.push_back(c);
    }
    return result;
}
int main() {
    Stack s;int x=0;assert(!s.pop(x));
    for(int i=1;i<=4;++i)assert(s.push(i));
    assert(!s.push(5));
    for(int i=4;i>=1;--i)assert(s.pop(x)&&x==i);
    assert(!s.pop(x));
    Queue q;assert(!q.pop(x));
    for(int i=1;i<=4;++i)assert(q.push(i));
    assert(!q.push(5)&&q.size()==4);
    assert(q.pop(x)&&x==1);assert(q.pop(x)&&x==2);
    assert(q.push(5));assert(q.push(6));
    for(int i=3;i<=6;++i)assert(q.pop(x)&&x==i);
    assert(q.size()==0&&!q.pop(x));
    assert(edit("ab#cd@xy#z")=="xz");assert(edit("##a")=="a");
    std::puts("stack_queue: PASS");
}
````
{% endraw %}

[下载 stack_queue.cpp](/assets/code/ds-review-2026/stack_queue.cpp)


## 链队列的最后一次出队

常见带头结点链队列维护 front、rear，空时两者指向头结点。入队链接到 rear.next 再移动 rear；出队删除 front.next。若被删的是最后一个数据结点，必须令 rear=front，否则 rear 指向已释放内存，下次入队会损坏结构。

采用头尾指针的链队列入队出队 O(1)。如果只保留头指针且每次遍历到尾再入队，则入队 O(n)，复杂度由实现决定，不由“链”这个名字决定。

## 自测与解析

数组长度 8、f=6、r=2，牺牲一格时长度为 (2-6+8)%8=4。再入队 3 个后长度 7，满；第 4 次入队必须拒绝。若队列满时 f=6，那么 r=5，而不是 6。

栈和队列何时用？撤销、括号匹配等需要最近记录优先时适合栈；按到达次序服务时适合队列。标准库 stack/queue 可以方便验证，但若考查循环队列数组实现，不能只写 STL 调用替代 front/rear 公式。

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
