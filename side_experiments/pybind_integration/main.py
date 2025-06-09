import gymnasium as gym
import wrapper

MEASURE_ITERS = 1000

env = gym.make('CartPole-v1')

w = wrapper.Wrapper(4, 2)

while True:
	obs, info = env.reset()
	w.experiment_init()
	sum_reward = 0.0
	done = False
	while True:
		is_done, action = w.experiment_step(obs)
		if is_done:
			break
		if not done:
			obs, reward, terminated, truncated, info = env.step(action)
			sum_reward += reward
			done = terminated or truncated
		else:
			sum_reward -= 0.01
	has_updated = w.experiment_end(sum_reward)
	if has_updated:
		sum_reward = 0.0
		for _ in range(MEASURE_ITERS):
			obs, info = env.reset()
			w.init()
			done = False
			while True:
				is_done, action = w.step(obs)
				if is_done:
					break
				if not done:
					obs, reward, terminated, truncated, info = env.step(action)
					sum_reward += reward
					done = terminated or truncated
				else:
					sum_reward -= 0.01
			w.end()
		print('sum_reward: ' + str(sum_reward))
