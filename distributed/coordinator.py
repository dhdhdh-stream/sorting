# Python3

import datetime
import errno
import os
import paramiko
import stat
import sys
import time

# INCLUDE_LOCAL = True
INCLUDE_LOCAL = False
INCLUDE_EC2 = True
# INCLUDE_EC2 = False

print('Starting...')

workers = []
ec2_workers = []

if INCLUDE_LOCAL:
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

while True:
	save_file = open('saves/main.txt', 'r')
	curr_timestamp = int(save_file.readline())
	save_file.close()
	curr_average_score = float('-inf')

	start_time = time.time()
	count = 0
	while True:
		if INCLUDE_LOCAL:
			for worker in workers:
				client = paramiko.SSHClient()
				client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
				client.connect(worker[1],
							   username=worker[2],
							   password=worker[3])

				client_sftp = client.open_sftp()

				for filename in client_sftp.listdir('workers/' + worker[0] + '/saves/'):
					if filename.startswith('possible') and 'temp' not in filename:
						print('workers/' + worker[0] + '/saves/' + filename)

						client_sftp.get('workers/' + worker[0] + '/saves/' + filename, 'saves/temp.txt')
						client_sftp.remove('workers/' + worker[0] + '/saves/' + filename)

						possible_file = open('saves/temp.txt', 'r')
						possible_timestamp = int(possible_file.readline())
						possible_average_score = float(possible_file.readline())
						possible_file.close()

						# TODO: potentially remove timestamp check so will keep great runs that finished after increment
						if possible_timestamp == curr_timestamp+1:
							count += 1
							if possible_average_score > curr_average_score:
								os.rename('saves/temp.txt', 'saves/main.txt')

								print('updated')

								curr_average_score = possible_average_score
							else:
								os.remove('saves/temp.txt')
						else:
							os.remove('saves/temp.txt')

				client_sftp.close()
				client.close()

		if INCLUDE_EC2:
			for worker in ec2_workers:
				client = paramiko.SSHClient()
				client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
				client.connect(worker[1],
							   username=worker[2],
							   key_filename=os.path.expanduser('~/kp1.pem'))

				client_sftp = client.open_sftp()

				for filename in client_sftp.listdir('workers/' + worker[0] + '/saves/'):
					if filename.startswith('possible') and 'temp' not in filename:
						print('workers/' + worker[0] + '/saves/' + filename)

						client_sftp.get('workers/' + worker[0] + '/saves/' + filename, 'saves/temp.txt')
						client_sftp.remove('workers/' + worker[0] + '/saves/' + filename)

						possible_file = open('saves/temp.txt', 'r')
						possible_timestamp = int(possible_file.readline())
						possible_average_score = float(possible_file.readline())
						possible_file.close()

						if possible_timestamp == curr_timestamp+1:
							count += 1
							if possible_average_score > curr_average_score:
								os.rename('saves/temp.txt', 'saves/main.txt')

								print('updated')

								curr_average_score = possible_average_score
							else:
								os.remove('saves/temp.txt')
						else:
							os.remove('saves/temp.txt')

				client_sftp.close()
				client.close()

		if count >= 40:
			break

		curr_time = time.time()
		# count needs to be reasonably large as experiments use low samples so may not be positive
		if count >= 20 and curr_time - start_time > 300:
			break

		time.sleep(20)

	print(datetime.datetime.now())
	print(curr_average_score)

	if INCLUDE_LOCAL:
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

	time.sleep(20)

print('Done')
