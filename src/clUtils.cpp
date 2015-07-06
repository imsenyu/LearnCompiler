#include "clUtils.h"
#include <string>
#include <stack>

using namespace std;
int clUtils::atoi(const string& str) {
    int ret = 0;
    for(auto ch : str) {
        ret *= 10;
        ret += ch-'0';
    }
    return ret;
}

string clUtils::itoa(int k) {
    string ret;
    stack<int> stk;
    if ( k==0 ) stk.push(0);
    else {
        while( k ) {
            stk.push( k%10 );
            k/=10;
        }
    }

    while(!stk.empty()) {
        ret.push_back( stk.top()+'0' );
        stk.pop();
    }
    return ret;

}

