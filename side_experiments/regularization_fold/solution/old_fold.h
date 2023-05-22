#ifndef FOLD_H
#define FOLD_H

#include <map>
#include <set>
#include <vector>

#include "action.h"
#include "problem.h"
#include "run_helper.h"
#include "scale.h"
#include "scope.h"
#include "state_network.h"

const int FOLD_STATE_EXPERIMENT = 0;	// with 1 new state
const int FOLD_STATE_EXPERIMENT_FAIL = 1;
const int FOLD_STATE_ADD_STATE = 2;
const int FOLD_STATE_REMOVE_REGULARIZATION = 3;
const int FOLD_STATE_DONE = 4;

const int FOLD_RESULT_FAIL = 0;
const int FOLD_RESULT_BRANCH = 1;
const int FOLD_RESULT_REPLACE = 2;

// no more outer, and instead inner can rewind

// calc all state updates, then apply all at end, so there are no ordering concerns

// actually, should try to not train new objects as much?
// - one advantage with having existing objects is to also make training easier?

// also, on explore, can reuse networks too, and not just treat as brand new actions
// - also gives the best chance for the backside to work correctly
//   - yeah, if use new, then might make backside worse too

class FoldHistory;
class ScopeHistory;
class Fold {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;
	std::vector<std::vector<Input*>> existing_scope_inputs;
	std::vector<Action> actions;

	// when trying to reuse objects, initially full copy, and train new with original 50/50 mix
	// then after, see if there's significant difference

	// or after training, have separate object merge step where swap one in place of other throughout the solution, and see if there's significant impact

	// as much as possible, try to reuse state from outer scopes

	// the way object replacement works is:
	// - first, new object gets created that works on entirely separate context
	// - then, original context gets reused in new context, but with new object being used instead
	// - new object is better, so it gets used in place from then on in new contexts, which become increasingly relevant

	// can also try re-applying in original context

	// keep fixed even if parent scope updates
	int num_existing_states;

	int sum_inner_inputs;
	std::vector<int> inner_input_start_indexes;
	std::vector<int> num_inner_inputs;	// keep track here fixed even if scope updates
	std::vector<Scale*> inner_scope_scale_mods;	// won't bother splitting between curr and test
	double end_mod;	// temporary to help measure misguess

	int curr_num_new_states;

	StateNetwork* curr_starting_score_network;
	/**
	 * state ordering:
	 * - new
	 * - new input
	 * - existing
	 */
	// maybe inherit state networks? but need to inherit for outside as well
	// actually, probably don't, too messy
	// - just stick with scopes instead
	//   - actions outside of scopes may not contain useful data anyways
	// - so actually, what happens for the actions doesn't end up really mattering
	//   - what is passed into the scopes is far more relevant
	//     - so actually don't retrain everything
	//       - focus should be on incremental improvements

	// it mainly matters what happens thoughout the entire solution
	// - but changing the entire solution might take too much effort?

	// in fact, new state doesn't matter that much
	// - it at most affects the sequence, but inner and back matter more eventually

	// maybe always try state as-is (pick among available options)
	// - and separately, look for better inputs

	// the rewind feature can just be for efficiency
	// - and actually, scopes can have a million inputs for everything within
	// - that way idea of scopes stays consistent
	//   - and it's just that some state won't be calculated until it's actually needed

	// training for score for new sequence doesn't matter in the long run as well
	// - focus for training should be on decision making for branch

	// for inner, assume that decisions are made well with state passed in?

	// maybe new sequences creates scopes in front, not within the scope

	// yeah, so goal is to always improve decision making at a single spot, and create scopes in front

	// actually no need to have score networks everywhere, only need when state changes
	// so for each state change, also have impact on score
	// - maybe also have 1 for obs
	// - or have 1 score network, but it's input only includes state changes
	//   - then on state addition, no need to directly update, but make state available and let score update on its own

	// actually, while adding new state, check if has score impact
	// - if not, then can rewind?
	// - it might signal a good vs. bad choice, but not have that much affect on overall score

	// yeah, so on new branch, look for new state for both original and new

	// treat new state for starting score network and new input for inner the same

	// divide between 3 processes:
	// - trying new sequences
	// - improving existing objects
	// - fitting new objects
	//   - there should be no harm in fitting new objects even if it doesn't fit?
	//   - or can measure impact to either

	// rewind is just last seen in any context
	// - so no need to run new calculation
	// - then if effective, keep doing that?

	// but how to handle initializing/re-initializing?
	// - there has to be clears/initializations
	//   - due to the way humans are able to identify distinct events
	//     - but this is where rewind makes sense
	//       - but then it doesn't impact score

	// it's sooo much cleaner if it was always the same object
	// - maybe scope just means simply "forget" what happened before?
	//   - it matters if process deeper, but can often simply not worry

