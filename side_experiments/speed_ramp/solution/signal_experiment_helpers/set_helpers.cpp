#include "signal_experiment.h"

#include "globals.h"
#include "scope.h"
#include "signal.h"

using namespace std;

void SignalExperiment::set_actions() {
	if (this->scope_context->default_signal == NULL) {
		geometric_distribution<int> num_actions_distribution(0.2);

		int num_pre = num_actions_distribution(generator);
		this->pre_action_initialized = vector<bool>(num_pre, false);
		this->pre_actions = vector<int>(num_pre, -1);

		int num_post = 5 + num_actions_distribution(generator);
		this->post_action_initialized = vector<bool>(num_post, false);
		this->post_actions = vector<int>(num_post, -1);
	} else {
		for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
			this->adjusted_previous_signals.push_back(
				new Signal(this->scope_context->signals[s_index]));
		}

		/**
		 * - simply try one change pre, one change post
		 */
		geometric_distribution<int> exit_distribution(0.5);
		geometric_distribution<int> length_distribution(0.2);
		/**
		 * - focus on extending rather than minimizing
		 *   - found signals likely to be safe
		 *     - not likely much to be gained from minimizing
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
		this->pre_action_initialized = vector<bool>(this->pre_actions.size(), true);
		int pre_insert_length = length_distribution(generator);
		for (int a_index = 0; a_index < pre_insert_length; a_index++) {
			this->pre_action_initialized.insert(this->pre_action_initialized.begin() + pre_index, false);
			this->pre_actions.insert(this->pre_actions.begin() + pre_index, -1);
		}
		for (int s_index = 0; s_index < (int)this->adjusted_previous_signals.size(); s_index++) {
			this->adjusted_previous_signals[s_index]->insert(
				true,
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
		this->post_action_initialized = vector<bool>(this->post_actions.size(), true);
		int post_insert_length = length_distribution(generator);
		for (int a_index = 0; a_index < post_insert_length; a_index++) {
			this->post_action_initialized.insert(this->post_action_initialized.begin() + post_index, false);
			this->post_actions.insert(this->post_actions.begin() + post_index, -1);
		}
		for (int s_index = 0; s_index < (int)this->adjusted_previous_signals.size(); s_index++) {
			this->adjusted_previous_signals[s_index]->insert(
				false,
				post_index,
				post_exit_index,
				post_insert_length);
		}
	}
}
