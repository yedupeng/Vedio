from audioop import reverse
from calendar import day_abbr
from shelve import Shelf
from loguru import logger
import serial

# 定义串口3
Uart2 = serial.Serial(  port="/dev/ttyAMA1",
                        bytesize=8,
                        baudrate=115200,
                        stopbits=1,
                        timeout=1000)

# 接收类
class Receive():
    def __init__(self) -> None:
        self.model = 0
        self.uart_buf = []
        self.state = 0


    def uart_read(self):
        if(Uart2.in_waiting>0):
            data = Uart2.read().hex()
            model = self.data_processing(data)
            return model


    # 数据处理
    def data_processing(self, data) -> int:
        if(self.state == 0):
            if(data == "0f"):
                self.state = 1
                self.uart_buf.append(data)
            else:
                self.state = 0

        elif(self.state == 1):
            if(data == "f0"):
                self.state = 2
                self.uart_buf.append(data)
            else:
                self.state = 0

        elif(self.state == 2):
            if(data == "20"):
                self.state = 3
                self.uart_buf.append(data)
            else:
                self.state = 0

        elif(self.state == 3):
            if(data == "02"):
                self.state = 4
                self.uart_buf.append(data)
            else:
                self.state = 0

        elif(self.state == 4):
            self.state = 5
            self.uart_buf.append(data)

        elif(self.state == 5):
            sum = 0
            for i in range(5):
                sum = sum + int(self.uart_buf[i],16)
            sum = sum % 256
            # print(sum)
            # print(int(data,16))
            data_16 = int(data, 16)
            if(data_16 == sum):
                # self.uart_buf.append(data)
                self.model = self.uart_buf[4]
                self.uart_buf = []
                self.state = 0
                # logger.info("Connect success!")
                return self.model
                
            else:
                self.state = 0


    # 发送串口数据
    def uart_send(self, data1, data2, data3, k, index):
        if index == 17:
            Uart2.write(self.pack_data_17(data1,data2,data3))

    # 测试：定义功能包17
    def pack_data_17(self, data1, data2, data3):
        datalist = [0x0f, 0xf0, 0x17, 0x03, data1, data2, data3]
        datalist.append(self.sum_check(datalist))
        data = bytearray(datalist)
        return data

    # 求和取余得发送包尾
    def sum_check(self, data_list):
        data_sum = 0
        for temp in data_list:
            data_sum = temp+data_sum
        return data_sum%256


if __name__ == "__main__":
    receive = Receive()
    while(1):
        model = receive.uart_read()
        while(model == "17"):
            logger.info("success to model 17")
            receive.uart_send(8,2,3,0,17)
