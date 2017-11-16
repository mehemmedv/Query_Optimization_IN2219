#include "QueryPlan.hpp"

#include "operator/CrossProduct.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Selection.hpp"
#include "operator/Operator.hpp"

unique_ptr<Operator> TableScanNode::execute()
{
    return move(scan);
}

unique_ptr<Operator> CrossProductNode::execute()
{
    return unique_ptr<CrossProduct>(
        new CrossProduct(left->execute(), right->execute()));
}

unique_ptr<Operator> SelectNode::execute()
{
    return unique_ptr<Selection>(
        new Selection(child->execute(), regl, regr));
}

unique_ptr<Operator> HashJoinNode::execute()
{
    return unique_ptr<HashJoin>(
        new HashJoin(left->execute(), right->execute(),
                     regl, regr));
}


void setRoot(unique_ptr<OperatorNode> root)
{
    this->root = root;
}

Register* getRegister(const string& bind, const string& attr)
{
    return registers[make_pair(bind, attr)];
}

unique_ptr<Operator> QueryPlan::execute()
{
    if (!root) {
        return unique_ptr<Operator>();
    } else {
        return root->execute();
    }
}

