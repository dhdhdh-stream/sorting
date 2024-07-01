#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "abstract_scope.h"
#include "context_layer.h"
#include "metrics.h"
#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
class AbstractExperiment;
class AbstractExperimentHistory;
class Network;
class NewActionExperiment;
class Problem;
class Solution;

class ScopeHistory;
class Scope : public AbstractScope {
public:
	std::vector<std::vector<int>> eval_input_scope_context_ids;
	std::vector<std::vector<AbstractScope*>> eval_input_scope_contexts;
	std::vector<std::vector<int>> eval_input_node_context_ids;
	std::vector<std::vector<AbstractNode*>> eval_input_node_contexts;
	std::vector<int> eval_input_obs_indexes;
	Network* eval_network;
	/**
	 * - not valid for every point in scope, only the end
	 *   - would be extremely expensive to keep updated
	 *     - addressed by train existing anyways
	 */

	/**
	 * - tie NewActionExperiment to scope instead of node
	 *   - so that can be tried throughout entire scope
	 */
	NewActionExperiment* new_action_experiment;

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

	void measure_activate(Metrics& metrics,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  ScopeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void new_action_capture_verify_activate(Problem* problem,
											std::vector<ContextLayer>& context,
											RunHelper& run_helper,
											ScopeHistory* history);
	void verify_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 ScopeHistory* history);
	void clear_verify();
	#endif /* MDEBUG */

	void clean_node(int scope_id,
					int node_id);

	void update_structure();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory : public AbstractScopeHistory {
public:
	AbstractExperimentHistory* callback_experiment_history;
	std::vector<int> callback_experiment_indexes;
	std::vector<int> callback_experiment_layers;

	ScopeHistory(Scope* scope);
	~ScopeHistory();

	AbstractScopeHistory* deep_copy();
};

#endif /* SCOPE_H */