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
