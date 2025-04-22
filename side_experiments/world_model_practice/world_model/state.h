/**
 * - don't worry about checking if, e.g., is bimodel
 *   - will be caught if branch is taken and splits
 *     - branch based off Spot instead of solution node
 *       - then can eventually split even without BranchNode?
 */

#ifndef STATE_H
#define STATE_H

class State {
public:
	double val_average;
	double val_standard_deviation;

};

#endif /* STATE_H */