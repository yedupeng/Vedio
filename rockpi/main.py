import loguru
from communite_module.Communications import SelfSerial
from detection_module.Detections import Detections

from loguru import logger
import cv2

import argparse

def make_args():
    parser = argparse.ArgumentParser('Flying Args')
    parser.add_argument('-point1', default='3rc', type=str, help='position color shape')
    parser.add_argument('-point2', default='5br', type=str, help='position color shape')
    return parser


if __name__ == '__main__':
    args = make_args().parse_args()
    logger.info('address:   {}'.format(args))

    self_serial = SelfSerial('/dev/ttyUSB0')
    detections = Detections()

    cap = cv2.VideoCapture(0)
    size = (640*0.5, 480*0.5)
    cap.set(3, size[0])
    cap.set(4, size[1])
    cap.set(10,150)

    mode = 0

    while True:
        ret,frame = cap.read()
        if ret:
            mode = self_serial.uart_read_mode(mode)

            # if mode == "17":
            #     msg = detections.find_color(frame)
            #     if msg:
            #         self_serial.uart_send_msg((17, ) + (msg))

            # elif mode == "18":
            #     detections.recognice_text(frame)

            # elif mode == "19":
            #     detections.find_target(frame)

            # elif mode == "20":
            #     detections.recogniced_line(frame)
            if mode == '21':
                msg = detections.dian_sai(args.point1, args.point2)
                if msg:
                    self_serial.uart_send_msg((21, ) + (msg))
            

            
    cap.release()
    cv2.destroyAllWindows()
