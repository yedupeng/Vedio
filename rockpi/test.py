import multiprocessing
from loguru import logger

def fly(queue_fly):
    queue = queue_fly
    queue.put('1234')
    print('flysend1')
    queue.put('5678')
    print('flysend2')
    date = queue.get()
    print('fly2:{}'.format(date))

def rockpi(queue_rockpi):
    queue = queue_rockpi
    date = queue.get()
    print('rockpi:{}'.format(date))

if __name__ == '__main__':
    queue =multiprocessing.Queue()
    camera_p = multiprocessing.Process(target=fly, args=(queue, ))
    serial_p = multiprocessing.Process(target=rockpi, args=((queue, )))

    camera_p.start()
    serial_p.start()

    camera_p.join()
    serial_p.join()