#ifndef EXPERIMENT_H
#define EXPERIMENT_H

class Experiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;
	int num_inner_inputs;
	std::vector<double> inner_input_scope_depths;	// -1 for new
	std::vector<double> inner_input_input_indexes;	// new_state_index for new
	std::vector<std::vector<int>> existing_scope_inputs;
	std::vector<Action> actions;

	int num_new_states;	// new inner inputs + 5
	/**
	 * - index 0 is global
	 * - contexts in the middle
	 * - last index is inner
	 */
	std::vector<std::map<int, std::vector<std::vector<Network*>>>> action_node_state_networks;
	// temporary to determine state needed
	std::vector<std::map<int, std::vector<Network*>>> action_node_score_networks;
	std::vector<std::vector<Network*>> scope_node_state_networks;

	Network* starting_score_network;

	std::vector<int> local_inner_input_mapping;
	std::vector<int> reverse_local_inner_input_mapping;

	// TODO: add map to update all inputs?
	// TODO: can also make all score networks temporary to measure misguess
	// - then separately, update types?
	// TODO: can also not backprop errors on most score networks, and only backprop errors on "key" networks?

	// or, can make the assumption that if is not passed into inner, then local state does not matter
	// and later on, sequence doesn't matter relative to inner
	// - so can ignore outer state?

	// TODO: practice extending state

	// TODO: let it be possible to lose track of something?
	// - so don't worry about updating state on inner branch?
	//   - but need to update later

	// TODO: maybe literally have fetch constructs
	// - add more structure to the solution
	// - so if inner needs, refetch from inside, and never pass from outside?
	// - then on the outside, can update state on scope node exit?

	// 2 possibilities:
	// - keep all state updated at all times
	//   - breaks the idea of scopes
	// - be selective about when to fetch/update state
	//   - breaks the idea of types

	// when fetching, still try to preserve state/types
	// - use the correlation trick

	// should state for score vs. inner input be treated differently?
	// - if does impact score, should keep state around
	//   - but don't worry if situation changes?
	//     - what if inner branch means that state is now relevant further?
	//       - scope has already been created?
	//         - maybe use early exit to expand scope?
	//       - actually, need to be able to expand scope
	//         - but how to evaluate if a hundred inner state should be continued outside?
	//       - or can fetch and create a new scope, but still, need to evaluate if a hundred inner state is still relevant?

	// maybe OK to have different state that's basically the same thing (i.e., completely correlated)
	// - but incompatible (i.e., different units)

	// can have map of scope nodes
	// - so can update all outer state cleanly

	// perhaps don't tie scopes, or at least actions, to state
	// - but instead to checkpoints
	//   - intuitively, have to commit to a certain path until the next checkpoint, and can then re-evaluate

	// actually, probably distinguish between:
	// - state necessary to do things optimally
	//   - doesn't have types?
	// - targets which an action can be applied
	//   - has types
	// no no, still likely the same thing:
	// - learn that sometimes, A matters, and sometimes B matters
	// - then when trying something new, have the option of looking for A or B

	// so some sort of fetching also makes sense because potentially want different things
	// - so more reason to not keep things updated
	//   - but how does this play with score networks?

	// so with fetching, state becomes easy
	// - but scopes become difficult
	//   - fetched states just last for the duration of the sequence so that's not too bad at least?

	// so 2 questions as always:
	// - how to update on inner branch?
	//   - if update on scope node, how to make networks trigger only on relevant branch?
	// - how to update if state starts to matter outside scope?

	// can never prevent there from being variables that could be the same/same type, but are not
	// - initially, everything is tried separately, so everything is disjointed
	//   - so need way to merge things together?

	// if want to look at correlation vs existing variables, can save last, and compare against last?

	// maybe calculate relation between existing variables
	// - and swap in as needed
	//   - in fact, the relation can be between types, not just variables?
	//     - yeah, variables between types may not be correlated, but the translation should be consistent
	//       - if there's no matching variable for something yet, can basically create one that makes sense
	// - or perhaps from multiple types to one?

	// so fetches are fetch something, then convert it into target

	// so if creating a new variable to carry to the outside, can calculate relation with existing inner variables
	// - then, from the outside, will effectively have access to the inner variable
	//   - but otherwise use new variable without worrying that it may be a duplicate

	// so how can a fetch work?
	// - maybe somehow separate scope into fetch parts and action parts?
	//   - when a variable is created is when action starts?
	//     - but it's dependencies start before it, so all of that can be the fetch?
	// - look through memory?
	//   - don't really like this, as seems limiting?
	//     - maybe can have special memory action type where things are replayed, but no real action is performed

	// or maybe don't use full scope
	// - but just try scope partially, with the idea being that it is used to initialize data

	// so for inner input:
	// - best is if input, or related type, is already there
	// - then, can consider running a related scope partially
	//   - can maybe include memory-running a scope?
	//     - but with memory, can't control what type is fetched?
	// - finally, pass none

	// maybe mark what state a scope is capable of fetching
	// - so can't fetch arbitrary from memory, though there is still choice
	//   - to fetch arbitrary, run scopes partially

	/**
	 * - new_states
	 * - inner input local
	 */
	std::vector<std::vector<Network*>> sequence_state_networks;
	std::vector<Network*> sequence_score_networks;


};

class ExperimentHistory {
public:
	Experiment* Experiment;

	bool can_zero;
	// TODO: 10% zero
	// - should be enough to converge because a step cannot depend on any particular future step

	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> starting_new_state_vals_snapshot;
	double existing_predicted_score;
};

#endif /* EXPERIMENT_H */