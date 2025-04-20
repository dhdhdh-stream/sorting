#ifndef STATE_H
#define STATE_H

class State {
public:
	double obs_average;
	double obs_standard_deviation;

	// TODO: can be probabilities
	std::vector<State*> connections;

	std::set<State*> distinct_from;

};

#endif /* STATE_H */