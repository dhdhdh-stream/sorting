#include "pattern.h"

#include "network.h"

using namespace std;

Pattern::Pattern(vector<int> actions,
				 vector<int> keypoints,
				 vector<double> keypoint_averages,
				 vector<double> keypoint_standard_deviations,
				 vector<int> inputs,
				 Network* network) {
	this->actions = actions;
	this->keypoints = keypoints;
	this->keypoint_averages = keypoint_averages;
	this->keypoint_standard_deviations = keypoint_standard_deviations;
	this->inputs = inputs;
	this->network = network;
}

Pattern::~Pattern() {
	delete this->network;
}
