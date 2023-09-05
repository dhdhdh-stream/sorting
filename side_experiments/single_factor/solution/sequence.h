#ifndef SEQUENCE_H
#define SEQUENCE_H

/**
 * TODO: inputs are just part of the sequence
 * - just as how a path contains actions that may or may not match
 * - don't bother trying to change during experiment
 *   - extremely tricky, as changing changes behavior of everything
 *     - makes learning/measuring unstable
 * - same with ending state
 * 
 * - and this actually seems sound
 *   - due to explore, know that this combination has worked at least once
 *     - so even if state is not perfect, still good chance to be progress
 * 
 * - simply modify state score modifiers
 */

/**
 * - for first layer, learn for every input
 * - for others, learn for input that is initialized locally
 * 
 * - types represent values to take initially
 *   - they hopefully help inner and after make good decisions while inputs are being learned
 *     - (cannot rely on seeding as pre and after context would not match)
 *   - if not, they can still serve as random inspiration during exploration
 *     - (don't worry about setting back, as may simply be early exit, and will have no impact)
 * 
 * - don't need last seen
 *   - unreliable as can't guarantee comes from the same spot
 *   - don't need for inspiration, as local may be enough
 */
const int SEQUENCE_INPUT_TYPE_NONE = 0;
const int SEQUENCE_INPUT_TYPE_LOCAL = 1;
const int SEQUENCE_INPUT_TYPE_PREVIOUS = 2;

const int SEQUENCE_INPUT_TYPE_COPY = 3;

class SequenceHistory;
class Sequence {
public:
	/**
	 * - non-loop
	 */
	std::vector<Scope*> scopes;

	/**
	 * - starting_node_ids[0] == node_ids[0][0]
	 * 
	 * - last node id can be any type
	 *   - others all have to be non-loop scope nodes
	 */
	std::vector<int> starting_node_ids;

	std::vector<int> input_types;
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;
	/**
	 * - negative indexing from end
	 *   - original scope is 0
	 */
	std::vector<int> input_local_scope_depths;
	std::vector<int> input_local_input_indexes;
	std::vector<int> input_previous_step_index;
	std::vector<int> input_previous_input_index;
	std::vector<bool> input_has_transform;
	std::vector<Transformation> input_transformations;

	std::vector<std::vector<int>> node_ids;
	/**
	 * - scope nodes always given empty context
	 *   - so will always result in reasonable sequence
	 *     - (though may not be optimal due to original scope's later explored early exits)
	 */

	AbstractExperiment* experiment;
	int step_index;		// 0 if loop

	std::map<int, std::vector<std::vector<StateNetwork*>>> state_networks;
	// save separately from experiment to more easily update lasso weights
	std::map<int, int> scope_furthest_layer_seen_in;

	std::vector<std::vector<StateNetwork*>> step_state_networks;

	std::vector<int> input_furthest_layer_needed_in;
	std::vector<std::vector<bool>> input_steps_needed_in;



};

class SequenceHistory {
public:
	Sequence* sequence;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	SequenceHistory(Sequence* sequence);
	~SequenceHistory();
};

#endif /* SEQUENCE_H */