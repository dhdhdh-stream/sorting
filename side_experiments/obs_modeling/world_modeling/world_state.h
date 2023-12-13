#ifndef WORLD_STATE_H
#define WORLD_STATE_H

class WorldState {
public:
	int id;

	double val_average;
	std::vector<double> state_val_impacts;

	double obs_average;
	std::vector<double> state_obs_impacts;

	std::vector<double> state_averages;

	std::vector<WorldState*> obs_transitions;
	std::vector<int> obs_indexes;
	std::vector<bool> obs_is_greater;

	std::map<Action*, std::vector<Transform*>> action_impacts;

	std::vector<WorldState*> action_transitions;
	std::vector<Action*> action_transition_actions;
	std::vector<std::vector<std::pair<int, int>>> action_transition_states;
	/**
	 * - TODO: find way to really limit action-state combinations
	 *   - types?
	 */

	WorldState* default_transition;

	/**
	 * - don't include obs_transitions in experiments
	 *   - would need to sequentially measure obs averages
	 */
	std::vector<AbstractExperiment*> obs_experiments;
	std::vector<int> obs_experiment_indexes;
	std::vector<bool> obs_experiment_is_greater;
	std::map<Action*, std::pair<std::vector<std::pair<int, int>>, AbstractExperiment*>> action_experiments;

	AbstractExperiment* experiment_hook;
	std::vector<double> hook_obs;
	std::vector<std::vector<double>> hook_state_vals;
	std::vector<std::pair<Action*, std::vector<std::vector<std::pair<double, double>>>>> hook_impact_vals;
	// HERE

};

#endif /* WORLD_STATE_H */