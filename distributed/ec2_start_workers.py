# Python3

import os
import paramiko
import select
import sys
import time

workers = []

workers_file = open(os.path.expanduser('~/ec2_workers.txt'), 'r')
for line in workers_file:
	arr = line.strip().split()
	workers.append([arr[0], arr[1], arr[2]])
workers_file.close()

clients = []
transports = []
channels = []

for w_index in range(len(workers)):
	client = paramiko.SSHClient()
	client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
	client.connect(workers[w_index][1],
				   username=workers[w_index][2],
				   key_filename=os.path.expanduser('~/kp1.pem'))
	clients.append(client)

	client_sftp = client.open_sftp()

	stdin, stdout, stderr = client.exec_command('mkdir workers')
	for line in iter(lambda:stdout.readline(2048), ''):
		print(line, end='')

	# create separate directory, executable, saves for each worker

	stdin, stdout, stderr = client.exec_command('mkdir workers/' + workers[w_index][0])
	for line in iter(lambda:stdout.readline(2048), ''):
		print(line, end='')

	client_sftp.put('worker', 'workers/' + workers[w_index][0] + '/worker')
	stdin, stdout, stderr = client.exec_command('chmod +x workers/' + workers[w_index][0] + '/worker')
	for line in iter(lambda:stdout.readline(2048), ''):
		print(line, end='')

	stdin, stdout, stderr = client.exec_command('mkdir workers/' + workers[w_index][0] + '/saves')
	for line in iter(lambda:stdout.readline(2048), ''):
		print(line, end='')

	client_sftp.put('saves/main.txt', 'workers/' + workers[w_index][0] + '/saves/main.txt')

	client_sftp.close()

	transport = client.get_transport()
	transports.append(transport)

	channel = transport.open_session()
	channels.append(channel)

	command = './workers/' + workers[w_index][0] + '/worker ' + 'workers/' + workers[w_index][0] + '/ ' + workers[w_index][0] + ' 2>&1'
	print(command)
	channel.exec_command(command)

	time.sleep(1)

while True:
	for w_index in range(len(workers)):
		rl, wl, xl = select.select([channels[w_index]],[],[],0.0)
		if len(rl) > 0:
			print(workers[w_index][0])
			print(channels[w_index].recv(1024))

	time.sleep(15)
