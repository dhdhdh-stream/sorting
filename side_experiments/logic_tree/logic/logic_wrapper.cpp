#include "logic_wrapper.h"

#include <iostream>

#include "abstract_logic_node.h"
#include "logic_helpers.h"
#include "logic_tree.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
#else
const int IMPROVEMENTS_PER_ITER = 10;
#endif /* MDEBUG */

LogicWrapper::LogicWrapper(AbstractProblem* problem) {
	this->logic_tree = init_helper(problem);
}

LogicWrapper::LogicWrapper(string path,
						   string name) {
	this->logic_tree = new LogicTree();
	this->logic_tree->load(path, name);
}

LogicWrapper::~LogicWrapper() {
	delete this->logic_tree;
}

void LogicWrapper::save(string path,
						string name) {
	this->logic_tree->save(path, name);
}
