#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:
	int num_scopes;
	std::map<int, Scope*> scopes;
	// 0 is root, starts with ACTION_START

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;



	void random_start();


};

#endif /* SOLUTION_H */