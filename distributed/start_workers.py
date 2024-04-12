# Python3

import os
import paramiko
import select
import sys
import time

workers = []

workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
for line in workers_file:
	arr = line.strip().split()
	workers.append([arr[0], arr[1], arr[2], arr[3]])
workers_file.close()

# simply use workers[0]
overall_client = paramiko.SSHClient()
overall_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
overall_client.connect(workers[0][1],
					   username=workers[0][2],
					   password=workers[0][3])

overall_client_sftp = overall_client.open_sftp()

stdin, stdout, stderr = overall_client.exec_command('mkdir workers')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

stdin, stdout, stderr = overall_client.exec_command('mkdir workers/saves')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

overall_client_sftp.put('worker', 'workers/worker')
stdin, stdout, stderr = overall_client.exec_command('chmod +x workers/worker')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

overall_client_sftp.put('saves/main.txt', 'workers/saves/main.txt')

overall_client_sftp.close()
overall_client.close()

clients = []
transports = []
channels = []

for w_index in range(len(workers)):
	client = paramiko.SSHClient()
	client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
	client.connect(workers[w_index][1],
				   username=workers[w_index][2],
				   password=workers[w_index][3])
	clients.append(client)

	client_sftp = client.open_sftp()

	stdin, stdout, stderr = client.exec_command('mkdir workers/' + workers[w_index][0])
	for line in iter(lambda:stdout.readline(2048), ''):
		print(line, end='')

	stdin, stdout, stderr = client.exec_command('mkdir workers/' + workers[w_index][0] + '/saves')
	for line in iter(lambda:stdout.readline(2048), ''):
		print(line, end='')

	client_sftp.close()

	transport = client.get_transport()
	transports.append(transport)

	channel = transport.open_session()
	channels.append(channel)

	command = './workers/worker ' + 'workers/' + workers[w_index][0] + '/ 2>&1'
	print(command)
	channel.exec_command(command)

	time.sleep(1)

while True:
	for w_index in range(len(workers)):
		rl, wl, xl = select.select([channels[w_index]],[],[],0.0)
		if len(rl) > 0:
			message = channels[w_index].recv(1024)

			if len(message) == 0:
				print('worker ' + workers[w_index][0] + ' failed');
				exit(1)

			print(workers[w_index][0])
			print(message)

	time.sleep(15)
