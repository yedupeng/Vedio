import cv2
import pytesseract
import numpy as np
import math
from loguru import logger

from utils.GetKeyboardInput import get_keyboard_input
from utils.SplitInt import get_gigh_low_data


class Detections():
    def __init__(self):
        self.pen_color = [[51,153,255],[255,0,255],[0,255,0]]
        self.blue = []
        self.red = []
        self.template = cv2.imread('/home/rock/code/Vedio/rockpi/samples/YuanDian.png')
        Methods = [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED, cv2.TM_CCORR, cv2.TM_CCORR_NORMED, cv2.TM_CCOEFF, cv2.TM_CCOEFF_NORMED]
        self.method = Methods[5]
        self.location = [0, 0]
        self.color = 0
        self.coner = 0

    def get_color(self,image):
        center = 0
        flag = 0
        kernel = np.ones((5,5),np.uint8)
        for flag in range(0,2):
            Img = image.copy()
            if flag == 0:
                low = np.array([0,137,138])
                high = np.array([179,255,255])
            elif flag == 1:
                low = np.array([90,101,39])
                high = np.array([179,255,255])
                #  腐蚀 膨胀消除噪点
            mask = cv2.erode(Img,kernel=kernel,iterations=2)
            mask = cv2.dilate(mask,kernel=kernel,iterations=2)
            #  得到感兴趣区
            mask = cv2.inRange(mask,low,high)
            #  找到边界
            cnts = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)[-2]
            try:
                #  得到色块区域并画出轮廓
                are_max = max(cnts, key=cv2.contourArea)
                area = cv2.contourArea(are_max)
                perimeter = cv2.arcLength(are_max,True)
                approx = cv2.approxPolyDP(are_max,0.02*perimeter,True)
                CornerNum = len(approx)                                                      # 角的数量
                rect = cv2.minAreaRect(are_max)
                box = cv2.boxPoints(rect)
                x, y, w, h = cv2.boundingRect(approx)
                if CornerNum ==3: 
                    print("triangle")
                    self.coner = CornerNum
                    self.color = flag
                elif CornerNum == 4:
                    self.coner = CornerNum
                    self.color = flag
                    print("Square")
                elif CornerNum > 4:
                    CornerNum == 5
                    print("Circle")
                    self.coner = CornerNum
                    self.color = flag
                else:
                    print("未识别到")
                    return 0
                cv2.drawContours(image, [np.int0(box)], -1, (0, 255, 255), 2)
            except:
                pass
                return 0
            print(str(self.coner)+str(self.color))
            return 1
        # cv2.imshow('camera', image)
        # cv2.imshow('mask',mask)
        # cv2.waitKey(1)


    #model 12以及13中调用的计算cos函数
    def angle_cos(self,p0, p1, p2):
        d1, d2 = (p0-p1).astype('float'), (p2-p1).astype('float')
        return abs( np.dot(d1, d2) / np.sqrt( np.dot(d1, d1)*np.dot(d2, d2) ) )

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

        low = np.array([0,119,105])
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
        # cv2.imshow('camera', img)
        # cv2.waitKey(50)
        if flag:
            target = [int(area_list[index][0]), int(area_list[index][1])]
            logger.info('mode 10:   {}, {}'.format(flag, target))
            return (flag, ) + get_gigh_low_data(target[0]) + get_gigh_low_data(target[1])
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
        tem = cv2.cvtColor(self.template, cv2.COLOR_RGB2GRAY)
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
        # cv2.imshow("img",can)
        # cv2.imshow("img2",img)
        # cv2.waitKey(50)
        if flag:
            centerpoint_int = [int(centerpoint[0]), int(centerpoint[1])]
            if centerpoint[0] != 0:
                logger.info('mode 20:   {}, {}'.format(flag, centerpoint_int))
                return (flag, ) + get_gigh_low_data(centerpoint_int[0]) + get_gigh_low_data(centerpoint_int[1])
        else:
            return 

    #model 12、13  前飞识别特征色块  右飞识别特征色块
    def find_color_forward(self,image):
        #  定义高低阈值范围
        centerpoint = (0,0)
        number = 0
        flag = 0
        self.color = 0
        self.coner = 3
        if self.color == 0:
            low = np.array([0,171,131])
            high = np.array([179,255,255])
        elif self.color == 1:
            low = np.array([90,101,39])
            high = np.array([179,255,255])
        #  得到感兴趣区
        mask = cv2.inRange(image,low,high)
        can = cv2.Canny(mask,50,150)
        list = []
        #  找到边界
        cnts = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)[-2]
        try:
            for i in cnts:
                area = cv2.contourArea(i)
                if area < 40:
                    pass
                else:
                    perimeter = cv2.arcLength(i,True)
                    approx = cv2.approxPolyDP(i,0.02*perimeter,True)
                    x, y, w, h = cv2.boundingRect(approx) 
                    CornerNum = len(approx)                                                      # 角的数量
                    rect = cv2.minAreaRect(i)
                    box = cv2.boxPoints(rect)
                    x, y, w, h = cv2.boundingRect(approx)
                    if CornerNum <5:
                        if CornerNum == self.coner:
                            if len(approx) == 4 and cv2.isContourConvex(approx):
                                approx = approx.reshape(-1, 2)
                                max_cos = np.max([self.angle_cos( approx[i], approx[(i+1) % 4], approx[(i+2) % 4] ) for i in range(4)])
                                # 只检测矩形（cos90° = 0）
                                if max_cos > 0.5:
                                    break
                                # 检测四边形（不限定角度范围）
                            cv2.drawContours(image, [np.int0(box)], -1, (0, 255, 255), 2)
                            perimeter = cv2.arcLength(i,True)
                            point_1 = box[0]    # 左上
                            point_2 = box[2]    # 右下
                            
                            centerpoint = ((point_1[0] + point_2[0])/2, (point_1[1] + point_2[1])/2)
                            list.append((centerpoint[0],centerpoint[1]))
                        else:
                            print("未识别到")
                    elif self.coner==5 and CornerNum>5:
                        S1=cv2.contourArea(i)
                        ell=cv2.fitEllipse(i)
                        rect = cv2.minAreaRect(i)
                        box = cv2.boxPoints(rect)
                        S2 = math.pi*ell[1][0]*ell[1][1]
                        if (S1/S2)>0.2 :#面积比例，可以更改，根据数据集。。。
                            image = cv2.ellipse(image, ell, (0, 255, 0), 2)
                            point_1 = box[0]    # 左上
                            point_2 = box[2]    # 右下
                            centerpoint = ((point_1[0] + point_2[0])/2, (point_1[1] + point_2[1])/2)
                            list.append((centerpoint[0],centerpoint[1]))

        except Exception as e:
            pass
        
        # cv2.imshow('camera', image)
        # cv2.imshow('mask', mask)
        # cv2.imshow('can', can)
        # cv2.waitKey(1)

        if list is not None:
            if len(list) == 1:
                flag = 1
                number = 1
                return (flag, ) + number +get_gigh_low_data(int(list[0][0])) + get_gigh_low_data(int(list[0][1]))
            elif len(list) == 2:
                flag = 1
                number = 2
                if list[0][0]<list[1][0]:
                    return (flag, ) + number + get_gigh_low_data(int(list[0][0])) + get_gigh_low_data(int(list[0][1]) + get_gigh_low_data(int(list[1][0])) + get_gigh_low_data(int(list[1][1])))
                else:
                    return (flag, ) + number + get_gigh_low_data(int(list[1][0])) + get_gigh_low_data(int(list[1][1]) + get_gigh_low_data(int(list[0][0])) + get_gigh_low_data(int(list[0][1])))