#ifndef STATE_WORLD_H
#define STATE_WORLD_H

class StateWorld {
public:
	/**
	 * - underlying vals
	 *   - not actual obs because may be multiple objects
	 *     - vals from all objects combine to give obs
	 */
	// (val, confidence)
	std::vector<std::vector<std::vector<double, double>>> vals;


};

#endif /* STATE_WORLD_H */