#ifndef H_QueryGraphFactory
#define H_QueryGraphFactory

#include "Database.hpp"
#include "QueryGraph.hpp"
#include "QueryPlan.hpp"
#include "SimpleParser.hpp"

using namespace std;

QueryGraph buildGraphFromParse(Database& db, const SqlParse& parse);

QueryPlan simplePlan(QueryGraph& graph);

Tree GOO(QueryGraph &querygraph);

#endif
