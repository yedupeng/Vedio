import cv2
from loguru import logger

for i in range(4):
    for j in range(10):
        cap = cv2.VideoCapture(i)
        ret, frame = cap.read()
        if ret:
            logger.debug('camera:{} is OK! shape is {}'.format(i, frame.shape))
            cv2.imshow(str(i), frame)
            cv2.waitKey(2000)
            cap.release()
            break
        logger.warning('cant open camera:{} !'.format(i))
        break
