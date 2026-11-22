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
