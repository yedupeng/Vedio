#include "Modes.h"
#include "Basic.h"
#include "stdlib.h"
#include <stdio.h>
#include "M35_Auto1.h"

#include "AC_Math.h"
#include "Receiver.h"
#include "InteractiveInterface.h"
#include "ControlSystem.h"
#include "MeasurementSystem.h"
#include "OLED_Screens.h"
#include "TM4C123.h"
#include "pin_map.h"
#include "gpio.h"
#include "drv_LED.h"
#include "pwm.h"
#include "drv_TFmini.h"

float M35_last_yaw = 0.0f;//��һ��ƫ����
float M35_now_yaw = 0.0f;//���ڵ�ƫ����
static void M35_Auto1_MainFunc();
static void M35_Auto1_enter();
static void M35_Auto1_exit();
extern vector3_float get_Position();



/*  ��Ե��  */
extern uint16_t get_U7position_z();
extern int8_t line_angle;
extern int16_t SDI_line_deviation[3];


float theta_n = 0;
float theta_l = 0;
static float limit_max_min( float input , float max , float min )
	{
		if( input > max )
			return max;
		else if( input < min )
			return min;
		else
			return input;
	}
static void correct_Yaw( float now_yaw , float last_yaw )
{
		Attitude_Control_set_Target_YawRate(degree2rad(now_yaw-last_yaw));
}

const Mode M35_Auto1 = 
{
	50 , //mode frequency
	M35_Auto1_enter , //enter
	M35_Auto1_exit ,	//exit
	M35_Auto1_MainFunc ,	//mode main func
};

typedef struct
{
	//�˳�ģʽ������
	uint16_t exit_mode_counter;
	
	//�Զ�����״̬��
	uint8_t auto_step1;	//0-��¼��ťλ��
											//1-�ȴ���ť������� 
											//2-�ȴ������� 
											//3-�ȴ�2��
											//4-����
											//5-�ȴ��������
	uint16_t auto_counter;
	float last_button_value;
}MODE_INF;
static MODE_INF* Mode_Inf;
extern uint8_t SDI_mode_contrl;//ʶ��˵�openmv����ģʽ
extern uint8_t SDI_mode_contrl_1;//�ײ�openmv����ģʽ�޸�
static void M35_Auto1_enter()
{	
	Led_setStatus( LED_status_running1 );
	//��ʼ��ģʽ����
	Mode_Inf = malloc( sizeof( MODE_INF ) );
	Mode_Inf->exit_mode_counter = 0;
	Mode_Inf->auto_step1 = Mode_Inf->auto_counter=0;
	Altitude_Control_Enable();
	OLED_Clear();
	OLED_Draw_Str8x6( "Auto" , 0 , 0);	OLED_Draw_Str8x6( "mode:35" , 0 , 128-7*6);
	OLED_Screen_M35_init();
	OLED_Update();
}
static void M35_Auto1_exit()
{
	Altitude_Control_Disable();
	Attitude_Control_Disable();
	
	free( Mode_Inf );
	OLED_Clear();
	OLED_Update();
}

