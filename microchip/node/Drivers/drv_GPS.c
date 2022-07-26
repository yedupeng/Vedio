#include "Basic.h"
#include "drv_GPS.h"

#include "STS.h"
#include "Sensors_Backend.h"

#include "TM4C123GH6PM.h"
#include "uart.h"
#include "sysctl.h"
#include "gpio.h"
#include "pin_map.h"
#include "interrupt.h"
#include "hw_ints.h"
#include "hw_gpio.h"
#include "Timer.h"
#include "udma.h"

static uint16_t ubloxInit_SendCounter = 0;
static const unsigned char ubloxInit[] = {
		0xb5, 0x62, 0x06, 0x24, 0x24, 0x00,  0x05, 0x00,  0x07, 0x03,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0x5d, 0xc9,	//NAV5
		0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12,             // set rate to 10Hz
		
		0xB5,0x62,0x06,0x01,0xF0,0x0A,0x00,0x04,0x23,
		0xB5,0x62,0x06,0x01,0xF0,0x09,0x00,0x03,0x21,
		0xB5,0x62,0x06,0x01,0xF0,0x00,0x00,0xFA,0x0F,
		0xB5,0x62,0x06,0x01,0xF0,0x01,0x00,0xFB,0x11,
		0xB5,0x62,0x06,0x01,0xF0,0x0D,0x00,0x07,0x29,
		0xB5,0x62,0x06,0x01,0xF0,0x06,0x00,0x00,0x1B,
		0xB5,0x62,0x06,0x01,0xF0,0x02,0x00,0xFC,0x13,
		0xB5,0x62,0x06,0x01,0xF0,0x07,0x00,0x01,0x1D,
		0xB5,0x62,0x06,0x01,0xF0,0x03,0x00,0xFD,0x15,
		0xB5,0x62,0x06,0x01,0xF0,0x04,0x00,0xFE,0x17,
		0xB5,0x62,0x06,0x01,0xF0,0x0E,0x00,0x08,0x2B,
		0xB5,0x62,0x06,0x01,0xF0,0x0F,0x00,0x09,0x2D,
		0xB5,0x62,0x06,0x01,0xF0,0x05,0x00,0xFF,0x19,
		0xB5,0x62,0x06,0x01,0xF0,0x08,0x00,0x02,0x1F,//7
		0xB5,0x62,0x06,0x01,0xF1,0x00,0x00,0xFB,0x12,
		0xB5,0x62,0x06,0x01,0xF1,0x01,0x00,0xFC,0x14,
		0xB5,0x62,0x06,0x01,0xF1,0x03,0x00,0xFE,0x18,
		0xB5,0x62,0x06,0x01,0xF1,0x04,0x00,0xFF,0x1A,
		0xB5,0x62,0x06,0x01,0xF1,0x05,0x00,0x00,0x1C,
		0xB5,0x62,0x06,0x01,0xF1,0x06,0x00,0x01,0x1E,
		
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0B ,0x30 ,0x00 ,0x45 ,0xC0,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0B ,0x50 ,0x00 ,0x65 ,0x00,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0B ,0x33 ,0x00 ,0x48 ,0xC6,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0B ,0x31 ,0x00 ,0x46 ,0xC2,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0B ,0x00 ,0x00 ,0x15 ,0x60,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x10 ,0x02 ,0x00 ,0x1C ,0x73,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x10 ,0x02 ,0x00 ,0x1C ,0x73,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x10 ,0x02 ,0x00 ,0x1C ,0x73,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x10 ,0x10 ,0x00 ,0x2A ,0x8F,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x21 ,0x0E ,0x00 ,0x39 ,0xBE,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x21 ,0x08 ,0x00 ,0x33 ,0xB2,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x21 ,0x09 ,0x00 ,0x34 ,0xB4,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x21 ,0x0B ,0x00 ,0x36 ,0xB8,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x21 ,0x0F ,0x00 ,0x3A ,0xC0,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x21 ,0x0D ,0x00 ,0x38 ,0xBC,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x05 ,0x00 ,0x19 ,0x67,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x09 ,0x00 ,0x1D ,0x6F,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x0B ,0x00 ,0x1F ,0x73,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x02 ,0x00 ,0x16 ,0x61,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x06 ,0x00 ,0x1A ,0x69,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x07 ,0x00 ,0x1B ,0x6B,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x21 ,0x00 ,0x35 ,0x9F,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x2E ,0x00 ,0x42 ,0xB9,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0A ,0x08 ,0x00 ,0x1C ,0x6D,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x60 ,0x00 ,0x6B ,0x02,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x22 ,0x00 ,0x2D ,0x86,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x31 ,0x00 ,0x3C ,0xA4,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x04 ,0x00 ,0x0F ,0x4A,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x40 ,0x00 ,0x4B ,0xC2,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x61 ,0x00 ,0x6C ,0x04,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x09 ,0x00 ,0x14 ,0x54,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x34 ,0x00 ,0x3F ,0xAA,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x01 ,0x00 ,0x0C ,0x44,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x02 ,0x00 ,0x0D ,0x46,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x07 ,0x00 ,0x12 ,0x50,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x35 ,0x00 ,0x40 ,0xAC,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x32 ,0x00 ,0x3D ,0xA6,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x06 ,0x00 ,0x11 ,0x4E,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x03 ,0x00 ,0x0E ,0x48,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x30 ,0x00 ,0x3B ,0xA2,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x24 ,0x00 ,0x2F ,0x8A,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x23 ,0x00 ,0x2E ,0x88,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x20 ,0x00 ,0x2B ,0x82,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x21 ,0x00 ,0x2C ,0x84,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x11 ,0x00 ,0x1C ,0x64,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x01 ,0x12 ,0x00 ,0x1D ,0x66,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x02 ,0x30 ,0x00 ,0x3C ,0xA5,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x02 ,0x31 ,0x00 ,0x3D ,0xA7,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x02 ,0x10 ,0x00 ,0x1C ,0x65,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x02 ,0x15 ,0x00 ,0x21 ,0x6F,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x02 ,0x11 ,0x00 ,0x1D ,0x67,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x02 ,0x13 ,0x00 ,0x1F ,0x6B,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x02 ,0x20 ,0x00 ,0x2C ,0x85,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x11 ,0x00 ,0x28 ,0x88,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x16 ,0x00 ,0x2D ,0x92,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x13 ,0x00 ,0x2A ,0x8C,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x04 ,0x00 ,0x1B ,0x6E,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x03 ,0x00 ,0x1A ,0x6C,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x12 ,0x00 ,0x29 ,0x8A,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x01 ,0x00 ,0x18 ,0x68,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x0D ,0x06 ,0x00 ,0x1D ,0x72,
		0xB5,0x62,0x06,0x01,0x03,0x00, 0x27, 0x01, 0x00 ,0x32 ,0xb6,	
		
		0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x03, 0x01, 0x0F, 0x49,           // set STATUS MSG rate		
		0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x07, 0x01, 0x13, 0x51,           // set PVT MSG rate
		
		//set baudrate to 115200
		0xB5 , 0x62 , 0x06 , 0x00 , 0x14 , 0x00 , 0x01 , 0x00 , 0x00 , 0x00 , 0xD0 , 0x08 , 0x00 , 0x00 , 0x00 , 0xC2 , 0x01 , 0x00 , 0x07 , 0x00 , 0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xBE , 0x72 ,	
		
};

