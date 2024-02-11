#ifndef OUTER_EXPERIMENT_H
#define OUTER_EXPERIMENT_H

const int OUTER_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int OUTER_EXPERIMENT_STATE_EXPLORE = 1;
const int OUTER_EXPERIMENT_STATE_MEASURE_NEW = 2;
const int OUTER_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 3;
const int OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW = 4;
const int OUTER_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 5;
const int OUTER_EXPERIMENT_STATE_VERIFY_2ND_NEW = 6;

class OuterExperimentOverallHistory;
class OuterExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	double curr_score;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_existing_scopes;
	std::vector<ScopeNode*> curr_potential_scopes;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;

	std::vector<double> target_val_histories;


};

class OuterExperimentOverallHistory : public AbstractExperimentHistory {
public:
	OuterExperimentOverallHistory(OuterExperiment* experiment);
};

#endif /* OUTER_EXPERIMENT_H */