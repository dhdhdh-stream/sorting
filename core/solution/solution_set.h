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
	int next_possible_new_scope_timestamp;

	double best_average_score;
	int best_timestamp;

	std::vector<Solution*> solutions;
	int curr_solution_index;

	// temp
	std::vector<int> score_type_counts;
	std::vector<double> score_type_impacts;

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