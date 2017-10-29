#include <iostream>
#include <memory>
#include <string>
#include <set>
#include <utility>
#include "Database.hpp"
#include "operator/Chi.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Selection.hpp"
#include "operator/Tablescan.hpp"

/*
Query with aggregation (but not supported by tinydb):
select distinct p.name
from professoren p, vorlesungen v, hoeren h
where
    p.persnr = v.gelesenvon
    and v.vorlnr = h.vorlnr
group by p.name, v.vorlnr
having count(*) >= 2

The same Query with joins:

select p.name
from professoren p, vorlesungen v, hoeren h1, hoeren h2
where
    p.persnr = v.gelesenvon
    and h1.matrnr != h2.matrnr
    and v.vorlnr = h1.vorlnr
    and v.vorlnr = h2.vorlnr
*/

int main() {
    Database db;
    db.open("data/uni");
    Table& professoren = db.getTable("professoren");
    Table& vorlesungen = db.getTable("vorlesungen");
    Table& hoeren = db.getTable("hoeren");

    auto p_scan = std::make_unique<Tablescan>(professoren);
    const Register* p_persnr = p_scan->getOutput("persnr");
    const Register* p_name = p_scan->getOutput("name");
    auto v_scan = std::make_unique<Tablescan>(vorlesungen);
    const Register* v_vorlnr = v_scan->getOutput("vorlnr");
    const Register* v_gelesenvon = v_scan->getOutput("gelesenvon");
    auto pv_join = std::make_unique<HashJoin>(std::move(p_scan), std::move(v_scan), p_persnr, v_gelesenvon);

    auto h1_scan = std::make_unique<Tablescan>(hoeren);
    const Register* h1_matrnr = h1_scan->getOutput("matrnr");
    const Register* h1_vorlnr = h1_scan->getOutput("vorlnr");
    auto h2_scan = std::make_unique<Tablescan>(hoeren);
    const Register* h2_matrnr = h2_scan->getOutput("matrnr");
    const Register* h2_vorlnr = h2_scan->getOutput("vorlnr");
  
    auto vh1_join = std::make_unique<HashJoin>(std::move(pv_join), std::move(h1_scan), v_vorlnr, h1_vorlnr);
    auto vh1h2_join = std::make_unique<HashJoin>(std::move(vh1_join), std::move(h2_scan), v_vorlnr, h2_vorlnr);
    auto h1h2_chi = std::make_unique<Chi>(std::move(vh1h2_join), Chi::NotEqual, h1_matrnr, h2_matrnr);
    const Register* h1h2_select_predicate = h1h2_chi->getResult();
    auto h1h2_select = std::make_unique<Selection>(std::move(h1h2_chi), h1h2_select_predicate);
    
    // removing reduntant professor names
    std::set<std::string> prof_names;
    h1h2_select->open();
    while (h1h2_select->next()) {
        prof_names.insert(p_name->getString());
    }
    h1h2_select->close();

    for (auto name : prof_names) {
        std::cout << name << std::endl;
    }

    return 0;
}
