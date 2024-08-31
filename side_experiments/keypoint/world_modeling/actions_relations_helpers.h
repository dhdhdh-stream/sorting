/**
 * - determine dimensions of actions
 * 
 * - don't worry about special conditions
 *   - i.e., terrain, walls/corners, loops, etc.
 *   - map out terrain separately
 * 
 * TODO: use linear algebra techniques?
 * - but doesn't work if there are loops?
 *   - need to use modular arithmetic?
 * 
 * - doesn't really work if effect of actions heavily depending on state
 *   - e.g., some actions control direction, others advance forward one step
 * 
 * - so estimating where you are not dependent on what actions were performed
 *   - but needs to be from a deeper layer of processing
 *     - what does this look like?
 * 
 * - world needs to be defined separately from actions?
 *   - but constructed through actions + obs?
 * 
 * - states not connected by actions, but by distance
 *   - and actions travel a certain distance
 *     - can simply set first action to (1.0, ...)?
 * 
 * - actions in a context can travel anywhere, even into new dimensions
 *   - Occam's Razor
 * 
 * - so try to keep dimensions low, distance low?
 *   - deconstruct world into basic shapes?
 *     - like in 3D modeling?
 *       - world consists of basic shapes?
 * 
 * - maybe return paths start as circles
 *   - then as states merge, move states to more correct positions
 * 
 * - then what about actions/context in the general case?
 * 
 * - direction must be important
 *   - and motion in a direction
 * 
 * - but there's like terrain and modifiers as well?
 *   - like global state?
 *   - maybe multiple world spaces?
 *     - must be multiple world spaces
 *     - so state can exist in its own space
 *       - and be processed in a continuous-ish way
 * 
 * - keep in mind localization is kind of best effort
 * 
 * - when to add state?
 *   - when there's variance that can be explained?
 *     - when there's correlation to something else
 */

/**
 * - mathematics and physics are exceedingly complex ways to model the world
 *   - but they are not likely the models that exist in human brains
 *     - they are likely algorithms for making predictions about the world
 *       - actions are manipulating values in different ways
 *       - reward function is how closely a value matches reality
 */
