#ifndef H_DPccp
#define H_DPccp

#include "Database.hpp"
#include "QueryGraph.hpp"
#include "QueryPlan.hpp"
#include "SimpleParser.hpp"

QueryPlan dpccpPlan(Database& db, QueryGraph& graph);

#endif