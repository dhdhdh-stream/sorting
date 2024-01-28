/**
 * - to learn state, have to provide action sequence
 *   - as cannot learn sequence and state at the same time
 *     - no way to recognize good sequences if cannot recognize state, and vice versa
 *   - if branching involved, must be separately learned first
 *     - i.e., as a scope
 *       - then include the scope in the sequence
 */

#ifndef STATE_SCENARIO_H
#define STATE_SCENARIO_H

#include <vector>

#include "action.h"

class Problem;

class StateScenario {
public:
	Problem* problem;

	/**
	 * TODO:
	 * - include compound actions, i.e., scopes
	 *   - include state connections between scopes
	 */
	std::vector<Action> sequence;

	virtual ~StateScenario() {};

	virtual double get_target_state() = 0;
};

#endif /* STATE_SCENARIO_H */