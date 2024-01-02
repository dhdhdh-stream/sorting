#ifndef TRY_INSTANCE_H
#define TRY_INSTANCE_H

#include <fstream>
#include <utility>
#include <vector>

class TryScopeStep;

class TryInstance {
public:
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<TryScopeStep*> potential_scopes;

	std::pair<int, std::pair<int,int>> exit;

	double result;

	TryInstance();
	TryInstance(std::ifstream& input_file);
	~TryInstance();

	void save(std::ofstream& output_file);
};

#endif /* TRY_INSTANCE_H */