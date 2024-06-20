/**
 * - don't use local eval, only higher layers
 *   - higher layers won't have missing sections due to explore
 *     - (unless recursion, which should hopefully be being prevented)
 * 
 * - mainly helps when training existing
 *   - gives good targets for all instances in a run despite them sharing the same result
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