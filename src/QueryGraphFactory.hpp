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

Tree QuickPick(QueryGraph &querygraph, int cnt_trees);

int find(int x, int* parent);

void Union(int l, int r, int* parent, int* rank, Tree** trees);

int Find(int x, int* parent);

#endif