static void GPS_Handler();
static void GPS_Server();
static inline void UbloxInit_StartSend()
{
	while( ( UART0->FR & (1<<5) ) == 0 )
		UART0->DR = ubloxInit[ ubloxInit_SendCounter++ ];
}

void init_drv_GPS()
{
	//ʹ��Uart0���ţ�Rx:PA0 Tx:PA1��
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	//ʹ��UART0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	
	//����GPIO
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIOA_BASE, GPIO_PIN_0 | GPIO_PIN_1);//GPIO��UARTģʽ����
		
	//����Uart
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet() , 115200,			
	                   (UART_CONFIG_WLEN_8 
										| UART_CONFIG_STOP_ONE 
										|	UART_CONFIG_PAR_NONE));	
	
//	while(1)
//	{
//		for( uint16_t i = 0 ; i < sizeof( ubloxInit ) ; ++i )
//		{
//			while( (UART0->FR & (1<<5)) != 0 );
//			UART0->DR = ubloxInit[i];
//		}
//		UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet() , 57600,			
//	                   (UART_CONFIG_WLEN_8 
//										| UART_CONFIG_STOP_ONE 
//										|	UART_CONFIG_PAR_NONE));	
//		delay(1.0f);
//		
//	}
	UARTFIFOEnable( UART0_BASE );
	UARTIntEnable(UART0_BASE,UART_INT_RX | UART_INT_RT | UART_INT_TX);//ʹ��UART0���ͽ����ж�		
	UARTIntRegister(UART0_BASE,GPS_Handler);//UART�жϵ�ַע��	
	IntPrioritySet(INT_UART0, INT_PRIO_7);
	
	STS_Add_Task( STS_Task_Trigger_Mode_RoughTime , 1.0f / 10 , 0 , GPS_Server );
}

