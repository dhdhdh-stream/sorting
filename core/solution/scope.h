/**
 * - the way "practice" works is:
 *   - scope shared between expensive instance and cheap instance
 *   - scope improved using cheap instances (i.e., practice)
 *   - benefits expensive instance
 * 
 * - exploring simply is trying random things
 *   - but the better explorer is the one who has more sophisticated things to try
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "metrics.h"
#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
class PassThroughExperimentHistory;
class Problem;

class ScopeHistory;
class Scope {
public:
	int id;
	std::string name;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	int default_starting_node_id;
	AbstractNode* default_starting_node;

	Scope();
	~Scope();

	void activate(AbstractNode* starting_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void random_existing_activate(AbstractNode* starting_node,
								  std::vector<Scope*>& scope_context,
								  std::vector<AbstractNode*>& node_context,
								  int& exit_depth,
								  AbstractNode*& exit_node,
								  int& random_curr_depth,
								  int& random_throw_id,
								  bool& random_exceeded_limit,
								  std::vector<AbstractNode*>& possible_nodes);
	void inner_random_existing_activate(AbstractNode* starting_node,
										std::vector<Scope*>& scope_context,
										std::vector<AbstractNode*>& node_context,
										int& exit_depth,
										AbstractNode*& exit_node,
										int& random_curr_depth,
										int& random_throw_id,
										bool& random_exceeded_limit);
	void random_path_activate(AbstractNode* starting_node,
							  std::vector<Scope*>& scope_context,
							  std::vector<AbstractNode*>& node_context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  int& random_curr_depth,
							  int& random_throw_id,
							  bool& random_exceeded_limit,
							  std::vector<std::vector<Scope*>>& possible_scope_contexts,
							  std::vector<std::vector<AbstractNode*>>& possible_node_contexts);
	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<Scope*>& scope_context,
							  std::vector<AbstractNode*>& node_context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  int& random_curr_depth,
							  int& random_throw_id,
							  bool& random_exceeded_limit,
							  int curr_depth,
							  std::vector<std::pair<int,AbstractNode*>>& possible_exits);
	void inner_random_exit_activate(AbstractNode* starting_node,
									std::vector<Scope*>& scope_context,
									std::vector<AbstractNode*>& node_context,
									int& exit_depth,
									AbstractNode*& exit_node,
									int& random_curr_depth,
									int& random_throw_id,
									bool& random_exceeded_limit);

	void step_through_activate(AbstractNode* starting_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   AbstractNode*& exit_node,
							   RunHelper& run_helper,
							   ScopeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode* starting_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper,
						 ScopeHistory* history);
	void clear_verify();
	#endif /* MDEBUG */

	void measure_activate(AbstractNode* starting_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  Metrics& metrics,
						  ScopeHistory* history);

	void reset();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<AbstractNodeHistory*> node_histories;

	PassThroughExperimentHistory* pass_through_experiment_history;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */