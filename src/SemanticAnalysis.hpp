#include <string>
#include <vector>
#include <exception>
#include <utility>
#include "Attribute.hpp"
#include "Database.hpp"
#include "SimpleParser.hpp"
#include <unordered_map>

class SemanticAnalysis {
private:
	Database& db;
	// relations in the query
	std::unordered_map<std::string, SqlBinding> relations;
	// check relation in the FROM clause
	void checkRelation(SqlBinding& rel);
	// resolve attribute
	Attribute checkAttribute(SqlAttribute& attr);
	Attribute checkAttribute(Token& attr);
	// check join condition
	void checkJoin(SqlPredicate& join);
public:
	explicit SemanticAnalysis(Database& db);
	~SemanticAnalysis() = default;

	class SemanticError: public std::runtime_error{
	public:
		SemanticError(const std::string& msg);
	};

	void analyze(SqlParse& result);

};

