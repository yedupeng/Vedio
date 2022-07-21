#include "Basic.h"
#include "AC_Math.h"
#include "drv_OpticalFlow.h"
#include "Sensors_Backend.h"
#include "MeasurementSystem.h"

#include "TM4C123GH6PM.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "hw_types.h"
#include "hw_ints.h"
#include "debug.h"
#include "fpu.h"
#include "gpio.h"
#include "pin_map.h"
#include "sysctl.h"
#include "uart.h"
#include "interrupt.h"
#include "timer.h"
#include "hw_gpio.h"

static void OpticalFlow_Handler();

void init_drv_OpticalFlow()
{
	//使能Uart1引脚（Rx:PB0 Tx:PB1）
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	//使能UART1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	
	//配置GPIO
	GPIOPinConfigure(GPIO_PB0_U1RX);
	GPIOPinConfigure(GPIO_PB1_U1TX);
	GPIOPinTypeUART(GPIOB_BASE, GPIO_PIN_0 | GPIO_PIN_1);//GPIO的UART模式配置
	
	//配置Uart
	//UART协议配置 波特率19200 8位 1停止位  无校验位	
	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet() , 460800,			
	                   (UART_CONFIG_WLEN_8 
										| UART_CONFIG_STOP_ONE 
										|	UART_CONFIG_PAR_NONE));	
	
	UARTFIFOEnable( UART1_BASE );
	UARTIntEnable(UART1_BASE,UART_INT_RX|UART_INT_RT);//使能UART0接收中断		
	UARTIntRegister(UART1_BASE,OpticalFlow_Handler);//UART中断地址注册	
	IntPrioritySet(INT_UART1, INT_PRIO_7);
	
	//注册传感器
	PositionSensorRegister( default_optical_flow_index , \
													Position_Sensor_Type_RelativePositioning , \
													Position_Sensor_DataType_s_xy , \
													Position_Sensor_frame_ENU , \
													0.1f , \
													false );
}

typedef struct
{
	uint8_t id;
	uint8_t role;
	int pos_x:24;	int pos_y:24;	int pos_z:24;		
	int vel_x:24;	int vel_y:24;	int vel_z:24;	
	int dis_0:24;	int dis_1:24;	int dis_2:24;	int dis_3:24;	int dis_4:24;	int dis_5:24;	int dis_6:24;	int dis_7:24;
	float imuGyro[3];
	float imuAcc[3];
	uint8_t reserved1[12];
	int16_t angle[3];
	float q[4];
	uint8_t reserved2[4];
	uint32_t localTime;
	uint32_t systemTime;
	uint8_t reserved3[1];
	uint8_t eop[3];
	uint16_t voltage;
	uint8_t reserved4[5];
}__PACKED _Uwb;
static const unsigned char packet_ID[2] = { 0x55 , 0x01 };

/*状态机*/
	static _Uwb  Uwb;
	static unsigned char rc_counter = 0;
	static signed char sum = 0;
/*状态机*/

static void OpticalFlow_Handler()
{
	static bool initiliazed = false;
	static float sin_Yaw, cos_Yaw;
	if( initiliazed==false )
	{
		if( get_Altitude_Measurement_System_Status()==Measurement_System_Status_Ready )
		{
			float iniYaw = Quaternion_getYaw(get_attitude());
			arm_sin_cos_f32( rad2degree(iniYaw), &sin_Yaw, &cos_Yaw );
			initiliazed = true;
		}
		else
			return;
	}
	
	uint32_t uart_err = UARTRxErrorGet( UART1_BASE );
	UARTIntClear( UART1_BASE , UART_INT_OE | UART_INT_RT );
	UARTRxErrorClear( UART1_BASE );
	
	//状态机接收数据
	while( ( UART1->FR & (1<<4) ) == false	)
	{
		//接收
		uint8_t rdata = UART1->DR & 0xff;
		
		if( rc_counter < sizeof(packet_ID) )
		{
			//接收包头
			if( rdata != packet_ID[ rc_counter ] )
			{
				rc_counter = 0;
				sum = 0;
			}
			else
			{
				++rc_counter;
				sum += rdata;
			}
		}
		else if( rc_counter < sizeof(packet_ID) + sizeof(_Uwb) )
		{	//接收数据
			( (unsigned char*)&Uwb )[ rc_counter - sizeof(packet_ID) ] = rdata;
			sum += rdata;
			++rc_counter;
		}
		else
		{	//校验
			if( sum == rdata )
			{
				if( Uwb.eop[0]>200 || Uwb.eop[1]>200 )
					PositionSensorSetInavailable(default_optical_flow_index);
				else 
				{					
					vector3_float pos, vel;
					pos.x = map_ENU2BodyHeading_x( Uwb.pos_x*0.1 , Uwb.pos_y*0.1 , sin_Yaw , cos_Yaw );
					pos.y = map_ENU2BodyHeading_y( Uwb.pos_x*0.1 , Uwb.pos_y*0.1 , sin_Yaw , cos_Yaw );
					pos.z = Uwb.pos_z * 0.1;
					vel.x = map_ENU2BodyHeading_x( Uwb.vel_x*0.1 , Uwb.vel_y*0.1 , sin_Yaw , cos_Yaw );
					vel.y = map_ENU2BodyHeading_y( Uwb.vel_x*0.1 , Uwb.vel_y*0.1 , sin_Yaw , cos_Yaw );
					vel.z = Uwb.vel_z * 0.01;
//						if( Uwb.eop[2] > 200 )
//							PositionSensorChangeDataType( default_uwb_sensor_index, Position_Sensor_DataType_s_xy );
//						else
						PositionSensorChangeDataType( default_optical_flow_index, Position_Sensor_DataType_s_xyz );
					double eop_xy = sqrtf( Uwb.eop[0]*Uwb.eop[0] + Uwb.eop[1]*Uwb.eop[1] );
					PositionSensorUpdatePosition( default_optical_flow_index, pos, true, 0.05f );
				}
			}
			rc_counter = 0;
			sum = 0;
		}
	}
}