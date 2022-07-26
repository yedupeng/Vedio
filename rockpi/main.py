import multiprocessing
from SerialTask import main_serial
from DetectionTask import main_detection


def camera_f(pipeline):
    main_detection('/dev/ttyUSB0', pipeline)

def serial_f(pipeline):
    main_serial('/dev/ttyUSB0', pipeline)


if __name__ == '__main__':
    pipe_camera, pipe_serial = multiprocessing.Pipe()

    camera_p = multiprocessing.Process(target=camera_f, args=(pipe_camera, ))
    serial_p = multiprocessing.Process(target=serial_f, args=(pipe_serial, ))

    camera_p.start()
    serial_p.start()

    camera_p.join()
    serial_p.join()
