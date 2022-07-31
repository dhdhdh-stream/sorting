import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.style as mplstyle
import time

LEFT = 0
STAY = 1
RIGHT = 2

def pretty_print_action(action):
	result = '('
	result += "{:.2f}".format(action[0])
	result += ', '
	if action[1] == LEFT:
		result += 'LEFT'
	elif action[1] == STAY:
		result += 'STAY'
	else:
		result += 'RIGHT'
	result += ')'
	return result

mplstyle.use('fast')
plt.ion()
fig, ax = plt.subplots()

while True:
	starting_underlying = []
	actions = []
	with open('../seq_display.txt') as file:
		num_underlying = int(file.readline())
		for _ in range(num_underlying):
			starting_underlying.append(float(file.readline()))
		num_actions = int(file.readline())
		for _ in range(num_actions):
			sline = file.readline().split(',')
			action = []
			action.append(float(sline[0]))
			action.append(int(sline[1]))
			actions.append(action)

	underlying = starting_underlying.copy()

	target_result = underlying.copy()
	target_result.sort()

	pointer = 0
	for a_index in range(len(actions)):
		plt.cla()
		ax.set_xlim([0, 200+50*len(underlying)])
		ax.set_ylim([0, 200])
		ax.axis('off')

		l_lim = patches.Rectangle((25, 125), 50, 50, facecolor='r')
		ax.add_patch(l_lim)

		for u_index in range(len(underlying)):
			rect = patches.Rectangle((75+50*u_index, 125), 50, 50, linewidth=1, edgecolor='black', facecolor='none')
			ax.add_patch(rect)
			ax.annotate("{:.2f}".format(underlying[u_index]), xy=(100+50*u_index, 150), fontsize=15, horizontalalignment='center', verticalalignment='center')

		r_lim = patches.Rectangle((75+50*(len(underlying)), 125), 50, 50, facecolor='r')
		ax.add_patch(r_lim)

		ax.annotate("", xy=(100+50*pointer, 120), xytext=(100+50*pointer, 100), arrowprops=dict(arrowstyle="->"))

		if a_index == 0:
			left_operation = '-'
		else:
			left_operation = pretty_print_action(actions[a_index-1])
		ax.annotate('Previous Action: '+left_operation, xy=(75+25*len(underlying), 85), fontsize=15, color='lightgrey', horizontalalignment='center', verticalalignment='center')
		right_operation = pretty_print_action(actions[a_index])
		ax.annotate('Upcoming Action: '+right_operation, xy=(75+25*len(underlying), 55), fontsize=15, horizontalalignment='center', verticalalignment='center')

		ax.annotate('#'+str(a_index), xy=(75+25*len(underlying), 25), fontsize=15, horizontalalignment='center', verticalalignment='center')

		plt.gca().set_aspect('equal')
		fig.canvas.draw()
		fig.canvas.flush_events()
		time.sleep(1.0)

		if pointer >= 0 and pointer < len(underlying):
			underlying[pointer] += actions[a_index][0]
		if actions[a_index][1] == LEFT:
			if pointer >= 0:
				pointer -= 1
		elif actions[a_index][1] == RIGHT:
			if pointer < len(underlying):
				pointer += 1

	plt.cla()
	ax.set_xlim([0, 200+50*len(underlying)])
	ax.set_ylim([0, 200])
	ax.axis('off')

	l_lim = patches.Rectangle((25, 125), 50, 50, facecolor='r')
	ax.add_patch(l_lim)

	for u_index in range(len(underlying)):
		rect = patches.Rectangle((75+50*u_index, 125), 50, 50, linewidth=1, edgecolor='black', facecolor='none')
		ax.add_patch(rect)
		ax.annotate("{:.2f}".format(underlying[u_index]), xy=(100+50*u_index, 150), fontsize=15, horizontalalignment='center', verticalalignment='center')

	r_lim = patches.Rectangle((75+50*(len(underlying)), 125), 50, 50, facecolor='r')
	ax.add_patch(r_lim)

	ax.annotate("", xy=(100+50*pointer, 120), xytext=(100+50*pointer, 100), arrowprops=dict(arrowstyle="->"))

	if len(actions) == 0:
		left_operation = '-'
	else:
		left_operation = pretty_print_action(actions[-1])
	ax.annotate('Previous Action: '+left_operation, xy=(75+25*len(underlying), 85), fontsize=15, color='lightgrey', horizontalalignment='center', verticalalignment='center')
	ax.annotate('Upcoming Action: HALT', xy=(75+25*len(underlying), 55), fontsize=15, horizontalalignment='center', verticalalignment='center')

	final_result = True
	for u_index in range(len(underlying)):
		if abs(underlying[u_index]-target_result[u_index]) > 0.1:
			final_result = False
			break

	if final_result:
		ax.annotate('#'+str(len(actions))+' (Score: 1.0)', xy=(75+25*len(underlying), 25), fontsize=15, color='green', horizontalalignment='center', verticalalignment='center')
	else:
		ax.annotate('#'+str(len(actions))+' (Score: 0.0)', xy=(75+25*len(underlying), 25), fontsize=15, color='red', horizontalalignment='center', verticalalignment='center')

	plt.gca().set_aspect('equal')
	fig.canvas.draw()
	fig.canvas.flush_events()
	time.sleep(5.0)
