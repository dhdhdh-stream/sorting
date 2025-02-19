#include "new_scope_experiment.h"

#include "abstract_node.h"
#include "globals.h"
#include "scope.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 10;
#else
const int NEW_SCOPE_NUM_DATAPOINTS = 100;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 500;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 2000;
#endif /* MDEBUG */

void NewScopeExperiment::test_activate(
		int test_index,
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		NewScopeExperimentOverallHistory* overall_history) {
	bool is_new;
	map<int, bool>::iterator is_new_it = overall_history->test_is_new.find(test_index);
	if (is_new_it == overall_history->test_is_new.end()) {
		uniform_int_distribution<int> is_active_distribution(0, 3);
		is_new = is_active_distribution(generator) == 0;
		overall_history->test_is_new[test_index] = is_new;
	} else {
		is_new = is_new_it->second;
	}

	if (is_new) {
		ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
		this->new_scope->activate(problem,
			run_helper,
			inner_scope_history);
		delete inner_scope_history;

		curr_node = this->test_exits[test_index];
	}
}

void NewScopeExperiment::test_update(
		NewScopeExperimentOverallHistory* overall_history,
		double target_val) {
	for (map<int, bool>::iterator it = overall_history->test_is_new.begin();
			it != overall_history->test_is_new.end(); it++) {
		if (it->second) {
			this->test_new_scores[it->first] += target_val;
			this->test_new_counts[it->first]++;
		} else {
			this->test_existing_scores[it->first] += target_val;
			this->test_existing_counts[it->first]++;
		}
	}

	for (int t_index = (int)this->test_starts.size()-1; t_index >= 0; t_index--) {
		if (this->test_new_counts[t_index] == NEW_SCOPE_NUM_DATAPOINTS
				|| this->test_new_counts[t_index] == NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double existing_score = this->test_existing_scores[t_index] / this->test_existing_counts[t_index];
			double new_score = this->test_new_scores[t_index] / this->test_new_counts[t_index];
			if (new_score <= existing_score) {
			#endif /* MDEBUG */
				this->test_starts[t_index]->experiment = NULL;

				this->test_starts.erase(this->test_starts.begin() + t_index);
				this->test_is_branch.erase(this->test_is_branch.begin() + t_index);
				this->test_exits.erase(this->test_exits.begin() + t_index);
				this->test_existing_scores.erase(this->test_existing_scores.begin() + t_index);
				this->test_existing_counts.erase(this->test_existing_counts.begin() + t_index);
				this->test_new_scores.erase(this->test_new_scores.begin() + t_index);
				this->test_new_counts.erase(this->test_new_counts.begin() + t_index);

				if (this->generalize_iter == -1) {
					this->result = EXPERIMENT_RESULT_FAIL;
					/**
					 * - only continue if first succeeds
					 */
				} else {
					this->generalize_iter++;
				}
			}
		} else if (this->test_new_counts[t_index] == NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double existing_score = this->test_existing_scores[t_index] / this->test_existing_counts[t_index];
			double new_score = this->test_new_scores[t_index] / this->test_new_counts[t_index];
			if (new_score <= existing_score) {
			#endif /* MDEBUG */
				this->test_starts[t_index]->experiment = NULL;

				this->test_starts.erase(this->test_starts.begin() + t_index);
				this->test_is_branch.erase(this->test_is_branch.begin() + t_index);
				this->test_exits.erase(this->test_exits.begin() + t_index);
				this->test_existing_scores.erase(this->test_existing_scores.begin() + t_index);
				this->test_existing_counts.erase(this->test_existing_counts.begin() + t_index);
				this->test_new_scores.erase(this->test_new_scores.begin() + t_index);
				this->test_new_counts.erase(this->test_new_counts.begin() + t_index);

				if (this->generalize_iter == -1) {
					this->result = EXPERIMENT_RESULT_FAIL;
					/**
					 * - only continue if first succeeds
					 */
				} else {
					this->generalize_iter++;
				}
			} else {
				this->successful_starts.push_back(this->test_starts[t_index]);
				this->successful_is_branch.push_back(this->test_is_branch[t_index]);
				this->successful_exits.push_back(this->test_exits[t_index]);

				this->test_starts.erase(this->test_starts.begin() + t_index);
				this->test_is_branch.erase(this->test_is_branch.begin() + t_index);
				this->test_exits.erase(this->test_exits.begin() + t_index);
				this->test_existing_scores.erase(this->test_existing_scores.begin() + t_index);
				this->test_existing_counts.erase(this->test_existing_counts.begin() + t_index);
				this->test_new_scores.erase(this->test_new_scores.begin() + t_index);
				this->test_new_counts.erase(this->test_new_counts.begin() + t_index);

				this->generalize_iter++;
			}
		}
	}

	if (this->successful_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