typedef struct
{
	uint8_t CLASS;
	uint8_t ID;
	uint16_t length;
	
	uint32_t iTOW;
	uint16_t y;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t valid;
	uint32_t t_Acc;
	int32_t nano;
	uint8_t fix_type;
	uint8_t flags;
	uint8_t flags2;
	uint8_t numSV;
	int32_t lon;
	int32_t lat;
	int32_t height;
	int32_t hMSL;
	uint32_t hAcc;
	uint32_t vAcc;
	int32_t velN;
	int32_t velE;
	int32_t velD;
	int32_t gSpeed;
	int32_t headMot;
	uint32_t sAcc;
	uint32_t headAcc;
	uint16_t pDOP;
	uint8_t res1[6];
	int32_t headVeh;
	uint8_t res2[4];
}__attribute__((packed)) UBX_NAV_PVT_Pack;
typedef enum
{
	//��ָ���������·��ͳ�ʼ����Ϣ
	GPS_Scan_Baud9600 = 9600 ,
	GPS_Scan_Baud38400 = 38400 ,
	GPS_Scan_Baud115200 = 115200 ,
	
	//����Ƿ����óɹ�
	GPS_Check_Baud ,
	//�ڵ�ǰ���������ٴη�������
	GPS_ResendConfig ,
}GPS_Scan_Operation;
static GPS_Scan_Operation current_scan_operation = GPS_Scan_Baud115200;
static TIME GPS_stable_start_time = {0};
static bool GPS_Check_Success = false;
static void GPS_Server()
{
	static unsigned short operation_counter = 0;
	static GPS_Scan_Operation last_Scan_Operation = GPS_Scan_Baud115200;
		
	if( GetPositionSensor(default_gps_sensor_index)->present == false )
	{
		
		switch( current_scan_operation )
		{
			
			case GPS_Scan_Baud9600:
			case GPS_Scan_Baud38400:
			case GPS_Scan_Baud115200:
			{
				if( operation_counter == 0 )
				{
					//����Uart
					UARTConfigSetExpClk( UART0_BASE , SysCtlClockGet() , (unsigned int)current_scan_operation,			
	                   (UART_CONFIG_WLEN_8 
										| UART_CONFIG_STOP_ONE 
										|	UART_CONFIG_PAR_NONE));	
				}					
				else if( operation_counter == 1 )
				{
					//���ͳ�ʼ����
					UbloxInit_StartSend();
				}
				else if( operation_counter < ceilf( 144000.0f / (float)current_scan_operation ) )
				{
				}
				else
				{
					operation_counter = 0;
					last_Scan_Operation = current_scan_operation;
					current_scan_operation = GPS_Check_Baud;
					
					//����Uart
					UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet() , 115200,			
	                   (UART_CONFIG_WLEN_8 
										| UART_CONFIG_STOP_ONE 
										|	UART_CONFIG_PAR_NONE));
					GPS_Check_Success = false;
					
					break;
				}
					
				++operation_counter;
				break;
			}
			
			case GPS_Check_Baud:
			{
				if( operation_counter < 10 )
				{
					if( GPS_Check_Success )
					{
						current_scan_operation = GPS_ResendConfig;
						operation_counter = 0;						
						break;
					}
				}
				else
				{
					switch( last_Scan_Operation )
					{
						case GPS_Scan_Baud9600:
							current_scan_operation = GPS_Scan_Baud38400;
							break;
						case GPS_Scan_Baud38400:
							current_scan_operation = GPS_Scan_Baud115200;
							break;
						case GPS_Scan_Baud115200:
							current_scan_operation = GPS_Scan_Baud9600;
							break;
						
						default:
							break;
					}
					operation_counter = 0;
					break;
				}
				
				++operation_counter;
				break;
			}
			
			case GPS_ResendConfig:
			{
				if( operation_counter == 0 )
				{
					//����Uart
					UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet() , 115200,			
	                   (UART_CONFIG_WLEN_8 
										| UART_CONFIG_STOP_ONE 
										|	UART_CONFIG_PAR_NONE));	
				}					
				else if( operation_counter == 1 )
				{
					//�ٴη��ͳ�ʼ����
					UbloxInit_StartSend();
				}
				else if( operation_counter < 3 )
				{
				}
				else
				{
					//ע�ᴫ����
					PositionSensorRegister( default_gps_sensor_index , \
																	Position_Sensor_Type_GlobalPositioning , \
																	Position_Sensor_DataType_sv_xy , \
																	Position_Sensor_frame_ENU , \
																	0.1f , \
																	true );
					Time_set_inValid( &GPS_stable_start_time );
					operation_counter = 0;
					current_scan_operation = GPS_Scan_Baud9600;
				}
				++operation_counter;
				
				break;
			}
			
		}
	}
	else
	{
		//GPS present
		const Position_Sensor* gps = GetPositionSensor( default_gps_sensor_index );
		if( get_pass_time( gps->last_update_time ) > 2.0f )
		{
			//GPS disconnected
			PositionSensorUnRegister( default_gps_sensor_index );
			Time_set_inValid( &GPS_stable_start_time );
			
			current_scan_operation = GPS_Scan_Baud115200;
			operation_counter = 0;
			last_Scan_Operation = GPS_Scan_Baud115200;
		}
	}
}

