#ifndef TRY_IMPACT_H
#define TRY_IMPACT_H

#include <map>
#include <utility>

class TryImpact {
public:
	/**
	 * - if over 100, rolling
	 */
	int count;
	double overall_impact;
	/**
	 * - if insert, use new; if remove, use original
	 */
	std::map<int, std::pair<int, double>> rel_pre_impacts;
	std::map<int, std::pair<int, double>> rel_post_impacts;

	TryImpact() {
		// do nothing
	}
};

#endif /* TRY_IMPACT_H */