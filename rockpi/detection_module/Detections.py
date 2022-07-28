import cv2
import pytesseract
import numpy as np
import math
from loguru import logger

from work_module.GetKeyboardInput import get_keyboard_input


class Detections():
    def __init__(self):
        self.flag_color = 0
        self.flag_A = 0
        self.flag_circle = 0
        self.flag_line = 0
        
        self.pen_color = [[51,153,255],[255,0,255],[0,255,0]]
        self.location = [0, 0]

    # 模式17  数据包类型17  数据三位  长度0x03
    def find_color(self,image):
        image = cv2.GaussianBlur(image, (7, 7), 0)  
        image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

        #  定义高低阈值范围
        low = np.array([0,98,182])
        high = np.array([61,255,255])
        kernel = np.ones((5,5),np.uint8)
        #  腐蚀 膨胀消除噪点
        mask = cv2.erode(image,kernel=kernel,iterations=2)
        mask = cv2.dilate(mask,kernel=kernel,iterations=1)
        #  得到感兴趣区
        mask = cv2.inRange(mask,low,high)
        #  找到边界
        cnts = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]
        try:
            #  得到色块区域并画出轮廓
            are_max = max(cnts, key=cv2.contourArea)
            rect = cv2.minAreaRect(are_max)
            box = cv2.boxPoints(rect)
            cv2.drawContours(image, [np.int0(box)], -1, (0, 255, 255), 2)
            point_1 = box[0]    # 左上
            point_2 = box[2]    # 右下
            centerpoint = ((point_1[0] + point_2[0])/20, (point_1[1] + point_2[1])/20)    
            self.flag_color = 1
            center_x = int(centerpoint[0])
            center_y = int(centerpoint[1])
        except:
            self.flag_color = 0
            center_x = 0
            center_y = 0
        cv2.imshow('camera', image)
        cv2.waitKey(1)
        if center_x != 0:
            logger.info('Find Coler:Point:({}, {})'.format(center_x, center_y))
        return (self.flag_color, center_x, center_y)
    
    def Template(self,img):
        x,y,w,h = 0,0,0,0
        Img = img.copy()
        gray = cv2.cvtColor(Img,cv2.COLOR_RGB2GRAY)
        thresh = cv2.threshold(gray,150,255,cv2.THRESH_BINARY_INV)[1]
        erode1 = cv2.erode(thresh, None, iterations=2)
        dia1 = cv2.dilate(erode1, None, iterations=1)
        contours,hierarchy= cv2.findContours(dia1,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
        cv2.drawContours(gray, contours, -1, (0, 255, 0), 3)
        hierarchy = np.squeeze(hierarchy)

        tem = cv2.cvtColor(self.template,cv2.COLOR_RGB2GRAY)
        erode = cv2.erode(tem, None, iterations=1)
        dia = cv2.dilate(erode, None, iterations=1)
        thresh_1 = cv2.threshold(dia,30,255,cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
        cnts = cv2.findContours(thresh_1.copy(), cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)[0]
        thresh_color = cv2.cvtColor(tem, cv2.COLOR_GRAY2RGB)
        cv2.drawContours(thresh_color, cnts, -1, (0, 255, 0), 1)

        min_pos = -1
        min_value = 2
        for i in range(len(contours)):
            value = cv2.matchShapes(cnts[0],contours[i],1,0.0)
            if value < min_value:
                min_value = value
                min_pos = i
        # 限定范围  防止噪点被识别
        if (cv2.matchShapes(cnts[0],contours[min_pos],1,0.0)<0.4):
            cv2.drawContours(img, contours[min_pos], -1, (0, 255, 0), 3)
        
        hstack = np.hstack([gray,thresh])
        cv2.imshow("img",hstack)
        cv2.imshow("img2",img)
        cv2.waitKey(1)
    
    def recognice_text(self,img):
        center_x = 0
        center_y = 0
        pytesseract.pytesseract.tesseract_cmd = "C:\Program Files\Tesseract-OCR\\tesseract.exe"
        H,W = img.shape[:2]
        boxes = pytesseract.image_to_boxes(img)
        for box in boxes.splitlines():
            if box[0] == 'A':
                b = box.split(' ')
                x, y, w, h = int(b[1]), int(b[2]), int(b[3]), int(b[4])
                cv2.rectangle(img, (x, H-y), (w, H-h), (0, 0, 255), 2)
                center_x = (x+w)/2
                center_y = (y+h)/2
                flag_A = 1
                break
            else:
                center_x = 0
                center_y = 0
                flag_A = 0

        cv2.imshow("img",img)
        cv2.waitKey(1)
        if center_x != 0:
            print(center_x)
        self.uart_send(flag_A,center_x,center_y,0,18)
    
    def find_target(self,img):
        center_x = 0
        center_y = 0
        Img = img.copy()
        gray = cv2.cvtColor(Img,cv2.COLOR_RGB2GRAY)
        kernel = np.ones((10, 10))
        erode = cv2.erode(gray, kernel, iterations=2)
        thresh_1 = cv2.threshold(erode,120,255,cv2.THRESH_BINARY)[1] 
        dia = cv2.dilate(thresh_1, kernel, iterations=1)
        can = cv2.Canny(dia,50,150)
        circles = cv2.HoughCircles(can,cv2.HOUGH_GRADIENT,1,100,param1=100,param2=30,minRadius=20,maxRadius=80)
        try:
            circles = np.uint16(np.around(circles.astype('float')))
            for i in circles[0,:]:
                cv2.circle(img,(i[0],i[1]),i[2],(0,0,255),2)
                cv2.circle(img,(i[0],i[1]),2,(255,0,0),3)
                center_x = i[0]
                center_y = i[1]
                flag_circle = 1
        except:
                center_x = 0
                center_y = 0
                flag_circle = 0
        cv2.imshow("img",img)
        cv2.imshow("Img",can)
        if center_x != 0:
            print(center_x)
        self.uart_send(flag_circle,center_x,center_y,0,19)

    def recogniced_line(self,img):
        bias = 0
        Img = img.copy()
        rows = Img.shape[0]
        cols = Img.shape[1]
        part1 = Img[rows//3:rows*2//3,0:cols*2//5]
        part2 = Img[rows//3:rows,cols*2//6:cols*4//6]
        part3 = Img[rows//3:rows*2//3,cols*3//5:cols]

        blurred1 = cv2.GaussianBlur(part1, (7, 7), 0)
        edge1s = cv2.Canny(blurred1,170,255)
        line1s = cv2.HoughLines(edge1s,1,np.pi/180,100)

        blurred2 = cv2.GaussianBlur(part2, (7, 7), 0)
        edge2s = cv2.Canny(blurred2,170,255)
        line2s = cv2.HoughLines(edge2s,1,np.pi/180,100)

        blurred3 = cv2.GaussianBlur(part3, (7, 7), 0)
        edge3s = cv2.Canny(blurred3,170,255)
        line3s = cv2.HoughLines(edge3s,1,np.pi/180,100)

        the1 = 0
        the2 = 0
        the3 = 0

        try:
            for line in line1s:
                rho1,theta1 = line[0]
                a = np.cos(theta1)
                b = np.sin(theta1)
                x0 = a*rho1
                y0 = b*rho1
                x1 = int(x0 + 1000*(-b))
                y1 = int(y0 + 1000*(a))
                x2 = int(x0 - 1000*(-b))
                y2 = int(y0 - 1000*(a))
                cv2.line(part1,(x1,y1),(x2,y2),(0,0,255),2)
                the1 = math.degrees(theta1)

        except:
            pass

        try:
            for line in line2s:
                rho2,theta2 = line[0]
                a = np.cos(theta2)
                b = np.sin(theta2)
                x0 = a*rho2
                y0 = b*rho2
                x1 = int(x0 + 1000*(-b))
                y1 = int(y0 + 1000*(a))
                x2 = int(x0 - 1000*(-b))
                y2 = int(y0 - 1000*(a))
                cv2.line(part2,(x1,y1),(x2,y2),(0,255,0),2)
                the2 = int(math.degrees(theta2))
        except:
            pass

        theta = 0
        cnts = cv2.findContours(edge2s, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]
        try:
            are_max = max(cnts, key=cv2.contourArea)
            area = cv2.contourArea(are_max)
            if area<60:
                pass
            else:
                rect = cv2.minAreaRect(are_max)
                box = cv2.boxPoints(rect)
                cv2.drawContours(part2, [np.int0(box)], -1, (0, 255, 255), 2)

                point_1 = box[0]    # 左上
                point_2 = box[2]    # 右下
                k1 = (point_1[1]-point_2[1])/(point_1[0]-point_2[0])
                theta = math.atan(k1)
                theta = math.degrees(theta)
            #print(theta)
        except:
            pass
            
        try:
            for line in line3s:
                rho3,theta3 = line[0]
                a = np.cos(theta3)
                b = np.sin(theta3)
                x0 = a*rho3
                y0 = b*rho3
                x1 = int(x0 + 1000*(-b))
                y1 = int(y0 + 1000*(a))
                x2 = int(x0 - 1000*(-b))
                y2 = int(y0 - 1000*(a))
                cv2.line(part3,(x1,y1),(x2,y2),(0,255,0),2)
                the3 = math.degrees(theta3)

        except:
            pass
        """
        霍夫左右微调 5
        色块右微调 6
        色块左微调 7
        左转90  1
        右转90  2
        左靠    3
        右靠    4
        直走    0
        """
        if the2 ==0:
            if the1!=0:
                bias = 3
                logger.info("左靠")
            elif the3!=0:
                bias = 4
                logger.info("右靠")
            else:
                if theta>-70 and theta<-10:
                    bias = 6
                elif theta<70 and theta>10:
                    bias = 7
                else:
                    bias = 0
                    #logger.info("继续走")
        elif the1 != 0 and the2 !=0:
            if 70<the1<110:
                logger.info("左转90")
                bias = 1
            elif 150<the1<180:
                bias = 5
            else:
                bias = 3
        elif the2 != 0 and the3 != 0:
            if 70<the3<110:
                logger.info("右转90")
                bias = 2
            elif 0<the3<35:
                bias = 5
            else:
                bias = 4
        cv2.imshow("edg2",edge2s)
        cv2.imshow("img1",part1)
        cv2.imshow("img2",part2)
        cv2.imshow("img3",part3)
        self.uart_send(self.flag_line,bias,the2,0,20)

    def transmit_keyboard_msg(self):
        keyboard_input = get_keyboard_input()
        points_split = []
        for i in keyboard_input:
            temp_h = i >> 8
            points_split.append(temp_h)
            points_split.append(i - (temp_h << 8))
        return tuple(points_split)