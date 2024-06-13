#ifndef INFO_SCOPE_NODE_H
#define INFO_SCOPE_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class InfoScope;
class Problem;
class ScopeHistory;
class Solution;

class InfoScopeNodeHistory;
class InfoScopeNode : public AbstractNode {
public:
	InfoScope* scope;

	int next_node_id;
	AbstractNode* next_node;

	InfoScopeNode();
	InfoScopeNode(InfoScopeNode* original,
				  Solution* parent_solution);
	~InfoScopeNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void explore_activate(Problem* problem,
						  RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class InfoScopeNodeHistory : public AbstractNodeHistory {
public:
	bool is_positive;

	InfoScopeNodeHistory();
	InfoScopeNodeHistory(InfoScopeNodeHistory* original);
};

#endif /* INFO_SCOPE_NODE_H */