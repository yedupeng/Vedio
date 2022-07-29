import loguru
from communite_module.Communications import SelfSerial
from detection_module.Detections import Detections

from loguru import logger
import cv2


if __name__ == '__main__':
    logger.info('System Started')

    self_serial = SelfSerial('COM4')
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
                pass

            #模式2
            elif mode == 12:
                pass

            #降落 return 5 flag x_h x_l y_h y_l
            elif mode == 20:
                pass

            elif mode == 50:
                pass
            

    cap.release()
    cv2.destroyAllWindows()
