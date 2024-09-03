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
 * 
 * - like, one might know the wave-particle duality, etc.
 *   - but the image of the world in one's mind is broad strokes, rather than a large number of particles moving according to physics
 * 
 * - in fact, maybe not needed to do world modeling?
 *   - may be other ways to localize
 *   - but it's something humans can do, regardless of if it's needed for solutions
 * 
 * - so maybe for solution building world model should be defined in terms of actions
 *   - physical distances don't matter
 *     - what actions need to be performed matters
 *   - e.g., rotating in place doesn't actually need to be in place
 *     - can have multiple states close together representing different angles
 * 
 * - before building solution, first try to identify location
 *   - so before even worrying about score, from new location reached, build out world model
 *     - and if compound action, take time to add states one by one
 *       - so if want to explore trying something, then continuing, know how to do so
 * 
 * - so never improvising
 *   - so doesn't suffer from Markov restrictions
 *     - i.e., doesn't suffer from delay
 *     - instead, path taken is known?
 *       - even when there is variation, travel along known paths
 * 
 * - but what if world changes?
 * 
 * - maybe just have localize/relocalize steps
 *   - regardless of where new you might have travelled, random until at a target spot for the next phase
 *     - so if world changes, not dependent on where previously travelled, but matches anew
 * 
 * - will need to capture features of an arbitrary location
 *   - test if adding capture feature sequence affects score
 * 
 * - doesn't need to always travel to a previous location
 *   - once a path is fixed, can try learning a spot
 *     - and localize to there from now on
 * 
 * - to localize a spot, after arriving:
 *   - look around to get an average val
 *     - maybe start looking slightly before?
 *   - look for a consistent key sequence
 *   - verify key sequence uniqueness
 *   - then broadly memorize what it takes to reach spot from different conditions
 *     - start recording from last localization?
 *   - also need to learn local localization sequence
 *     - fluctuations that improve chance of finding without harming score
 *       - can maybe have decision making as well?
 *   - initially, obviously need no movement as already there
 *     - but as changes are added, need to add
 * 
 * - key sequence is learning a network comparing key vs. random
 *   - a good key sequence is one in which a good network can be learned
 * 
 * - to memorize a spot dynamically:
 *   - need to learn a key sequence
 *     - key sequence should generate high contrast while not affecting score
 * 
 * - key sequence needs to end in spot that is ready to continue
 * 
 * - returning to spots is a subgoal
 * 
 * - how to know at right spot
 *   - learn network that evaluates follow up steps
 *   - how to know what's wrong spot?
 *     - randomly perform actions, including compound actions, then use as training data
 *   - are these networks single use or should be retrained?
 * 
 * - localization is quite expensive to train and maintain
 *   - should there be restrictions on their placement?
 *   - any change can impact multiple localizations
 *     - any change would need to update all localizations it can affect
 *   - perhaps localizations should be nested
 *     - then localizations are committing
 * - unless general world model
 *   - how to get from point A to point B applies everywhere in the solution
 * 
 * - so back to needing world model
 * 
 * - and probably just need to be able to create super large, super dense world/action map
 * 
 * - if the world is constantly changing, then world model can look like a graph going in one direction
 *   - "same" location can be represented as multiple state at different times
 * 
 * - don't need all possible paths from anywhere to anywhere
 *   - don't need shortest path
 *   - just need a path
 *     - so if reached new spot, if can backtrack to any known location, that's enough
 *       - BTW, it is reach new spot, then continue on new spot, then finally getting back
 * 
 * - what makes a spot identical?
 *   - so score + obs + random movement w/ obs
 *   - so what about going back to previous spot?
 *     - maybe doesn't make sense anymore?
 *     - maybe when creating a path, randomly add localization nodes?
 *       - doesn't have affect immediately, but makes sure future changes always go back to same spot?
 *     - no no, should probably still be world/time/action model + all to all
 * 
 * - maybe divide world into things that can be rewinded, and things that can't/have to be reset
 *   - but this is not strict either? sometimes things can be rewinded while certain conditions hold
 *   - and not reset, but strictly progressing in some way
 * 
 * - when exploring a new sequence, before worrying about how good it is, first worry about where you are?
 *   - how good you are depends on follow up
 *     - for every increment (for compound actions)
 * 
 * - return sequence not part of solution BTW
 *   - part of localization node
 *   - as sequence changes depending on new changes
 * 
 * - where you are depends on what you see as well
 * 
 * - hmm... a localization node can represent multiple location
 * 
 * - cleanest is still world model + return to
 * 
 * - perhaps have to give motion/contrast
 *   - have a window and use delta to determine movement
 * 
 * - world motion to the left could be:
 *   - stay in place, rotated right
 *   - moved right
 *   - stay in place, world move/rotated left
 * - there can also be rotation of the world
 * 
 * - for the applications I will likely choose, this should be good enough for now?
 * 
 * - if in no constrast situation, then use average?
 */
