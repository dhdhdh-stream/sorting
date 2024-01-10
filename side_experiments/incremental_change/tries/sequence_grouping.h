#ifndef SEQUENCE_GROUPING_H
#define SEQUENCE_GROUPING_H

class ScopeCreationHistory {
public:
	/**
	 * - from random_activate
	 */
	std::vector<std::vector<int>> scope_contexts;
	std::vector<std::vector<int>> node_contexts;


};

class SequenceGrouping {
public:
	/**
	 * - creation history
	 *   - nodes within scope from run
	 */
	std::vector<std::vector<int>> scope_contexts;
	std::vector<std::vector<int>> node_contexts;

	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<ScopeCreationHistory*> scope_creation_histories;

	std::map<int, Sequence*> sequences;

};

#endif /* SEQUENCE_GROUPING_H */