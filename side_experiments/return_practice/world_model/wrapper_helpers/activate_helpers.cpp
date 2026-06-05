#include "wrapper.h"

#include "abstract_node.h"
#include "run.h"
#include "solution.h"
#include "utilities.h"
#include "world_model.h"
#include "world_model_helpers.h"

using namespace std;

void Wrapper::init(Run* run) {
	run->wrapper = this;

	run->node_context = this->solution->nodes[0];

	run->state = vector<double>(this->curr_model->num_states, 0.0);

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */
}

pair<bool,int> Wrapper::step(vector<double> obs,
							 Run* run) {
	obs_helper(obs,
			   run->state,
			   this);

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (run->node_context == NULL) {
			is_next = true;
			is_done = true;
		} else {
			run->node_context->step(action,
									is_next,
									run);
		}
	}

	return pair<bool,int>{is_done, action};
}
