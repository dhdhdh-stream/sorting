#include "logic_wrapper.h"

#include "abstract_logic_node.h"
#include "logic_experiment.h"
#include "logic_helpers.h"
#include "logic_tree.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
#else
const int IMPROVEMENTS_PER_ITER = 20;
#endif /* MDEBUG */

LogicWrapper::LogicWrapper(AbstractProblem* problem) {
	this->logic_tree = init_helper(problem);

	this->best_experiment = NULL;
	this->improvement_iter = 0;
}

LogicWrapper::LogicWrapper(string path,
						   string name) {
	this->logic_tree = new LogicTree();
	this->logic_tree->load(path, name);

	this->best_experiment = NULL;
	this->improvement_iter = 0;
}

LogicWrapper::~LogicWrapper() {
	delete this->logic_tree;
}

void LogicWrapper::update() {
	for (map<int, AbstractLogicNode*>::iterator it = this->logic_tree->nodes.begin();
			it != this->logic_tree->nodes.end(); it++) {
		if (it->second->experiment != NULL
				&& it->second->experiment->state == LOGIC_EXPERIMENT_STATE_DONE) {
			if (it->second->experiment->improvement > 0.0) {
				if (this->best_experiment == NULL) {
					this->best_experiment = it->second->experiment;
				} else if (it->second->experiment->improvement > this->best_experiment->improvement) {
					delete this->best_experiment;
					this->best_experiment = it->second->experiment;
				} else {
					delete it->second->experiment;
				}

				this->improvement_iter++;
			} else {
				delete it->second->experiment;
			}

			it->second->experiment = NULL;
		}
	}

	if (this->improvement_iter >= IMPROVEMENTS_PER_ITER) {
		for (map<int, AbstractLogicNode*>::iterator it = this->logic_tree->nodes.begin();
				it != this->logic_tree->nodes.end(); it++) {
			if (it->second->experiment != NULL) {
				delete it->second->experiment;
				it->second->experiment = NULL;
			}
		}

		this->best_experiment->add(this->logic_tree);
		this->best_experiment = NULL;
		this->improvement_iter = 0;
	}
}

void LogicWrapper::save(string path,
						string name) {
	this->logic_tree->save(path, name);
}
