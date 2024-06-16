/**
 * - despite significant effort to learn and maintain, eval is worth it because:
 *   - reduces variance of target_val, improving training data quality
 *     - leads to better decision making and quicker progress
 *   - gives scopes identity, pushing improvements to be in the same direction
 *     - i.e., prevents thrashing
 */

#ifndef EVAL_HELPERS_H
#define EVAL_HELPERS_H

#include <vector>

class Scope;
class ScopeHistory;
class Solution;

double calc_score(ScopeHistory* scope_history);

void update_eval();

#endif /* EVAL_HELPERS_H */