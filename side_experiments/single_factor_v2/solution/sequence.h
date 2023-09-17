#ifndef SEQUENCE_H
#define SEQUENCE_H

class Sequence {
public:
	// TODO: have both input and output, which can connect to stuff in back

	// - can also try setting values to +1/-1

	// - to prevent branches from screwing things up, states should only be taking into account if ending seen?
	//   - otherwise, if XOR, error signals will squash before branch to 0.0

	// TODO: set any state in context?

	/**
	 * - new scopes
	 *   - copied from non-loops
	 */
	Scope* potential;

	// TODO: when constructing potential, need to pass in all the state that needs to be passed out

	// - for least squares, probably save all values in vectors first because things are dynamic
	//   - then transfer all into a matrix when need to calculate

};

#endif /* SEQUENCE_H */