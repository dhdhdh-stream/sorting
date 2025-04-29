/**
 * - pre and post always match
 *   - but pre may not need to be performed if always the same
 * 
 * - on post catch miss:
 *   - if explore, simply assume failure
 *   - if is known path can succeed, assume neutral result
 * 
 * - when modifying:
 *   - can add exact match
 *   - can add fuzzy-match?
 *     - if fuzzy-match to multi-match, match if 1 match
 *   - can add info
 * 
 * - if too selective, will miss too often
 *   - even on paths where pattern is known to appear
 * - if not selective enough, will catch noise
 *   - lowering prediction accuracy
 * - also needs to try to always hit relevant information
 * - also needs to not impact score
 */

// distinguish between patterns that you can return to vs. patterns that you have 1-shot to see
// - divide problems by if it's easy to return to patterns?

#ifndef PATTERN_H
#define PATTERN_H

class Pattern {
public:
	Scope* pattern;
	std::vector<Input> fixed_points;

	std::vector<Input> score_inputs;
	

	
};

#endif /* PATTERN_H */