/**
 * @author BryanDu
 * @email yuchen200008@qq.com
 * @date 2021/11/24
 */

/// A statement in advance
/// this programme is only fit to the compile experiment of hit_yan XD
/// so i used some magic tricks here, this programme may not (i'm pretty sure it would not) fit to other grammars,
/// only the grammar i declared in production.txt would do.

#include <iostream>
#include <sstream>
#include <utility>
#include "fstream"
#include "vector"
#include "map"
#include "string"
#include "list"
#include "stack"
#include "algorithm"

using namespace std;
map<string, vector<string >> FirstSet;///< First Set map, contains first sets of every NonTerminal symbol
map<string, vector<string >> FollowSet;///< Follow Set map, contains follow sets of every NonTerminal symbol
map<string, int> NonTerminalMap;///< All NonTerminal symbols
map<string, int> TerminalMap;///< all Terminal symbols
vector<map<string, pair<string, int>>> ActionTable;///< part1 of SLR analyze table
vector<map<string, pair<string, int>>> GotoTable;///< part2 of SLR analyze table
vector<pair<int, pair<string, int>>> cache;///< status transmit cache
stack<int> StatusStack;
stack<string> SymbolStack;
int closure_cnt = 0;

const map<int, string> keyword_map = {{1,  "if"},
                                      {2,  "else"},
                                      {3,  "while"},
                                      {4,  "int"},
                                      {5,  "float"},
                                      {6,  "+"},
                                      {7,  "-"},
                                      {8,  "*"},
                                      {9,  "/"},
                                      {10, ">"},
                                      {11, "<"},
                                      {12, "="},
                                      {13, "("},
                                      {14, ")"},
                                      {15, ";"},
                                      {16, ">="},
                                      {17, "<="},
                                      {18, "!="},
                                      {19, "=="},
                                      {20, "'"},
                                      {21, "id"},
                                      {22, "digit"},
                                      {23, "$"}
};///< all keywords of the grammar G

/**
 *
 * @param string1 target string to be tested
 * @return if the target string is a NonTerminalSymbol or not
 */
bool IsNonTerminal(const string &string1) {
    return (NonTerminalMap.find(string1) != NonTerminalMap.end());
}

/**
 * @brief use to store one specific production
 */
struct Production {
    string V;///< left part of the production
    vector<string> P;///< right part of the production
};

bool operator==(const Production &p, const Production &q) {
    if (p.V == q.V && p.P == q.P) {
        return true;
    }
    return false;
}

/**
 * @brief split the target string with space ' '
 * @param str target string
 * @return list of sub strings
 */
vector<string> split(const string &str) {
    vector<string> t;
    istringstream iss(str);
    string token;
    while (getline(iss, token, ' ')) {
        t.push_back(token);
    }
    return t;
}

/**
 * @brief pre-process the productions, read productions from text file and generate Production list
 * @param file production file pointer
 * @return list of productions
 */
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

/**
 * @brief item of the LR(0) grammar
 */
class Item {
public:
    int position_of_dot;///< position of dot in the item
    Production p;///< corresponding production
    ulong len;///< length of the production's right part

    bool operator==(const Item &t) const {
        return (p.P == t.p.P && position_of_dot == t.position_of_dot);
    }

    Item(int
         pod, Production
         production) {
        position_of_dot = pod;
        p = std::move(production);
        len = p.P.size();
    }

    void display() {
        cout << p.V << "->";
        for (int i = 0; i < p.P.size(); i++) {
            if (i == position_of_dot) {
                cout << '.';
            }
            cout << p.P[i];
        }
        cout << endl;

    }
};

/**
 * @brief generate items of all the productions
 * @param p_list list of productions
 * @return list of items
 */
vector<Item> Create_Item_List(vector<Production> p_list) {
    vector<Item> item_list;
    for (auto it = p_list.begin(); it != p_list.end(); it++) {
        for (int i = 0; i <= it->P.size(); i++) {
            if (it->P[0] != "e") {
                Item t(i, *it);
                item_list.push_back(t);
            }
        }
    }
    Production production;
    production.V = "D";
    production.P = {"e"};
    Item t(0, production);
    item_list.push_back(t);
    return item_list;
}

