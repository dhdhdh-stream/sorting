#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <vector>

class Rule;

class Solution {
public:
	std::vector<Rule*> rules;

	double average_val;
	double top_5_percentile;
	double top_5_percentile_average_val;

	Solution();
	~Solution();

	void init();
	void load(std::string path,
			  std::string name);

	void print();

	void save(std::string path,
			  std::string name);
};

#endif /* SOLUTION_H */