#include "Database.hpp"
#include "SimpleParser.hpp"
#include "operator/SqlQuery.hpp"
#include "operator/Printer.hpp"
#include "SemanticAnalysis.hpp"
#include "QueryGraphFactory.hpp"
#include "QueryGraph.hpp"
#include "QueryPlan.hpp"

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
    
    QueryGraph querygraph = buildGraphFromParse(db, result);
    Tree tree = querygraph.GOO();
    
    string graphviz = querygraph.graphviz();
    

    
    cout << "# Query graph in DOT format:" << endl << graphviz << endl << endl;
    
    cout << "# Results:" << endl;
    
    unique_ptr<SqlQuery> query(new SqlQuery(db, result));
    Printer out(move(query));
    
    out.open();
    while (out.next());
    out.close();
    
    cout << "------done------" << endl << endl;
}

int main()
{
    Database db;
    db.open("data/uni");
    
    
    //string sample = "select name, titel from professoren p, vorlesungen v where p.persnr = v.gelesenvon";
    string sample = "select name from studenten s, hoeren h, vorlesungen v where s.matrnr = h.matrnr and h.vorlnr=v.vorlnr and v.titel='Ethik'\0";
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
