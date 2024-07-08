#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:

	std::vector<Scope*> scopes;

	int max_num_actions;
	int num_actions_limit;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

};

#endif /* SOLUTION_H */