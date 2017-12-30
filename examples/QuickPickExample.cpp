#include "IteratorTools.hpp"
#include "Database.hpp"
#include "SimpleParser.hpp"
#include "operator/SqlQuery.hpp"
#include "operator/Printer.hpp"
#include "SemanticAnalysis.hpp"
#include "QueryGraphFactory.hpp"
#include "ExhaustiveTransformation2.hpp"

#include <bitset>


using namespace std;

void execute_query(Database& db, string str)
{
    stringstream inpstream(str);
    
    SimpleParser parser;
    SqlParse result;
    
    try {
        result = parser.parse_stream(inpstream);
    } catch(SqlParseException se) {
        cout << "Semantic Error: " << se.what() << endl;
        return;
    }
    
    auto semanticAnalyzer = new SemanticAnalysis(db); // semantic analysis
    
    try {
        semanticAnalyzer->analyze(result);
    } catch(SemanticAnalysis::SemanticError se) {
        cout << "Semantic Error: " << se.what() << endl;
        return;
    }
    
    cout << endl;
    
    auto graph = buildGraphFromParse(db, result);
        
    string graphviz = graph.graphviz();
    cout << "# Query graph in DOT format:" << endl << graphviz << endl << endl;
    
    auto tree = QuickPick(graph, 100);
    
    tree.print(graph);
}

int main()
{
    Database db;
    db.open("data/tpch/tpch");
    
    
    vector<string> samples = {"select s_acctbal, s_name, n_name, p_partkey, p_mfgr, s_address, s_phone, s_comment from part p, supplier s, partsupp ps, nation n, region r where p.p_partkey = ps.ps_partkey and s.s_suppkey = ps.ps_suppkey and p.p_size = 15 and s.s_nationkey = n.n_nationkey and n.n_regionkey = r.r_regionkey and r.r_name = 'EUROPE'",
        "select n_name from customer c, orders o, lineitem l, supplier s, nation n, region r where c.c_custkey = o.o_custkey and l.l_orderkey = o.o_orderkey and l.l_suppkey = s.s_suppkey and c.c_nationkey = s.s_nationkey and s.s_nationkey = n.n_nationkey and n.n_regionkey = r.r_regionkey and r.r_name = 'ASIA'",
        "select ps_partkey from part p, partsupp ps, supplier s, nation n, region r where p.p_partkey = ps.ps_partkey and s.s_suppkey = ps.ps_suppkey and s.s_nationkey = n.n_nationkey and n.n_regionkey = r.r_regionkey and r.r_name = 'EUROPE'"};
    //string sample = "select name from studenten s, hoeren h, vorlesungen v where s.matrnr = h.matrnr and h.vorlnr=v.vorlnr and v.titel='Ethik'\0";
    for (auto& sample : samples) {
        cout << "> " << sample << endl;
        execute_query(db, sample);
        cout << endl;
    }
    
    string s;
    cout << "> "; getline(cin, s);
    while (s != "quit") {
        execute_query(db, s);
        cout << "> "; getline(cin, s);
    }
    
    char c;
    cin>>c;
    
    return 0;
}