static void GPS_Handler()
{
	UARTIntClear( UART0_BASE , UART_INT_OE | UART_INT_RT );
	UARTRxErrorClear( UART0_BASE );
	UARTIntClear( UART0_BASE , UART_INT_TX );
	
	//����UbloxInit
	if( ubloxInit_SendCounter > 0 )
	{
		if( ubloxInit_SendCounter >= sizeof( ubloxInit ) )
			ubloxInit_SendCounter = 0;
		else
		{		
			while( ( UART0->FR & (1<<5) ) == 0 )
				UART0->DR = ubloxInit[ ubloxInit_SendCounter++ ];
		}
	}
	
	//״̬����������
	while( ( UART0->FR & (1<<4) ) == false	)
	{
		//����
		uint8_t r_data = UART0->DR & 0xff;
		
		if( GetPositionSensor( default_gps_sensor_index )->present == true || current_scan_operation == GPS_Check_Baud )
		{
			__attribute__((aligned(4))) static unsigned char frame_datas[150];
			static unsigned int frame_datas_ind = 0;
			static unsigned short frame_datas_length;
			static unsigned char read_state = 0;	//0=

			static unsigned char CK_A , CK_B;	//checksum
			
			frame_datas[ frame_datas_ind++ ] = r_data;
			switch( read_state )
			{
				case 0:	//�Ұ�ͷ
				{				
					if( frame_datas_ind == 1 )
					{
						if( r_data != 0xb5 )
							frame_datas_ind = 0;
					}
					else
					{
						if( r_data == 0x62 )	//header found
						{
							read_state = 1;
							frame_datas_ind = 0;
							CK_A = CK_B = 0;	//reset checksum
						}		
						else
							frame_datas_ind = 0;
					}	
					break;
				}
				case 1:	//��Class ID�Ͱ�����
				{
					CK_A += r_data;
					CK_B += CK_A;
					if( frame_datas_ind == 4 )
					{
						frame_datas_length = (*(unsigned short*)&frame_datas[2]) + 4;
						if( frame_datas_length > 4 && frame_datas_length < 150 )
							read_state = 2;						
						else
							read_state = frame_datas_ind = 0;
					}
					break;
				}
				case 2:	//��������
				{
					CK_A += r_data;
					CK_B += CK_A;
					
					if( frame_datas_ind == frame_datas_length )
					{
						//payload read completed
						read_state = 3;
					}
					break;
				}
				case 3://У��
				{
					if( frame_datas_ind == frame_datas_length + 1 )
					{
						if( r_data != CK_A )
							read_state = frame_datas_ind = 0;
					}
					else
					{
						if( r_data == CK_B )
						{
							//У��ɹ�
							const Position_Sensor* gps = GetPositionSensor( default_gps_sensor_index);
							
							if( gps->present == true )
							{
								if( frame_datas[0] == 0x01 && frame_datas[1] == 0x07 )	//Navigation Position Velocity Time frame
								{									
									UBX_NAV_PVT_Pack* pack = (UBX_NAV_PVT_Pack*)frame_datas;
				
									
									bool available = gps->available;
									bool z_available = (gps->sensor_DataType == Position_Sensor_DataType_sv_xyz);
									if( ((pack->flags & 1) == 1) && (pack->fix_type == 0x03) && (pack->numSV >= 5) )
									{
										if( gps->available == false )
										{
											if( pack->hAcc < 2500 )
											{
												if( Time_isValid( GPS_stable_start_time ) == false )
													GPS_stable_start_time = get_TIME_now();
												else if( get_pass_time( GPS_stable_start_time ) > 3.0f )
												{
													available = true;
													Time_set_inValid( &GPS_stable_start_time );
												}
											}
											else
												Time_set_inValid( &GPS_stable_start_time );
										}
										else
										{
											if( pack->hAcc > 3500 )
												available = z_available = false;
										}
										
									}
									else
									{
										available = z_available = false;
										Time_set_inValid( &GPS_stable_start_time );
									}
									
									if( available )
									{
										if( gps->sensor_DataType == Position_Sensor_DataType_sv_xyz )
										{
											if( pack->vAcc > 5500 )
												z_available = false;
										}
										else
										{
											if( pack->vAcc < 3800 )
												z_available = true;
										}
										
										vector3_float velocity;
										velocity.y = pack->velN * 0.1f;	//North
										velocity.x = pack->velE * 0.1f;	//East
										velocity.z = -pack->velD * 0.1f;	//Up
										
										vector3_double position_Global;
										position_Global.x = pack->lat * 1e-7;
										position_Global.y = pack->lon * 1e-7;
										position_Global.z = pack->hMSL * 1e-1;
//										if( gps->available == false && available == true )
//											position_Global.z = pack->hMSL * 1e-1;
//										else
//											position_Global.z = gps->position.z + get_pass_time( gps->last_update_time )*velocity.z;
										
										if( z_available )
											PositionSensorChangeDataType( default_gps_sensor_index , Position_Sensor_DataType_sv_xyz );
										else
											PositionSensorChangeDataType( default_gps_sensor_index , Position_Sensor_DataType_sv_xy );	
										PositionSensorUpdatePositionGlobalVel( default_gps_sensor_index , position_Global , velocity , true , 0.15f );
																	
									}
									else
										PositionSensorSetInavailable( default_gps_sensor_index );
									
								}	//PVT pack								
								
							}//GPS present
							else if( current_scan_operation == GPS_Check_Baud )
							{
								if( frame_datas[0] == 0x01 && frame_datas[1] == 0x07 )	//Navigation Position Velocity Time frame
									GPS_Check_Success = true;
							}							
						}
						read_state = frame_datas_ind = 0;					
					}
				}
			}
		}
	}
}