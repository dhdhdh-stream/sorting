/**
 * - not for abstraction for now
 *   - only for supporting input transforms
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <vector>

#include "abstract_node.h"

class Scope {
public:
	std::vector<AbstractNode*> nodes;

	Scope(std::vector<AbstractNode*> nodes);
	Scope(Scope* original);
	~Scope();

	void activate(int& curr_spot,
				  int& curr_0_index,
				  std::vector<int>& spots,
				  std::vector<bool>& switches,
				  int& num_actions);
	void fetch_context(std::vector<Scope*>& scope_context,
					   std::vector<int>& node_context,
					   int& curr_num_action,
					   int target_num_action);
	void print(int& curr_spot,
			   int& curr_0_index);
};

#endif /* SCOPE_H */