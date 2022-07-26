from communite_module.Communications import SelfSerial
from loguru import logger


def main_serial(device, pipeline):
    self_serial = SelfSerial(device, pipeline)
    while True:
        msg = self_serial.pipe_read_msg()
        self_serial.uart_send_msg(msg)
        logger.info('Uart Send:{}'.format(msg))



