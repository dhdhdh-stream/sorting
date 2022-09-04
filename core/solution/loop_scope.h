#ifndef LOOP_SCOPE_H
#define LOOP_SCOPE_H

class LoopScope : public SolutionNode {
public:
	int num_states;

	std::vector<SolutionNode*> top;
	std::vector<SolutionNode*> forward;
	std::vector<SolutionNode*> back;

	Network* halt_score_network;
	Network* halt_certainty_network;

	Network* no_halt_score_network;
	Network* no_halt_certainty_network;

	// add back backwards path
};

#endif /* LOOP_SCOPE_H */