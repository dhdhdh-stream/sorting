#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

const int OUTER_TYPE_INPUT = 0;
const int OUTER_TYPE_LOCAL = 1;
const int OUTER_TYPE_TEMP = 2;

class Scope;
class ScopeHistory;

class SequenceHistory;
class Sequence {
public:
	std::vector<int> input_types;
	// input_inner_layer always 0
	// input_inner_is_local always false
	std::vector<int> input_inner_indexes;
	/**
	 * - negative indexing
	 */
	std::vector<int> input_scope_depths;
	std::vector<int> input_outer_types;
	std::vector<void*> input_outer_indexes;
	std::vector<double> input_init_vals;

	std::vector<int> output_inner_indexes;
	std::vector<int> output_scope_depths;
	std::vector<int> output_outer_types;
	std::vector<void*> output_outer_indexes;

	AbstractNode* starting_node;
	Scope* scope;

	ScopeNode* scope_node_placeholder;

	Sequence();
	~Sequence();

	void activate(Problem& problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  SequenceHistory* history);
};

class SequenceHistory {
public:
	Sequence* sequence;

	ScopeHistory* scope_history;

	SequenceHistory(Sequence* sequence);
	SequenceHistory(SequenceHistory* original);
	~SequenceHistory();
};

#endif /* SEQUENCE_H */