	// yeah, so it's always rolling, but you don't start "remembering" from the front always

	// can change pressure to go to 0 based on distance to starting factor, so info is more localized

	// divide scopes as previous
	// - issue with dividing ahead is recursion again (and dealing with existing scope)
	// - but dividing scope as previous feels low impact?
	// - important state needed for the back is not captured until later anyways

	// ...already going all the way forward, what if go all the way backward as well
	// - each state network modifies state and modifies score
	//   - don't worry about creating scopes forward and back
	//     - too complicated
	//     - shouldn't be too impactful as everything is meant to be used in combination with new middle

	// could also have preferred strat, and only begin rewinding and adding state when preferred strat cannot be anymore
	// - so anything that isn't the preferred strat won't affect score network

	// for back, maybe split score networks, and have some that activate only when certain paths taken previously
	// - when certain variables initialized maybe

	// one issue with trying to create scopes outer is that after creation, scopes are chaotic
	// - not clear where things start or stop
	// - not to mention that a lot of indexes are already locked in

	// yeah, so have to keep in mind that scopes are best effort?
	// - don't have to split super well, cleanly, or hierarchicly in a good way as long as trying full sequences, and not isolated scopes

	// or can treat recursive properly with re-initializing variables

	// add weights on recurrent network input, and put zero pressure on those
	// - add more pressure the further away
	// - add more pressure the higher the variable

	// maybe don't worry too much about splitting new sequence
	// - just look to create a new inner scope if appropriate?

	// check impact of obs
	// don't update any state until end
	// - so obs impact won't come through other state
	// keep if obs has impact

	std::vector<std::vector<StateNetwork*>> curr_sequence_state_networks;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_scope_state_networks;
	std::vector<StateNetwork*> curr_score_networks;

	int test_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_outer_state_networks;
	StateNetwork* test_starting_score_network;

	int test_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> test_state_networks;
	std::vector<StateNetwork*> test_score_networks;	// compare against curr_score_networks rather than score, as easier to measure
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_inner_state_networks;

	std::vector<bool> curr_inner_inputs_needed;
	std::vector<bool> test_inner_inputs_needed;
	// set state_networks_not_needed and state_not_needed_locally to match

	double* existing_average_score;
	double* existing_score_variance;
	double* existing_average_misguess;
	double* existing_misguess_variance;

	double seed_start_predicted_score;
	double seed_start_scale_factor;
	std::vector<double> seed_state_vals_snapshot;
	ScopeHistory* seed_outer_context_history;
	double seed_target_val;
	// don't worry about sequence for seed, as due to updates for inner scope, seed may quickly become irrelevant

	double curr_branch_average_score;
	double curr_branch_existing_average_score;
	double curr_replace_average_score;
	double curr_replace_average_misguess;
	double curr_replace_misguess_variance;

	double test_branch_average_score;
	double test_branch_existing_average_score;
	double test_replace_average_score;
	double test_replace_average_misguess;
	double test_replace_misguess_variance;

	int is_recursive;

	int experiment_result;

	bool experiment_added_state;

	int remove_inner_input_index;

	int clean_outer_scope_index;
	std::set<int> curr_outer_scopes_needed;
	std::set<std::pair<int, int>> curr_outer_contexts_needed;
	std::set<int> reverse_test_outer_scopes_needed;
	std::set<std::pair<int, int>> reverse_test_outer_contexts_needed;

	int clean_outer_node_index;
	int clean_outer_state_index;
	std::map<int, std::vector<std::vector<bool>>> curr_outer_state_networks_not_needed;
	std::map<int, std::vector<std::vector<bool>>> test_outer_state_networks_not_needed;

	int clean_inner_scope_index;
	std::set<int> curr_inner_scopes_needed;
	std::set<std::pair<int, int>> curr_inner_contexts_needed;
	std::set<int> reverse_test_inner_scopes_needed;
	std::set<std::pair<int, int>> reverse_test_inner_contexts_needed;

	int clean_inner_node_index;
	int clean_inner_state_index;
	std::map<int, std::vector<std::vector<bool>>> curr_inner_state_networks_not_needed;
	std::map<int, std::vector<std::vector<bool>>> test_inner_state_networks_not_needed;

	int clean_inner_step_index;

	std::vector<std::vector<bool>> curr_state_networks_not_needed;
	std::vector<std::vector<bool>> test_state_networks_not_needed;

	std::vector<std::vector<bool>> curr_state_not_needed_locally;
	std::vector<std::vector<bool>> test_state_not_needed_locally;

	std::vector<int> curr_num_states_cleared;
	std::vector<int> test_num_states_cleared;

