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
