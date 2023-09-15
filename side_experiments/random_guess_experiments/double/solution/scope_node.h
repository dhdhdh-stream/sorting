#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <vector>

#include "abstract_node.h"
#include "scope.h"

class ScopeNode : public AbstractNode {
public:
	std::vector<int> indexes;
	std::vector<int> target_indexes;

	Scope* scope;

	ScopeNode(std::vector<int> indexes,
			  std::vector<int> target_indexes,
			  Scope* scope);
	ScopeNode(ScopeNode* original);
	~ScopeNode();

	void activate(int& curr_spot,
				  int& curr_0_index,
				  int& curr_1_index,
				  std::vector<int>& spots_0,
				  std::vector<bool>& switches_0,
				  std::vector<int>& spots_1,
				  std::vector<bool>& switches_1,
				  int& num_actions);
	void fetch_context(std::vector<Scope*>& scope_context,
					   std::vector<int>& node_context,
					   int& curr_num_action,
					   int target_num_action);
	void print(int& curr_spot,
			   int& curr_0_index,
			   int& curr_1_index);
};

#endif /* SCOPE_NODE_H */