	int fold_node_scope_id;
	int fold_node_scope_index;

	int state;
	int state_iter;
	int sub_iter;
	double sum_error;

	Fold(std::vector<int> scope_context,
		 std::vector<int> node_context,
		 int exit_depth,
		 std::vector<bool> is_inner_scope,
		 std::vector<int> existing_scope_ids,
		 std::vector<Action> actions,
		 double* existing_average_score,
		 double* existing_score_variance,
		 double* existing_average_misguess,
		 double* existing_misguess_variance,
		 double seed_start_predicted_score,
		 double seed_start_scale_factor,
		 std::vector<double> seed_state_vals_snapshot,
		 ScopeHistory* seed_outer_context_history,
		 double seed_target_val);
	Fold(std::ifstream& input_file,
		 int scope_id,
		 int scope_index);
	~Fold();

	void score_activate(std::vector<double>& state_vals,
						double& predicted_score,
						double& scale_factor,
						std::vector<ScopeHistory*>& context_histories,
						RunHelper& run_helper,
						FoldHistory* history);
	void sequence_activate(Problem& problem,
						   std::vector<double>& state_vals,
						   std::vector<bool>& states_initialized,
						   double& predicted_score,
						   double& scale_factor,
						   RunHelper& run_helper,
						   FoldHistory* history);
	void sequence_backprop(std::vector<double>& state_errors,
						   std::vector<bool>& states_initialized,
						   double target_val,
						   double final_diff,
						   double final_misguess,
						   double& predicted_score,
						   double& scale_factor,
						   double& scale_factor_error,
						   RunHelper& run_helper,
						   FoldHistory* history);
	void score_backprop(std::vector<double>& state_errors,
						double target_val,
						double& predicted_score,
						double& scale_factor,
						double& scale_factor_error,
						RunHelper& run_helper,
						FoldHistory* history);

	void experiment_increment();

