#include <iostream>
#include <sstream>
#include <utility>
#include "fstream"
#include "vector"
#include "map"
#include "string"
#include "list"
#include "algorithm"

using namespace std;
map<string, vector<string >> FirstSet;
map<string, vector<string >> FollowSet;
map<string, int> NonTerminalMap;
map<string, int> TerminalMap;
int closure_cnt = 0;
int cnt = 0;
bool IsNonTerminal(const string &string1) {
    return (NonTerminalMap.find(string1) != NonTerminalMap.end());
}

struct Production {
    string V;
    vector<string> P;
};

vector<string> split(const string &str) {
    vector<string> t;
    istringstream iss(str);
    string token;
    while (getline(iss, token, ' ')) {
        t.push_back(token);
    }
    return t;
}

vector<Production> Production_Pre_Process(ifstream *file) {
    vector<Production> p_list;
    string p;
    while (getline(*file, p)) {
        Production t;
        t.V = p[0];
        NonTerminalMap.insert(pair<string, int>(p.substr(0, 1), 1));
        t.P = split(p.substr(6));
        for (auto it = t.P.begin(); it != t.P.end(); it++) {
            if ((*it)[0] > 'Z' || (*it)[0] < 'A') {
                TerminalMap.insert(pair<string, int>(*it, 1));
            }
        }
        p_list.push_back(t);

    }
    return p_list;
}


class Item {
public:
    int position_of_dot;
    Production p;
    ulong len;
    bool operator==(const Item& t) const{
        return (p.P==t.p.P && position_of_dot==t.position_of_dot);
    }
    Item(int pod, Production production) {
        position_of_dot = pod;
        p = std::move(production);
        len = p.P.size();
    }
};

vector<Item> Create_Item_List(vector<Production> p_list) {
    vector<Item> item_list;
    for (auto it = p_list.begin(); it != p_list.end(); it++) {
        for (int i = 0; i <= it->P.size(); i++) {
            Item t(i, *it);
            item_list.push_back(t);
        }
    }
    return item_list;
}

class Closure {
public:
//    int num_of_closure{};
    vector<Item> items;
    bool operator==(const Closure &tgt) const{
        return (items==tgt.items);
    }
};

Closure Compute_Closure_I(int num_of_closure, list<Item> target_list, vector<Item> &global_list) {
    Closure c;
    vector<Item> t;
//    c.num_of_closure = num_of_closure;
    for (auto it = target_list.begin(); it != target_list.end(); it++) {
        t.push_back(*it);
        if (it->position_of_dot != it->len) {
            if (IsNonTerminal(it->p.P[it->position_of_dot])) {
                for (auto it_ = global_list.begin(); it_ != global_list.end(); it_++) {
                    if (it_->position_of_dot == 0 && it_->p.V == it->p.P[it->position_of_dot]) {
                        if (IsNonTerminal(it_->p.P[it_->position_of_dot])){
                            if (find(target_list.begin(),target_list.end(),*it_)==target_list.end())
                            target_list.push_back(*it_);
                        } else
                        if (find(t.begin(),t.end(),*it_)==t.end())
                            t.push_back(*it_);
                    }
                }
            }
        }
    }
    c.items = t;
    return c;
}

void GenerateSymbolTable(vector<string>& table){
    for (auto & it : NonTerminalMap){
        table.push_back(it.first);
    }
    for (auto & it : TerminalMap){
        table.push_back(it.first);
    }
}

void Go(Closure i, const string& x, vector<Item>& global, list<Closure>& closure_set) {
    list<Item> t;
    for (auto it = i.items.begin(); it != i.items.end(); it++) {
        if (it->position_of_dot != it->len && it->p.P[it->position_of_dot] == x) {
            Item _t(it->position_of_dot+1, it->p);
            t.push_back(_t);
        }
    }
    if (!t.empty()) {
        Closure ii=Compute_Closure_I(++closure_cnt, t, global);
        auto it = find(closure_set.begin(),closure_set.end(),ii);
        if (it == closure_set.end())
            closure_set.push_back(ii);
    }
}

void Merge(vector<string>& dest, vector<string>& source){
    for (auto it = source.begin();it!=source.end();it++){
        auto it_ = find(dest.begin(),dest.end(),*it);
        if (it_==dest.end()){
            dest.push_back(*it);
        }
    }
}

void Merge(vector<string>&dest, string& source){
    auto it = find(dest.begin(), dest.end(), source);
    if (it == dest.end()){
        dest.push_back(source);
    }
}

void GenerateFirstSet(map<string,vector<string>>& firstset,const string& target, vector<Production>& plist){
    vector<Production> t;
    vector<string > s;
    for(auto it=plist.begin();it!=plist.end();it++){
        if (it->V==target){
            t.push_back(*it);
        }
    }
    for (auto it = t.begin();it!=t.end();it++){
        if (IsNonTerminal(it->P[0])){
            if (it->P[0]!=target) {
                GenerateFirstSet(firstset, it->P[0], plist);
                Merge(s, firstset[it->P[0]]);
            }
            if (it->P.size()>1 && IsNonTerminal(it->P[1])){
                if (it->P[1]!=target){
                    GenerateFirstSet(firstset, it->P[1], plist);
                    Merge(s, firstset[it->P[1]]);
                }
            }
        } else {
            Merge(s, it->P[0]);
        }
    }
    firstset.insert(pair<string, vector<string>>(target, s));
}

void GenerateFollowSet(map<string, vector<string>>& followset, string& target){

}

int main() {
    ifstream file;
    file.open("production.txt");
    if (!file.is_open()) { cout << "error"; }
    vector<string> symbol_table;
    vector<Production> p;
    vector<Item> i;
    p = Production_Pre_Process(&file);
    i = Create_Item_List(p);
    GenerateSymbolTable(symbol_table);
    for (auto it = NonTerminalMap.begin();it!=NonTerminalMap.end();it++){
        GenerateFirstSet(FirstSet,it->first,p);
    }
    Closure i_0, i1;
    list<Item> ll = {i[0]};
    i_0 = Compute_Closure_I(closure_cnt, ll, i);
    list<Closure> g;
    g.push_back(i_0);
    for (auto it = g.begin();it!=g.end();it++){
        cout<<cnt++<<endl;
        for (auto it_ = symbol_table.begin();it_!=symbol_table.end();it_++){
            Go(*it,*it_,i, g);
        }
    }
    cout<<"end";



    return 0;
}
