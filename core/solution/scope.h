#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "metrics.h"
#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
class AbstractExperiment;
class AbstractExperimentHistory;
class Network;
class Problem;
class Solution;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<std::vector<int>> eval_input_scope_context_ids;
	std::vector<std::vector<Scope*>> eval_input_scope_contexts;
	std::vector<std::vector<int>> eval_input_node_context_ids;
	std::vector<std::vector<AbstractNode*>> eval_input_node_contexts;
	std::vector<int> eval_input_obs_indexes;
	Network* eval_network;
	double eval_score_standard_deviation;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<int> verify_scope_history_sizes;
	#endif /* MDEBUG */

	Scope();
	~Scope();

	void activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void new_action_activate(AbstractNode* starting_node,
							 std::set<AbstractNode*>& included_nodes,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* history);
	void new_action_activate(Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* history);

	void measure_activate(Metrics& metrics,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  ScopeHistory* history);

	void step_through_activate(Problem* problem,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   ScopeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 ScopeHistory* history);
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<AbstractNode*, AbstractNodeHistory*> node_histories;

	AbstractExperimentHistory* callback_experiment_history;
	std::vector<int> callback_experiment_indexes;
	std::vector<int> callback_experiment_layers;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */