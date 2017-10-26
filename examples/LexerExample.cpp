#include "Lexer.hpp"

#include <iostream>
#include <fstream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
int main()
{
    Lexer mylex(cin, '\n');
    //select * from momo,coco, hoho where jojo=6.2 and lolo="82" and koko=hoho and zozo = "li lo" and altan=5 and truth=true
    
    cout << "Lexer output: " << endl;
    while ((*mylex).type != Token::Type::tok_eof)
    {
        cout << "\t";
        if ((*mylex).is_literal()) {
            Token::Type mytype = (*mylex).type;
            if (mytype == Token::Type::tok_lit_int)
                cout << "int(" << (*mylex).intValue << "): ";
            if (mytype == Token::Type::tok_lit_dbl)
                cout << "double(" << (*mylex).doubleValue << "): ";
            if (mytype == Token::Type::tok_lit_bool)
                cout << "bool(" << (*mylex).boolValue << "): ";
            if (mytype == Token::Type::tok_lit_str)
                cout << "string: ";
        }
        cout << (*mylex).value << endl;
        ++mylex;
    }
    
    char c;
    cin>>c;
    return 0;
}