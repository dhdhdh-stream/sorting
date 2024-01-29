#ifndef SCENARIO_EXPERIMENT_H
#define SCENARIO_EXPERIMENT_H

#include <vector>

#include "run_helper.h"

class Scenario;
class Scope;
class ScopeHistory;

const int SCENARIO_EXPERIMENT_STATE_LEARN_AVERAGE = 0;
const int SCENARIO_EXPERIMENT_STATE_LEARN = 1;

const int SCENARIO_EXPERIMENT_STATE_DONE = 2;

class ScenarioExperiment {
public:
	Scenario* scenario_type;

	int state;
	int state_iter;
	int sub_state_iter;

	Scope* scope;

	double is_sequence_average;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<double> predicted_is_sequence_histories;
	std::vector<bool> is_sequence_histories;

	ScenarioExperiment(Scenario* scenario_type);
	~ScenarioExperiment();

	void activate(Scenario* scenario,
				  RunHelper& run_helper);
	void backprop(bool is_sequence);
};

#endif /* SCENARIO_EXPERIMENT_H */