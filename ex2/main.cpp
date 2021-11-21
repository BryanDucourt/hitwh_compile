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
vector<map<string,pair<string ,int>>> ActionTable;
vector<map<string,pair<string ,int>>> GotoTable;
vector<pair<int, pair<string, int>>> cache;
int closure_cnt = 0;
int cnt = 0;

bool IsNonTerminal(const string &string1) {
    return (NonTerminalMap.find(string1) != NonTerminalMap.end());
}

struct Production {
    string V;
    vector<string> P;
};
bool operator == (const Production& p,const Production& q){
    if (p.V==q.V&&p.P==q.P){
        return true;
    }
    return false;
}
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

    bool operator==(const Item &t) const {
        return (p.P == t.p.P && position_of_dot == t.position_of_dot);
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
    int num_of_closure{};
    vector<Item> items;

    bool operator==(const Closure &tgt) const {
        return (items == tgt.items);
    }
};

Closure Compute_Closure_I(int num_of_closure, list<Item> target_list, vector<Item> &global_list) {
    Closure c;
    vector<Item> t;
    c.num_of_closure = num_of_closure;
    for (auto it = target_list.begin(); it != target_list.end(); it++) {
        t.push_back(*it);
        if (it->position_of_dot != it->len) {
            if (IsNonTerminal(it->p.P[it->position_of_dot])) {
                for (auto it_ = global_list.begin(); it_ != global_list.end(); it_++) {
                    if (it_->position_of_dot == 0 && it_->p.V == it->p.P[it->position_of_dot]) {
                        if (IsNonTerminal(it_->p.P[it_->position_of_dot])) {
                            if (find(target_list.begin(), target_list.end(), *it_) == target_list.end())
                                target_list.push_back(*it_);
                        } else if (find(t.begin(), t.end(), *it_) == t.end())
                            t.push_back(*it_);
                    }
                }
            }
        }
    }
    c.items = t;
    return c;
}

void GenerateSymbolTable(vector<string> &table) {
    for (auto &it: NonTerminalMap) {
        table.push_back(it.first);
    }
    for (auto &it: TerminalMap) {
        table.push_back(it.first);
    }
}

void Go(Closure i, const string &x, vector<Item> &global, list<Closure> &closure_set) {
    list<Item> t;
    for (auto it = i.items.begin(); it != i.items.end(); it++) {
        if (it->position_of_dot != it->len && it->p.P[it->position_of_dot] == x) {
            Item _t(it->position_of_dot + 1, it->p);
            t.push_back(_t);
        }
    }
    if (!t.empty()) {
        Closure ii = Compute_Closure_I(++closure_cnt, t, global);
        auto it = find(closure_set.begin(), closure_set.end(), ii);
        if (it == closure_set.end())
            closure_set.push_back(ii);
        else closure_cnt--;
        cache.emplace_back(i.num_of_closure, pair<string, int>(x, ii.num_of_closure));
    }
}

void Merge(vector<string> &dest, vector<string> &source) {
    for (auto it = source.begin(); it != source.end(); it++) {
        auto it_ = find(dest.begin(), dest.end(), *it);
        if (it_ == dest.end()) {
            dest.push_back(*it);
        }
    }
}

void Merge(vector<string> &dest, string &source) {
    auto it = find(dest.begin(), dest.end(), source);
    if (it == dest.end()) {
        dest.push_back(source);
    }
}

