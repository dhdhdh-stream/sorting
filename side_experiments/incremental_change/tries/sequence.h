#ifndef SEQUENCE_H
#define SEQUENCE_H

class Sequence : public AbstractSequence {
public:
	int id;

	SequenceGrouping* parent;

	int start_index;
	int end_index;

	/**
	 * - remaining from sequence_grouping
	 */
	std::vector<int> step_indexes;
	std::vector<PotentialScopeNode*> potential_scopes;

	std::vector<std::pair<Change, AbstractSequence*>> tries;
	/**
	 * - track successes for ideas of what to try next
	 */
	std::vector<std::pair<Change, Sequence*>> successes;


};

#endif /* SEQUENCE_H */