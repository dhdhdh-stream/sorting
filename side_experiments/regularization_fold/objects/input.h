/**
 * Notes:
 * - don't/can't reuse previous objects as they may have different dependencies
 *   - so recalculate each time
 */

// can have scope dependencies
// processing should start after last dependency?

// if so, should mix in a lot of zero-ing to become robust against starting and stopping

// if have dependency on local, then take it with everything
// - local can't have reverse dependency
// - so save all state in context
//   - and on fetches, add state too so that inner can use

// - only save changes to save memory

// - run all dependencies at once, and they become input to scope

// - then inputs can again be scopeless
//   - yeah, inputs don't start at root because they don't need to, not because they can't
//     - they can if required
// - or, the processing is scopeless, but is still scope-based due to dependencies

// maybe everytime a rewind happens, make sure context history is completely updated
// - and pop as exit context

// if context is from earlier and hasn't been continually updated, update context history

#ifndef INPUT_H
#define INPUT_H

const int INPUT_DEPENDENCY_TYPE_LOCAL = 0;
const int INPUT_DEPENDENCY_TYPE_FETCH = 1;

class Input {
public:
	int dependency_type;
	int context_index;
	int local_index;
	Object* fetch_object;

	std::vector<Input*> dependencies;
};

#endif /* INPUT_H */