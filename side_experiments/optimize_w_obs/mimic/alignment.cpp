#include "alignment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "sample.h"
#include "scope.h"
#include "solution.h"

using namespace std;

double Alignment::score() {
	double result = 0.0;

	int last_match = -1;
	for (int s_index = 0; s_index < (int)this->step_nodes.size(); s_index++) {
		if (this->step_nodes[s_index].size() > 0) {
			result += sqrt((double)(s_index - last_match - 1));

			result += sqrt((double)(this->step_nodes[s_index].size()-1));

			last_match = s_index;
		}
	}
	result += sqrt((double)(this->step_nodes.size() - last_match));

	return result;
}

string print_helper(int move) {
	switch (move) {
	case -1:
		return "NOO";
	case 0:
		return "UP ";
	case 1:
		return "RIG";
	case 2:
		return "DOW";
	case 3:
		return "LEF";
	case 4:
		return "CLI";
	case 5:
		return "FLA";
	case 6:
		return "DOU";
	}

	return "   ";
}

void Alignment::print() {
	for (int s_index = 0; s_index < (int)this->step_nodes.size()-1; s_index++) {
		if (this->step_nodes[s_index].size() == 0) {
			cout << "   " << " " << print_helper(this->sample->actions[s_index].move) << endl;
		} else {
			for (int n_index = 0; n_index < (int)this->step_nodes[s_index].size()-1; n_index++) {
				Scope* scope = solution->scopes[this->step_nodes[s_index][n_index].first.back()];
				ActionNode* node = (ActionNode*)scope->nodes[this->step_nodes[s_index][n_index].second.back()];

				cout << print_helper(node->action.move) << " " << "   " << endl;
			}
			{
				Scope* scope = solution->scopes[this->step_nodes[s_index].back().first.back()];
				ActionNode* node = (ActionNode*)scope->nodes[this->step_nodes[s_index].back().second.back()];

				cout << print_helper(node->action.move) << " " << print_helper(this->sample->actions[s_index].move) << endl;
			}
		}
	}
	{
		for (int n_index = 0; n_index < (int)this->step_nodes.back().size(); n_index++) {
			Scope* scope = solution->scopes[this->step_nodes.back()[n_index].first.back()];
			ActionNode* node = (ActionNode*)scope->nodes[this->step_nodes.back()[n_index].second.back()];

			cout << print_helper(node->action.move) << " " << "   " << endl;
		}
	}
}
