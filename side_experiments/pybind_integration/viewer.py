import gymnasium as gym
import sys
import wrapper

if len(sys.argv) == 2:
	filename = sys.argv[1]
else:
	filename = 'main.txt'

env = gym.make('CartPole-v1', render_mode='human')

w = wrapper.Wrapper(4, 'saves/', filename)

obs, info = env.reset()
w.init()
sum_reward = 0.0
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

w.save_for_display('../', 'display.txt')
