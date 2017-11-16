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


void QueryPlan::setRoot(unique_ptr<OperatorNode> root)
{
    this->root = root;
}

const Register* QueryPlan::getRegister(const string& bind, const string& attr)
{
    return registers[bind][attr];
}

unique_ptr<Operator> QueryPlan::execute()
{
    if (!root) {
        return unique_ptr<Operator>();
    } else {
        return root->execute();
    }
}

unordered_map<string, unique_ptr<Tablescan>> QueryPlan::init(Database& db, const vector<SqlBinding>& bindings)
{
    unordered_map<string, unique_ptr<Tablescan>> retval;    //binding, scan
    
    for (auto& bind : bindings) {
        Table& mytable = db.getTable(bind.relation.value);
        unique_ptr<Tablescan> myscan(new Tablescan(mytable));
        
        for (int i = 0; i < mytable.getAttributeCount(); i++) {
            const Attribute& attr = mytable.getAttribute(i);
            const Register* reg = myscan->getOutput(attr.getName());
            registers[bind.binding.value][attr.getName()] = reg;
        }
        
        retval.emplace(bind.binding.value, move(myscan));
    }
    
    return retval;
}