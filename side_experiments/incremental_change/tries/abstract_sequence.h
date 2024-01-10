#ifndef ABSTRACT_SEQUENCE_H
#define ABSTRACT_SEQUENCE_H

const int SEQUENCE_TYPE_SEQUENCE = 0;
const int SEQUENCE_TYPE_STUB = 1;

class AbstractSequence {
public:
	int type;

	/**
	 * - combined if branch
	 */
	double score;
	/**
	 * - if pass through
	 */
	double information;
	int timestamp;


};

#endif /* ABSTRACT_SEQUENCE_H */