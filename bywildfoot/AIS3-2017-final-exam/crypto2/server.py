#!/usr/bin/env python3
import hashlib
import os
import base64
import random

import socketserver
import flag_leaker
from user_db import users
from sifur import *
from easysock import *

class EncryptedServer(socketserver.BaseRequestHandler):
    def handle(self):
        print('Connected from %s:%d' % self.client_address)

        cipher = Sifur()
        ns = NumberSocket(self.request)

        # send public key
        for i in cipher.pub:
            ns.write(i)
        # handshake
        k, l = ns.read(), ns.read()
        response = cipher.second(k, l)
        ns.write(response)

        S = EasySocket(SifurSocket(ns, cipher))

        chall = os.urandom(16)
        S.writeline(base64.b64encode(chall).decode())
        v = int(S.readline())

        if hashlib.sha1(b'%s%d' % (chall, v)).hexdigest()[:5] != '12345':
            ns.write(0)
            return

        S.writeline('User?')
        user = S.readline().strip()
        S.writeline('Pass?')
        password = S.readline().strip()

        if users.get(user) != password:
            S.writeline('fail')
            return
        else:
            S.writeline('welcome')

        while True:
            data = S.readline()[:-1]
            if not data:
                break
            if data == 'detail':
                S.writeline('--- service code begin ---')
                S.writeline(open(flag_leaker.__file__).read())
                S.writeline('--- service code end ---')
            elif data == 'exit':
                S.writeline('bye')
                break
            elif data == 'get-flag':
                flag_leaker.run(S)
            elif data.startswith('echo '):
                S.writeline(data[5:])

            S.writeline('=== end of command ===')

    def finish(self):
        print('Connection closed from %s:%d' % self.client_address)

port = int(os.getenv('PORT', '5566'))
socketserver.TCPServer.allow_reuse_address = True
server = socketserver.TCPServer(('0.0.0.0', port), EncryptedServer)
print('Listening on %s:%d' % ('0.0.0.0', port))
server.serve_forever()
