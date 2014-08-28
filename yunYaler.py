# Copyright (c) 2011, Yaler GmbH, Switzerland
# All rights reserved
# based on TimeService.py by Yaler GmbH
# modified for Arduino Yun by Bo Peterson www.asynkronix.se
# 
# Updated 2014-08-28 with timeout to solve stability issue
# Note: this version has not been fully tested yet. 

import sys
import time
import socket

sys.path.insert(0, '/usr/lib/python2.7/bridge/') 

try:                             
	from bridgeclient import BridgeClient as bridgeclient
	value = bridgeclient()   
except ImportError:
	print 'Couldn''t import bridgeclient, will only work in nobridge mode'

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

def getStringBeforePattern (pattern, s):
	beforePattern=''
	x = [0] * len(pattern)
	i = j = t = 0
	while True:
		k = 0
		match = True
		while (k != len(pattern)) and match:
			if i + k == j:
				c=s.recv(1)
				beforePattern += c
				x[j % len(x)] = c
				j += 1
			t = x[(i + k) % len(x)]
			match = pattern[k] == t
			k += 1
		i += 1
		if match or (t == ''):
			break
	if match:
		beforePattern=beforePattern[:(len(beforePattern)-len(pattern))]
	
	return match,beforePattern

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

def accept(host, port, id, usebridge):
	rest='{"command":"unsupported","action":"unsupported"}' # if /arduino not found in url
	putStatus=False
	x = [0] * 3
	while True:
		acceptable=False
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	
		s.settimeout(75)
		try: #to connect
			s.connect((host, port))
		except socket.error:
			#the network might be down
			if not usebridge:
				print('Couldn''t connect at '+time.strftime('%H:%M:%S', time.localtime()))
			s.close()
			s=None
			time.sleep(60) #wait one minute before reconnecting. 
			break

		try: #to send
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
				#find the calling url and parse it
				if acceptable and (x[0] == '1') and (x[1] == '0') and (x[2] == '1'):
					keepGoing, requestUrl = getStringBeforePattern(' HTTP/1.1', s)
					pos=requestUrl.lower().find('/arduino')
					if pos!=-1:
						rest=requestUrl.lower()[pos+8:]
						if usebridge:
							value.put('rest',rest)
							now=time.strftime('%H:%M:%S', time.localtime())
							value.put('time',now)
							putStatus=True
						else:
							print rest
						
				if not acceptable or (x[0] != '2') or (x[1] != '0') or (x[2] != '4'):
					break
		
		except socket.timeout:
			if not usebridge:
				print('Timeout at '+time.strftime('%H:%M:%S', time.localtime()))
			s.close()
			time.sleep(1)
				
		if not acceptable or (x[0] != '1') or (x[1] != '0') or (x[2] != '1'):
			s.close()
			s = None
		
		if not acceptable or (x[0] != '3') or (x[1] != '0') or (x[2] != '7'):
			break

	return s, rest, putStatus

#init
usebridge=False
if len(sys.argv)>3:
	if sys.argv[3]=='bridge':
		usebridge=True

#loop
while True:		
	s,rest,putStatus = accept(sys.argv[1], 80, sys.argv[2],usebridge)
	time.sleep(0.3) # this should be about 50% longer than interval in arduino sketch
	if putStatus:
		reply=value.get('answer')
	else:
		reply=rest # url must contain /arduino
	if s is not None:
		s.send(
			'HTTP/1.1 200 OK\r\n'
			'Connection: close\r\n'
			'Content-Length: '+str(len(reply))+'\r\n\r\n' + reply)
		time.sleep(0.001)
		s.close()
