// TODO: allow child experiments as well

#ifndef NEW_INFO_EXPERIMENT_H
#define NEW_INFO_EXPERIMENT_H

const int NEW_INFO_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE = 1;
const int NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO = 2;
const int NEW_INFO_EXPERIMENT_STATE_TRAIN_NEW = 3;


class NewInfoExperimentHistory;
class NewInfoExperiment : public AbstractExperiment {
public:

	double existing_average_score;
	double existing_score_standard_deviation;

	int explore_type;

	std::vector<int> curr_sequence_step_types;
	std::vector<ActionNode*> curr_sequence_actions;
	std::vector<ScopeNode*> curr_sequence_scopes;
	AbstractNode* curr_sequence_exit_next_node;

	double best_sequence_surprise;
	std::vector<int> best_sequence_step_types;
	std::vector<ActionNode*> best_sequence_actions;
	std::vector<ScopeNode*> best_sequence_scopes;
	AbstractNode* best_sequence_exit_next_node;

	double info_score;
	Scope* new_info_subscope;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_network_average_misguess;
	double existing_network_misguess_standard_deviation;

	double new_average_score;

	std::vector<double> new_linear_weights;
	std::vector<std::vector<int>> new_network_input_indexes;
	Network* new_network;
	double new_network_average_misguess;
	double new_network_misguess_standard_deviation;



	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

};

#endif /* NEW_INFO_EXPERIMENT_H */