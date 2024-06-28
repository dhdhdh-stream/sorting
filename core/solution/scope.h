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
class FamiliarityNetwork;
class Network;
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

	std::vector<FamiliarityNetwork*> familiarity_networks;
	std::vector<double> input_means;
	std::vector<double> input_standard_deviations;
	std::vector<double> familiarity_average_misguesses;
	std::vector<double> familiarity_misguess_standard_deviations;
	/**
	 * - use familiarity to find changes that leave existing solution unaffected
	 *   - i.e., changes that return to origin
	 *     - such changes are more likely to be useful elsewhere
	 */

	// transformations less important
	// - in-place scopes more important?

	// a lot of solutions that are tested are in place to begin with

	// when not in place, unclear if good/bad

	// also, if changes aren't a particular form, then might delete factors so even in place changes don't look like it

	// even with transformations, still need good in-place scope as followup to get good score
	// - and good in-place scope would probably work well without transformations anyways
	//   - so good in-place scopes are the key

	// maybe separate in place scopes
	// - only use by themselves and only in place
	//   - don't chain initially

	/**
	 * - used mainly to help prevent recursion during explore
	 */
	std::set<Scope*> scopes_used;
	std::set<InfoScope*> info_scopes_used;

	Scope();
	~Scope();

	void activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void random_activate(AbstractNode* starting_node,
						 std::vector<AbstractNode*>& possible_nodes);
	void random_exit_activate(AbstractNode* node_context,
							  bool is_branch,
							  std::vector<AbstractNode*>& possible_pre_exits,
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