#include "IteratorTools.hpp"
#include "Database.hpp"
#include "DPccp.hpp"
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
    
    
    
    //unique_ptr<SqlQuery> query(new SqlQuery(db, result));
    //auto plan = dpccpPlan(db, graph);
    auto plan = exhaustiveTransformation2Plan(db, graph);
    
    cout << "# Plan:" << endl;
    
    cout << plan << endl;
    
    cout << "# Results:" << endl;
    {
        Printer out(plan.execute(result));
        //Printer out(move(query));
        
        out.open();
        while (out.next());
        out.close();
    }
    
    cout << "------done------" << endl << endl;
}

int main()
{
    Database db;
    db.open("data/tpch/tpch");
    
    
    string sample = "select l_orderkey, o_orderkey, c_custkey from lineitem l, orders o, customer c where l.l_orderkey=o.o_orderkey and o.o_custkey=c.c_custkey and c.c_name='Customer#000014993'";
    //string sample = "select name from studenten s, hoeren h, vorlesungen v where s.matrnr = h.matrnr and h.vorlnr=v.vorlnr and v.titel='Ethik'\0";
    cout << "> " << sample << endl;
    execute_query(db, sample);
    
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
