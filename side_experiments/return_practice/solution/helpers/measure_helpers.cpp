#include "solution_helpers.h"

#include "network.h"
#include "run.h"
#include "test_indirect.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 40;
#else
const int MEASURE_NUM_ITERS = 4000;
#endif /* MDEBUG */

void measure_helper(Wrapper* wrapper,
					double& score_average,
					double& misguess_average) {
	ProblemType* problem_type = new TypeTestIndirect();

	double sum_scores = 0.0;
	double sum_misguess = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_NUM_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		Run* run = new Run();

		wrapper->init(run);

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs,
												run);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		sum_scores += target_val;

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(run->state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}
		sum_misguess += (target_val - sum_score) * (target_val - sum_score);

		delete run;

		delete problem;
	}

	score_average = sum_scores / MEASURE_NUM_ITERS;
	misguess_average = sum_misguess / MEASURE_NUM_ITERS;;

	delete problem_type;
}
