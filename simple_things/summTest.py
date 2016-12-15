from ctypes import *
import time
import cProfile



def testImportMean():
    tmp_dll = cdll.LoadLibrary("/home/nick/python/dllTests/libmean.so")

    summ = 0
    for i in range(0, 1000):
        for j in range(0, 1000):
            summ += tmp_dll.mean(c_int(i), c_int(j))

    return summ


def testImportIterations():
    tmp_dll = cdll.LoadLibrary("/home/nick/python/dllTests/libmean.so")

    summ = tmp_dll.NIterationsLoop(c_int(1000))

    return summ


def testNative():
    summ = 0
    for i in range(0, 1000):
        for j in range(0, 1000):
            summ += i + j

    return summ


def testAll():
    start = time.time()
    summ = testImportMean()
    end = time.time()
    print('testImportMean() : ', end - start)
    print('summ = ', summ)

    start = time.time()
    testImportIterations()
    end = time.time()
    print('testImportIterations() : ', end - start)
    print('summ = ', summ)

    start = time.time()
    testNative()
    end = time.time()
    print('testNative() : ', end - start)
    print('summ = ', summ)


cProfile.run('testImportMean()')
