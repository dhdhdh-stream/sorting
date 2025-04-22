/**
 * - if from branch, narrows the state of spots
 * - if from action, impact from action
 *   - generalized to over time
 */

#ifndef IMPACT_H
#define IMPACT_H

class Impact {
public:
	Spot* next_spot;

	std::vector<Spot*> spots;
	std::vector<State*> states;


};

#endif /* IMPACT_H */