#include "SemanticAnalysis.hpp"

using namespace std;

SemanticAnalysis::SemanticAnalysis(Database& db): db(db) { }

SemanticAnalysis::SemanticError::SemanticError(const string& msg): runtime_error(msg) {}

void SemanticAnalysis::analyze(SqlParse& result){
	for (auto& r: result.bindings){
		checkRelation(r);
	}

	/// TODO: * case here
	
	for (auto& attr: result.projection){
		checkAttribute(attr);
	}

	for (auto& join: result.predicates){
		checkJoin(join);
	}

}

void SemanticAnalysis::checkRelation(SqlBinding& rel){
	std::string key = rel.binding.value;
	if (relations.find(key)!= relations.end()){
		throw SemanticError("Duplicate relation name in the FROM clause: "+ key);
	}
	try{
		db.getTable(rel.relation.value);
	} catch(...){
		throw SemanticError("Table "+key + " does not exist");
	}
	relations[key] = rel;
}

void SemanticAnalysis::checkJoin(SqlPredicate& join){
	Attribute left = checkAttribute(join.lhs);
	Attribute right = checkAttribute(join.rhs);
	if (left.getType() == right.getType())
		return;
	if (left.getType() != right.getType() && right.getType() != Attribute::Type::Constant && left.getType() != Attribute::Type::Constant){
		throw SemanticError("Joining attributes of incompatible type: "+left.getName()+" and "+right.getName());
	}
}

Attribute SemanticAnalysis::checkAttribute(Token& attr){
	Attribute resolvedAttr;
	// find the relation that that consists attr as attribute name
	for (auto it = relations.begin(); it != relations.end(); ++it ){
		auto rel = it -> second;
		
		// find the attribute in the relation
		Table& table = db.getTable(rel.relation.value);
		int index = table.findAttribute(attr.value);
		if (index == -1){
		    continue;
		}
		resolvedAttr = table.getAttribute(index);

		return std::move(resolvedAttr);
	}
	
	throw SemanticError("Attribute 1"+ attr.value +" does not exist in any relation");
}

Attribute SemanticAnalysis::checkAttribute(SqlAttribute& attr){
	
	Attribute resolvedAttr;
	// find the relation that that consists attr as attribute name
	SqlBinding temp;
	if(relations.find(attr.binding.value) != relations.end()){
		temp.binding = attr.binding;
		temp.relation = relations[attr.binding.value].relation;
	} else{
		if(attr.binding.type == 0 && attr.attribute.type == 0){
			resolvedAttr.setTypeConstant();
			return std::move(resolvedAttr);
		}
		throw SemanticError("Attribute 2"+ attr.attribute.value +" does not exist in any relation");
	}
		
	Table& table = db.getTable(temp.relation.value);
	int index = table.findAttribute(attr.attribute.value);
	if (index == -1){
		throw SemanticError("Attribute 3"+ attr.attribute.value +" does not exist in any relation");
	}
	resolvedAttr = table.getAttribute(index);

	return std::move(resolvedAttr);
}