	void experiment_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
												ScopeHistory* scope_history,
												FoldHistory* history);
	void experiment_score_activate(std::vector<double>& state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   std::vector<ScopeHistory*>& context_histories,
								   RunHelper& run_helper,
								   FoldHistory* history);
	void experiment_inner_scope_activate_helper(std::vector<double>& new_state_vals,
												ScopeHistory* scope_history,
												int step_index,
												FoldHistory* history);
	void experiment_sequence_activate(Problem& problem,
									  std::vector<double>& state_vals,
									  std::vector<bool>& states_initialized,
									  double& predicted_score,
									  double& scale_factor,
									  RunHelper& run_helper,
									  FoldHistory* history);
	void experiment_backprop(std::vector<double>& state_errors,
							 std::vector<bool>& states_initialized,
							 double target_val,
							 double final_diff,
							 double final_misguess,
							 double& predicted_score,
							 double& scale_factor,
							 RunHelper& run_helper,
							 FoldHistory* history);

	void seed_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
										  ScopeHistory* scope_history,
										  std::vector<std::vector<StateNetworkHistory*>>& outer_state_network_histories);
	void seed_train();

	void experiment_end();
	void add_inner_state_end();
	void add_outer_state_end();

	void remove_inner_input_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
														ScopeHistory* scope_history,
														FoldHistory* history);
	void remove_inner_input_score_activate(std::vector<double>& state_vals,
										   double& predicted_score,
										   double& scale_factor,
										   std::vector<ScopeHistory*>& context_histories,
										   RunHelper& run_helper,
										   FoldHistory* history);
	void remove_inner_input_inner_scope_activate_helper(std::vector<double>& new_state_vals,
														ScopeHistory* scope_history,
														int step_index,
														FoldHistory* history);
	void remove_inner_input_sequence_activate(Problem& problem,
											  std::vector<double>& state_vals,
											  std::vector<bool>& states_initialized,
											  double& predicted_score,
											  double& scale_factor,
											  RunHelper& run_helper,
											  FoldHistory* history);
	void remove_inner_input_backprop(std::vector<double>& state_errors,
									 std::vector<bool>& states_initialized,
									 double target_val,
									 double final_diff,
									 double final_misguess,
									 double& predicted_score,
									 double& scale_factor,
									 RunHelper& run_helper,
									 FoldHistory* history);

	void remove_inner_input_end();

	void experiment_to_clean();

	void clean_increment();

	void remove_outer_scope_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
														ScopeHistory* scope_history,
														std::vector<int>& curr_scope_context,
														std::vector<int>& curr_node_context,
														RunHelper& run_helper,
														FoldHistory* history,
														std::vector<double>& test_new_outer_state_vals);
	void remove_outer_scope_score_activate(std::vector<double>& state_vals,
										   std::vector<ScopeHistory*>& context_histories,
										   RunHelper& run_helper,
										   FoldHistory* history);
	void remove_outer_scope_inner_scope_activate_helper(std::vector<double>& new_state_vals,
														ScopeHistory* scope_history,
														RunHelper& run_helper,
														int step_index,
														FoldHistory* history,
														std::vector<double>& test_new_state_vals,
														std::vector<std::vector<std::vector<StateNetworkHistory*>>>& test_inner_state_network_histories);
	void remove_outer_scope_sequence_activate(Problem& problem,
											  std::vector<double>& state_vals,
											  std::vector<bool>& states_initialized,
											  double& predicted_score,
											  double& scale_factor,
											  RunHelper& run_helper,
											  FoldHistory* history);

	void remove_outer_scope_end();

	void remove_outer_scope_network_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		RunHelper& run_helper,
		FoldHistory* history,
		std::vector<double>& test_new_outer_state_vals);
	void remove_outer_scope_network_score_activate(
		std::vector<double>& state_vals,
		std::vector<ScopeHistory*>& context_histories,
		RunHelper& run_helper,
		FoldHistory* history);
	void remove_outer_scope_network_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		RunHelper& run_helper,
		int step_index,
		FoldHistory* history);
	void remove_outer_scope_network_sequence_activate(
		Problem& problem,
		std::vector<double>& state_vals,
		std::vector<bool>& states_initialized,
		double& predicted_score,
		double& scale_factor,
		RunHelper& run_helper,
		FoldHistory* history);

	void remove_outer_scope_network_end();

	void clean_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
										   ScopeHistory* scope_history,
										   FoldHistory* history);
	void clean_score_activate(std::vector<double>& state_vals,
							  std::vector<ScopeHistory*> context_histories,
							  FoldHistory* history);
	void clean_inner_scope_activate_helper(std::vector<double>& new_state_vals,
										   std::vector<bool>& new_state_vals_initialized,
										   ScopeHistory* scope_history,
										   std::vector<int>& curr_scope_context,
										   std::vector<int>& curr_node_context,
										   RunHelper& run_helper,
										   int step_index,
										   FoldHistory* history,
										   std::vector<double>& test_new_state_vals,
										   std::vector<bool>& test_new_state_vals_initialized,
										   std::vector<std::vector<std::vector<StateNetworkHistory*>>>& test_inner_state_network_histories);
	void clean_sequence_activate(Problem& problem,
								 std::vector<double>& state_vals,
								 std::vector<bool>& states_initialized,
								 double& predicted_score,
								 double& scale_factor,
								 RunHelper& run_helper,
								 FoldHistory* history);
	void clean_sequence_backprop(std::vector<double>& state_errors,
								 std::vector<bool>& states_initialized,
								 double target_val,
								 double final_diff,
								 double final_misguess,
								 double& predicted_score,
								 double& scale_factor,
								 double& scale_factor_error,
								 RunHelper& run_helper,
								 FoldHistory* history);
	void clean_score_backprop(std::vector<double>& state_errors,
							  double target_val,
							  double& predicted_score,
							  double& scale_factor,
							  double& scale_factor_error,
							  RunHelper& run_helper,
							  FoldHistory* history);

	void remove_inner_scope_end();
	void remove_inner_scope_network_end();
	
	void clean_transform_helper();
	bool remove_inner_network_transform_helper();
	bool remove_inner_state_transform_helper();
	bool clear_inner_state_transform_helper();

	void remove_inner_network_end();
	void remove_inner_state_end();
	void clear_inner_state_end();

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);

	void remove_outer_scope_from_load();
	void remove_outer_scope_network_from_load();
	void remove_inner_scope_from_load();
	void remove_inner_scope_network_from_load();
	void remove_inner_network_from_load();
	void remove_inner_state_from_load();
	void clear_inner_state_from_load();
};

class FoldHistory {
public:
	Fold* fold;

	// to track between score and sequence
	std::vector<double> new_outer_state_vals;
	std::vector<double> test_new_outer_state_vals;

	std::vector<std::vector<StateNetworkHistory*>> outer_state_network_histories;
	double starting_score_update;
	StateNetworkHistory* starting_score_network_history;

	std::vector<std::vector<StateNetworkHistory*>> test_outer_state_network_histories;
	double test_starting_score_update;
	StateNetworkHistory* test_starting_score_network_history;

	std::vector<std::vector<StateNetworkHistory*>> state_network_histories;
	std::vector<ScopeHistory*> inner_scope_histories;
	std::vector<double> score_network_updates;
	std::vector<StateNetworkHistory*> score_network_histories;
	std::vector<std::vector<std::vector<StateNetworkHistory*>>> inner_state_network_histories;

	FoldHistory(Fold* fold);
	~FoldHistory();
};

#endif /* FOLD_H */