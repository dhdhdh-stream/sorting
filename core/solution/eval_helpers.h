#ifndef EVAL_HELPERS_H
#define EVAL_HELPERS_H

#include <vector>

class Scope;
class ScopeHistory;

double calc_score(ScopeHistory* scope_history);
double calc_score_w_drop(ScopeHistory* scope_history);

void update_eval(Scope* parent_scope,
				 std::vector<ScopeHistory*>& scope_histories,
				 std::vector<double>& target_val_histories);

#endif /* EVAL_HELPERS_H */