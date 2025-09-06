/**
 * - predict score relative to current average
 *   - more likely for signal to predict relative than global true
 *     - so predicting relative leads to less adjustments
 */

#ifndef SIGNAL_H
#define SIGNAL_H

#include <fstream>
#include <vector>

class SignalInstance;
class Solution;

class Signal {
public:
	/**
	 * - separate actions/obs for signal
	 *   - don't have to worry about changes destroying signal
	 *   - don't have to worry about branching
	 * 
	 * - simply only worry about signal at end
	 *   - impact of an explore will be calculated through train_existing step
	 * 
	 * - signals need to be perfect
	 *   - at least for everything so far
	 *   - at least when predicting good
	 *   - anytime a trap is hit, either learn to recognize or give up on signal
	 * 
	 * - always perform signal actions
	 *   - as experiments done in their presence
	 *   - but for SignalExperiments, use current actions rather than original actions
	 */
	std::vector<int> signal_pre_actions;
	std::vector<int> signal_post_actions;
	std::vector<SignalInstance*> instances;
	double default_guess;
	/**
	 * - simply average of random explore
	 *   - doesn't need to be accurate
	 *     - just a bad value that can be learned to avoid
	 */

	Signal();
	Signal(std::ifstream& input_file);
	~Signal();

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_H */