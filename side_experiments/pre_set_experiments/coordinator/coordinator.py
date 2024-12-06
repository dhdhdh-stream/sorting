from collections import deque
from queue import Queue

import task_thread
import task_tree

print('Starting...')

task_tree = TaskTree()

# task_tree_file = open('saves/task_tree.txt', 'r')
# task_tree = TaskTree(task_tree_file)
# task_tree_file.close()

tasks = deque()
task_tree.add_tasks(tasks)

workers = Queue()
workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
for line in workers_file:
	arr = line.strip().split()
	workers.put([arr[0], arr[1], arr[2], arr[3]])
workers_file.close()

task_threads = []
while True:
	for t_index in range(len(task_threads)-1, -1, -1):
		task_threads[t_index].check_status()

		if task_threads[t_index].curr_iter == EXPLORE_ITERS:
			task_threads[t_index].close()

			task_threads[t_index].tasknode.finalize(
				task_threads[t_index].index)

			task_tree.init_and_add_tasks(tasks)

			task_tree_file = open('saves/task_tree.txt', 'w')
			task_tree.save(task_tree_file)
			task_tree_file.close()

			workers.put(task_threads[t_index].worker)

			del task_threads[t_index]

	if not workers.empty():
		if len(tasks) == 0:
			task_tree.expand()
			task_tree.init_and_add_tasks(tasks)

			task_tree_file = open('saves/task_tree.txt', 'w')
			task_tree.save(task_tree_file)
			task_tree_file.close()

		worker = workers.get()
		task = tasks.popleft()

		task_thread = TaskThread(worker, task[0], task[1])

		task_threads.append(task_thread)

	time.sleep(20)
