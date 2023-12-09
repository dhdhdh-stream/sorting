#ifndef HMM_H
#define HMM_H

#include <vector>

class HiddenState;

class HMM {
public:
	std::vector<HiddenState*> hidden_states;
	/**
	 * - simply starting at hidden_states[0]
	 */

	HMM();
	~HMM();

	void init();

	void update(std::vector<int>& action_sequence,
				double target_val);
	void explore(std::vector<int>& action_sequence,
				 double target_val);
};

#endif /* HMM_H */