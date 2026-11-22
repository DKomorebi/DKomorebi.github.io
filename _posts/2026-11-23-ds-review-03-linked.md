---
layout: post
title: 数据结构复习（三）：单链表前驱、删除与逆置
date: 2026-11-23 00:00:00 +0800
categories:
- 数据结构复习
category_path: 课程学习/数据结构/测验复习
description: 数据结构复习（三）：单链表前驱、删除与逆置：结合实际文件、步骤和练习解析。
keywords: 数据结构, C++, 顺序表, 链表, 栈, 队列, 复习
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 原作业的头结点是谁创建的

address_book_linked.cpp 的 main 中定义 SLNode head，是局部对象；SListInit 只令 head.next=NULL。它不是 malloc 创建的，因此销毁函数只释放动态联系人结点，不能 free(&head)。头结点不保存有效联系人、不计入人数。

链表头指针是一个变量，头结点是一个结点，首元结点是第一个有效元素。这三个名词不同。带头结点空表是 head.next==NULL；无头结点链表的空指针判断则通常是 head==NULL。


课程文件：`单链表通讯录作业/address_book_linked.cpp`（原代码节选）。

{% raw %}
````cpp
void SListDelete(SLNode* head, const char* name){
    SLNode* p = head;
    while(p->next != NULL){
        if(strcmp(p->next->data.name, name) == 0){
            SLNode* q = p->next;
            p->next = q->next;
            free(q);
            return;
        }
        p = p->next;
    }
}
````
{% endraw %}


## 删除时为什么寻找前驱

p 是待删除结点的前驱，q=p->next 是真正要删的结点。先令 p->next=q->next，使链条跳过 q，再 free(q)。若先释放 q 再读 q->next，就是释放后访问。若仅执行 p=q->next，只改变局部游标，不会改变链表的连接。

头结点使删除第一位联系人也遵循同样步骤。原函数找到第一个同名联系人便 return，所以它删除首个匹配项，不是所有重复姓名。

## 三个常见复杂度判断

已知前驱，插删连接 O(1)；按姓名查找前驱 O(n)；原作业尾插没有尾指针，需要遍历，单次 O(n)。增加尾指针可让尾插 O(1)，但必须同步维护空表、删尾和清空等状态。

## 逆置要先保留下一个位置

下面采用三个指针：previous 表示已经逆置的前缀，current 是待处理结点，following 暂存旧后继。把 current.next 指回 previous，再前进两根游标。循环结束把 head.next 指向新首元。


下面是独立教学示例的完整代码，采用 C++11。下载后可单独编译。

{% raw %}
````cpp
#include <cassert>
#include <cstdio>
struct Node { int data; Node* next; };
void pushFront(Node& head,int value) {
    head.next=new Node{value,head.next};
}
bool eraseFirst(Node& head,int value) {
    Node* prev=&head;
    while(prev->next && prev->next->data!=value) prev=prev->next;
    if(!prev->next) return false;
    Node* victim=prev->next;
    prev->next=victim->next;
    delete victim; return true;
}
void reverse(Node& head) {
    Node* current=head.next;
    Node* previous=nullptr;
    while(current) {
        Node* following=current->next;
        current->next=previous;
        previous=current;
        current=following;
    }
    head.next=previous;
}
void clear(Node& head) {
    while(head.next) {
        Node* victim=head.next;
        head.next=victim->next;
        delete victim;
    }
}
int main() {
    Node head{0,nullptr};
    assert(!eraseFirst(head,1)); reverse(head);
    pushFront(head,3); pushFront(head,2); pushFront(head,1);
    assert(head.next->data==1);
    assert(eraseFirst(head,1)); assert(eraseFirst(head,3));
    assert(head.next->data==2 && head.next->next==nullptr);
    assert(eraseFirst(head,2) && !head.next);
    pushFront(head,3); pushFront(head,2); pushFront(head,1);
    reverse(head);
    assert(head.next->data==3 && head.next->next->next->data==1);
    clear(head); clear(head); assert(!head.next);
    std::puts("linked_list: PASS");
}
````
{% endraw %}

[下载 linked_list.cpp](/assets/code/ds-review-2026/linked_list.cpp)


## 手算 1→2→3 的逆置

开始 previous=NULL、current=1。第一轮得到 1→NULL，current=2；第二轮得到 2→1→NULL，current=3；第三轮得到 3→2→1→NULL，current=NULL。头结点最后接到 3。每个结点处理一次，时间 O(n)，额外指针空间 O(1)。

示例用 new/delete 管理动态结点，和原作业 malloc/free 是两套配对方式；不要把示例的 delete 直接换进原函数而仍用 malloc 创建结点。head 本身仍是局部对象。

## 自测

删唯一结点后 head.next 应为 NULL；删不存在值应保持结构；重复 clear 应安全；头插输入 1、2、3 得到 3、2、1。若需要保持输入顺序，应尾插或在最后逆置。

为什么“已知待删结点指针就一定 O(1) 删除”不够准确？单链表一般需要前驱才能改变前向链接；通过复制后继的特殊技巧存在尾结点和对象身份限制，不能作为通用删除定理。

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
