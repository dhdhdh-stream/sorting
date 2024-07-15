/**
 * TODO: possible for significant gains to come from outer/overall scope, which should be saved
 */

#ifndef SOLUTION_SET_H
#define SOLUTION_SET_H

#include <string>
#include <vector>

class Solution;

const int STALL_BEFORE_NEW = 4;

const int MERGE_SIZE = 4;

class SolutionSet {
public:
	int timestamp;
	double average_score;

	double best_average_score;
	int best_timestamp;

	std::vector<Solution*> solutions;
	int curr_solution_index;

	SolutionSet();
	SolutionSet(SolutionSet* original);
	~SolutionSet();

	void init();
	void load(std::string path,
			  std::string name);

	void increment();

	void save(std::string path,
			  std::string name);
};

#endif /* SOLUTION_SET_H */