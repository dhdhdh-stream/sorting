#include "solution_helpers.h"

#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "globals.h"
#include "multi_commit_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

const int NUM_MEASURE = 20000;

void multi_measure(vector<AbstractExperiment*>& candidates) {
	vector<double> target_vals;
	vector<vector<int>> influence_indexes;
	map<AbstractExperiment*, int> influence_mapping;

	for (int c_index = 0; c_index < (int)candidates.size(); c_index++) {
		MultiCommitExperiment* multi_commit_experiment = (MultiCommitExperiment*)candidates[c_index];
		multi_commit_experiment->state = MULTI_COMMIT_EXPERIMENT_STATE_OUTER_MEASURE;

		influence_mapping[candidates[c_index]] = (int)influence_mapping.size();
	}

	auto start_time = chrono::high_resolution_clock::now();

	for (int iter_index = 0; iter_index < NUM_MEASURE; iter_index++) {
		run_index++;

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->experiment_activate(
				problem,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		delete scope_history;
		delete problem;

		target_vals.push_back(target_val);

		vector<int> curr_influence_indexes;
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			MultiCommitExperimentHistory* multi_commit_experiment_history
				= (MultiCommitExperimentHistory*)it->second;
			if (multi_commit_experiment_history->is_active) {
				curr_influence_indexes.push_back(influence_mapping[it->first]);
			}
		}
		influence_indexes.push_back(curr_influence_indexes);
	}

	double sum_target_vals = 0.0;
	for (int h_index = 0; h_index < NUM_MEASURE; h_index++) {
		sum_target_vals += target_vals[h_index];
	}
	double average_target_val = sum_target_vals / NUM_MEASURE;
	cout << "average_target_val: " << average_target_val << endl;

	Eigen::MatrixXd inputs(NUM_MEASURE, candidates.size());
	for (int i_index = 0; i_index < NUM_MEASURE; i_index++) {
		for (int m_index = 0; m_index < (int)candidates.size(); m_index++) {
			inputs(i_index, m_index) = 0.0;
		}
	}
	for (int h_index = 0; h_index < NUM_MEASURE; h_index++) {
		for (int i_index = 0; i_index < (int)influence_indexes[h_index].size(); i_index++) {
			inputs(h_index, influence_indexes[h_index][i_index]) = 1.0;
		}
	}

	Eigen::VectorXd outputs(NUM_MEASURE);
	for (int h_index = 0; h_index < NUM_MEASURE; h_index++) {
		outputs(h_index) = target_vals[h_index] - average_target_val;
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		weights = Eigen::VectorXd(candidates.size());
		for (int f_index = 0; f_index < (int)candidates.size(); f_index++) {
			weights(f_index) = 0.0;
		}
	}

	cout << "weights:" << endl;
	for (int c_index = 0; c_index < (int)candidates.size(); c_index++) {
		cout << c_index << ": " << weights(c_index) << endl;
	}

	int best_index = -1;
	double best_impact = numeric_limits<double>::lowest();
	for (int c_index = 0; c_index < (int)candidates.size(); c_index++) {
		if (weights(c_index) > best_impact) {
			best_index = c_index;
			best_impact = weights(c_index);
		}
	}

	vector<int> counts(candidates.size(), 0);
	for (int h_index = 0; h_index < (int)influence_indexes.size(); h_index++) {
		for (int i_index = 0; i_index < (int)influence_indexes[h_index].size(); i_index++) {
			counts[influence_indexes[h_index][i_index]]++;
		}
	}
	cout << "counts:" << endl;
	for (int c_index = 0; c_index < (int)candidates.size(); c_index++) {
		cout << c_index << ": " << counts[c_index] << endl;
	}

	// temp
	for (int c_index = 0; c_index < (int)candidates.size(); c_index++) {
		Solution* duplicate = new Solution(solution);

		int last_updated_scope_id = candidates[c_index]->scope_context->id;

		candidates[c_index]->finalize(duplicate);

		Scope* experiment_scope = duplicate->scopes[last_updated_scope_id];
		clean_scope(experiment_scope,
					duplicate);

		double sum_score = 0.0;
		double sum_true_score = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
			duplicate->scopes[0]->measure_activate(
				problem,
				run_helper,
				scope_history);
			delete scope_history;

			double target_val = problem->score_result();
			sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
				- run_helper.num_analyze * solution->curr_time_penalty;
			sum_true_score += target_val;

			delete problem;
		}

		cout << "curr_score: " << sum_score / MEASURE_ITERS << endl;

		delete duplicate;
	}

	for (int c_index = 0; c_index < (int)candidates.size(); c_index++) {
		if (c_index == best_index) {
			candidates[c_index]->finalize(solution);
			delete candidates[c_index];
		} else {
			candidates[c_index]->result = EXPERIMENT_RESULT_FAIL;
			candidates[c_index]->finalize(NULL);
			delete candidates[c_index];
		}
	}

	solution->clear_experiments();

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		clean_scope(solution->scopes[s_index],
					solution);

		if (solution->scopes[s_index]->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
			solution->scopes[s_index]->exceeded = true;
		}
		if (solution->scopes[s_index]->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
			solution->scopes[s_index]->exceeded = false;
		}
	}
}
