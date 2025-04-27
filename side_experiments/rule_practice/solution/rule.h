#ifndef RULE_H
#define RULE_H

#include <fstream>
#include <set>
#include <vector>

class AbstractCondition;

class Rule {
public:
	std::vector<AbstractCondition*> conditions;
	int move;

	Rule();
	Rule(Rule* original);
	Rule(std::ifstream& input_file);
	~Rule();

	bool is_hit(std::vector<std::vector<double>>& obs_history,
				std::vector<int>& move_history);
	void apply(std::set<int>& possible_moves);

	void save(std::ofstream& output_file);
};

#endif /* RULE_H */