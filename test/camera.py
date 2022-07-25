import cv2
from loguru import logger
import loguru

for i in range(4):
    for i in range(10):
        cap = cv2.VideoCapture(i)
        ret, frame = cap.read()
        if ret:
            logger.info('camera:{} is OK! shape is {}'.format(i, frame.shape))
            break
        logger.warning('cant open camera:{} !'.format(i))
