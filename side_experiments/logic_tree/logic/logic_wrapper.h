#ifndef LOGIC_WRAPPER_H
#define LOGIC_WRAPPER_H

#include <string>

class AbstractProblem;
class LogicExperiment;
class LogicTree;

class LogicWrapper {
public:
	LogicTree* logic_tree;

	LogicWrapper(AbstractProblem* problem);
	LogicWrapper(std::string path,
				 std::string name);
	~LogicWrapper();

	void save(std::string path,
			  std::string name);
};

#endif /* LOGIC_WRAPPER_H */