#ifndef STATE_SCENARIO_EXPERIMENT_H
#define STATE_SCENARIO_EXPERIMENT_H

#include <vector>

#include "run_helper.h"

class Scope;
class ScopeHistory;
class StateScenario;

const int STATE_SCENARIO_EXPERIMENT_STATE_LEARN_AVERAGE = 0;
const int STATE_SCENARIO_EXPERIMENT_STATE_LEARN = 1;

const int STATE_SCENARIO_EXPERIMENT_STATE_DONE = 2;

class StateScenarioExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	Scope* scope;

	double state_average;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<double> predicted_state_histories;
	std::vector<double> target_state_histories;

	StateScenarioExperiment(StateScenario* scenario);
	~StateScenarioExperiment();

	void activate(StateScenario* scenario,
				  RunHelper& run_helper);
	void backprop(double target_state);
};

#endif /* STATE_SCENARIO_EXPERIMENT_H */