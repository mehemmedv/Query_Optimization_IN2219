#ifndef H_operator_SqlQuery
#define H_operator_SqlQuery
//---------------------------------------------------------------------------
#include <vector>
#include <memory>
#include "Database.hpp"
#include "Register.hpp"
#include "SimpleParser.hpp"
#include "operator/Operator.hpp"
#include "operator/Tablescan.hpp"

//---------------------------------------------------------------------------


class SqlQuery : public Operator
{
private:
    map<string, unique_ptr<Tablescan>> bindings;
    map<string, map<string, const Register*>> registers;
    vector<const Register*> projection;
    vector<unique_ptr<Register>> constantRegs;
    /// The input
    std::unique_ptr<Operator> input;
public:
    /// Constructor
    SqlQuery(Database& db, const SqlParse& parse);
    /// Destructor
    ~SqlQuery();

    /// Open the operator
    void open();
    /// Get the next tuple
    bool next();
    /// Close the operator
    void close();

    /// Get all produced values
    std::vector<const Register*> getOutput() const;
};

#endif