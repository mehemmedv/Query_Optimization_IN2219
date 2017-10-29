#include <iostream>
#include <memory>
#include <utility>
#include "Database.hpp"
#include "operator/Chi.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Selection.hpp"
#include "operator/Tablescan.hpp"

/*
Query:
select s2.name
from studenten s1, studenten s2, hoeren h1, hoeren h2
where
    s1.matrnr = h1.matrnr  
    and s2.matrnr = h2.matrnr 
    and h1.vorlnr = h2.vorlnr 
    and s2.matrnr != s1.matrnr
    and s1.name = 'Schopenhauer' 
*/

int main() {
    Database db;
    db.open("data/uni");
    Table& studenten = db.getTable("studenten");
    Table& hoeren = db.getTable("hoeren");

    auto s1_scan = std::make_unique<Tablescan>(studenten);
    const Register* s1_name = s1_scan->getOutput("name");
    const Register* s1_matrnr = s1_scan->getOutput("matrnr");
    Register s1_name_const;
    s1_name_const.setString("Schopenhauer");
    auto s1_select = std::make_unique<Selection>(std::move(s1_scan), s1_name, &s1_name_const);

    auto h1_scan = std::make_unique<Tablescan>(hoeren);
    const Register* h1_matrnr = h1_scan->getOutput("matrnr");
    const Register* h1_vorlnr = h1_scan->getOutput("vorlnr");
    auto s1h1_join = std::make_unique<HashJoin>(std::move(s1_select), std::move(h1_scan), s1_matrnr, h1_matrnr);

    auto h2_scan = std::make_unique<Tablescan>(hoeren);
    const Register* h2_matrnr = h2_scan->getOutput("matrnr");
    const Register* h2_vorlnr = h2_scan->getOutput("vorlnr");
    
    auto s2_scan = std::make_unique<Tablescan>(studenten);
    const Register* s2_matrnr = s2_scan->getOutput("matrnr");
    const Register* s2_name = s2_scan->getOutput("name");
    
    auto s1h1h2_join = std::make_unique<HashJoin>(std::move(s1h1_join), std::move(h2_scan), h1_vorlnr, h2_vorlnr);
    
    auto s1s2h1h2_join = std::make_unique<HashJoin>(std::move(s1h1h2_join), std::move(s2_scan), h2_matrnr, s2_matrnr);
    
    auto s1s2_chi = std::make_unique<Chi>(std::move(s1s2h1h2_join), Chi::NotEqual, s1_matrnr, s2_matrnr);
    const Register* s1s2_select_pred = s1s2_chi->getResult();
    auto s1s2_select = std::make_unique<Selection>(std::move(s1s2_chi), s1s2_select_pred);

    
    s1s2_select->open();
    while (s1s2_select->next()) {
        std::cout << s2_name->getString() << std::endl;
    }
    s1s2_select->close();

    return 0;
}
