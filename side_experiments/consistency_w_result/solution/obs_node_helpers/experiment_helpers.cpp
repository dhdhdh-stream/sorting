#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "factor.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ObsNode::experiment_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	if (run_helper.check_match && this->is_init) {
		if (this->check_consistency) {
			double factor = abs(obs[0] - this->average_val) / this->standard_deviation;
			run_helper.match_factors.push_back(factor);
		} else {
			for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
				map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories
					.find(this->matches[m_index].node_context[0]);
				if (it != scope_history->node_histories.end()) {
					ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;

					double predicted_score = early_history->obs_history[0] * this->matches[m_index].weight + this->matches[m_index].constant;
					double factor = abs(obs[0] - predicted_score) / this->matches[m_index].standard_deviation;
					run_helper.match_factors.push_back(factor);

					break;
				}
			}
		}
	}

	// temp
	// cout << this->parent->id << " " << this->id << endl;
	// cout << "obs[0]: " << obs[0] << endl;
	// cout << "this->average_val: " << this->average_val << endl;
	// cout << "this->standard_deviation: " << this->standard_deviation << endl;
	// cout << "this->check_consistency: " << this->check_consistency << endl;
	// cout << "this->min_standard_deviation: " << this->min_standard_deviation << endl;
	// cout << "this->min_average_val: " << this->min_average_val << endl;
	// cout << "this->max_standard_deviation: " << this->max_standard_deviation << endl;
	// cout << "this->max_average_val: " << this->max_average_val << endl;
	// if (this->check_consistency) {
	// 	double factor = abs(obs[0] - this->average_val) / this->standard_deviation;
	// 	run_helper.match_factors.push_back(factor);

	// 	cout << "factor: " << factor << endl;
	// } else {
	// 	for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
	// 		map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories
	// 			.find(this->matches[m_index].node_context[0]);
	// 		if (it != scope_history->node_histories.end()) {
	// 			ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;

	// 			double predicted_score = early_history->obs_history[0] * this->matches[m_index].weight + this->matches[m_index].constant;
	// 			double factor = abs(obs[0] - predicted_score) / this->matches[m_index].standard_deviation;

	// 			cout << "this->matches[m_index].node_context[0]: " << this->matches[m_index].node_context[0] << endl;
	// 			cout << "predicted_score: " << predicted_score << endl;
	// 			cout << "early_history->obs_history[0]: " << early_history->obs_history[0] << endl;
	// 			cout << "this->matches[m_index].standard_deviation: " << this->matches[m_index].standard_deviation << endl;
	// 			cout << "this->matches[m_index].average_distance: " << this->matches[m_index].average_distance << endl;
	// 			cout << "factor: " << factor << endl;

	// 			break;
	// 		}
	// 	}
	// }

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->activate(
			this,
			false,
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}