/**
 * @brief Closure of a set of items
 */
class Closure {
public:
    int num_of_closure{};///< num of this closure, will be referenced in later process
    vector<Item> items;///< set of items

    bool operator==(const Closure &tgt) const {
        return (items == tgt.items);
    }

    void display() {
        cout << "closure" << num_of_closure << endl;
        for (auto it = items.begin(); it != items.end(); it++) {
            it->display();
        }
    }
};

/**
 * @brief compute the closure of given set of Items
 * @param num_of_closure number of the result Closure
 * @param target_list list of target Items
 * @param global_list list of all Items
 * @return Closure of given set of Items
 */
Closure Compute_Closure_I(int num_of_closure, list<Item> target_list, vector<Item> &global_list) {
    Closure c;
    vector<Item> t;
    c.num_of_closure = num_of_closure;
    /** through the target_list of Items */
    for (auto it = target_list.begin(); it != target_list.end(); it++) {
        t.push_back(*it);
        /** exclude Items like A->αB. */
        if (it->position_of_dot != it->len) {
            /** if the symbol X after the dot is Non Terminal*/
            if (IsNonTerminal(it->p.P[it->position_of_dot])) {
                /** search all Items for Items looks like X->α.βγ,no matter where the dot is */
                for (auto it_ = global_list.begin(); it_ != global_list.end(); it_++) {
                    if (it_->position_of_dot == 0 && it_->p.V == it->p.P[it->position_of_dot]) {
                        /** if the symbol Y after the dot is Non Terminal */
                        if (IsNonTerminal(it_->p.P[it_->position_of_dot])) {
                            /** if the Item does not belong to target_list, add it to the end and keep the computation */
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

/**
 * @brief generate table of all symbols in the given grammar
 * @param table
 */
void GenerateSymbolTable(vector<string> &table) {
    for (auto &it: NonTerminalMap) {
        table.push_back(it.first);
    }
    for (auto &it: TerminalMap) {
        table.push_back(it.first);
    }
}

/**
 * @brief the goto function, computes the next Closure when the given Closure reads a string x
 * @param i current Closure
 * @param x input string
 * @param global list of all Items
 * @param closure_set set of all Closures
 */
void Go(Closure i, const string &x, vector<Item> &global, list<Closure> &closure_set) {
    list<Item> t;
    /** find every Item in i that would take x as the next input */
    for (auto it = i.items.begin(); it != i.items.end(); it++) {
        if (it->position_of_dot != it->len && it->p.P[it->position_of_dot] == x) {
            Item _t(it->position_of_dot + 1, it->p);
            t.push_back(_t);
        }
    }
    if (!t.empty()) {
        /** compute the Closure of the selected Items */
        Closure ii = Compute_Closure_I(++closure_cnt, t, global);
        auto it = find(closure_set.begin(), closure_set.end(), ii);
        /** if ii is not in closure_set, add it to the end */
        if (it == closure_set.end()) {
            closure_set.push_back(ii);
            /** store the state transition  */
            cache.emplace_back(i.num_of_closure, pair<string, int>(x, ii.num_of_closure));
        } else {
            closure_cnt--;
            cache.emplace_back(i.num_of_closure, pair<string, int>(x, it->num_of_closure));
        }

    }
}
/**
 * @brief compute the intersection of 2 set of strings
 * @param dest set1
 * @param source set2
 * @example set1=['a','b','c','d','e'],
 * set2=['b','c','d','f','h'],
 * Merge(set1,set2)=['a','b','c','d','e','f','h']
 */
void Merge(vector<string> &dest, vector<string> &source) {
    for (auto it = source.begin(); it != source.end(); it++) {
        auto it_ = find(dest.begin(), dest.end(), *it);
        if (it_ == dest.end()) {
            dest.push_back(*it);
        }
    }
}
/**
 * @brief overloading of Merge, takes set of string and a target string as param
 * @param dest set of strings
 * @param source target string
 * @example set=['a','b','c','d','e'],str='h',Merge(set,str)=['a','b','c','d','e','h']
 */
void Merge(vector<string> &dest, string &source) {
    auto it = find(dest.begin(), dest.end(), source);
    if (it == dest.end()) {
        dest.push_back(source);
    }
}
/**
 * @brief generate the First Set of the given Non Terminal symbols
 * @param firstset map of first sets
 * @param target target Non Terminal symbol
 * @param plist all the productions
 */
void GenerateFirstSet(map<string, vector<string>> &firstset, const string &target, vector<Production> &plist) {
    vector<Production> t;
    vector<string> s;
    /** find all the productions like target->αβ */
    for (auto it = plist.begin(); it != plist.end(); it++) {
        if (it->V == target) {
            t.push_back(*it);
        }
    }
    for (auto it = t.begin(); it != t.end(); it++) {
        /** if the first symbol is Non Terminal */
        if (IsNonTerminal(it->P[0])) {
            if (it->P[0] != target) {
                /** recursively compute the First Set of the first symbol */
                /// should've do check for whether the computation is done earlier here,
                /// but i got kinda lazy so here we are, no check
                /// however, you don't need to be worried if this could lead to error. the insert method of std::map
                /// would not cover the original contents if there already exists a specific key. so this would do no harm
                GenerateFirstSet(firstset, it->P[0], plist);
                Merge(s, firstset[it->P[0]]);
            }
            /// magic trick alert! this is only to deal with the production P->DS, since this is the only production
            /// that have 2 continues Non Terminal symbols in the right part.
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
/**
 * @brief generate the follow set of the given Non Terminal symbol
 * @param followset map of all follow sets
 * @param target target symbol
 * @param plist all the productions
 */
void GenerateFollowSet(map<string, vector<string>> &followset, const string &target, vector<Production> &plist) {
    vector<Production> target_productions;
    vector<string> results;
    /// magic trick alert! obviously this is just me being lazy again
    if (target == "P") {
        string s = "$";
        Merge(results, s);
    }
    /** find all productions that looks like *->α target β */
    for (auto it = plist.begin(); it != plist.end(); it++) {
        auto t_it = find(it->P.begin(), it->P.end(), target);
        if (t_it != it->P.end()) {
            target_productions.push_back(*it);
        }
    }

    for (auto it = target_productions.begin(); it != target_productions.end(); it++) {
        /** find the position of target in the production */
        auto t_it = find(it->P.begin(), it->P.end(), target);
        /** if the production look like *->α target β */
        if (t_it - it->P.begin() < it->P.size() - 1) {
            /** next symbol is a Non Terminal symbol, production like *->αAB */
            if (IsNonTerminal(*(t_it + 1))) {
                /** put every item in first(B) into follow A */
                vector<string> tmp = FirstSet[*(t_it + 1)];
                /** exclude epsilon */
                auto tt_it = find(tmp.begin(), tmp.end(), "e");
                if (tt_it != tmp.end()) {
                    tmp.erase(tt_it);
                    /** put every item in follow A into follow B */
                    if (followset.find(it->V) == followset.end())
                        GenerateFollowSet(followset, it->V, plist);
                    Merge(results, followset[it->V]);
                }
                Merge(results, tmp);
            } else {
                /** next symbol is Terminal *-αAβ */
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
/**
 * @param i current status
 * @param x incoming string x
 * @return next status
 */
int SearchCache(int i, const string &x) {
    for (auto it = cache.begin(); it != cache.end(); it++) {
        if (it->first == i && it->second.first == x) {
            return it->second.second;
        }
    }
    return 0;
}
/**
 * @param c Closure to compute
 * @param global global Item list
 * @param productions global production list
 */
void GenerateAnalyzeTable(Closure &c, vector<Item> &global, vector<Production> &productions) {
    map<string, pair<string, int>> p, q;
    ActionTable.push_back(p);
    GotoTable.push_back(q);
    for (auto it = c.items.begin(); it != c.items.end(); it++) {
        if (it->p.V == "P") {
            ActionTable[c.num_of_closure]["$"] = pair("acc", -1);
        }
        if (it->position_of_dot != it->len) {
            string tmp = it->p.P[it->position_of_dot];
            int i = SearchCache(c.num_of_closure, tmp);
//            if (i!=-1) {
            if (!IsNonTerminal(tmp)) {
                if (tmp == "e") {
                    for (auto iterator = TerminalMap.begin(); iterator != TerminalMap.end(); iterator++) {
                        auto iter = find(productions.begin(), productions.end(), it->p);
                        pair<string, int> pair1("R", iter - productions.begin() - 1);
                        ActionTable[c.num_of_closure].insert(
                                pair<string, pair<string, int>>(iterator->first, pair1));
                    }
                } else {
                    ActionTable[c.num_of_closure][tmp] = pair("S", i);
                }
            } else {
                GotoTable[c.num_of_closure][tmp] = pair("S", i);
            }
//            } else {
//
//            }
        } else {
            string tmp = it->p.V;
            for (auto iterator = FollowSet[tmp].begin(); iterator != FollowSet[tmp].end(); iterator++) {
                auto iter = find(productions.begin(), productions.end(), it->p);
                ActionTable[c.num_of_closure][*iterator] = pair("R", iter - productions.begin());
            }
        }
    }
}
/**
 * @brief load token list from text file
 * @return token list in order
 */
vector<pair<int, string>> LoadTokenList() {
    vector<pair<int, string >> data;
    ifstream ifile("token.txt");
    string p;
    while (getline(ifile, p)) {
        vector<string> t;
        t = split(p);
        pair<int, string> q(atoi(t[0].c_str()), t[1]);
        data.push_back(q);
    }
    data.push_back(pair<int, string>(0, "$"));
    return data;

}

void Display(Production &production) {
    cout << production.V << "->";
    for (auto i = production.P.begin(); i != production.P.end(); i++) {
        cout << *i;
    }
    cout << endl;
}

int main() {
    vector<string> symbol_table;
    vector<Production> p;
    vector<Item> i;

    vector<pair<int, string >> tokenlist;
    tokenlist = LoadTokenList();

    ifstream file;
    file.open("production.txt");
    if (!file.is_open()) {
        cout << "error";
        exit(0);
    }
    p = Production_Pre_Process(&file);
    i = Create_Item_List(p);

    GenerateSymbolTable(symbol_table);
    for (auto it = NonTerminalMap.begin(); it != NonTerminalMap.end(); it++) {
        GenerateFirstSet(FirstSet, it->first, p);
    }
    for (auto it = NonTerminalMap.begin(); it != NonTerminalMap.end(); it++) {
        GenerateFollowSet(FollowSet, it->first, p);
    }
    SymbolStack.push("$");
    StatusStack.push(0);
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
//    for (auto it = g.begin(); it != g.end(); it++) {
//        it->display();
//    }

    //构造SLR语法分析表
    for (auto iter = g.begin(); iter != g.end(); iter++) {
        GenerateAnalyzeTable(*iter, i, p);
    }
    auto it = tokenlist.begin();
    while (true) {
        int curstate = StatusStack.top();
        string input = keyword_map.at(it->first);
        if (ActionTable[curstate][input].first == "S") {
            StatusStack.push(ActionTable[curstate][input].second);
            SymbolStack.push(input);
            it++;
        } else if (ActionTable[curstate][input].first == "R") {
            Production t = p[ActionTable[curstate][input].second];
            for (int iter = 0; iter < t.P.size(); iter++) {
                SymbolStack.pop();
                StatusStack.pop();
            }
            curstate = StatusStack.top();
            int next = SearchCache(curstate, t.V);
            StatusStack.push(next);
            SymbolStack.push(t.V);
            Display(t);
        } else if (ActionTable[curstate][input].first == "acc") {
            cout << "accepted!";
            break;
        } else {
            cout << "err";
            break;
        }
    }


    return 0;
}
