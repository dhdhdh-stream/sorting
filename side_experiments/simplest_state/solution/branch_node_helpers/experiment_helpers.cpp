#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);
			if (wrapper->run_type != RUN_TYPE_EXPLORE) {
				InitNetworkHistory* init_network_history = new InitNetworkHistory(this->init_networks[n_index]);
				this->init_networks[n_index]->save(init_network_history);
				wrapper->network_histories.push_back(init_network_history);
			}

			if (wrapper->run_type == RUN_TYPE_EXISTING) {
				this->prev_init_networks[n_index]->activate(wrapper->prev_state,
															obs);
			}

			if (wrapper->partial_state.size() > 0) {
				uniform_int_distribution<int> add_noise_distribution(0, 9);
				if (add_noise_distribution(generator) == 0) {
					for (int i_index = 0; i_index < (int)this->init_networks[n_index]->init_states.size(); i_index++) {
						int state = this->init_networks[n_index]->init_states[i_index];
						normal_distribution<double> distribution(0.0, wrapper->solution->state_diffs[state]);
						wrapper->partial_state[state] += distribution(generator);
					}
				}

				this->init_networks[n_index]->activate(wrapper->partial_state,
													   obs);
				InitNetworkHistory* init_network_history = new InitNetworkHistory(this->init_networks[n_index]);
				this->init_networks[n_index]->save(init_network_history);
				wrapper->partial_network_histories.push_back(init_network_history);
			}
		}
	}

	uniform_int_distribution<int> on_distribution(0, this->ramp_num_gears);
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
	} else if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
	} else if (this->ramp < this->ramp_num_gears
			&& this->ramp < on_distribution(generator)) {
		wrapper->node_context.back() = this->original_next_node;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		BranchNodeHistory* history = new BranchNodeHistory(this);
		history->index = (int)scope_history->node_histories.size();
		scope_history->node_histories[this->id] = history;

		if (this->dependencies.size() > 0) {
			history->obs = obs;
		}

		bool is_branch;
		this->original_network->activate(wrapper->state);
		this->branch_network->activate(wrapper->state);
		uniform_int_distribution<int> maintain_distribution(0, 9);
		if (wrapper->run_type == RUN_TYPE_EXISTING
				&& maintain_distribution(generator) == 0) {
			this->prev_original_network->activate(wrapper->prev_state);
			this->prev_branch_network->activate(wrapper->prev_state);
			if (this->prev_branch_network->output->acti_vals(0) >= this->prev_original_network->output->acti_vals(0)) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		} else {
			if (this->branch_network->output->acti_vals(0) >= this->original_network->output->acti_vals(0)) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#endif /* MDEBUG */

		history->is_branch = is_branch;

		if (is_branch) {
			if (wrapper->run_type == RUN_TYPE_EXISTING) {
				this->consec_original = 0;
				this->consec_branch++;
			}

			if (wrapper->run_type == RUN_TYPE_EXISTING) {
				ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->branch_network);
				this->branch_network->save(score_network_history);
				wrapper->network_histories.push_back(score_network_history);
			}

			if (wrapper->partial_state.size() > 0) {
				this->branch_network->activate(wrapper->partial_state);
				ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->branch_network);
				this->branch_network->save(score_network_history);
				wrapper->partial_network_histories.push_back(score_network_history);
			}

			wrapper->node_context.back() = this->branch_next_node;

			if (this->branch_experiment != NULL) {
				this->branch_experiment->experiment_check_activate(
					obs,
					wrapper);
			}
		} else {
			if (wrapper->run_type == RUN_TYPE_EXISTING) {
				this->consec_original++;
				this->consec_branch = 0;
			}

			if (wrapper->run_type == RUN_TYPE_EXISTING) {
				ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->original_network);
				this->original_network->save(score_network_history);
				wrapper->network_histories.push_back(score_network_history);
			}

			if (wrapper->partial_state.size() > 0) {
				this->original_network->activate(wrapper->partial_state);
				ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->original_network);
				this->original_network->save(score_network_history);
				wrapper->partial_network_histories.push_back(score_network_history);
			}

			wrapper->node_context.back() = this->original_next_node;

			if (this->original_experiment != NULL) {
				this->original_experiment->experiment_check_activate(
					obs,
					wrapper);
			}
		}
	}
}
