#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "metrics.h"
#include "run_helper.h"

class Problem;
class Scope;
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
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void explore_activate(Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);
	void explore_activate(Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void measure_activate(Metrics& metrics,
						  AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void new_action_capture_verify_activate(AbstractNode*& curr_node,
											Problem* problem,
											std::vector<ContextLayer>& context,
											RunHelper& run_helper);
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	AbstractScopeHistory* scope_history;

	ScopeNodeHistory();
	ScopeNodeHistory(ScopeNodeHistory* original);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */