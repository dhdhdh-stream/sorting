#ifndef CANDIDATE_H
#define CANDIDATE_H

class Candidate {
public:
	int branching_node_index;

	std::vector<SolutionNode*> path;
	// don't worry about loops for now

	double average;
	double misguess;
};

#endif /* CANDIDATE_H */