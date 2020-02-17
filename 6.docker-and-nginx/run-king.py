import zmq
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d.axes3d

window = plt.figure()
view = window.gca(projection='3d')

context = zmq.Context()
sock = context.socket(zmq.REQ)
sock.connect("tcp://www.dungbeetles.xyz:27017")

while True:
    sock.send_json({ 'type': 'get' })
    res = sock.recv_json()
    #res = {'ret': {'47.75.207.201:6004 9': 14, '47.75.207.201:6004 8': 15, '47.75.207.201:6004 7': 7, '47.75.207.201:6004 6': 19, '47.75.207.201:6004 5': 16, '47.75.207.201:6004 4': 7, '47.75.207.201:6004 3': 11, '47.75.207.201:6004 2': 26, '47.75.207.201:6004 1': 17, '47.75.207.201:6004 0': 20}, 'error': None}
    
    plt.cla()
    x = [];y = [];z = []
    n = 0
    for c in res['ret'].items():
        x.append(n)
        y.append(c[1])
        z.append(0)
        n += 1

    view.scatter(x,y,z)
#    view.set(xlim=(0,100),ylim=(0,100),zlim=(0,10))
    plt.pause(.1)

plt.ioff()
plt.show()
