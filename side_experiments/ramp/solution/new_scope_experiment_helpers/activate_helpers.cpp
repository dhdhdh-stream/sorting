#include "new_scope_experiment.h"

#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_1_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_5_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_10_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_25_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_50_PERCENT_NUM_DATAPOINTS = 2;
#else
const int MEASURE_1_PERCENT_NUM_DATAPOINTS = 10;
const int MEASURE_5_PERCENT_NUM_DATAPOINTS = 50;
const int MEASURE_10_PERCENT_NUM_DATAPOINTS = 100;
const int MEASURE_25_PERCENT_NUM_DATAPOINTS = 250;
const int MEASURE_50_PERCENT_NUM_DATAPOINTS = 500;
#endif /* MDEBUG */

void NewScopeExperiment::activate(AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
		= run_helper.experiment_histories.find(this);
	NewScopeExperimentHistory* history;
	if (it == run_helper.experiment_histories.end()) {
		history = new NewScopeExperimentHistory(this);
		run_helper.experiment_histories[this] = history;
	} else {
		history = (NewScopeExperimentHistory*)it->second;
	}

	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_1_PERCENT:
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_5_PERCENT:
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_10_PERCENT:
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_25_PERCENT:
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_50_PERCENT:
		run_helper.num_experiment_instances++;
		if (history->is_active) {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scope_context->new_scope);
			this->scope_context->new_scope->activate(
				problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;

			curr_node = this->exit_next_node;
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_SUCCESS:
		if (history->is_active) {
			curr_node = this->successful_scope_node;
		}
		break;
	}
}

void NewScopeExperiment::backprop(double target_val,
								  AbstractExperimentHistory* history,
								  set<Scope*>& updated_scopes) {
	NewScopeExperimentHistory* new_scope_experiment_history = (NewScopeExperimentHistory*)history;

	if (new_scope_experiment_history->is_active) {
		this->new_sum_score += target_val;
		this->new_count++;
	} else {
		this->existing_sum_score += target_val;
		this->existing_count++;
	}

	this->state_iter++;
	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_1_PERCENT:
		if (this->new_count >= MEASURE_1_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = NEW_SCOPE_EXPERIMENT_STATE_MEASURE_5_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_5_PERCENT:
		if (this->new_count >= MEASURE_5_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = NEW_SCOPE_EXPERIMENT_STATE_MEASURE_10_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_10_PERCENT:
		if (this->new_count >= MEASURE_10_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = NEW_SCOPE_EXPERIMENT_STATE_MEASURE_25_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_25_PERCENT:
		if (this->new_count >= MEASURE_25_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = NEW_SCOPE_EXPERIMENT_STATE_MEASURE_50_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_50_PERCENT:
		if (this->new_count >= MEASURE_50_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->successful_scope_node = new ScopeNode();
				this->successful_scope_node->parent = this->scope_context;
				this->successful_scope_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				this->successful_scope_node->scope = this->scope_context->new_scope;

				if (this->exit_next_node == NULL) {
					this->successful_scope_node->next_node_id = -1;
					this->successful_scope_node->next_node = NULL;
				} else {
					this->successful_scope_node->next_node_id = this->exit_next_node->id;
					this->successful_scope_node->next_node = this->exit_next_node;
				}

				this->successful_scope_node->is_init = false;
				this->successful_scope_node->init_experiment = this;

				for (int e_index = 0; e_index < (int)this->scope_context->test_experiments.size(); e_index++) {
					if (this->scope_context->test_experiments[e_index] == this) {
						this->scope_context->test_experiments.erase(this->scope_context->test_experiments.begin() + e_index);
						break;
					}
				}
				this->scope_context->successful_experiments.push_back(this);

				this->state = NEW_SCOPE_EXPERIMENT_STATE_SUCCESS;
			} else {
				this->scope_context->generalize_iter++;

				delete this;
				return;
			}
		}
		break;
	}
}
