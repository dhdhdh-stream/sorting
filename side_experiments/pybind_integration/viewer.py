import gymnasium as gym
import pickle
import sys
import time
import wrapper

if len(sys.argv) == 2:
	filename = sys.argv[1]
else:
	filename = 'main.txt'

env = gym.make('LunarLander-v3', render_mode='human')

w = wrapper.Wrapper(8, 'saves/', filename)

obs, info = env.reset()
w.init()
solution_done = False
sum_reward = 0.0
while True:
	if solution_done:
		action = env.action_space.sample()
	else:
		solution_done, s_action = w.step(obs)
		if solution_done:
			continue
		action = pickle.loads(s_action)

	obs, reward, terminated, truncated, info = env.step(action)

	print('reward: ' + str(reward))

	sum_reward += reward
	if terminated or truncated:
		break
w.end()

print('sum_reward: ' + str(sum_reward))

# start_time = time.perf_counter()
# w.save('saves/', 'main.txt')
# end_time = time.perf_counter()
# elapsed_time = end_time - start_time
# print(f"Elapsed time: {elapsed_time} seconds")

w.save_for_display('../', 'display.txt')
