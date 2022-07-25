import multiprocessing
from communicate import
from rockpi.communicate import Receive 


def camera_f(pipeline):
    pass

def serial_f(pipeline):
    receive = Receive('/dev/ttyUSB0')
    pipe = pipeline

    

    


if __name__ == '__main__':
    pipe_camera, pipe_serial = multiprocessing.Pipe()

    camera_p = multiprocessing.Process(target=camera_f, args=(pipe_camera, ))
    serial_p = multiprocessing.Process(target=serial_f, args=(pipe_serial, ))

    camera_p.start()
    serial_p.start()

    camera_p.join()
    serial_p.join()
