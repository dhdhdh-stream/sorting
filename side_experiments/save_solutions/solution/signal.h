// TODO: have to improve signal quality, have to learn from mistakes
// - or else will get trapped

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
	 * - don't worry about signals within
	 *   - simply live with imperfect signals
	 *   - will always be signals missed within explores anyways
	 *     - and will always be outer conflicts as well
	 *   - and will always need sequence after explore to be meaningful anyways
	 * 
	 * - signals always predict immediate outer layer
	 *   - instead of true score
	 *     - but units still in true score
	 *       - so can be directly compared against each other
	 *   - to boost signal strength
	 */
	std::vector<int> signal_pre_actions;
	std::vector<int> signal_post_actions;
	std::vector<SignalInstance*> instances;
	double miss_average_guess;

	double signal_positive_misguess_average;
	double signal_positive_misguess_standard_deviation;
	double signal_misguess_average;
	double signal_misguess_standard_deviation;
	/**
	 * - simply save between updates
	 */

	Signal();
	Signal(std::ifstream& input_file);
	~Signal();

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_H */