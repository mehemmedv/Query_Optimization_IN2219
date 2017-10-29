#include "Database.hpp"
#include "SimpleParser.hpp"
#include "operator/SqlQuery.hpp"
#include "operator/Printer.hpp"
#include "SemanticAnalysis.hpp"


using namespace std;

void execute_query(Database& db, string str)
{
    stringstream inpstream(str);
    
    SimpleParser parser;
    auto result = parser.parse_stream(inpstream);
    
    auto semanticAnalyzer = new SemanticAnalysis(db);
    semanticAnalyzer->analyze(result);
    
    unique_ptr<SqlQuery> query(new SqlQuery(db, result));
    Printer out(move(query));
    
    out.open();
    while (out.next());
    out.close();
    
    cout << "------done------" << endl;
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
