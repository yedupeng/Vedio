from itertools import count
import cv2
import numpy as np
import pytesseract

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
        self.template = cv2.imread("E:\\Fly_nobody\\Node\\template\\2.jpg")
        Methods = [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED, cv2.TM_CCORR, cv2.TM_CCORR_NORMED, cv2.TM_CCOEFF, cv2.TM_CCOEFF_NORMED]
        self.method = Methods[5]
        self.location = [0, 0]

        # 找到颜色
        """
        注意:drawContours是画白色区域的轮廓
        """
    def find_color(self,image):
        #  定义高低阈值范围
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
        except:
            pass
        cv2.imshow('camera', image)
        cv2.waitKey(1)

    def Template(self,img):
        # 对摄像头图像进行处理
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
        pytesseract.pytesseract.tesseract_cmd = "C:\Program Files\Tesseract-OCR\\tesseract.exe"
        H,W = img.shape[:2]
        boxes = pytesseract.image_to_boxes(img)
        for box in boxes.splitlines():
             if box[0] == 'A':
                b = box.split(' ')
                x, y, w, h = int(b[1]), int(b[2]), int(b[3]), int(b[4])
                cv2.rectangle(img, (x, H-y), (w, H-h), (0, 0, 255), 2)
        cv2.imshow("img",img)
        cv2.waitKey(1)


if __name__ == "__main__":
    fun = Function()
    while True:
        id,frame = cap.read()
        if model == 1:
            image = cv2.GaussianBlur(frame, (5, 5), 0)  
            imgHSV = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            fun.find_color(imgHSV)
        elif model == 2:
            fun.Template(frame)
        else:
            fun.recognice_text(frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()

