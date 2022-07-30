from communite_module.Communications import SelfSerial
from detection_module.Detections import Detections

from loguru import logger
import cv2


if __name__ == '__main__':
    self_serial = SelfSerial('/dev/ttyUSB0')
    detections = Detections()

    cap = cv2.VideoCapture(0)
    size = (640*0.5, 480*0.5)
    cap.set(3, size[0])
    cap.set(4, size[1])
    cap.set(10,150)

    mode = 0


    cap2 = cv2.VideoCapture(0)
    cap2.set(3, size[0])
    cap2.set(4, size[1])
    cap2.set(10,150)

    logger.info('System Starting')
    while True:
        if mode != 12:
            ret,frame = cap.read()
        elif mode == 12:
            ret,frame = cap2.read()

        if ret:
            mode = self_serial.uart_read_mode(mode)

            #发送上线消息
            if mode == 0:
                self_serial.uart_send_msg(0, (1, ))

            elif mode == 1:
                msg = detections.transmit_keyboard_msg()
                if msg:
                    self_serial.uart_send_msg(1, msg)
                mode = 50

            elif mode == 10:
                msg = detections.find_all(frame)
                if msg:
                    self_serial.uart_send_msg(10, msg)

            #摄像头识别
            elif mode == 11:
                msg = detections.get_color()
                if msg:
                    self_serial.uart_send_msg(11, (msg, ))

            #模式2 右飞
            elif mode == 12:
                msg = detections.find_color_forward(frame)
                if msg:
                    self_serial.uart_send_msg(12, msg)
            
            #模式二 前飞
            elif mode == 13:
                msg = detections.find_color_forward(frame)
                if msg:
                    self_serial.uart_send_msg(13, msg)

            #降落 return 5 flag x_h x_l y_h y_l
            elif mode  == 20:
                msg = detections.Template(frame)
                if msg:
                    self_serial.uart_send_msg(20, msg)

            elif mode == 50:
                pass
            

    cap.release()
    cv2.destroyAllWindows()
