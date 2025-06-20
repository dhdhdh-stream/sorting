/**
 * - without patterns to reduce variance, difficult to make progress on low samples
 * 
 * - assume already 0'd
 */

#ifndef PATTERN_H
#define PATTERN_H

#include <vector>

#include "input.h"

class Network;
class ScopeHistory;

class Pattern {
public:
	std::vector<Input> keypoints;
	Network* keypoint_network;

	std::vector<Input> inputs;
	Network* predict_network;

	Pattern();
	~Pattern();

	void activate(bool& has_match,
				  double& predicted,
				  ScopeHistory* scope_history);
};

#endif /* PATTERN_H */