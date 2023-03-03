#include "fold.h"

using namespace std;

void Fold::add_inner_state_explore_off_path_score_activate() {

}

void Fold::add_inner_state_explore_off_path_sequence_activate() {

}

void Fold::add_inner_state_update_score_activate(vector<double>& local_state_vals,
												 vector<double>& input_vals,
												 vector<int>& context_iter,
												 vector<ContextHistory*> context_histories,
												 RunHelper& run_helper,
												 FoldHistory* history) {
	// no special case
}


// actually, potentially changes results, so do as part of explore
void Fold::add_inner_state_update_sequence_activate() {
	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);
	vector<double> new_outer_state_vals = history->new_outer_state_vals;


}
