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
