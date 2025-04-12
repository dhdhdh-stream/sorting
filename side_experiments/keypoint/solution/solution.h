#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:
	int timestamp;
	double curr_score;

	std::vector<Scope*> scopes;

	std::vector<double> obs_average_val;
	std::vector<double> obs_variance;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

};

#endif /* SOLUTION_H */