void GenerateFirstSet(map<string, vector<string>> &firstset, const string &target, vector<Production> &plist) {
    vector<Production> t;
    vector<string> s;
    for (auto it = plist.begin(); it != plist.end(); it++) {
        if (it->V == target) {
            t.push_back(*it);
        }
    }
    for (auto it = t.begin(); it != t.end(); it++) {
        if (IsNonTerminal(it->P[0])) {
            if (it->P[0] != target) {
                GenerateFirstSet(firstset, it->P[0], plist);
                Merge(s, firstset[it->P[0]]);
            }
            if (it->P.size() > 1 && IsNonTerminal(it->P[1])) {
                if (it->P[1] != target) {
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

void GenerateFollowSet(map<string, vector<string>> &followset, const string &target, vector<Production> &plist) {
    vector<Production> target_productions;
    vector<string> results;
    if (target == "P") {
        string s = "$";
        Merge(results, s);
    }
// 挑选所有包含目标符号的产生式
    for (auto it = plist.begin(); it != plist.end(); it++) {
        auto t_it = find(it->P.begin(), it->P.end(), target);
        if (t_it != it->P.end()) {
            target_productions.push_back(*it);
        }
    }

    for (auto it = target_productions.begin(); it != target_productions.end(); it++) {
        //在产生式中搜索target的位置
        auto t_it = find(it->P.begin(), it->P.end(), target);
        //产生式形如αAB/αAβ
        if (t_it - it->P.begin() < it->P.size() - 1) {
            //下一个符号为非终结符号--αAB
            if (IsNonTerminal(*(t_it + 1))) {
                //将B的first集中所有元素放入follow A中
                vector<string> tmp = FirstSet[*(t_it + 1)];
                //删除epsilon
                auto tt_it = find(tmp.begin(), tmp.end(), "e");
                if (tt_it != tmp.end()) {
                    tmp.erase(tt_it);
                    //将follow A中的元素全部放入follow B中
                    if (followset.find(it->V) == followset.end())
                        GenerateFollowSet(followset, it->V, plist);
                    Merge(results, followset[it->V]);
                }
                Merge(results, tmp);
            } else {
                // 下一个符号为终结符号--αAβ
                Merge(results, *(t_it + 1));
            }
        } else {
            if (it->V != target) {
                if (followset.find(it->V) == followset.end()) {
                    GenerateFollowSet(followset, it->V, plist);
                }
                Merge(results, followset[it->V]);
            }
        }
    }
    followset.insert(pair<string, vector<string>>(target, results));
}

int SearchCache(int i, const string &x) {
    for (auto it = cache.begin(); it != cache.end(); it++) {
        if (it->first == i && it->second.first == x) {
            return it->second.second;
        }
    }
    return 0;
}

void GenerateAnalyzeTable(Closure &c, int no, vector<Item> &global,vector<Production>& productions) {
    map<string, pair<string ,int>> p,q;
    ActionTable.push_back(p);
    GotoTable.push_back(q);
    for (auto it = c.items.begin(); it != c.items.end(); it++) {
        if (it->position_of_dot != it->len) {
            string tmp = it->p.P[it->position_of_dot];
            int i = SearchCache(c.num_of_closure, tmp);
            if (!IsNonTerminal(tmp)) {
                ActionTable[c.num_of_closure][tmp] = pair("S",i);
            } else {
                GotoTable[c.num_of_closure][tmp] = pair("S", i);
            }
        } else {
            string tmp = it->p.V;
            for (auto iterator = FollowSet[tmp].begin(); iterator != FollowSet[tmp].end(); iterator++){
                auto iter = find(productions.begin(),productions.end(),it->p);
                ActionTable[c.num_of_closure][*iterator] = pair("R",iter-productions.begin());
            }
        }
    }
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
    for (auto it = NonTerminalMap.begin(); it != NonTerminalMap.end(); it++) {
        GenerateFirstSet(FirstSet, it->first, p);
    }
    for (auto it = NonTerminalMap.begin(); it != NonTerminalMap.end(); it++) {
        GenerateFollowSet(FollowSet, it->first, p);
    }

    //计算文法G的规范LR（0）集族
    Closure i_0, i1;
    list<Item> ll = {i[0]};
    i_0 = Compute_Closure_I(closure_cnt, ll, i);
    list<Closure> g;
    g.push_back(i_0);
    for (auto it = g.begin(); it != g.end(); it++) {
        for (auto it_ = symbol_table.begin(); it_ != symbol_table.end(); it_++) {
            Go(*it, *it_, i, g);
        }
    }


    //构造SLR语法分析表
    for (auto iter =g.begin(); iter != g.end(); iter++) {
        GenerateAnalyzeTable(*iter,0,i,p);
    }
    cout << "end";


    return 0;
}
