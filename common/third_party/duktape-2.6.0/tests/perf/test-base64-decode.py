import math
import random

def test():
    tmp1 = []
    tmp2 = []

    print('build')
    for i in xrange(1024):
        tmp1.append(chr(random.randint(0, 127)))
    tmp1 = ''.join(tmp1)
    for i in xrange(1024):
        tmp2.append(tmp1)
    tmp2 = ''.join(tmp2)
    tmp2 = tmp2.encode('base64')

    print(len(tmp2))
    print('run')
    for i in xrange(2000):
        res = tmp2.decode('base64')

test()
