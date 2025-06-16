import gymnasium as gym
import pickle
import sys
import wrapper

if len(sys.argv) == 2:
	filename = sys.argv[1]
else:
	filename = 'main.txt'

MEASURE_ITERS = 1000

env = gym.make('LunarLander-v3')

w = wrapper.Wrapper(8, 'saves/', filename)

sum_reward = 0.0
for _ in range(MEASURE_ITERS):
	obs, info = env.reset()
	w.init()
	solution_done = False
	while True:
		if solution_done:
			action = env.action_space.sample()
		else:
			solution_done, s_action = w.step(obs)
			if solution_done:
				continue
			action = pickle.loads(s_action)

		obs, reward, terminated, truncated, info = env.step(action)
		sum_reward += reward
		if terminated or truncated:
			break
	w.end()

print('sum_reward: ' + str(sum_reward))
