#ifndef CANDIDATE_H
#define CANDIDATE_H

const int CANDIDATE_BRANCH = 0;
const int CANDIDATE_REPLACE = 1;

class Candidate {
public:
	int type;

	virtual void apply();
	virtual void clean();	// if not chosen
};

#endif /* CANDIDATE_H */