#include "predict_run.h"

#include "experiment.h"

using namespace std;

PredictRun::~PredictRun() {
	for (map<Experiment*, ExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		delete it->second;
	}
}
