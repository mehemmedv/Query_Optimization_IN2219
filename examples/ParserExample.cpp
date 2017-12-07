#include "IteratorTools.hpp"
#include "Database.hpp"
#include "DPccp.hpp"
#include "SimpleParser.hpp"
#include "operator/SqlQuery.hpp"
#include "operator/Printer.hpp"
#include "SemanticAnalysis.hpp"
#include "QueryGraphFactory.hpp"

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
    auto plan = dpccpPlan(db, graph);
    
    cout << "# Plan:" << endl;
    
    cout << plan << endl;
    
    cout << "# Results:" << endl;
    
    Printer out(plan.execute());
    //Printer out(move(query));
    
    out.open();
    while (out.next());
    out.close();
    
    cout << "------done------" << endl << endl;
}

int main()
{
    Database db;
    db.open("data/uni");
    
    
    string sample = "select * from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s1.name = 'Schopenhauer'";
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
