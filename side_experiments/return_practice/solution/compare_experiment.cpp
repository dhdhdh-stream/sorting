#include "compare_experiment.h"

#include "network.h"

using namespace std;

CompareExperiment::~CompareExperiment() {
	delete this->original_network;
	delete this->branch_network;
}

CompareExperimentHistory::CompareExperimentHistory(CompareExperiment* experiment) {
	this->experiment = experiment;

	this->hit_branch = false;
}

CompareExperimentState::CompareExperimentState(CompareExperiment* experiment) {
	this->experiment = experiment;
}
