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

class AbstractExperiment;
class AbstractExperimentHistory;
class Problem;
class Scope;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* scope;
	int starting_node_id;
	AbstractNode* starting_node;
	std::set<int> exit_node_ids;
	std::set<AbstractNode*> exit_nodes;

	int next_node_id;
	AbstractNode* next_node;

	std::map<int, int> catch_ids;
	std::map<int, AbstractNode*> catches;

	ScopeNode();
	~ScopeNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);

	void random_existing_activate(AbstractNode*& curr_node,
								  std::vector<Scope*>& scope_context,
								  std::vector<AbstractNode*>& node_context,
								  int& inner_exit_depth,
								  AbstractNode*& inner_exit_node,
								  int& random_curr_depth,
								  int& random_throw_id,
								  bool& random_exceeded_limit,
								  std::vector<AbstractNode*>& possible_nodes);
	void inner_random_existing_activate(AbstractNode*& curr_node,
										std::vector<Scope*>& scope_context,
										std::vector<AbstractNode*>& node_context,
										int& inner_exit_depth,
										AbstractNode*& inner_exit_node,
										int& random_curr_depth,
										int& random_throw_id,
										bool& random_exceeded_limit);
	void random_path_activate(AbstractNode*& curr_node,
							  std::vector<Scope*>& scope_context,
							  std::vector<AbstractNode*>& node_context,
							  int& inner_exit_depth,
							  AbstractNode*& inner_exit_node,
							  int& random_curr_depth,
							  int& random_throw_id,
							  bool& random_exceeded_limit,
							  std::vector<std::vector<Scope*>>& possible_scope_contexts,
							  std::vector<std::vector<AbstractNode*>>& possible_node_contexts);
	void random_exit_activate(AbstractNode*& curr_node,
							  std::vector<Scope*>& scope_context,
							  std::vector<AbstractNode*>& node_context,
							  int& inner_exit_depth,
							  AbstractNode*& inner_exit_node,
							  int& random_curr_depth,
							  int& random_throw_id,
							  bool& random_exceeded_limit,
							  int curr_depth,
							  std::vector<std::pair<int,AbstractNode*>>& possible_exits);
	void inner_random_exit_activate(AbstractNode*& curr_node,
									std::vector<Scope*>& scope_context,
									std::vector<AbstractNode*>& node_context,
									int& inner_exit_depth,
									AbstractNode*& inner_exit_node,
									int& random_curr_depth,
									int& random_throw_id,
									bool& random_exceeded_limit);

	void step_through_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   AbstractNode*& exit_node,
							   RunHelper& run_helper,
							   ScopeNodeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history);
	#endif /* MDEBUG */

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  Metrics& metrics,
						  ScopeNodeHistory* history);

	void reset();

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