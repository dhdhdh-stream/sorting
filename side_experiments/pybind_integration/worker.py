import gymnasium as gym
import sys
import time
import wrapper

MEASURE_ITERS = 1000

EXPLORE_ITERS = 60

if len(sys.argv) != 3:
	print("Usage: python3 worker.py [path] [filename]")
	exit()

path = argv[1]
filename = argv[2]

env = gym.make('CartPole-v1')

w = wrapper.Wrapper(4, 2, path, filename)

start_time = time.time()
while True:
	curr_time = time.time()
	if curr_time - start_time > 20:
		print('a')
		start_time = curr_time

	obs, info = env.reset()
	w.experiment_init()
	sum_reward = 0.0
	while True:
		is_done, action = w.experiment_step(obs)
		if is_done:
			break
		obs, reward, terminated, truncated, info = env.step(action)
		sum_reward += reward
		if terminated or truncated:
			break
	has_updated = w.experiment_end(sum_reward)
	if has_updated:
		sum_reward = 0.0
		for _ in range(MEASURE_ITERS):
			obs, info = env.reset()
			w.init()
			while True:
				is_done, action = w.step(obs)
				if is_done:
					break
				obs, reward, terminated, truncated, info = env.step(action)
				sum_reward += reward
				if terminated or truncated:
					break
			w.end()
		print('sum_reward: ' + str(sum_reward))

		w.save(path, filename)

		timestamp = subprocess.Popen('head -n 1 ' + path + filename, shell=True, stdout=subprocess.PIPE).stdout.read().strip()
		if int(timestamp) >= EXPLORE_ITERS:
			break
