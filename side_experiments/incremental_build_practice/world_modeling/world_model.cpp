#include "world_model.h"

#include "world_state.h"

using namespace std;

WorldModel::WorldModel() {
	// do nothing
}

WorldModel::~WorldModel() {
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		delete this->states[s_index];
	}
}
