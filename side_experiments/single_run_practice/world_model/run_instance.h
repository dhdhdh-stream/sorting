#ifndef RUN_INSTANCE_H
#define RUN_INSTANCE_H

class RunInstance {
public:
	std::set<State*> states;

	State* curr_state;


};

#endif /* RUN_INSTANCE_H */