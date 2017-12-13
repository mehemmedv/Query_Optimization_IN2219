#ifndef H_ExhaustiveTransformation2
#define H_ExhaustiveTransformation2

#include "Database.hpp"
#include "QueryGraph.hpp"
#include "QueryPlan.hpp"
#include "SimpleParser.hpp"

QueryPlan exhaustiveTransformation2Plan(Database& db, QueryGraph& graph);


#endif