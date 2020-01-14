#!/usr/bin/python3
import signal
import sys
import os
import time
import string

if sys.version_info < (3, 0): # For python2
	from urlparse import parse_qs
else: # For python3
	from urllib.parse import parse_qs

from base64 import b64encode as b64e
from base64 import b64decode as b64d
from Crypto.Cipher import AES

FLAG = UNKNOWN_FLAG
KEY = UNKNOWN_KEY
IV = UNKNOWN_IV

blockSize = 16

if sys.version_info < (3, 0): # For python2
	input = raw_input


class AESCryptor:

	def __init__(self, key, iv):

		self.KEY = key
		self.IV = iv
		self.aes = AES.new(self.KEY, AES.MODE_ECB, self.IV)


	def encrypt(self, data):
		return self.aes.encrypt(self.pad(data))


	def decrypt(self, data):
		return self.unpad(self.aes.decrypt(data))

	def pad(self, data):
		num = blockSize - len(data) % blockSize
		return data + chr(num) * num

	def unpad(self, data):

		lastValue = 0

		if type(data[-1]) is int:
			lastValue = data[-1]
		else:
			lastValue = ord(data[-1])

		return data[:len(data)-lastValue]


aes = AESCryptor(KEY, IV)


def bye(s):

	print(s)
	exit(0)


def alarm(time):

	signal.signal(signal.SIGALRM, lambda signum, frame: bye('Too slow!'))
	signal.alarm(time)


def printFlag():

	print(FLAG)


def register():

	name = input('What is your name? ').strip()

	for c in name:
		if c not in string.ascii_letters:
			bye('Invalid characters.(Only alphabets are permitted)')

	pwd = input('Give me your password: ').strip()

	for c in pwd:
		if c not in string.ascii_letters:
			bye('Invalid characters. (Only alphabets are permitted)')

	pattern = 'name=' + name + '&role=student' + '&password=' + pwd

	print('This is your token: ' + b64e(aes.encrypt(pattern)).decode())


def login():

	token = input('Give me your token: ').strip()
	name = input('Give me your username: ').strip().encode()
	pwd = input('Give me your password: ').strip().encode()

	try:
		pt = aes.decrypt(b64d(token))
		data = parse_qs(pt, strict_parsing=True)

		if name != data[b'name'][0] or pwd != data[b'password'][0]:
			print('Authentication failed')
			return

		print('Hello %s' % data[b'name'][0].decode())

		if b'admin' in data[b'role']:
			print('Hi admin:')
			printFlag()

	except Exception:
		print('Something went wrong!! QAQ')



def main():

	alarm(60)
	print('Select your choice: ')
	print('0 : Register')
	print('1 : Login')
	num = int(input().strip())

	if num == 0:
		register()
	elif num == 1:
		login()

if __name__ == '__main__':

	main()


