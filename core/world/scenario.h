/**
 * - provide both sequence to learn state, i.e., attention, as well as payoff, i.e., sequence
 * 
 * - easy to capture and share as actions fixed
 */

#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>

#include "action.h"

class Problem;

class Scenario {
public:
	Problem* problem;

	virtual ~Scenario() {};

	virtual void get_attention(std::vector<int>& types,
							   std::vector<Action>& actions,
							   std::vector<std::string>& scopes) = 0;
	virtual void get_sequence(std::vector<int>& types,
							  std::vector<Action>& actions,
							  std::vector<std::string>& scopes) = 0;

	virtual bool should_perform_sequence() = 0;

	virtual std::string get_name() = 0;
};

#endif /* SCENARIO_H */