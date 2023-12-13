#ifndef WORLD_STATE_H
#define WORLD_STATE_H

class WorldState {
public:
	int id;

	double obs_average;
	std::map<int, double> state_obs_impacts;
	double val_average;
	std::map<int, double> state_val_impacts;

	/**
	 * - both obs and state
	 * 
	 * - simply split on single factor and measure improvement
	 */
	std::vector<WorldState*> obs_transitions;


	std::vector<AbstractNode*> actions;
	std::vector<std::vector<Transform*>> action_impacts;

	std::vector<WorldState*> action_transitions;

};

#endif /* WORLD_STATE_H */