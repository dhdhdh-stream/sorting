#include "loop_experiment.h"

using namespace std;

void LoopExperiment::seed_input_experiment_activate_helper(
		vector<vector<double>>& input_vals, 
		LoopExperimentHistory* seed_history,
		LoopExperimentHistory* history) {
	for (int i_index = 0; i_index < (int)seed_history->sequence_histories.size(); i_index++) {
		run_helper.scale_factor *= this->scale_mod->weight;

		SequenceHistory* sequence_history = new SequenceHistory(this->sequence);
		history->sequence_histories.push_back(sequence_history);
		this->sequence->seed_input_activate(seed_history->sequence_histories[i_index],
											sequence_history);

		run_helper.scale_factor /= this->scale_mod->weight;
	}
}

void LoopExperiment::seed_input_scope_activate_helper(
		) {

}
