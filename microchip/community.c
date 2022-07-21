/*** �ײ�openmv���� ***/
#include "Basic.h"
#include "drv_SDI.h"

#include "Quaternion.h"
#include "MeasurementSystem.h"

#include "STS.h"
#include "Sensors_Backend.h"
#include "RingBuf.h"
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

#define BUFFER_SIZE 64
int16_t test_1;
int16_t test_2;
int16_t test_3;

//�����ж�
static void UART3_Handler();

/*���ͻ�����*/
	static uint8_t tx_buffer[BUFFER_SIZE];
	static RingBuf_uint8_t Tx_RingBuf;
/*���ͻ�����*/

/*���ջ�����*/
	static uint8_t rx_buffer[BUFFER_SIZE];
	static RingBuf_uint8_t Rx_RingBuf;
/*���ջ�����*/

static bool SDI_RCTrigger( unsigned int Task_ID );
static void SDI_Server( unsigned int Task_ID );
static bool SDI_TXTrigger( unsigned int Task_ID );
static void SDI_Tx_model_contrl( unsigned int Task_ID );

void init_drv_SDI()
{
	//ʹ��Uart3���ţ�Rx:PC6 Tx:PC7��
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	//ʹ��UART3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);
	
	//����GPIO
	GPIOPinConfigure(GPIO_PC6_U3RX);
	GPIOPinConfigure(GPIO_PC7_U3TX);
	GPIOPinTypeUART(GPIOC_BASE, GPIO_PIN_6 | GPIO_PIN_7);//GPIO��UARTģʽ����
		
	//����Uart
	UARTConfigSetExpClk(UART3_BASE, SysCtlClockGet() , 115200,			
	                   (UART_CONFIG_WLEN_8 
										| UART_CONFIG_STOP_ONE 
										|	UART_CONFIG_PAR_NONE));	
	
	//��ʼ��������
	RingBuf_uint8_t_init( &Tx_RingBuf , tx_buffer , BUFFER_SIZE );
	RingBuf_uint8_t_init( &Rx_RingBuf , rx_buffer , BUFFER_SIZE );
	
	//���ô��ڽ����ж�
	UARTIntEnable( UART3_BASE , UART_INT_RX | UART_INT_RT );
	UARTIntRegister( UART3_BASE , UART3_Handler );
	
	//����DMA����
	uDMAChannelControlSet( UDMA_PRI_SELECT | UDMA_CH17_UART3TX , \
		UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1 );
	UARTDMAEnable( UART3_BASE , UART_DMA_TX );
	UARTIntRegister( UART3_BASE , UART3_Handler );	
	uDMAChannelAssign(UDMA_CH17_UART3TX  );	
	
	//���ж�
	IntPrioritySet( INT_UART3 , INT_PRIO_5 );
	IntEnable( INT_UART3 );
	
	
//	//ע�ᴫ����
//	PositionSensorRegister( 3 , Position_Sensor_Type_RelativePositioning , Position_Sensor_DataType_s_xy , Position_Sensor_frame_ENU , 0.1f , false );
	//��Ӽ򵥶��ο���Э���������
	STS_Add_Task( STS_Task_Trigger_Mode_Custom , 0 , SDI_RCTrigger , SDI_Server );//����openmv������������
	STS_Add_Task( STS_Task_Trigger_Mode_Custom , 0 , SDI_TXTrigger , SDI_Tx_model_contrl );//���͸�openmv������������
}
uint8_t SDI_mode_contrl = 0;//״̬�ⲿ���������openmvʶ��ģʽ  ��M35 ����
uint8_t SDI_tc_mode = 0;//�ݴ� SDI_mode_contrl
static bool SDI_TXTrigger( unsigned int Task_ID )
{
	if( SDI_mode_contrl != 0 )
	{
		SDI_tc_mode = SDI_mode_contrl;
		SDI_mode_contrl = 0;//ִ��һ�η������񼴹رշ���
		return true;
	}
	return false; 
}

static void SDI_Tx_model_contrl( unsigned int Task_ID )
{//openmv����ģʽ����
	uint8_t tc_buf[6] = {0};
	uint8_t tc_cnt = 0;
	uint8_t sum = 0;
	uint16_t tc_data_buf = 0;
	
	tc_buf[0] = 0x0f;//��ͷ1
	tc_buf[1] = 0xf0;//��ͷ2
	
	tc_buf[2] = 0x20;//ģʽλ24
	tc_buf[3] = 0x02;//���ݳ���
	switch(SDI_tc_mode)
	{//openmvָ�����
		case 0xff:
		{
			tc_buf[4] = 0xff;//��ʶ��ģʽ
		}break;
		case 0x17:
		{
			tc_buf[4] = 0x17;
		}break;
	}
	tc_buf[5] = (tc_buf[0] + tc_buf[1] + tc_buf[2] + tc_buf[3] + tc_buf[4])%0x100;
	Uart3_Send(tc_buf,6);
}

static bool SDI_RCTrigger( unsigned int Task_ID )
{
	if( Uart3_DataAvailable() )
		return true;
	return false;
}

