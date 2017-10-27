#include "Database.hpp"
#include "SimpleParser.hpp"
#include "operator/SqlQuery.hpp"
#include "operator/Printer.hpp"

using namespace std;

int main()
{
    string sample = "select name from studenten s, hoeren h, vorlesungen v where s.matrnr = h.matrnr and h.vorlnr=v.vorlnr and v.titel='Ethik'\0";
    cout << sample << endl;
    stringstream samplestream(sample);
    
    Database db;
    db.open("data/uni");
    
    SimpleParser parser;
    auto result = parser.parse_stream(samplestream);
    
    unique_ptr<SqlQuery> query(new SqlQuery(db, result));
    Printer out(move(query));
    
    out.open();
    while (out.next());
    out.close();
    
    cout << "------done------" << endl;
    
    char c;
    cin>>c;
    
    return 0;
}