#ifndef INFO_PASS_THROUGH_EXPERIMENT_H
#define INFO_PASS_THROUGH_EXPERIMENT_H



class InfoPassThroughExperimentHistory;
class InfoPassThroughExperiment : public AbstractExperiment {
public:

	double existing_average_score;

	double curr_score;
	InfoScope* curr_info_scope;
	bool curr_is_negate;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_score;
	InfoScope* best_info_scope;
	bool best_is_negate;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_scopes;
	AbstractNode* best_exit_next_node;

	std::vector<std::vector<AbstractNode*>> eval_input_node_contexts;
	std::vector<int> eval_input_obs_indexes;

	double positive_average_score;

	std::vector<double> positive_linear_weights;
	std::vector<std::vector<int>> positive_network_input_indexes;
	Network* positive_network;
	double positive_network_average_misguess;
	double positive_network_misguess_standard_deviation;

	double negative_average_score;

	std::vector<double> negative_linear_weights;
	std::vector<std::vector<int>> negative_network_input_indexes;
	Network* negative_network;
	double negative_network_average_misguess;
	double negative_network_misguess_standard_deviation;

};

#endif /* INFO_PASS_THROUGH_EXPERIMENT_H */