import cv2
import pytesseract
import numpy as np
import math
from loguru import logger

from utils.GetKeyboardInput import get_keyboard_input
from utils.SplitInt import get_gigh_low_data


class Detections():
    def __init__(self):
        self.flag_color = 0
        self.flag_A = 0
        self.flag_circle = 0
        self.flag_line = 0
        
        self.pen_color = [[51,153,255],[255,0,255],[0,255,0]]
        self.location = [0, 0]

    #mode 1 键盘输入
    def transmit_keyboard_msg(self):
        keyboard_input = get_keyboard_input()
        points_split = []
        for data in keyboard_input:
            points_split.extend(get_gigh_low_data(data))
        return tuple(points_split)

    #mode 10 飞到目标上空，定位最近目标，返回中心坐标
    def find_all(self,img):
        centerpoint = (0,0)
        image = cv2.GaussianBlur(img, (7, 7), 0)  
        imgHSV = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

        low = np.array([0,109,80])
        high = np.array([179,255,255])
        mask = cv2.inRange(imgHSV,low,high)
        length = 0
        index = 0
        flag = 0
        len_list = []
        area_list = []
        cnts = cv2.findContours(mask, cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)[-2]
        try:
            for i in cnts:
                area = cv2.contourArea(i)
                perimeter = cv2.arcLength(i,True)
                if area<300 or perimeter<200:
                    pass
                else:
                    flag = 1                                            # 角的数量
                    rect = cv2.minAreaRect(i)
                    box = cv2.boxPoints(rect)
                    cv2.drawContours(img, [np.int0(box)], -1, (0, 255, 255), 2)
                    point_1 = box[0]    # 左上
                    point_2 = box[2]    # 右下
                    centerpoint = ((point_1[0] + point_2[0])/2, (point_1[1] + point_2[1])/2)
                    length = math.sqrt((centerpoint[0]-320)**2+(centerpoint[1]-240)**2)
                    area_list.append((centerpoint[0],centerpoint[1]))
                    len_list.append(length)
            index = len_list.index(min(len_list))
        except Exception as e:
            flag = 0
        cv2.imshow('camera', img)
        cv2.waitKey(1)
        if flag:
            logger.info('mode 10:   {}, {}'.format(flag, area_list[index]))
            return ((flag, ) + get_gigh_low_data(int(area_list[index][0])) + get_gigh_low_data(int(area_list[index][1])))
        else:
            return (flag, 0, 0, 0, 0)

    #mode 20 降落识别
    def Template(self,img):
        flag = 0
        centerpoint = (0,0)
        Img = img.copy()
        blur = cv2.GaussianBlur(Img, (13, 13), 0)
        can = cv2.Canny(blur,50,150)
        contours,hierarchy= cv2.findContours(can,cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
        cv2.drawContours(Img, contours, -1, (0, 255, 0), 3)
        hierarchy = np.squeeze(hierarchy)


        # 对模板图像进行处理
        tem = cv2.cvtColor(self.template,cv2.COLOR_RGB2GRAY)
        erode = cv2.erode(tem, None, iterations=1)
        dia = cv2.dilate(erode, None, iterations=1)
        thresh_1 = cv2.threshold(dia,30,255,cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
        cnts = cv2.findContours(thresh_1.copy(), cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)[0]
        thresh_color = cv2.cvtColor(tem, cv2.COLOR_GRAY2RGB)
        cv2.drawContours(thresh_color, cnts, -1, (0, 255, 0), 1)
        min_pos = -1
        min_value = 2
        # 将边缘一个一个进行轮廓匹配  找出value最小值（最佳值）
        for i in range(len(contours)):
            value = cv2.matchShapes(cnts[0],contours[i],1,0.0)
            if value < min_value:
                min_value = value
                min_pos = i
        # 限定范围  防止噪点被识别
        try:
            flag = 1
            if (cv2.matchShapes(cnts[0],contours[min_pos],1,0.0)<0.5):
                cv2.drawContours(img, contours[min_pos], -1, (0, 255, 0), 3)
                rect = cv2.minAreaRect(contours[min_pos])
                box = cv2.boxPoints(rect)
                point_1 = box[0]    # 左上
                point_2 = box[2]    # 右下
                centerpoint = ((point_1[0] + point_2[0])/2, (point_1[1] + point_2[1])/2)
        except:
            flag = 0
        cv2.imshow("img",can)
        cv2.imshow("img2",img)
        cv2.waitKey(1)
        if flag != 0:
            logger.info(flag,int(centerpoint[0]),int(centerpoint[1]))
            return (flag, get_gigh_low_data(int(centerpoint[0])), get_gigh_low_data(int(centerpoint[1])))
        else:
            print(flag,0,0)
            return (flag,0,0)