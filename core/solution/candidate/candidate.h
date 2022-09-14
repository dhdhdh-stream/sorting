#ifndef CANDIDATE_H
#define CANDIDATE_H

const int CANDIDATE_BRANCH = 0;
const int CANDIDATE_REPLACE = 1;
const int CANDIDATE_START_BRANCH = 2;
const int CANDIDATE_START_REPLACE = 3;

class Candidate {
public:
	int type;

	virtual ~Candidate() {};

	virtual void apply() = 0;
	virtual void clean() = 0;	// if not chosen
};

#endif /* CANDIDATE_H */