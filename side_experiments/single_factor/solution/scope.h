/**
 * - scopes are just a bit of abstraction to try to promote the reuse of ideas
 *   - scopes roughly capture when certain state is relevant, but is extremely imprecise
 *     - the actual corresponding actions may start sooner or later, and may end sooner or later
 *   - in addition, may need to constantly break up scopes to use elsewhere
 *   - but as long as scopes, inner scopes, etc., are being created and reused, should be good enough to make progress
 * 
 * - specifically, here, the states created are too loose
 *   - i.e., they likely include more actions than is relevant for the state
 *     - so from the outside, there's more that's abstracted, but from the inside, will miss possible sub-scopes
 *       - though sub-scopes will likely be created on branch
 *         - so again, probably good enough
 */

#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_states;
	/**
	 * - when new state added, can be added twice
	 *   - once as local
	 *   - once as input
	 */
	std::vector<bool> state_initialized_locally;

	bool is_loop;
	std::vector<int> scope_context;
	std::vector<int> node_context;
	ScoreNetwork* continue_score_network;
	ScoreNetwork* continue_misguess_network;
	ScoreNetwork* halt_score_network;
	ScoreNetwork* halt_misguess_network;
	int furthest_successful_halt;

	std::vector<AbstractNode*> nodes;

	/**
	 * - alternate way of storing initialized locally information for scope nodes
	 */
	std::vector<int> initialized_locally_indexes;
	std::vector<Scale*> ending_score_scales;

	std::vector<std::vector<int>> used_class_ids;

	int remeasure_counter;

	Scope(int id,
		  int num_states,
		  std::vector<bool> state_initialized_locally,
		  bool is_loop,
		  ScoreNetwork* continue_score_network,
		  ScoreNetwork* continue_misguess_network,
		  ScoreNetwork* halt_score_network,
		  ScoreNetwork* halt_misguess_network,
		  );



};

class ScopeHistory {
public:
	Scope* scope;

	bool exceeded_depth;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	std::vector<double> initialized_locally_val_snapshots;
	std::vector<double> initialized_locally_weight_snapshots;


};

class RemeasureScopeHistory {
public:
	Scope* scope;

	bool train_continue;

	std::vector<ScoreNetworkHistory*> continue_score_network_histories;
	std::vector<double> continue_score_network_outputs;
	std::vector<ScoreNetworkHistory*> continue_misguess_network_histories;
	std::vector<double> continue_misguess_network_outputs;
	std::vector<double> halt_score_snapshots;
	std::vector<double> halt_misguess_snapshots;

	ScoreNetworkHistory* halt_score_network_history;
	double halt_score_network_output;
	ScoreNetworkHistory* halt_misguess_network_history;
	double halt_misguess_network_output;



};

#endif /* SCOPE_H */