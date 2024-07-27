#include "world_state.h"

using namespace std;

WorldState::WorldState() {
	// do nothing
}

WorldState::WorldState(WorldState* original) {
	this->id = original->id;

	this->state = original->state;

	this->estimated_x = original->estimated_x;
	this->estimated_y = original->estimated_y;

	this->evidence = original->evidence;
}
