#include "scope_node.h"

#include "constants.h"
#include "globals.h"
#include "potential_commit.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::commit_gather_activate(AbstractNode*& curr_node,
									   Problem* problem,
									   vector<ContextLayer>& context,
									   RunHelper& run_helper,
									   int& node_count,
									   AbstractNode*& potential_node_context,
									   bool& potential_is_branch) {
	context.back().node_id = this->id;

	this->scope->commit_gather_activate(problem,
										context,
										run_helper,
										node_count,
										potential_node_context,
										potential_is_branch);

	curr_node = this->next_node;

	if (solution->timestamp >= MAINTAIN_ITERS
			|| (this->parent->id == 0 || this->parent->id > solution->num_existing_scopes)) {
		uniform_int_distribution<int> select_distribution(0, node_count);
		node_count++;
		if (select_distribution(generator) == 0) {
			potential_node_context = this;
			potential_is_branch = false;
		}
	}
}

void ScopeNode::commit_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper,
								PotentialCommit* potential_commit) {
	context.back().node_id = this->id;

	this->scope->commit_activate(problem,
								 context,
								 run_helper,
								 potential_commit);

	curr_node = this->next_node;

	if (potential_commit->node_context == this) {
		potential_commit->activate(curr_node,
								   problem,
								   context,
								   run_helper);
	}
}
