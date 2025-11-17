#include "logic_experiment.h"

using namespace std;

void LogicExperiment::activate(vector<double>& obs,
							   double target_val) {
	switch (this->state) {
	case LOGIC_EXPERIMENT_STATE_GATHER:
		gather_activate(obs,
						target_val);
		break;
	case LOGIC_EXPERIMENT_STATE_MEASURE:
		measure_activate(obs,
						 target_val);
		break;
	}
}
