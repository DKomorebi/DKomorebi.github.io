---
layout: post
title: 数据结构复习（二）：从顺序表通讯录理解插入和删除
date: 2026-11-23 00:00:00 +0800
categories:
- 数据结构复习
category_path: 课程学习/数据结构/测验复习
description: 数据结构复习（二）：从顺序表通讯录理解插入和删除：结合实际文件、步骤和练习解析。
keywords: 数据结构, C++, 顺序表, 链表, 栈, 队列, 复习
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 对照原作业的三个字段

address_book.cpp 中 SL 包含 a、size、capacity。a 指向一段一次分配 100 人的空间，size 初始为 0；这个版本没有自动扩容，所以不能把它说成容量不足会自行增长的 vector。


课程文件：`通讯录作业/address_book.cpp`（原代码节选）。

{% raw %}
````cpp
void SeqListInsert(SL* ps, int pos, SQDataType x){
    assert(pos >= 0 && pos <= ps->size);
    assert(ps->size < ps->capacity);
    for(int i = ps->size - 1; i >= pos; i--){
        ps->a[i + 1] = ps->a[i];
    }
    ps->a[pos] = x;
    ps->size++;
}
````
{% endraw %}


## 插入为什么从后往前搬

初始 [10,20,30,40]，在下标 1 插入 99。先将 40 搬到 4，再将 30 搬到 3，再将 20 搬到 2，最后写 a[1]=99，得到 [10,99,20,30,40]。若从前往后依次覆盖，后面的原值可能尚未读取就丢失。

长度 n、插入下标 pos 时移动 n-pos 个元素。表尾插入移动零次，在容量足够的本版本为 O(1)；表头插入移动 n 次，为 O(n)。等概率选择 n+1 个插入位置时平均移动 n/2 次。

## 删除为什么从前往后搬

删除下标 1 的 20，依次将 30、40 前移，size 减一，得到 [10,30,40]。数组最后一个物理单元可能仍保留旧值，但它已在有效区间外；不要因为未清零就断言元素还存在。

删除下标 pos 移动 n-pos-1 次。等概率删除一个有效元素时平均 (n-1)/2 次。按姓名删除还要先做线性查找，整体最坏 O(n)。

## assert 与普通返回值的区别

原插入使用 assert 检查下标与容量，适合课堂验证前提，但发布模式可能禁用断言。下面独立示例使用 bool 返回非法操作，并在 main 中用断言验证行为。它缩小容量为 5，是为了容易触发满表，不是替换原联系人数据类型。


下面是独立教学示例的完整代码，采用 C++11。下载后可单独编译。

{% raw %}
````cpp
#include <cassert>
#include <cstdio>
struct SeqList { int a[5]{}; int size=0; };
bool insert(SeqList& s,int pos,int value) {
    if(pos<0 || pos>s.size || s.size==5) return false;
    for(int i=s.size;i>pos;--i) s.a[i]=s.a[i-1];
    s.a[pos]=value; ++s.size; return true;
}
bool erase(SeqList& s,int pos,int& removed) {
    if(pos<0 || pos>=s.size) return false;
    removed=s.a[pos];
    for(int i=pos;i+1<s.size;++i) s.a[i]=s.a[i+1];
    --s.size; return true;
}
int find(const SeqList& s,int value) {
    for(int i=0;i<s.size;++i) if(s.a[i]==value) return i;
    return -1;
}
int main() {
    SeqList s; int out=0;
    assert(!erase(s,0,out)); assert(!insert(s,1,99));
    assert(insert(s,0,10)); assert(insert(s,1,30));
    assert(insert(s,1,20)); assert(s.a[1]==20);
    assert(insert(s,3,40)); assert(insert(s,4,50));
    assert(!insert(s,5,60)); assert(find(s,30)==2);
    assert(erase(s,0,out) && out==10 && s.a[0]==20);
    assert(erase(s,s.size-1,out) && out==50);
    while(s.size) assert(erase(s,0,out));
    assert(find(s,99)==-1 && !erase(s,0,out));
    std::puts("seq_list: PASS");
}
````
{% endraw %}

[下载 seq_list.cpp](/assets/code/ds-review-2026/seq_list.cpp)


## 重点边界

空表可以在 0 插入，不能在 0 删除；长度 n 可在 n 插入，不能在 n 删除；满表即使 pos 合法也不能越界写。读取失败时输出参数 removed 不应被当成有效删除结果。

练习：长度 6 的表在下标 2 插入，移动 4 次；删除原表下标 2，移动 3 次。请先数有效元素，不要把 capacity=100 代入 n。再解释一次“尾插 O(1)”的前提：容量已足够，且不包含外部查找操作。

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
