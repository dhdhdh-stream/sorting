# Python3

import datetime
import errno
import os
import paramiko
import stat
import sys
import time

# INCLUDE_EC2 = True
INCLUDE_EC2 = False

print('Starting...')

workers = []
ec2_workers = []

workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
for line in workers_file:
	arr = line.strip().split()
	workers.append([arr[0], arr[1], arr[2], arr[3]])
workers_file.close()

if INCLUDE_EC2:
	ec2_workers_file = open(os.path.expanduser('~/ec2_workers.txt'), 'r')
	for line in ec2_workers_file:
		arr = line.strip().split()
		ec2_workers.append([arr[0], arr[1], arr[2]])
	ec2_workers_file.close()

solution_file = open('saves/main.txt', 'r')
curr_id = int(solution_file.readline())
solution_file.close()

while True:
	updated = False

	solution_file = open('saves/main.txt', 'r')
	main_id = int(solution_file.readline())
	solution_file.close()

	if main_id > curr_id:
		print('main updated')

		curr_id = main_id

		updated = True
	else:
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

					client_sftp.get('workers/' + worker[0] + '/saves/' + worker[0] + '.txt',
									'saves/main.txt')

					curr_id = worker_curr_id

					updated = True

			except IOError:
				pass

			client_sftp.close()
			client.close()

			if updated:
				break

		if not updated and INCLUDE_EC2:
			for worker in ec2_workers:
				client = paramiko.SSHClient()
				client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
				client.connect(worker[1],
							   username=worker[2],
							   key_filename=os.path.expanduser('~/kp1.pem'))

				client_sftp = client.open_sftp()

				try:
					worker_solution_file = client_sftp.file('workers/' + worker[0] + '/saves/' + worker[0] + '.txt', 'r')
					worker_curr_id = int(worker_solution_file.readline())
					worker_solution_file.close()

					if worker_curr_id > curr_id:
						print('ec2 ' + worker[0] + ' updated')

						client_sftp.get('workers/' + worker[0] + '/saves/' + worker[0] + '.txt',
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
		print(datetime.datetime.now())

		# simply use workers[0]
		client = paramiko.SSHClient()
		client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
		client.connect(workers[0][1],
					   username=workers[0][2],
					   password=workers[0][3])

		client_sftp = client.open_sftp()

		try:
			client_sftp.put('saves/main.txt', 'workers/saves/main_temp.txt')
			stdin, stdout, stderr = client.exec_command('mv workers/saves/main_temp.txt workers/saves/main.txt')
			for line in iter(lambda:stdout.readline(2048), ''):
				print(line, end='')

		except IOError:
			pass

		client_sftp.close()
		client.close()

		if INCLUDE_EC2:
			# simply use ec2_workers[0]
			ec2_client = paramiko.SSHClient()
			ec2_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
			ec2_client.connect(ec2_workers[0][1],
							   username=ec2_workers[0][2],
							   key_filename=os.path.expanduser('~/kp1.pem'))

			ec2_client_sftp = ec2_client.open_sftp()

			try:
				ec2_client_sftp.put('saves/main.txt', 'workers/saves/main_temp.txt')
				stdin, stdout, stderr = ec2_client.exec_command('mv workers/saves/main_temp.txt workers/saves/main.txt')
				for line in iter(lambda:stdout.readline(2048), ''):
					print(line, end='')

			except IOError:
				pass

			ec2_client_sftp.close()
			ec2_client.close()

		# extra sleep to give workers time to update to help avoid a race condition
		time.sleep(20)

	time.sleep(20)

print('Done')
