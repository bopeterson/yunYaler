# Copyright (c) 2011, Yaler GmbH, Switzerland
# All rights reserved

import sys
import time
import socket

def find (pattern, s):
	x = [0] * len(pattern)
	i = j = t = 0
	while True:
		k = 0
		match = True
		while (k != len(pattern)) and match:
			if i + k == j:
				x[j % len(x)] = s.recv(1)
				j += 1
			t = x[(i + k) % len(x)]
			match = pattern[k] == t
			k += 1
		i += 1
		if match or (t == ''):
			break
	return match

def location(s):
	host = ''
	port = 80
	if find('\r\nLocation: http://', s):
		x = s.recv(1)
		while (x != '') and (x != ':') and (x != '/'):
			host += x
			x = s.recv(1)
		if x == ':':
			port = 0
			x = s.recv(1)
			while (x != '') and (x != '/'):
				port = 10 * port + ord(x) - ord('0')
				x = s.recv(1)
	return host, port

def accept(host, port, id):
	x = [0] * 3
	while True:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((host, port))
		while True:
			s.send(
				'POST /' + id + ' HTTP/1.1\r\n'
				'Upgrade: PTTH/1.0\r\n'
				'Connection: Upgrade\r\n'
				'Host: ' + host + '\r\n\r\n')
			j = 0
			while j != 12:
				x[j % 3] = s.recv(1)
				j += 1
			if (x[0] == '3') and (x[1] == '0') and (x[2] == '7'):
				host, port = location(s)
			acceptable = find('\r\n\r\n', s)
			if not acceptable or (x[0] != '2') or (x[1] != '0') or (x[2] != '4'):
				break
		if not acceptable or (x[0] != '1') or (x[1] != '0') or (x[2] != '1'):
			s.close()
			s = None
		if not acceptable or (x[0] != '3') or (x[1] != '0') or (x[2] != '7'):
			break
	return s

while True:
	s = accept(sys.argv[1], 80, sys.argv[2])
	s.send(
		'HTTP/1.1 200 OK\r\n'
		'Connection: close\r\n'
		'Content-Length: 8\r\n\r\n' +
		time.strftime('%H:%M:%S', time.localtime()))
	time.sleep(0.001)
	s.close()

