#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:

	/**
	 * - don't have last seen vals
	 *   - not reliable when can't track where it came from/guarantee it comes from same spot
	 *     - don't need for exploration when it's just for inspiration anyways
	 *       - and can get inspiration just from local
	 *         - (and don't worry about local resetting -- if early exit, will have no impact)
	 */
	std::map<int, double> last_seen_vals;

};

#endif /* RUN_HELPER_H */