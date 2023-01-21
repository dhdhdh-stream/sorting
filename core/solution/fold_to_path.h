#ifndef FOLD_TO_PATH_H
#define FOLD_TO_PATH_H

#include <vector>

#include "action.h"
#include "branch.h"
#include "finished_step.h"
#include "fold.h"
#include "fold_network.h"
#include "scope.h"

void fold_to_path(std::vector<FinishedStep*> finished_steps,
				  int& sequence_length,
				  std::vector<bool>& is_inner_scope,
				  std::vector<Scope*>& scopes,
				  std::vector<Action>& actions,
				  std::vector<std::vector<FoldNetwork*>>& inner_input_networks,
				  std::vector<std::vector<int>>& inner_input_sizes,
				  std::vector<double>& scope_scale_mod,
				  std::vector<int>& step_types,
				  std::vector<Branch*>& branches,
				  std::vector<Fold*>& folds,
				  std::vector<FoldNetwork*>& score_networks,
				  std::vector<double>& average_scores,
				  std::vector<double>& average_misguesses,
				  std::vector<double>& average_inner_scope_impacts,
				  std::vector<double>& average_local_impacts,
				  std::vector<double>& average_inner_branch_impacts,
				  std::vector<bool>& active_compress,
				  std::vector<int>& compress_new_sizes,
				  std::vector<FoldNetwork*>& compress_networks,
				  std::vector<int>& compress_original_sizes);

#endif /* FOLD_TO_PATH_H */