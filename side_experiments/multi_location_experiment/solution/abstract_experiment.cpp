#include "abstract_experiment.h"

#include "globals.h"

using namespace std;

bool AbstractExperiment::further_than(AbstractExperiment* other) {
	if (this->state < other->state) {
		return false;
	} else if (this->state > other->state) {
		return true;
	} else {
		if (this->state_iter < other->state_iter) {
			return false;
		} else if (this->state_iter > other->state_iter) {
			return true;
		} else {
			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				return false;
			} else {
				return true;
			}
		}
	}
};
