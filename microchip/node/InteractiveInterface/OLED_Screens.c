#include "OLED_Screens.h"
#include "InteractiveInterface.h"
#include <stdlib.h>
#include <stdio.h>

#include "MeasurementSystem.h"
#include "Receiver.h"
#include "Sensors.h"

/*基本信息页面：
		传感器状态
		接收机状态
		定位状态
		姿态角、垂直速度		
*/
extern uint16_t get_position_z();
extern uint16_t get_SDI_tcmode();
extern uint16_t get_point_find();
extern uint16_t get_change_find();
extern int16_t get_line_dx();
extern uint16_t test1();
extern uint16_t test2();
extern uint16_t test3();
	void OLED_Screen_BasicInf_init()
	{
		OLED_Clear_Lines(1,7);
		OLED_Draw_Str8x6( "Sensors:01234567" , 2 , 0 );	OLED_Draw_Str8x6( "Rc:" , 2 , 128-4*6 );	OLED_Draw_TickCross8x6( false , 2 , 128-1*6);
		OLED_Draw_Str8x6( "Pos:" , 3 , 4*6 );	
		OLED_Draw_Str8x6( "Mag:" , 4 , 4*6 );	OLED_Draw_Str8x6( "Bat:0.0" , 4 , 128-8*6 );
		OLED_Draw_Str8x6( "Measurements:" , 5 , 0 );	OLED_Draw_Str8x6( "FP:" , 5 , 128-4*6 );	OLED_Draw_TickCross8x6( false , 5 , 128-1*6);
		OLED_Draw_Str8x6( "Rol:0.0   Pit:0.0" , 6 , 12 );
		OLED_Draw_Str8x6( "Yaw:0.0   VSp:0.0" , 7 , 12 );
		OLED_Screen_BasicInf_Refresh();
	}
	void OLED_Screen_M35_init() //35模式屏幕初始化
	{
		//OLED_Draw_Str8x6( "high:0.0         " , 1 , 12 );
		OLED_Draw_Str8x6( "SDImode:0.0      " , 1 , 12 );
		OLED_Draw_Str8x6( "Ax:0.0     Ay:0,0" , 2 , 12 );
		OLED_Draw_Str8x6( "b0:0.0           " , 3 , 12 );
		OLED_Draw_Str8x6( "b1:0.0           " , 4 , 12 );
		OLED_Draw_Str8x6( "b2:0.0           " , 5 , 12 );
//		OLED_Draw_Str8x6( "findpole:0.0     " , 5 , 12 );
//		OLED_Draw_Str8x6( "findpoint:0.0    " , 6 , 12 );
//		OLED_Draw_Str8x6( "Bx:0.0     By:0.0" , 7 , 12 );
		OLED_Screen_M35_Refresh();
	}
		void OLED_Screen_M35_Refresh()
	{
		char M35_str[8];
		//基本信息
//		sprintf( M35_str , "%5d" , get_position_z() );
//		OLED_Draw_Str8x6( M35_str , 1 , 12+4*6);
		sprintf( M35_str , "%5d" , get_SDI_tcmode() );
		OLED_Draw_Str8x6( M35_str , 1 , 12+4*6);
		sprintf( M35_str , "%5d" , get_SDI_tcmode() );
		OLED_Draw_Str8x6( M35_str , 2 , 12+4*6);
		sprintf( M35_str , "%5d" , test1() );
		OLED_Draw_Str8x6( M35_str , 2 , 12+14*6);
		sprintf( M35_str , "%5d" , test2() );
		OLED_Draw_Str8x6( M35_str , 3 , 12+4*6);
		sprintf( M35_str , "%5d" , test3() );
		OLED_Draw_Str8x6( M35_str , 4 , 12+4*6);
		sprintf( M35_str , "%5d" , get_SDI_tcmode() );
		OLED_Draw_Str8x6( M35_str , 5 , 12+4*6);
		//sprintf( M35_str , "%5d" , get_line_dy() );
		//OLED_Draw_Str8x6( M35_str , 5 , 12+14*6);
		//sprintf( M35_str , "%5d" , get_A_dx() );
		//OLED_Draw_Str8x6( M35_str , 5 , 12+4*6);
		//sprintf( M35_str , "%5d" , get_A_dx() );
		//OLED_Draw_Str8x6( M35_str , 6 , 12+4*6);
		//sprintf( M35_str , "%5d" , get_A_dx() );
		//OLED_Draw_Str8x6( M35_str , 7 , 12+4*6);
		//sprintf( M35_str , "%5d" , get_A_dx() );
		//OLED_Draw_Str8x6( M35_str , 7 , 12+14*6);
	}
	void OLED_Screen_BasicInf_Refresh()
	{
		char num_str[8];
		
		//RC状态
		const Receiver* rc = get_current_Receiver();
		if( rc->present == false )
			OLED_Draw_TickCross8x6( false , 2 , 128-1*6 );
		else if( rc->connected == false )
			OLED_Draw_Point8x6( 2 , 128-1*6 );
		else
			OLED_Draw_TickCross8x6( true , 2 , 128-1*6 );
	
		//位置传感器状态
		for( unsigned char i = 0 ; i < Position_Sensors_Count ; ++i )
		{
			const Position_Sensor* sensor = GetPositionSensor(i);
			if( sensor->present == false )
				OLED_Draw_TickCross8x6( false , 3 , 8*6 + i*6 );
			else if( sensor->available == true )
				OLED_Draw_TickCross8x6( true , 3 , 8*6 + i*6 );
			else
				OLED_Draw_Point8x6( 3 , 8*6 + i*6 );
		}
		
		//磁力状态
		for( unsigned char i = 0 ; i < IMU_Sensors_Count ; ++i )
		{
			const IMU_Sensor* sensor = GetMagnetometer(i);
			if( sensor->present == false )
				OLED_Draw_TickCross8x6( false , 4 , 8*6 + i*6 );
			else
				OLED_Draw_TickCross8x6( true , 4 , 8*6 + i*6 );
		}		
		
		//电压
		sprintf( num_str , "%4.1f" , getBatteryVoltage() );
		OLED_Draw_Str8x6( num_str , 4 , 128-4*6);
		
		//水平定位状态
		OLED_Draw_TickCross8x6( get_Position_Measurement_System_Status() == Measurement_System_Status_Ready , 5 , 128-1*6);
		
		//基本信息
    sprintf( num_str , "%5.1f" , rad2degree( Quaternion_getRoll( get_Airframe_attitude() ) ) );
		OLED_Draw_Str8x6( num_str , 6 , 12+4*6);
		sprintf( num_str , "%5.1f" , rad2degree( Quaternion_getPitch( get_Airframe_attitude() ) ) );
		OLED_Draw_Str8x6( num_str , 6 , 12+14*6);
		sprintf( num_str , "%5.1f" , rad2degree( Quaternion_getYaw( get_Airframe_attitude() ) ) );
		OLED_Draw_Str8x6( num_str , 7 , 12+4*6);
		sprintf( num_str , "%5.1f" , get_VelocityENU().z );
		OLED_Draw_Str8x6( num_str , 7 , 12+14*6);
	}
/*基本信息页面*/
	
/*遥控原始数据页面：
		遥控原始数据显示	
*/
	void OLED_Screen_RawRC_init()
	{
		OLED_Clear_Lines(1,7);
		OLED_Draw_Str8x6( "Rc:" , 4 , 128-4*6 );	OLED_Draw_TickCross8x6( false , 4 , 128-1*6);
		OLED_Screen_RawRC_Refresh();
	}
	void OLED_Screen_RawRC_Refresh()
	{
		const Receiver* rc = get_current_Receiver();
		for( unsigned char i = 0 ; i < 8 ; ++i )
			OLED_Draw_VerticalProgressBar24x14( rc->raw_data[i] , 1 , i*16 );
		for( unsigned char i = 0 ; i < 8 ; ++i )
			OLED_Draw_VerticalProgressBar24x14( rc->raw_data[i+8] , 5 , i*16 );
		if( rc->present == false )
			OLED_Draw_TickCross8x6( false , 4 , 128-1*6 );
		else if( rc->connected == false )
			OLED_Draw_Point8x6( 4 , 128-1*6 );
		else
			OLED_Draw_TickCross8x6( true , 4 , 128-1*6 );
	}
/*遥控原始数据页面*/