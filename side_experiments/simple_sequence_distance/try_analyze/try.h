/**
 * - don't try to "learn" world model using RNNs, etc.
 *   - unlikely to be meaningful without a large number of tries
 *   - will likely cost a lot to keep updated
 */

#ifndef TRY_H
#define TRY_H

class Try {
public:
	std::vector<int> sequence;
	double result;

	Try() {
		// do nothing
	}
};

#endif /* TRY_H */