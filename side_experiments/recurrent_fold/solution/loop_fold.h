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

// remove branches, pass throughs, etc., when creating inner

// what if inner scope needs extra state?
// yeah, need to add for normal folds as well
// pass in new state to fetch new information

// state order:
// - all local and input state becomes input state
// - 

#ifndef LOOP_FOLD_H
#define LOOP_FOLD_H

const int LOOP_FOLD_STATE_EXPLORE = 0;

const int LOOP_FOLD_STATE_MEASURE = 1;

const int LOOP_FOLD_STATE_EXPLORE_FAIL = 2;

const int LOOP_FOLD_STATE_ADD_STATE = 3;
// there is no inner state

// actually, divide between outer and inner because new scope is always added

const int LOOP_FOLD_STATE_EXPLORE_DONE = 4;

const int LOOP_FOLD_STATE_REMOVE_SCOPE = 5;
// include for current scope
const int LOOP_FOLD_STATE_REMOVE_NETWORK = 6;

const int LOOP_FOLD_STATE_DONE = 10;

class LoopFold {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	// keep fixed even if parent scope updates
	int num_local_states;
	int num_input_states;

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;

	// redo all state networks, but keep inner indexes
	std::vector<bool> inner_input_is_local;
	std::vector<int> inner_input_indexes;
	std::vector<int> inner_input_target_indexes;

	std::vector<int> inner_input_end_indexes;

	int state;
	int state_iter;
	int sub_state_iter;
	double sum_error;

	int curr_num_new_states;
	std::vector<bool> starting_state_network_target_is_local;
	std::vector<int> starting_state_network_target_indexes;
	std::vector<StateNetwork*> starting_state_networks;
	StateNetwork* continue_score_network;
	StateNetwork* continue_misguess_network;
	StateNetwork* halt_score_network;
	StateNetwork* halt_misguess_network;

};

#endif /* LOOP_FOLD_H */