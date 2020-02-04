import zmq
import json

datastore = {}

context = zmq.Context()
sock = context.socket(zmq.REP)
sock.bind("tcp://*:27017")

while True:
    req = sock.recv_json()
    print("Received request: %s" % req)

    if req['type'] == 'set':
        datastore[req['id']] = req['value']
        sock.send_json({ 'error': None })
    elif req['type'] == 'get':
        sock.send_json({ 'ret': datastore, 'error': None })
    else:
        sock.send_json({ 'error': '未知请求！' })