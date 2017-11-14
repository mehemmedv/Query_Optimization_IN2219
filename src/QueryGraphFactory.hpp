#ifndef H_QueryGraphFactory
#define H_QueryGraphFactory

#include "Database.hpp"
#include "QueryGraph.hpp"
#include "SimpleParser.hpp"

using namespace std;

QueryGraph buildGraphFromParse(Database& db, const SqlParse& parse);

#endif