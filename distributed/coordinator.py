# Python3

import errno
import os
import paramiko
import stat
import sys
import time

print('Starting...')

workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
for line in workers_file:
	arr = line.strip().split()
	workers.append([arr[0], arr[1], arr[2], arr[3]])
workers_file.close()

solution_file = open('saves/main/solution.txt', 'r')
curr_id = int(solution_file.readline())
solution_file.close()

while True:
	updated = False

	for worker in workers:
		client = paramiko.SSHClient()
		client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
		client.connect(worker[1],
					   username=worker[2],
					   password=worker[3])

		client_sftp = client.open_sftp()

		try:
			worker_solution_file = client_sftp.file('workers/' + worker[0] + '/saves/' + worker[0] + '.txt', 'r')
			worker_curr_id = int(worker_solution_file.readline())
			worker_solution_file.close()

			if worker_curr_id > curr_id:
				print(worker[0] + ' updated')

				sftp_client.get('workers/' + worker[0] + '/saves/' + worker[0] + '.txt',
								'saves/main.txt')

				curr_id = worker_curr_id

				updated = True

		except IOError:
			pass

		client_sftp.close()
		client.close()

		if updated:
			break

	if updated:
		for worker in workers:
			client = paramiko.SSHClient()
			client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
			client.connect(worker[1],
						   username=worker[2],
						   password=worker[3])

			client_sftp = client.open_sftp()

			client_sftp.put('saves/main.txt', 'workers/' + workers[w_index][0] + '/saves/main_temp.txt')
			client_sftp.rename('workers/' + workers[w_index][0] + '/saves/main_temp.txt', 'workers/' + workers[w_index][0] + '/saves/main.txt');

			client_sftp.close()
			client.close()

		# extra sleep to give workers time to update to help avoid a race condition
		time.sleep(40)

	time.sleep(20)

print('Done')
