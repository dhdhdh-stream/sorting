#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int num_input;	// also becomes output

	// TODO: when exploring, add num_input to fold at the top
	// if multiple inner scopes, later num_inputs above
	// later num_inputs may not have dependencies early, which can later reduce number of states
	// or maybe add at the bottom, and new input at the top?
};

#endif /* SCOPE_H */