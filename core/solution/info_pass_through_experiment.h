// /**
//  * - for InfoBranchExperiment, would need to:
//  *   - first learn positive/negative for branch
//  *   - then learn score/misguess for existing vs. branch
//  * - also will not be able to filter branch sequence well as not based on score
//  */

// #ifndef INFO_PASS_THROUGH_EXPERIMENT_H
// #define INFO_PASS_THROUGH_EXPERIMENT_H

// #include <vector>

// #include "abstract_experiment.h"
// #include "context_layer.h"
// #include "run_helper.h"

// class AbstractNode;
// class ActionNode;
// class InfoScope;
// class InfoScopeNode;
// class Network;
// class Problem;
// class Scope;
// class ScopeHistory;
// class Solution;

// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEGATIVE = 2;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_POSITIVE = 3;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE = 4;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 5;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST = 6;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 7;
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND = 8;
// #if defined(MDEBUG) && MDEBUG
// const int INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY = 9;
// #endif /* MDEBUG */

// const double DISABLE_NEGATIVE_PERCENTAGE = 0.05;
// const double DISABLE_POSITIVE_PERCENTAGE = 0.95;

// class InfoPassThroughExperimentHistory;
// class InfoPassThroughExperiment : public AbstractExperiment {
// public:
// 	InfoScope* info_scope_context;

// 	int num_instances_until_target;

// 	int state;
// 	int state_iter;
// 	int sub_state_iter;

// 	double existing_average_score;
// 	double existing_score_standard_deviation;

// 	double new_score;
// 	/**
// 	 * - swap in fully from explore
// 	 *   - so removed sequence needs to not have big impact
// 	 */
// 	InfoScope* info_scope;
// 	bool is_negate;
// 	std::vector<int> step_types;
// 	std::vector<ActionNode*> actions;
// 	std::vector<InfoScopeNode*> scopes;
// 	AbstractNode* exit_next_node;

// 	ActionNode* ending_node;

// 	std::vector<AbstractNode*> input_node_contexts;
// 	std::vector<int> input_obs_indexes;

// 	double negative_average_score;

// 	std::vector<double> negative_linear_weights;
// 	std::vector<std::vector<int>> negative_network_input_indexes;
// 	Network* negative_network;
// 	double negative_average_misguess;
// 	double negative_misguess_standard_deviation;

// 	double positive_average_score;

// 	std::vector<double> positive_linear_weights;
// 	std::vector<std::vector<int>> positive_network_input_indexes;
// 	Network* positive_network;
// 	double positive_average_misguess;
// 	double positive_misguess_standard_deviation;

// 	int negative_count;
// 	int positive_count;

// 	int new_state;

// 	std::vector<ScopeHistory*> i_scope_histories;
// 	std::vector<double> i_target_val_histories;

// 	#if defined(MDEBUG) && MDEBUG
// 	std::vector<Problem*> verify_problems;
// 	std::vector<unsigned long> verify_seeds;
// 	std::vector<double> verify_negative_scores;
// 	std::vector<double> verify_positive_scores;
// 	#endif /* MDEBUG */

// 	InfoPassThroughExperiment(InfoScope* info_scope_context,
// 							  Scope* scope_context,
// 							  AbstractNode* node_context,
// 							  bool is_branch);
// 	~InfoPassThroughExperiment();
// 	void decrement(AbstractNode* experiment_node);

// 	void info_scope_activate(RunHelper& run_helper);

// 	bool activate(AbstractNode* experiment_node,
// 				  bool is_branch,
// 				  AbstractNode*& curr_node,
// 				  Problem* problem,
// 				  std::vector<ContextLayer>& context,
// 				  RunHelper& run_helper);
// 	bool back_activate(Problem* problem,
// 					   ScopeHistory*& subscope_history,
// 					   bool& result_is_positive,
// 					   RunHelper& run_helper);
// 	void backprop(double target_val,
// 				  RunHelper& run_helper);

// 	void measure_existing_activate(InfoPassThroughExperimentHistory* history);
// 	void measure_existing_backprop(double target_val,
// 								   RunHelper& run_helper);

// 	void explore_activate(AbstractNode*& curr_node,
// 						  Problem* problem,
// 						  RunHelper& run_helper);
// 	void explore_backprop(double target_val,
// 						  RunHelper& run_helper);

// 	void train_negative_activate(AbstractNode*& curr_node,
// 								 Problem* problem,
// 								 std::vector<ContextLayer>& context,
// 								 RunHelper& run_helper,
// 								 InfoPassThroughExperimentHistory* history);
// 	bool train_negative_back_activate(ScopeHistory*& subscope_history,
// 									  bool& result_is_positive,
// 									  RunHelper& run_helper);
// 	void train_negative_backprop(double target_val,
// 								 RunHelper& run_helper);

// 	void train_positive_activate(AbstractNode*& curr_node,
// 								 Problem* problem,
// 								 std::vector<ContextLayer>& context,
// 								 RunHelper& run_helper,
// 								 InfoPassThroughExperimentHistory* history);
// 	bool train_positive_back_activate(ScopeHistory*& subscope_history,
// 									  bool& result_is_positive,
// 									  RunHelper& run_helper);
// 	void train_positive_backprop(double target_val,
// 								 RunHelper& run_helper);

// 	void measure_activate(AbstractNode*& curr_node,
// 						  Problem* problem,
// 						  std::vector<ContextLayer>& context,
// 						  RunHelper& run_helper,
// 						  InfoPassThroughExperimentHistory* history);
// 	void measure_back_activate(ScopeHistory*& subscope_history,
// 							   bool& result_is_positive,
// 							   RunHelper& run_helper);
// 	void measure_backprop(double target_val,
// 						  RunHelper& run_helper);

// 	void verify_existing_backprop(double target_val,
// 								  RunHelper& run_helper);

// 	void verify_activate(AbstractNode*& curr_node,
// 						 Problem* problem,
// 						 std::vector<ContextLayer>& context,
// 						 RunHelper& run_helper,
// 						 InfoPassThroughExperimentHistory* history);
// 	void verify_back_activate(ScopeHistory*& subscope_history,
// 							  bool& result_is_positive,
// 							  RunHelper& run_helper);
// 	void verify_backprop(double target_val,
// 						 RunHelper& run_helper);

// 	#if defined(MDEBUG) && MDEBUG
// 	void capture_verify_activate(AbstractNode*& curr_node,
// 								 Problem* problem,
// 								 std::vector<ContextLayer>& context,
// 								 RunHelper& run_helper,
// 								 InfoPassThroughExperimentHistory* history);
// 	void capture_verify_back_activate(Problem* problem,
// 									  ScopeHistory*& subscope_history,
// 									  bool& result_is_positive,
// 									  RunHelper& run_helper);
// 	void capture_verify_backprop();
// 	#endif /* MDEBUG */

// 	void finalize(Solution* duplicate);
// 	void new_branch(Solution* duplicate);
// 	void new_pass_through(Solution* duplicate);
// };

// class InfoPassThroughExperimentHistory : public AbstractExperimentHistory {
// public:
// 	int instance_count;

// 	InfoPassThroughExperimentHistory(InfoPassThroughExperiment* experiment);
// };

// #endif /* INFO_PASS_THROUGH_EXPERIMENT_H */