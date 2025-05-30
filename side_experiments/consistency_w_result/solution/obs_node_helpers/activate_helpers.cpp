#include "obs_node.h"

#include <iostream>

#include "factor.h"
#include "scope.h"
#include "problem.h"

using namespace std;

void ObsNode::activate(AbstractNode*& curr_node,
					   Problem* problem,
					   RunHelper& run_helper,
					   ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	// temp
	// cout << this->parent->id << " " << this->id << endl;
	// bool has_match = false;
	// for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
	// 	map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories
	// 		.find(this->matches[m_index].node_context[0]);
	// 	if (it != scope_history->node_histories.end()) {
	// 		ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;

	// 		double predicted_score = early_history->obs_history[0] * this->matches[m_index].weight + this->matches[m_index].constant;
	// 		double factor = abs(obs[0] - predicted_score) / this->matches[m_index].standard_deviation;

	// 		cout << "obs[0]: " << obs[0] << endl;
	// 		cout << "predicted_score: " << predicted_score << endl;
	// 		cout << "early_history->obs_history[0]: " << early_history->obs_history[0] << endl;
	// 		cout << "this->matches[m_index].standard_deviation: " << this->matches[m_index].standard_deviation << endl;
	// 		cout << "factor: " << factor << endl;

	// 		has_match = true;
	// 		break;
	// 	}
	// }
	// if (!has_match) {
	// 	double factor = abs(obs[0] - this->average_val) / this->standard_deviation;
	// 	run_helper.match_factors.push_back(factor);

	// 	cout << "obs[0]: " << obs[0] << endl;
	// 	cout << "this->average_val: " << this->average_val << endl;
	// 	cout << "this->standard_deviation: " << this->standard_deviation << endl;
	// 	cout << "factor: " << factor << endl;
	// }

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	curr_node = this->next_node;
}
