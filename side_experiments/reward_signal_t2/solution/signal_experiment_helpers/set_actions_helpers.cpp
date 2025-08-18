#include "signal_experiment.h"

#include "globals.h"
#include "scope.h"

using namespace std;

void SignalExperiment::set_actions() {
	uniform_int_distribution<int> action_distribution(0, 2);
	/**
	 * TODO: add easy way to fetch action
	 */
	if (this->scope_context->signals.size() == 0) {
		geometric_distribution<int> num_actions_distribution(0.2);
		int num_pre = num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_pre; a_index++) {
			this->pre_actions.push_back(action_distribution(generator));
		}
		int num_post = 5 + num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_post; a_index++) {
			this->post_actions.push_back(action_distribution(generator));
		}
	} else {
		for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
			this->signals.push_back(new Signal(this->scope_context->signals[s_index]));
		}

		/**
		 * - simply try one change pre, one change post
		 */
		geometric_distribution<int> exit_distribution(0.2);
		geometric_distribution<int> length_distribution(0.3);
		/**
		 * - length smaller than exit to try to reduce actions
		 */

		uniform_int_distribution<int> pre_index_distribution(0, this->scope_context->signal_pre_actions.size());
		int pre_index = pre_index_distribution(generator);
		int pre_exit_index;
		while (true) {
			pre_exit_index = pre_index + exit_distribution(generator);
			if (pre_exit_index <= (int)this->scope_context->signal_pre_actions.size()) {
				break;
			}
		}
		this->pre_actions = this->scope_context->signal_pre_actions;
		this->pre_actions.erase(this->pre_actions.begin() + pre_index,
			this->pre_actions.begin() + pre_exit_index);
		int pre_insert_length = length_distribution(generator);
		for (int a_index = 0; a_index < pre_insert_length; a_index++) {
			this->pre_actions.insert(this->pre_actions.begin() + pre_index, action_distribution(generator));
		}
		for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
			this->signals[s_index]->insert(true,
										   pre_index,
										   pre_exit_index,
										   pre_insert_length);
		}

		uniform_int_distribution<int> post_index_distribution(0, this->scope_context->signal_post_actions.size());
		int post_index = post_index_distribution(generator);
		int post_exit_index;
		while (true) {
			post_exit_index = post_index + exit_distribution(generator);
			if (post_exit_index <= (int)this->scope_context->signal_post_actions.size()) {
				break;
			}
		}
		this->post_actions = this->scope_context->signal_post_actions;
		this->post_actions.erase(this->post_actions.begin() + post_index,
			this->post_actions.begin() + post_exit_index);
		int post_insert_length = length_distribution(generator);
		for (int a_index = 0; a_index < post_insert_length; a_index++) {
			this->post_actions.insert(this->post_actions.begin() + post_index, action_distribution(generator));
		}
		for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
			this->signals[s_index]->insert(false,
										   post_index,
										   post_exit_index,
										   post_insert_length);
		}
	}
}
