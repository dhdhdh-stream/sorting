#ifndef INFO_PASS_THROUGH_EXPERIMENT_H
#define INFO_PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class InfoScope;
class InfoScopeNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

const int INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
const int INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_EXISTING = 2;
const int INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW = 3;
const int INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE = 4;
const int INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING = 5;
const int INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY = 6;
#if defined(MDEBUG) && MDEBUG
const int INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY = 7;
#endif /* MDEBUG */

class InfoPassThroughExperimentHistory;
class InfoPassThroughExperiment : public AbstractExperiment {
public:
	int num_instances_until_target;

	int state;
	int state_iter;
	int sub_state_iter;
	int explore_iter;

	double existing_average_score;

	double info_score;
	/**
	 * - swap in fully from explore
	 *   - so removed sequence needs to not have big impact
	 */
	std::vector<ActionNode*> actions;
	AbstractNode* exit_next_node;

	ActionNode* ending_node;

	std::vector<AbstractNode*> existing_input_node_contexts;
	std::vector<int> existing_input_obs_indexes;
	Network* existing_network;

	std::vector<AbstractNode*> new_input_node_contexts;
	std::vector<int> new_input_obs_indexes;
	Network* new_network;

	double combined_score;

	std::vector<AbstractScopeHistory*> scope_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	InfoPassThroughExperiment(AbstractScope* scope_context,
							  AbstractNode* node_context,
							  int score_type);
	~InfoPassThroughExperiment();
	void decrement(AbstractNode* experiment_node);

	void info_pre_activate(RunHelper& run_helper);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	bool info_back_activate(Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							bool& is_positive);
	void back_activate(std::vector<ContextLayer>& context,
					   RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_activate(InfoPassThroughExperimentHistory* history);
	void measure_existing_info_back_activate(std::vector<ContextLayer>& context,
											 RunHelper& run_helper);
	void measure_existing_back_activate(std::vector<ContextLayer>& context,
										RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem);
	void explore_info_back_activate(std::vector<ContextLayer>& context,
									RunHelper& run_helper);
	void explore_back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void train_existing_activate(AbstractNode*& curr_node);
	bool train_existing_info_back_activate(std::vector<ContextLayer>& context,
										   RunHelper& run_helper,
										   bool& is_positive);
	void train_existing_back_activate(std::vector<ContextLayer>& context,
									  RunHelper& run_helper);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node);
	bool train_new_info_back_activate(std::vector<ContextLayer>& context,
									  RunHelper& run_helper,
									  bool& is_positive);
	void train_new_back_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	void measure_activate(AbstractNode*& curr_node);
	void measure_info_back_activate(std::vector<ContextLayer>& context,
									RunHelper& run_helper,
									bool& is_positive);
	void measure_back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_info_back_activate(std::vector<ContextLayer>& context,
											RunHelper& run_helper);
	void verify_existing_back_activate(std::vector<ContextLayer>& context,
									   RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(AbstractNode*& curr_node);
	void verify_info_back_activate(std::vector<ContextLayer>& context,
								   RunHelper& run_helper,
								   bool& is_positive);
	void verify_back_activate(std::vector<ContextLayer>& context,
							  RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(AbstractNode*& curr_node);
	void capture_verify_info_back_activate(Problem* problem,
										   std::vector<ContextLayer>& context,
										   RunHelper& run_helper,
										   bool& is_positive);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class InfoPassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	std::vector<double> existing_predicted_scores;

	InfoPassThroughExperimentHistory(InfoPassThroughExperiment* experiment);
};

#endif /* INFO_PASS_THROUGH_EXPERIMENT_H */