static void SDI_Server( unsigned int Task_ID )
{
	//�򵥶��ο���Э�����
	
	/*״̬������*/
		static uint8_t rc_step1 = 0;	//0�����հ�ͷ'A' 'C'
																	//1������1�ֽ���Ϣ���
																	//2������1�ֽ���Ϣ����
																	//3���������ݰ�����
																	//4������2�ֽ�У��
		static uint8_t rc_step2 = 0;
	
		#define MAX_SDI_PACKET_SIZE 11
		static uint8_t msg_type;
		static uint8_t msg_length;
		ALIGN4 static uint8_t msg_pack[MAX_SDI_PACKET_SIZE]={0};
		static uint8_t sumB;
		
		#define reset_SDI_RC ( rc_step1 = rc_step2 = 0 )
	/*״̬������*/
	
	uint8_t rc_buf[20];
	uint8_t length = read_Uart3( rc_buf , 20 );
	for( uint8_t i = 0 ; i < length ; ++i )
	{
		uint8_t r_data = rc_buf[i];
		switch( rc_step1 )
		{
			case 0 :
				//���հ�ͷ0f f0
			sumB=0;
				if(r_data == 0x0f)
				{
					sumB += r_data;
					rc_step1 = 1;
				}
				break;
			case 1:
				if(r_data == 0xf0)
				{
					sumB += r_data;
					rc_step1 = 2;
				}
				else reset_SDI_RC;
				break;
			case 2:
				//������Ϣ���
				if(r_data > 0x10 && r_data < 0x30)//openmvģʽ
				{
					msg_type = r_data;
					sumB += r_data;
					rc_step1 = 3;
					rc_step2 = 0;
				}
				else reset_SDI_RC;
				break;
			
			case 3:
				//������Ϣ����
				if( r_data > MAX_SDI_PACKET_SIZE )
				{
					reset_SDI_RC;
					break;
				}
				msg_length = r_data;
				sumB += r_data;
				rc_step1 = 4;
				rc_step2 = 0;
				break;
				
			case 4:
				//�������ݰ�
				msg_pack[ rc_step2 ++] = r_data;
				sumB += r_data;
				if( rc_step2 >= msg_length )
				{
					rc_step1 = 5;
					rc_step2 = 0;
				}
				break;
				
			case 5:
				//����У��λ
				if((sumB%256) == r_data)
				{
					if(msg_type == 0x17)
					{
						test_1 = msg_pack[0];
						test_2 = msg_pack[1];
						test_3 = msg_pack[2];
					}
				}
				reset_SDI_RC;			
				break;
			}
	}
}

uint16_t test1()
{
	return test_1;
}

uint16_t test2()
{
	return test_2;
}

uint16_t test3()
{
	return test_3;
}



uint16_t get_SDI_tcmode()
{
	return SDI_tc_mode;
}

void Uart3_Send( const uint8_t* data , uint16_t length )
{
	IntDisable( INT_UART3 );
	
	//��ȡʣ��Ļ������ռ�
	int16_t buffer_space = RingBuf_uint8_t_get_Freesize( &Tx_RingBuf );
	//��ȡDMA�д����͵��ֽ���
	int16_t DMA_Remain = uDMAChannelSizeGet( UDMA_CH17_UART3TX );
	
	//����Ҫ���͵��ֽ���
	int16_t max_send_count = buffer_space - DMA_Remain;
	if( max_send_count < 0 )
		max_send_count = 0;
	uint16_t send_count = ( length < max_send_count ) ? length : max_send_count;
	
	//���������ֽ�ѹ�뻺����
	RingBuf_uint8_t_push_length( &Tx_RingBuf , data , send_count );
//	for( uint8_t i = 0 ; i < send_count ; ++i )
//		RingBuf_uint8_t_push( &Tx_RingBuf , data[i] );
	
	//��ȡDMA�����Ƿ����
	if( uDMAChannelIsEnabled( UDMA_CH17_UART3TX ) == false )
	{
		//DMA�����
		//���Լ�������
		uint16_t length;
		uint8_t* p = RingBuf_uint8_t_pop_DMABuf( &Tx_RingBuf , &length );
		if( length )
		{
			uDMAChannelTransferSet( UDMA_PRI_SELECT | UDMA_CH17_UART3TX , \
				UDMA_MODE_BASIC , p , (void*)&UART3->DR , length );
			uDMAChannelEnable( UDMA_CH17_UART3TX );
		}
	}
	IntEnable( INT_UART3 );
}
static void UART3_Handler()
{
	UARTIntClear( UART3_BASE , UART_INT_OE );
	UARTRxErrorClear( UART3_BASE );
	while( ( UART3->FR & (1<<4) ) == false	)
	{
		//����
		uint8_t rdata = UART3->DR;
		RingBuf_uint8_t_push( &Rx_RingBuf , rdata );
	}
	
	if( uDMAChannelIsEnabled( UDMA_CH17_UART3TX ) == false )
	{
		uint16_t length;
		uint8_t* p = RingBuf_uint8_t_pop_DMABuf( &Tx_RingBuf , &length );
		if( length )
		{
			uDMAChannelTransferSet( UDMA_PRI_SELECT | UDMA_CH17_UART3TX , \
				UDMA_MODE_BASIC , p , (void*)&UART3->DR , length );
			uDMAChannelEnable( UDMA_CH17_UART3TX );
		}
	}
}
uint16_t read_Uart3( uint8_t* data , uint16_t length )
{
	IntDisable( INT_UART3 );
	uint8_t read_bytes = RingBuf_uint8_t_pop_length( &Rx_RingBuf , data , length );
	IntEnable( INT_UART3 );
	return read_bytes;
}
uint16_t Uart3_DataAvailable()
{
	IntDisable( INT_UART3 );
	uint16_t bytes2read = RingBuf_uint8_t_get_Bytes2read( &Rx_RingBuf );
	IntEnable( INT_UART3 );
	return bytes2read;
}