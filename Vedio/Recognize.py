from itertools import count
from tkinter.messagebox import NO
import cv2
import numpy as np
import pytesseract
import math

Width = 640*0.5
Height = 480*0.5
cap = cv2.VideoCapture(1)
cap.set(3, Width)
cap.set(4, Height)
cap.set(10,150)
model = 0

class Function():
    def __init__(self) -> None:
        #  自定义画笔颜色
        self.pen_color = [[51,153,255],[255,0,255],[0,255,0]]
        self.template = cv2.imread("E:\\Fly_nobody\\Node\\template\\3.jpg")
        Methods = [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED, cv2.TM_CCORR, cv2.TM_CCORR_NORMED, cv2.TM_CCOEFF, cv2.TM_CCOEFF_NORMED]
        self.method = Methods[5]
        self.location = [0, 0]

        # 找到颜色
        """
        注意:drawContours是画白色区域的轮廓
        """
    def find_color(self,image):
        #  定义高低阈值范围
        center = 0
        low = np.array([17,123,199])
        high = np.array([179,255,255])
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
            left_point_x = np.min(box[:, 0])
            right_point_x = np.max(box[:, 0])
            center = (left_point_x+right_point_x)/2
        except:
            pass
        cv2.imshow('camera', image)
        cv2.waitKey(1)
        if center != 0:
            print(center)
            return center

    def Template(self,img):
        # 对摄像头图像进行处理
        x,y,w,h = 0,0,0,0
        Img = img.copy()
        gray = cv2.cvtColor(Img,cv2.COLOR_RGB2GRAY)
        thresh = cv2.threshold(gray,150,255,cv2.THRESH_BINARY_INV)[1]
        erode1 = cv2.erode(thresh, None, iterations=2)
        dia1 = cv2.dilate(erode1, None, iterations=1)
        contours,hierarchy= cv2.findContours(dia1,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
        cv2.drawContours(gray, contours, -1, (0, 255, 0), 3)
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
        if (cv2.matchShapes(cnts[0],contours[min_pos],1,0.0)<0.4):
            cv2.drawContours(img, contours[min_pos], -1, (0, 255, 0), 3)
        
        hstack = np.hstack([gray,thresh])
        cv2.imshow("img",hstack)
        cv2.imshow("img2",img)
        cv2.waitKey(1)

    def recognice_text(self,img):
        center = 0
        pytesseract.pytesseract.tesseract_cmd = "C:\Program Files\Tesseract-OCR\\tesseract.exe"
        H,W = img.shape[:2]
        boxes = pytesseract.image_to_boxes(img)
        for box in boxes.splitlines():
             if box[0] == 'A':
                b = box.split(' ')
                x, y, w, h = int(b[1]), int(b[2]), int(b[3]), int(b[4])
                cv2.rectangle(img, (x, H-y), (w, H-h), (0, 0, 255), 2)
                center = (x+w)/2
                break
        cv2.imshow("img",img)
        cv2.waitKey(1)
        if center != 0:
            print(center)
            return center
        
    def find_target(self,img):
        center = 0
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
                center = i[0]
        except:
            pass
        cv2.imshow("img",img)
        cv2.imshow("Img",can)
        if center != 0:
            print(center)
            return center

    def recogniced_line(self,img):
        Img = img.copy()
        rows = Img.shape[0]
        cols = Img.shape[1]
        part1 = Img[rows//3:rows,0:cols*2//5]
        part2 = Img[rows//3:rows,cols*2//6:cols*4//6]
        part3 = Img[rows//3:rows,cols*3//5:cols]

        blurred1 = cv2.GaussianBlur(part1, (7, 7), 0)
        edge1s = cv2.Canny(blurred1,170,255)
        line1s = cv2.HoughLines(edge1s,1,np.pi/180,140)

        blurred2 = cv2.GaussianBlur(part2, (7, 7), 0)
        edge2s = cv2.Canny(blurred2,170,255)
        line2s = cv2.HoughLines(edge2s,1,np.pi/180,140)

        blurred3 = cv2.GaussianBlur(part3, (7, 7), 0)
        edge3s = cv2.Canny(blurred3,170,255)
        line3s = cv2.HoughLines(edge3s,1,np.pi/180,140)

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
                the2 = math.degrees(theta2)

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
        if the2 ==0:
            if the1!=0:
                print("右偏了，向左靠")
            elif the3!=0:
                print("左偏了，向右靠")
        elif the1 != 0 and the2 !=0:
            if 90<the1<130:
                print("右转90")
            elif 150<the1<180:
                print("右微调")
            else:
                print("左靠")
        elif the2 != 0 and the3 != 0:
            if 90<the3<130:
                print("左转90")
            elif 0<the3<35:
                print("左微调")
            else:
                print("右靠")
        cv2.imshow("img1",part1)
        cv2.imshow("img2",part2)
        cv2.imshow("img3",part3)


if __name__ == "__main__":
    fun = Function()
    while True:
        id,frame = cap.read()
        if model == 1:
            image = cv2.GaussianBlur(frame, (7, 7), 0)  
            imgHSV = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            fun.find_color(imgHSV)
        elif model == 2:
            fun.Template(frame)
        elif model == 3:
            fun.recognice_text(frame)
        elif model == 4:
            fun.find_target(frame)
        else:
            fun.recogniced_line(frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()

