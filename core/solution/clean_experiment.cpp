#include "clean_experiment.h"

using namespace std;

CleanExperiment::CleanExperiment(vector<int> scope_context,
								 vector<int> node_context) {
	this->type = EXPERIMENT_TYPE_CLEAN;

	this->scope_context = scope_context;
	this->node_context = node_context;

	this->average_remaining_experiments_from_start = 1.0;

	this->state = CLEAN_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->existing_score = 0.0;
	this->new_score = 0.0;
}

CleanExperiment::~CleanExperiment() {
	// do nothing
}

CleanExperimentOverallHistory::CleanExperimentOverallHistory(
		CleanExperiment* experiment) {
	this->experiment = experiment;
}
