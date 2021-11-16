#include <iostream>
#include <utility>
#include "map"
#include "vector"
#include "fstream"

#define String std::string
#define Vector std::vector


const std::map<String, int> keyword_map = {{"if",    1},
                                           {"else",  2},
                                           {"while", 3},
                                           {"int",   4},
                                           {"float", 5}
};
const std::map<String, int> OP_map = {{"+", 6},
                                      {"-", 7},
                                      {"*", 8},
                                      {"/", 9},
                                      {">", 10},
                                      {"<", 11},
                                      {"=", 12},
                                      {"(", 13},
                                      {")", 14},
                                      {";", 15},
                                      {">=", 16},
                                      {"<=", 17},
                                      {"!=", 18},
                                      {"==", 19},
                                      {"'", 20},
                                      {"ID", 21}
};

void outputSymbol() {
    std::cout << "#################################" << std::endl;
    std::cout<<"符号表"<<std::endl;
    std::cout << "符号  \t种别码" << "  \t" << "符号  \t种别码" << std::endl;
    std::cout << "if" << "  \t" << "1" << "    \t" << "=" << "    \t" << "12" << std::endl;
    std::cout << "else" << "  \t" << "2" << "    \t" << "(" << "    \t" << "13" << std::endl;
    std::cout << "while" << "  \t" << "3" << "    \t" << ")"<< "    \t"  << "14" << std::endl;
    std::cout << "int" << "  \t" << "4" << "    \t" << ";" << "    \t" << "15"<< std::endl;
    std::cout << "float" << "  \t" << "5" << "    \t" << ">=" << "    \t" << "16"<< std::endl;
    std::cout << "+" << "    \t" << "6" << "    \t" << "<=" << "    \t" << "17"<< std::endl;
    std::cout << "-" << "    \t" << "7" << "    \t" << "!=" << "    \t" << "18"<< std::endl;
    std::cout << "*" << "    \t" << "8" << "    \t" << "==" << "    \t" << "19"<< std::endl;
    std::cout << "/" << "    \t" << "9" << "    \t" << "'" << "    \t" << "20"<< std::endl;
    std::cout << ">" << "    \t" << "10" << "    \t" << "ID" << "    \t" << "21"<< std::endl;
    std::cout << "<" << "    \t" << "11" << std::endl;
    std::cout << "#################################" << std::endl;
}

class Token {
public:
    Token(int _id, String _token) {
        id = _id;
        token = std::move(_token);
    }

    int id;
    String token;

    void output() const {
        std::cout << '(' << id << ',' << token << ')' << std::endl;
    }
};


class TokenList {
public:
    Vector<Token> list;

    int addTokens(Vector<Token> t) {
        list.insert(list.end(), t.begin(), t.end());
        return 0;
    }

    int display() {
        for (auto &it: list) {
            it.output();
        }
        return 0;
    }
};

bool isDigit(char i) {
    return (i >= '0' && i <= '9');
}

bool isLetter(char i) {
    return ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z'));
}

Vector<Token> scan(String target,int line) {
    Vector<Token> tempV;
    String::iterator it = target.begin();
    String t;
    while (it != target.end()) {
        if (*it == ' ') {
            it++;
        } else {
            if (isDigit(*it)) {
                while (isDigit(*it)) {
                    t += *it;
                    it++;
                }
                int id = keyword_map.at("int");
                Token token(id, t);
                tempV.push_back(token);
                t.clear();

            } else if (isLetter(*it)) {
                int id;
                while (isLetter(*it) || isDigit(*it)) {
                    t += *it;
                    it++;
                }
                if (keyword_map.find(t) != keyword_map.end()) {
                    id = keyword_map.at(t);
                } else {
                    id = OP_map.at("ID");
                }
                Token token(id, t);
                tempV.push_back(token);
                t.clear();
            } else {
                int id;
                t = *it;
                it++;
                if (OP_map.find(t)!=OP_map.end()) {
                    id = OP_map.at(t);
                } else {
                    int pos = it - target.begin();
                    std::cout<<"[InvalidSymbolError] illegal character at "<<line<<":"<<pos<<std::endl;
                    std::cout<<target<<std::endl;
                    String foo = String(pos,' ') + "^";
                    std::cout<<foo<<std::endl;
                    id = 0;
                }
                Token token(id, t);
                tempV.push_back(token);
                t.clear();
            }
        }
    }
    return tempV;
}


int main() {
    TokenList tokenList;
    String filename;
    std::ifstream file;
    Vector<String> source;
    while (true) {
        std::cout << "请输入目标文件名：" << std::endl;
        std::cin >> filename;
        file.open(filename);
        if (!file.is_open()) {
            std::cout << "文件不存在或文件名不正确！" << std::endl;
        } else break;
    }
    std::cout << "#################################" << std::endl;
    std::cout << "源代码:" << std::endl;
    String s;
    int count = 1;
    while (getline(file, s)) {
        std::cout <<count<<" "<< s << std::endl;
        count ++;
        source.push_back(s);
    }
    outputSymbol();


    for (Vector<String>::iterator it = source.begin();it<source.end();it++) {
        String t = *it;
        Vector<Token> temp = scan(t,it-source.begin());
        tokenList.addTokens(temp);

    }
    std::cout << "#################################" << std::endl;
    std::cout << "token串表" << std::endl;
    tokenList.display();
    return 0;
}
