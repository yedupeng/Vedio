from communite_module.communicate import Receive
from loguru import logger

def main_serial(pipeline):
    pipe = pipeline
    receive = Receive('/dev/ttyUSB0')
    while True:
        data = pipe.recv()
        receive.uart_send(data)
        logger.info('send success   data:{}'.format(data))
        


