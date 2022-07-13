from itertools import count
import cv2
import numpy as np

Width = 640*0.5
Height = 480*0.5
cap = cv2.VideoCapture(0)
cap.set(3, Width)
cap.set(4, Height)
cap.set(10,150)
model = 0

class Function():
    def __init__(self) -> None:
        #  自定义画笔颜色
        self.pen_color = [[51,153,255],[255,0,255],[0,255,0]]
        self.template = cv2.imread("E:\\Fly_nobody\\Node\\template\\1.png",0)
        Methods = [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED, cv2.TM_CCORR, cv2.TM_CCORR_NORMED, cv2.TM_CCOEFF, cv2.TM_CCOEFF_NORMED]
        self.method = Methods[5]
        self.location = [0, 0]

        # 找到颜色
    def find_color(self,image):
        low = np.array([17,123,199])
        high = np.array([179,255,255])
        kernel = np.ones((5,5),np.uint8)
        mask = cv2.erode(image,kernel=kernel,iterations=2)
        mask = cv2.dilate(mask,kernel=kernel,iterations=1)
        mask = cv2.inRange(mask,low,high)
        cnts = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]
        try:
            are_max = max(cnts, key=cv2.contourArea)
            rect = cv2.minAreaRect(are_max)
            box = cv2.boxPoints(rect)
            cv2.drawContours(image, [np.int0(box)], -1, (0, 255, 255), 2)
        except:
            pass
        cv2.imshow('camera', image)
        cv2.waitKey(1)

    def Template(self,img):
        # 对原图像进行处理
        kernel_e = np.ones((5, 5), np.uint8)
        kernel_d = np.ones((5, 5), np.uint8)
        _,thresh = cv2.threshold(img,0,255,0)
        img_e= cv2.erode(thresh,kernel_e,iterations=1)
        img_d = cv2.dilate(img_e,kernel_d,iterations=1)
        contours,hierarchy = cv2.findContours(img_d,2,1)
        contours_list = []
        for img_contour in contours:
            contours_list.append(img_contour)
        max_area = max(contours_list,key=cv2.contourArea)
        i = contours_list.index(max_area)

        _,thresh_1 = cv2.threshold(self.template,0,255,0)
        contours1, hierarchy1 = cv2.findContours(thresh_1,2,1)
        Con = contours1[0]

        ret = cv2.matchShapes(Con,contours[i],1,0)
        if ret < 0.2:
            cv2.drawContours(img,[contours[i]],0,[255,0,0],3)
        cv2.imshow('camera', img)
        cv2.imshow('camera_1', thresh_1)
        cv2.waitKey(1)

if __name__ == "__main__":
    fun = Function()
    while True:
        id,frame = cap.read()
        if model == 13:
            image = cv2.GaussianBlur(frame, (5, 5), 0)  
            imgHSV = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            fun.find_color(imgHSV)
        else:
            frame = cv2.cvtColor(frame,cv2.COLOR_RGB2GRAY)
            fun.Template(frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()

