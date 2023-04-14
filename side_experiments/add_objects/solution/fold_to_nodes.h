#ifndef FOLD_TO_PATH_H
#define FOLD_TO_PATH_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "scope.h"

void fold_to_nodes(Scope* parent_scope,
				   Fold* fold,
				   std::vector<AbstractNode*>& new_outer_nodes);

#endif /* FOLD_TO_PATH_H */