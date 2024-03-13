#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractExperiment;
class AbstractExperimentHistory;
class Problem;
class Scope;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* scope;

	int next_node_id;
	AbstractNode* next_node;

	std::map<int, int> catch_ids;
	std::map<int, AbstractNode*> catches;

	std::vector<AbstractExperiment*> experiments;

	ScopeNode();
	~ScopeNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);

	void step_through_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   RunHelper& run_helper,
							   ScopeNodeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history);
	#endif /* MDEBUG */

	void success_reset();
	void fail_reset();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	int throw_id;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */