from loguru import logger
import serial


# 接收类
class SelfSerial():
    def __init__(self, device):
        self.uart = serial.Serial(port=device, bytesize=8, baudrate=115200, stopbits=1, timeout=0)
        self.msg_mode = 0
        self.uart_buf = []
        self.state = 0

    #多进程_检测线程接口_获取模式
    def uart_read_mode(self, lastmode):
        if self.uart.in_waiting > 0:
            data = self.uart.read().hex()
            mode = self.data_processing(data)
            if mode:
                if lastmode != mode:
                    logger.info('Get Command. Run Model:{}'.format(mode))
                return mode
            else:
                return lastmode
        else:
            return lastmode

    #多进程 串口线程接口 往串口发送数据
    def uart_send_msg(self, msg):
        #17:色块 18:字符 19:圆 20:巡线
        mode = msg[0]
        # if mode == 17: 
        #     # 
        #     self.uart.write(self.pack_data_17(msg[1], msg[2], msg[3]))

        # elif mode == 18:
        #     #
        #     self.uart.write(self.pack_data_18(msg[1], msg[2], msg[3]))

        # elif mode == 19:
        #     #
        #     self.uart.write(self.pack_data_19(msg[1], msg[2], msg[3]))

        # elif mode == 20:
        #     #
        #     self.uart.write(self.pack_data_20(msg[1], msg[2], msg[3]))

        if mode == 21:
            self.uart.write(self.pack_data_20())
        


    #串口读取数据处理函数
    def data_processing(self, data):
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
            data_16 = int(data, 16)
            if(data_16 == sum):
                self.model = self.uart_buf[4]
                self.uart_buf = []
                self.state = 0
                return self.model
            else:
                self.state = 0

    #串口读取数据求和校验函数
    def sum_check(self, data_list):
        data_sum = 0
        for temp in data_list:
            data_sum = temp+data_sum
        return data_sum % 256

    #定义发送数据包 返回list
    def pack_data_17(self, data1, data2, data3):
        datalist = [0x0f, 0xf0, 0x17, 0x03, data1, data2, data3]
        datalist.append(self.sum_check(datalist))
        data = bytearray(datalist)
        return data
    
    def pack_data_18(self, data1, data2, data3):
        datalist = [0x0f, 0xf0, 0x18, 0x03, data1, data2, data3]
        datalist.append(self.sum_check(datalist))
        data = bytearray(datalist)
        return data

    def pack_data_19(self, data1, data2, data3):
        datalist = [0x0f, 0xf0, 0x19, 0x03, data1, data2, data3]
        datalist.append(self.sum_check(datalist))
        data = bytearray(datalist)
        return data

    def pack_data_20(self, data1, data2, data3):
        datalist = [0x0f, 0xf0, 0x20, 0x03, data1, data2, data3]
        datalist.append(self.sum_check(datalist))
        data = bytearray(datalist)
        return data

    def pack_data_21(self, ):
        pass

