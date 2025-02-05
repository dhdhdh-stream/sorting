#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "run_helper.h"

class Problem;
class Scope;
class ScopeHistory;
class Solution;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* scope;

	int next_node_id;
	AbstractNode* next_node;

	ScopeNode();
	ScopeNode(ScopeNode* original,
			  Solution* parent_solution);
	~ScopeNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory* scope_history);
	void new_scope_capture_verify_activate(AbstractNode*& curr_node,
										   Problem* problem,
										   RunHelper& run_helper,
										   ScopeHistory* scope_history);
	#endif /* MDEBUG */

	void clear_experiments();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	ScopeNodeHistory(ScopeNode* node);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */