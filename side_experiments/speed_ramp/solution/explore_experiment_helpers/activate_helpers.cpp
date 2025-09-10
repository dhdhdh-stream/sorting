#include "explore_experiment.h"

#include "solution_wrapper.h"

using namespace std;

const int LAST_NUM_TRACK = 1000;

void ExploreExperiment::check_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExploreExperimentHistory* history;
		map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it =
			wrapper->explore_histories.find(this);
		if (it == wrapper->explore_histories.end()) {
			history = new ExploreExperimentHistory(this,
												   wrapper);
			wrapper->explore_histories[this] = history;
		} else {
			history = it->second;
		}

		history->num_instances++;

		switch (this->state) {
		case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:

			break;
		case EXPLORE_EXPERIMENT_STATE_EXPLORE:

			break;
		case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:

			break;
		}
	}
}

void ExploreExperiment::experiment_step(vector<double>& obs,
										int& action,
										bool& is_next,
										bool& fetch_action,
										SolutionWrapper* wrapper) {

}

void ExploreExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {

}

void ExploreExperiment::experiment_exit_step(SolutionWrapper* wrapper) {

}

void ExploreExperiment::backprop(double target_val,
								 ExploreExperimentHistory* history,
								 SolutionWrapper* wrapper) {
	if (this->last_num_explore.size() >= LAST_NUM_TRACK) {
		this->sum_num_explore -= this->last_num_explore.front();
		this->last_num_explore.pop_front();
		this->last_num_explore.push_back((int)wrapper->explore_histories.size());
		this->sum_num_explore += (int)wrapper->explore_histories.size();
	} else {
		this->last_num_explore.push_back((int)wrapper->explore_histories.size());
		this->sum_num_explore += (int)wrapper->explore_histories.size();
	}

	if (this->last_num_instances.size() >= LAST_NUM_TRACK) {
		this->sum_num_instances -= this->last_num_instances.front();
		this->last_num_instances.pop_front();
		this->last_num_instances.push_back(history->num_instances);
		this->sum_num_instances += history->num_instances;
	} else {
		this->last_num_instances.push_back(history->num_instances);
		this->sum_num_instances += history->num_instances;
	}



}
