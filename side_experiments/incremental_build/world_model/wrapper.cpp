#include "wrapper.h"

#include "problem.h"
#include "world_model.h"

using namespace std;

Wrapper::Wrapper(ProblemType* problem_type) {
	this->num_obs = problem_type->num_obs();
	this->num_actions = problem_type->num_possible_actions();

	this->world_model = new WorldModel();
}

Wrapper::~Wrapper() {
	delete this->world_model;
}
