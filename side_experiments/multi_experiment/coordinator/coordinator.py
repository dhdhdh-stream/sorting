import os
import paramiko
import time

from collections import deque
from queue import Queue

from task_thread import EXPLORE_ITERS
from task_thread import TaskThread
from task_tree import TaskTree

print('Starting...')

workers = Queue()
workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
for line in workers_file:
	arr = line.strip().split()
	workers.put([arr[0], arr[1], arr[2], arr[3]])
workers_file.close()

# simply use any worker
initialize_worker = workers.get()
initialize_client = paramiko.SSHClient()
initialize_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
initialize_client.connect(initialize_worker[1],
						  username=initialize_worker[2],
						  password=initialize_worker[3])
initialize_client_sftp = initialize_client.open_sftp()
stdin, stdout, stderr = initialize_client.exec_command('mkdir distributed')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')
try:
	initialize_client_sftp.put('worker', 'distributed/worker')
except IOError as e:
	print(e)
stdin, stdout, stderr = initialize_client.exec_command('chmod +x distributed/worker')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')
initialize_client.close()
workers.put(initialize_worker)

# task_tree = TaskTree()

task_tree_file = open('saves/task_tree.txt', 'r')
task_tree = TaskTree(task_tree_file)
task_tree_file.close()

tasks = deque()
task_tree.init_and_add_tasks(tasks)

task_tree_file = open('saves/task_tree.txt', 'w')
task_tree.save(task_tree_file)
task_tree_file.close()

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

	while not workers.empty():
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

	time.sleep(30)
