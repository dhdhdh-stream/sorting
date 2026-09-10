#include "refine.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

Refine::Refine(SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_REFINE;

	this->start_iter = wrapper->iters_since_update;
	this->existing_sum_scores = 0.0;
	this->existing_count = 0;
	this->new_sum_scores = 0.0;
	this->new_count = 0;

	this->epoch_iter = 0;
	this->run_iter = 0;
}

Refine::~Refine() {
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			noop_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			action_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			scope_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				branch_node->branch_experiment = NULL;
			} else {
				branch_node->original_experiment = NULL;
			}
		}
		break;
	}

	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}
}

RefineHistory::RefineHistory(Refine* refine) {
	this->refine = refine;

	uniform_int_distribution<int> is_active_distribution(0, 9);
	this->is_active = is_active_distribution(generator);
}

RefineState::RefineState(Refine* refine) {
	this->experiment = refine;
}
