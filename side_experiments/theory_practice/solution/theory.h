/**
 * - if theory fails, responsibility is on outermost layer
 *   - which is also the layer that detects the failure
 * 
 * - when failure occurs, have:
 *   - original theory, which had been largely robust
 *   - new edge case, which breaks theory
 * - need to generalize edge case:
 *   - ideally, find info/signals in the world
 *     - need world model, as action sequence likely too limited
 *   - at worst, generalize action sequences that cause issues
 */

#ifndef THEORY_H
#define THEORY_H

class Theory {
public:

};

#endif /* THEORY_H */