static void M35_Auto1_MainFunc()
{
	const Receiver* rc = get_current_Receiver();
	OLED_Screen_M35_Refresh();
	OLED_Update();
	if( rc->available == false )
		//����
	{
		//���ջ�������
		Position_Control_set_XYLock();
		Position_Control_set_TargetVelocityZ( -50 );
		return;
	}
	float throttle_stick = rc->data[0];
	float yaw_stick = rc->data[1];
	float pitch_stick = rc->data[2];
	float roll_stick = rc->data[3];	
	//-------------opencv������-----------  
	if(rc->data[4]<40&&rc->data[5]>60)
		SDI_mode_contrl=0x17;
	else if(rc->data[4]>60&&rc->data[5]>60)
		SDI_mode_contrl=0x17;
	//-----------------------------------y
	/*�ж��˳�ģʽ*/
		if( throttle_stick < 5 && yaw_stick < 5 && pitch_stick < 5 && roll_stick > 95 )
		{
			if( ++Mode_Inf->exit_mode_counter >= 50 )
			{
				change_Mode( 01 );
				return;
			}
		}
		else
			Mode_Inf->exit_mode_counter = 0;
	/*�ж��˳�ģʽ*/
		
	//�ж�ҡ���Ƿ����м�
	bool sticks_in_neutral = 
		in_symmetry_range_offset_float( throttle_stick , 5 , 50 ) && \
		in_symmetry_range_offset_float( yaw_stick , 5 , 50 ) && \
		in_symmetry_range_offset_float( pitch_stick , 5 , 50 ) && \
		in_symmetry_range_offset_float( roll_stick , 5 , 50 );
	
	extern vector3_float SDI_Point;
	extern TIME SDI_Time;
	//ը��ʱǿ�ƹرյ��
	if(get_lean_angle_cosin()<0.4f)
	{
		Altitude_Control_Disable();
		Attitude_Control_Disable();
		change_Mode(01);
	}
	static float hight=120.0f; //���и߶�
	if( sticks_in_neutral && get_Altitude_Measurement_System_Status() == Measurement_System_Status_Ready )
	{
		//ҡ�����м�
		//ִ���Զ�����		
		//ֻ����λ����Чʱ��ִ���Զ�����
		
		//��ˮƽλ�ÿ���
		Position_Control_Enable();
		switch( Mode_Inf->auto_step1 )
		{
			case 0:
				Mode_Inf->auto_step1=1;
				Mode_Inf->auto_counter=0;
				break;
			
			case 1:
			{	
				if( get_is_inFlight() == false && rc->data[5]<40 )
				{

						Led_setSignal( LED_signal_2 );
						Mode_Inf->auto_step1=2;
						Mode_Inf->auto_counter = 0;
					  break;
			 }
		  }
			
			case 2:
				{
					if(++Mode_Inf->auto_counter>300)
					{
						Led_setSignal(LED_signal_2);
						Mode_Inf->auto_step1=3;
						Mode_Inf->auto_counter=0;
					}
					break;
				}
				
			case 3:
			{
				Position_Control_set_TargetVelocityZ(10.0f);
				Mode_Inf->auto_step1 = 4;
				Mode_Inf->auto_counter=0;
				break;
			}
			
			case 4:
			{
				if(++Mode_Inf->auto_counter>100)
				{
					Mode_Inf->auto_step1 = 5;
					Mode_Inf->auto_counter=0;
				}
				break;
			}
			
			case 5:
			{
				if(get_U7position_z()>=100)
				{
					Position_Control_set_ZLock();
					Position_Control_set_XYLock();
					Attitude_Control_set_YawLock();
					Mode_Inf->auto_step1 = 6;
					Mode_Inf->auto_counter=0;
				}break;
			}
			
			case 6:
			{
				Position_Control_set_TargetVelocityBodyHeadingXY_AngleLimit(10.0f,0.0f,0.08,0.08);
				Mode_Inf->auto_step1 = 7;
				Mode_Inf->auto_counter=0;
				break;
			}
			
			case 7:
			{
				if(++Mode_Inf->auto_counter>150)
				{
					Position_Control_set_XYLock();
					Attitude_Control_set_YawLock();
					Mode_Inf->auto_step1 = 8;
					Mode_Inf->auto_counter=0;
				}
				break;
			}
			
			case 8:
			{
				if(++Mode_Inf->auto_counter>150)
				{
					Mode_Inf->auto_step1 = 9;
					Mode_Inf->auto_counter=0;
				}
				break;
			}
			
			case 9:
			{
				theta_l = fabs(Quaternion_getYaw( get_Airframe_attitude()));
				if(fabs(theta_n-theta_l)>=degree2rad(85)&&fabs(theta_n-theta_l)<=degree2rad(95))
				{
					Attitude_Control_set_YawLock();
					Mode_Inf->auto_step1 = 10;
					Mode_Inf->auto_counter=0;
				}else
				{
					Attitude_Control_set_Target_YawRate(degree2rad(10));
				}break;
			}
			
			case 10:
			{
					Position_Control_set_TargetVelocityBodyHeadingXY_AngleLimit(10.0f,0.0f,0.08,0.08);
					Mode_Inf->auto_step1 = 11;
					Mode_Inf->auto_counter=0;
				break;
			}
			
			case 11:
			{
				if(++Mode_Inf->auto_counter>100)
				{
					Position_Control_set_XYLock();
					Attitude_Control_set_YawLock();
					Mode_Inf->auto_step1 = 12;
					Mode_Inf->auto_counter=0;
				}
				break;
			}
			
			case 12:
			{
				if(++Mode_Inf->auto_counter>100)
				{
					Mode_Inf->auto_step1 = 32;
					Mode_Inf->auto_counter=0;
				}
				break;
			}
				
			case 32:
				//����
				if( get_Position_ControlMode() == Position_ControlMode_Position )
				{
					Position_Control_set_TargetVelocityZ( -50 );
					Mode_Inf->auto_step1=33;
					Mode_Inf->auto_counter=0;
				}
				break;
			/******�����½�״̬���жϸ߶�ʹ��ƽ������************************************/			
			case 33:
				if(get_Position().z<40)
				{
					Position_Control_set_TargetVelocityZ( -15 );
					Mode_Inf->auto_step1=34;
					Mode_Inf->auto_counter=0;
				}
				break;
			case 34:
				//�ȴ��������
				if( get_is_inFlight() == false )
				{
					change_Mode( 1 );
					Mode_Inf->auto_counter=0;
				}
				break;
			/******��������״̬����ֹը��************************************/
			case 35:
				if(++Mode_Inf->auto_counter==250)//ֹͣ5��
				{
					Mode_Inf->auto_step1=32;
					Mode_Inf->auto_counter=0;
				}
		}
	}
	else
	{
		ManualControl:
		//ҡ�˲����м�
		//�ֶ�����
		Mode_Inf->auto_step1 = Mode_Inf->auto_counter=0;
		if(throttle_stick<50)
		{
			change_Mode( 32 );
		}else
		  change_Mode(01);
//		//�ر�ˮƽλ�ÿ���
//		Position_Control_Disable();
//		//�߶ȿ�������
//		if( in_symmetry_range_offset_float( throttle_stick , 5 , 50 ) )
//			Position_Control_set_ZLock();
//		else
//			Position_Control_set_TargetVelocityZ( ( throttle_stick - 50.0f ) * 6 );

//		//ƫ����������
//		if( in_symmetry_range_offset_float( yaw_stick , 5 , 50 ) )
//			Attitude_Control_set_YawLock();
//		else
//			Attitude_Control_set_Target_YawRate( ( 50.0f - yaw_stick )*0.05f );
//		
//		//Roll Pitch��������
//	Attitude_Control_set_Target_RollPitch( \
//			( roll_stick 	- 50.0f )*0.015f, \
//			( pitch_stick - 50.0f )*0.015f );
	}
}
