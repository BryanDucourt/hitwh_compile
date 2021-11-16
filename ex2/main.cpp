#include <iostream>
#include <sstream>
#include <utility>
#include "fstream"
#include "vector"
#include "map"
#include "string"

using namespace std;

map<string, int> NonTerminalMap;
map<string, int> TerminalMap;
int closure_cnt = 0;
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
    int num_of_closure{};
    vector<Item> items;
};

Closure Compute_Closure_I(int num_of_closure, vector<Item> target_list, vector<Item> &global_list) {
    Closure c;
    vector<Item> t;
    c.num_of_closure = num_of_closure;
    for (auto it = target_list.begin(); it != target_list.end(); it++) {
        t.push_back(*it);
        if (it->position_of_dot != it->len) {
            if (IsNonTerminal(it->p.P[it->position_of_dot])) {
                for (auto it_ = global_list.begin(); it_ != global_list.end(); it_++) {
                    if (it_->position_of_dot == 0 && it_->p.V == it->p.P[it->position_of_dot]) {
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

void Go(Closure i, const string& x, vector<Item>& global, vector<Closure>& closure_set) {
    vector<Item> t;
    for (auto it = i.items.begin(); it != i.items.end(); it++) {
        if (it->position_of_dot != it->len && it->p.P[it->position_of_dot] == x) {
            Item _t(it->position_of_dot+1, it->p);
            t.push_back(_t);
        }
    }
    if (!t.empty())
        closure_set.push_back(Compute_Closure_I(++closure_cnt,t,global));

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
    Closure c;
    vector<Item> ll = {i[0]};
    c = Compute_Closure_I(closure_cnt, ll, i);
    vector<Closure> g;
    Go(c,"D",i, g);

    return 0;
}
