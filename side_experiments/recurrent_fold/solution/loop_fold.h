// TODO: for loops, because needs lots of 0, 1, 2, so only loop from existing?
// probably add like loop fold that can still add state
// state used to setup halt and continue networks

// should explore nodes be starting point or ending point?
// should be ending point
// yeah, info to halt or continue
// makes no sense if starting point, if following sequence is good, why not ending point for it
// and if not sure about following sequence, then why explore

// or maybe pick starting and ending point that are both good?
// or probably not -- too restrictive

// but weird for ending point for exploring
// - more natural to start from starting point, and then have the option of 0ing
// - so actually, have to start from starting

// but then, how to explore from starting?
// - explore weight doesn't apply to loop starts, mainly to loop ends
// - maybe loop start can temporarily inherit explore weight from loop end?
// - yeah, have best of both worlds
// ...except weird for how to continue into the end

// remove branches, pass throughs, etc., when creating inner?
// - but loses branches that might be useful
//   - but including branches means that there might be infinite to try?
// yeah, start new

// redo inner scopes
// - actually, maybe even not, because nodes from different scopes involved

// start with existing state though and adjust

// maybe effectively don't have 0s
// always start from 1 iteration and repeat (which can be 0 times)
// solves all the explore, start vs. end, state problems
// solves every problem actually -- makes everything into a pure plus

// if want to include loop with all of its decision making in new sequences, then have to create scope?
// or can just count on explore to grab outer scope

// actually probably cleaner to have inner/outer split, and new scope

#ifndef LOOP_FOLD_H
#define LOOP_FOLD_H

const int LOOP_FOLD_STATE_EXPLORE = 0;

const int LOOP_FOLD_STATE_MEASURE = 1;

const int LOOP_FOLD_STATE_EXPLORE_FAIL = 2;

const int LOOP_FOLD_STATE_ADD_OUTER_STATE = 3;
const int LOOP_FOLD_STATE_ADD_INNER_STATE = 4;

const int LOOP_FOLD_STATE_EXPLORE_DONE = 5;

const int LOOP_FOLD_STATE_REMOVE_OUTER_SCOPE = 6;
const int LOOP_FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK = 7;

const int LOOP_FOLD_STATE_REMOVE_INNER_SCOPE = 8;
const int LOOP_FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK = 9;

const int LOOP_FOLD_STATE_REMOVE_INNER_NETWORK = 10;
const int LOOP_FOLD_STATE_REMOVE_INNER_STATE = 11;
const int LOOP_FOLD_STATE_CLEAR_INNER_STATE = 12;

const int LOOP_FOLD_STATE_DONE = 13;

// can result in branch as original contains additional branches
const int LOOP_FOLD_RESULT_FAIL = 0;
const int LOOP_FOLD_RESULT_BRANCH = 1;
const int LOOP_FOLD_RESULT_REPLACE = 2;

class LoopFold {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;

	int num_local_states;
	int num_input_states;

	int sum_inner_inputs;
	std::vector<int> inner_input_start_indexes;
	std::vector<int> num_inner_inputs;
	// TODO: inner and ending scale_mods

	int state;
	int state_iter;
	int sub_state_iter;
	double sum_error;

	int curr_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_outer_state_networks;

	StateNetwork* curr_continue_score_network;
	StateNetwork* curr_continue_misguess_network;
	StateNetwork* curr_halt_score_network;
	StateNetwork* curr_halt_misguess_network;

	std::vector<StateNetwork*> curr_starting_state_networks;

	int curr_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> curr_state_networks;
	std::vector<StateNetwork*> curr_score_networks;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_inner_state_networks;

	int test_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_outer_state_networks;

	StateNetwork* test_continue_score_network;
	StateNetwork* test_continue_misguess_network;
	StateNetwork* test_halt_score_network;
	StateNetwork* test_halt_misguess_network;

	std::vector<StateNetwork*> test_starting_state_networks;

	int test_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> test_state_networks;
	std::vector<StateNetwork*> test_score_networks;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_inner_state_networks;

	double* existing_average_score;
	double* existing_score_variance;
	double* existing_average_misguess;
	double* existing_misguess_variance;

	double curr_branch_average_score;
	double curr_existing_average_improvement;
	double curr_replace_average_score;
	double curr_replace_average_misguess;
	double curr_replace_misguess_variance;

	double test_branch_average_score;
	double test_existing_average_improvement;
	double test_replace_average_score;
	double test_replace_average_misguess;
	double test_replace_misguess_variance;

	int explore_result;

	bool explore_added_state;

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

	// TODO: add seeding

	LoopFold(std::vector<int> scope_context,
			 std::vector<int> node_context,
			 int sequence_length,
			 std::vector<bool> is_inner_scope,
			 std::vector<int> existing_scope_ids,
			 double* existing_average_score,
			 double* existing_score_variance,
			 double* existing_average_misguess,
			 double* existing_misguess_variance);
	LoopFold(std::ifstream& input_file,
			 int scope_id,
			 int scope_index);
	~LoopFold();

	
};

#endif /* LOOP_FOLD_H */