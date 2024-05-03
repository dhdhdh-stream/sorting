/**
 * - don't run often as not efficient
 *   - unable to filter decision sequence well
 */

// #ifndef INFO_BRANCH_EXPERIMENT_H
// #define INFO_BRANCH_EXPERIMENT_H



// class InfoBranchExperimentHistory;
// class InfoBranchExperiment : public AbstractExperiment {
// public:

// 	std::vector<AbstractNode*> eval_input_node_contexts;
// 	std::vector<int> eval_input_obs_indexes;

// 	double positive_average_score;

// 	std::vector<double> positive_linear_weights;
// 	std::vector<std::vector<int>> positive_network_input_indexes;
// 	Network* positive_network;
// 	double positive_network_average_misguess;
// 	double positive_network_misguess_standard_deviation;

// 	double negative_average_score;

// 	std::vector<double> negative_linear_weights;
// 	std::vector<std::vector<int>> negative_network_input_indexes;
// 	Network* negative_network;
// 	double negative_network_average_misguess;
// 	double negative_network_misguess_standard_deviation;

// 	double existing_average_score;
// 	double existing_score_standard_deviation;

// 	/**
// 	 * - assume predicted_score for existing is always existing_average_score
// 	 */

// 	std::vector<AbstractNode*> decision_input_node_contexts;
// 	std::vector<int> decision_input_obs_indexes;

// 	std::vector<double> existing_misguess_linear_weights;
// 	std::vector<std::vector<int>> existing_misguess_network_input_indexes;
// 	Network* existing_misguess_network;
// 	double existing_misguess_network_average_misguess;
// 	double existing_misguess_network_misguess_standard_deviation;

// 	double new_average_score;

// 	std::vector<double> new_score_linear_weights;
// 	std::vector<std::vector<int>> new_score_network_input_indexes;
// 	Network* new_score_network;
// 	double new_score_network_average_misguess;
// 	double new_score_network_misguess_standard_deviation;

// 	double new_average_misguess;

// 	std::vector<double> new_misguess_linear_weights;
// 	std::vector<std::vector<int>> new_misguess_network_input_indexes;
// 	Network* new_misguess_network;
// 	double new_misguess_network_average_misguess;
// 	double new_misguess_network_misguess_standard_deviation;

// };

// #endif /* INFO_BRANCH_EXPERIMENT_H */