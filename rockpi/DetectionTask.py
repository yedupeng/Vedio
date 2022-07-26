from re import S
from loguru import logger
import serial
import numpy as np
import cv2
import math

from communite_module.Communications import SelfSerial
from detection_module import Detections
from loguru import logger

# 接收类
class Communite():
    def __init__(self, pipeline) -> None:
        self.Uart2 = serial.Serial( port="/dev/ttyUSB0",
                                    bytesize=8,
                                    baudrate=115200,
                                    stopbits=1,
                                    timeout=1000)
        self.model = 0
        self.uart_buf = []
        self.state = 0
        self.pipe = pipeline

        self.pen_color = [[51,153,255],[255,0,255],[0,255,0]]
        Methods = [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED, cv2.TM_CCORR, cv2.TM_CCORR_NORMED, cv2.TM_CCOEFF, cv2.TM_CCOEFF_NORMED]
        self.method = Methods[5]
        self.location = [0, 0]

    

def main_detection(pipeline):
    self_serial = SelfSerial('/dev/ttyUSB0', pipeline)
    detections = Detections()

    cap = cv2.VideoCapture(3)
    size = (640*0.5, 480*0.5)
    cap.set(3, size[0])
    cap.set(4, size[1])
    cap.set(10,150)

    mode = 0

    while True:
        ret,frame = cap.read()
        if ret:
            mode = self_serial.uart_read_mode(mode)

            if mode == "17":
                image = cv2.GaussianBlur(frame, (7, 7), 0)  
                imgHSV = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
                detections.find_color(imgHSV)
                
            if mode == "18":
                detections.recognice_text(frame)

            if mode == "19":
                detections.find_target(frame)

            if mode == "20":
                detections.recogniced_line(frame)
    cap.release()
    cv2.destroyAllWindows()