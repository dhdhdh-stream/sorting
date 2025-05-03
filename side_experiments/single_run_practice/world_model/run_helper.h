#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	int num_actions;

	FixedPoint* curr_location;

	RunHelper() {
		this->num_actions = 0;

		this->curr_location = NULL;
	};
};

#endif /* RUN_HELPER_H */