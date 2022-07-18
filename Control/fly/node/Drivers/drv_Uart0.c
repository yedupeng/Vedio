////�ײ��򶥲����ӵ�openmv

//#include "Basic.h"
//#include "drv_Uart0.h"

//#include "Quaternion.h"
//#include "MeasurementSystem.h"

//#include "STS.h"
//#include "Sensors_Backend.h"
//#include "RingBuf.h"

//#include "TM4C123GH6PM.h"
//#include "uart.h"
//#include "sysctl.h"
//#include "gpio.h"
//#include "pin_map.h"
//#include "interrupt.h"
//#include "hw_ints.h"
//#include "hw_gpio.h"
//#include "Timer.h"
//#include "udma.h"
//#include "drv_LED.h"
//#include "InteractiveInterface.h"

//#define Uart0_BUFFER_SIZE 64

////�����ж�
//static void UART0_Handler();

///*���ͻ�����*/
//	static uint8_t Uart0_tx_buffer[Uart0_BUFFER_SIZE];
//	static RingBuf_uint8_t Uart0_Tx_RingBuf;
///*���ͻ�����*/

///*���ջ�����*/
//	static uint8_t Uart0_rx_buffer[Uart0_BUFFER_SIZE];
//	static RingBuf_uint8_t Uart0_Rx_RingBuf;
///*���ջ�����*/

//static bool Uart0_RCTrigger( unsigned int Task_ID );
//static void Uart0_Server( unsigned int Task_ID );

//static bool Uart0_TXTrigger( unsigned int Task_ID );
//static void Uart0_Tx_model_contrl( unsigned int Task_ID );

//void init_drv_Uart0()
//{
//	//ʹ��Uart0���ţ�Rx:PA0 Tx:PA1��
//	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//	//ʹ��UART0
//	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
//	
//	//����GPIO
//	GPIOPinConfigure(GPIO_PA0_U0RX);
//	GPIOPinConfigure(GPIO_PA1_U0TX);
//	GPIOPinTypeUART(GPIOA_BASE, GPIO_PIN_0 | GPIO_PIN_1);//GPIO��UARTģʽ����
//		
//	//����Uart
//	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet() , 115200,			
//	                   (UART_CONFIG_WLEN_8 
//										| UART_CONFIG_STOP_ONE 
//										|	UART_CONFIG_PAR_NONE));	
//	
//	//��ʼ��������
//	RingBuf_uint8_t_init( &Uart0_Tx_RingBuf , Uart0_tx_buffer , Uart0_BUFFER_SIZE );
//	RingBuf_uint8_t_init( &Uart0_Rx_RingBuf , Uart0_rx_buffer , Uart0_BUFFER_SIZE );
//	
//	//���ô��ڽ����ж�
//	UARTIntEnable( UART0_BASE , UART_INT_RX | UART_INT_RT);
//	UARTIntRegister( UART0_BASE , UART0_Handler );
//	
//	//����DMA����
//	uDMAChannelControlSet( UDMA_PRI_SELECT | UDMA_CH9_UART0TX , \
//		UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1 );
//	UARTDMAEnable( UART0_BASE , UART_DMA_TX );
//	UARTIntRegister( UART0_BASE , UART0_Handler );	
//	uDMAChannelAssign(UDMA_CH9_UART0TX  );	
//	
//	//���ж�
//	IntPrioritySet( INT_UART0 , INT_PRIO_7 );
//	IntEnable( INT_UART0 );
//	
//	
////	//ע�ᴫ����
////	PositionSensorRegister( 3 , Position_Sensor_Type_RelativePositioning , Position_Sensor_DataType_s_xy , Position_Sensor_frame_ENU , 0.1f , false );
//	//��Ӽ򵥶��ο���Э���������
//	STS_Add_Task( STS_Task_Trigger_Mode_Custom , 0 , Uart0_RCTrigger , Uart0_Server );//����openmv������������
//	STS_Add_Task( STS_Task_Trigger_Mode_Custom , 0 , Uart0_TXTrigger , Uart0_Tx_model_contrl );//���͸�openmv������������
//}



//uint8_t Uart0_mode_contrl = 0;//״̬�ⲿ���������openmvʶ��ģʽ  ��M35 ����
//uint8_t Uart0_tc_mode = 0;//�ݴ� SDI_mode_contrl
//static bool Uart0_TXTrigger( unsigned int Task_ID )
//{
//	if( Uart0_mode_contrl != 0 )
//	{
//		Uart0_tc_mode = Uart0_mode_contrl;
//		Uart0_mode_contrl = 0;//ִ��һ�η������񼴹رշ���
//		return true;
//	}
//	return false; 
//}

//static void Uart0_Tx_model_contrl( unsigned int Task_ID )
//{//openmv����ģʽ����
//	uint8_t tc_buf[6] = {0};
//	uint8_t tc_cnt = 0;
//	uint8_t sum = 0;
//	uint16_t tc_data_buf = 0;
//	
//	tc_buf[0] = 0xAA;//��ͷ1
//	tc_buf[1] = 0x55;//��ͷ2
//	
//	tc_buf[2] = 0x18;//ģʽλ24
//	tc_buf[3] = 0x02;//���ݳ���
//	switch(Uart0_tc_mode)
//	{//openmvָ�����
//		case 0xff:
//		{
//			tc_buf[4] = 0xff;//��ʶ��ģʽ
//		}break;
//		
//		case 3://ʶ����ĸA
//		{
//			tc_buf[4] = 0x0B;
//		}break;
//		
//		case 13://����ɨ��ֲ��ģʽ
//		{
//			tc_buf[4] = 0x16;
//		}break;
//		case 1:
//			{//			����ת�䣬�رռ�����                                       
//			tc_buf[4] = 2;
//		}break;
//		case 11:
//		{// ת��������򿪼����㣨�����ӳ�1s��㣩
//			tc_buf[4] = 3;
//		}break;
//		case 103:
//		{																						//B����3ģʽ������B�˺󣬿�ʼ3��Ѳ��
//			tc_buf[4] = 103;
//		}break;
//	}
//	tc_buf[5] = (tc_buf[0] + tc_buf[1] + tc_buf[2] + tc_buf[3] + tc_buf[4])%0x100;
//	Uart0_Send(tc_buf,6);
//}

//static bool Uart0_RCTrigger( unsigned int Task_ID )
//{
//	if( Uart0_DataAvailable() )
//		return true;
//	return false;
//}



//uint8_t Uart0_A_cx=0;//A��ĸ��������ɫ��x����
//uint8_t Uart0_A_cy=0;//A��ĸ��������ɫ��y����
//int16_t Uart0_A_cx1=0;//A��ĸ��������ɫ���x
//int16_t Uart0_A_cy1=0;//A��ĸ��������ɫ���y
//int8_t black_tag;//������߱�־λ��100Ϊ�ҵ�
//uint8_t black_y[3]={0};//����������������
//int16_t black_dy[3]={0};//������߾������ĵľ���
//int8_t white_tag;//������߱�־λ��1Ϊ�ҵ�
//int16_t white_dy;//������߾������ĵľ���
//int16_t msg_pack1;
//int16_t msg_pack2;
//static void Uart0_Server ( unsigned int Task_ID )
//{
//	//�򵥶��ο���Э�����
//	
//	/*״̬������*/
//		static uint8_t rc_step1 = 0;	//0�����հ�ͷ'A' 'C'
//																	//1������1�ֽ���Ϣ���
//																	//2������1�ֽ���Ϣ����
//																	//3���������ݰ�����
//																	//4������2�ֽ�У��
//		static uint8_t rc_step2 = 0;
//	
//		#define MAX_Uart0_PACKET_SIZE 11
//		static uint8_t msg_type;
//		static uint8_t msg_length;
//		ALIGN4 static uint8_t msg_pack[MAX_Uart0_PACKET_SIZE]={0};
//		static uint8_t sumB;
//		#define reset_Uart0_RC ( rc_step1 = rc_step2 = 0 )
//	/*״̬������*/
//	
//	uint8_t rc_buf[20];
//	uint8_t length = read_Uart0( rc_buf , 20 );
//	for( uint8_t i = 0 ; i < length ; ++i )
//	{
//		uint8_t r_data = rc_buf[i];
//		
//		switch( rc_step1 )
//		{
//			case 0 :
//				//���հ�ͷAA55
//				sumB=0;
//				if(r_data == 0xAA)
//				{
//					sumB += r_data;
//					rc_step1 = 1;
//				}
//				break;
//			case 1:
//				if(r_data == 0x55)
//				{
//					sumB += r_data;
//					rc_step1 = 2;
//				}
//				else reset_Uart0_RC;
//				break;
//			case 2:
//				//������Ϣ���
//				if(r_data > 0x10 && r_data < 0x30)//openmvģʽ
//				{
//					msg_type = r_data;
//					sumB += r_data;
//					rc_step1 = 3;
//					rc_step2 = 0;
//				}
//				else reset_Uart0_RC;
//				break;
//			
//			case 3:
//				//������Ϣ����
//				if( r_data > MAX_Uart0_PACKET_SIZE )
//				{
//					reset_Uart0_RC;
//					break;
//				}
//				msg_length = r_data;
//				sumB += r_data;
//				rc_step1 = 4;
//				rc_step2 = 0;
//				break;
//				
//			case 4:
//				//�������ݰ�
//				msg_pack[ rc_step2 ++] = r_data;
//				sumB += r_data;
//				if( rc_step2 >= msg_length )
//				{
//					rc_step1 = 5;
//					rc_step2 = 0;
//				}
//				break;
//				
//			case 5:
//				//����У��λ
//				if(sumB == r_data)
//				{
//					if(msg_type == 0x12)//׷����ĸA 
//					{
//						Uart0_A_cx=msg_pack[0];
//						Uart0_A_cx1=(235-Uart0_A_cx);//��������ʱ��ƫ��Ӧ������
//						Uart0_A_cy=msg_pack[1];
//						Uart0_A_cy1=(40-Uart0_A_cy);//ǰ��,Ϊ����ʱƫ�£�Ӧ��ǰ��
//					}
//					
//					else if(msg_type == 0x13)//�жϺ���				
//					{
//						black_y[0]=msg_pack[0];
//						black_y[1]=msg_pack[1];
//						black_y[2]=msg_pack[2];
//						black_tag=msg_pack[3];
//						black_dy[0]=(black_y[0]-275);
//						black_dy[1]=(black_y[1]-275);
//						black_dy[2]=(black_y[2]-275);
//					}
//					
//					
//					else if(msg_type == 0x20)//Ѳ��
//					{
//						
//					}
//					
//					else if(msg_type == 0x15)//�������ҵ���
//					{
//				
//					}
//					
//					
//					else if(msg_type == 0x18)//��������˾���
//					{
//						
//					}
//					
//				}
//				
//				reset_Uart0_RC;		
//					
//				break;
//		}
//	}
//}


//	int8_t get_A_cy1()
//	{
//		return Uart0_A_cy1;
//	}
//	int8_t get_A_cx1()
//	{
//		return Uart0_A_cx1;
//	}
//	uint16_t get__Uart0_tcmode()
//	{
//		return Uart0_tc_mode;
//	}
//int8_t get_y2_y()
//	{
//		return black_dy[2];
//	}
//	int8_t get_y1_y()
//	{
//		return black_dy[1];
//	}
//	int8_t get_y_y()
//	{
//		return black_dy[0];
//	}
//	int8_t get_black_tag()
//	{
//		return black_tag;
//	}
//void Uart0_Send( const uint8_t* data , uint16_t length )
//{
//	IntDisable( INT_UART0 );
//	
//	//��ȡʣ��Ļ������ռ�
//	int16_t buffer_space = RingBuf_uint8_t_get_Freesize( &Uart0_Tx_RingBuf );
//	//��ȡDMA�д����͵��ֽ���
//	int16_t DMA_Remain = uDMAChannelSizeGet( UDMA_CH9_UART0TX );
//	
//	//����Ҫ���͵��ֽ���
//	int16_t max_send_count = buffer_space - DMA_Remain;
//	if( max_send_count < 0 )
//		max_send_count = 0;
//	uint16_t send_count = ( length < max_send_count ) ? length : max_send_count;
//	
//	//���������ֽ�ѹ�뻺����
//	RingBuf_uint8_t_push_length( &Uart0_Tx_RingBuf , data , send_count );
////	for( uint8_t i = 0 ; i < send_count ; ++i )
////		RingBuf_uint8_t_push( &Uart0_Tx_RingBuf , data[i] );
//	
//	//��ȡDMA�����Ƿ����
//	if( uDMAChannelIsEnabled( UDMA_CH9_UART0TX ) == false )
//	{
//		//DMA�����
//		//���Լ�������
//		uint16_t length;
//		uint8_t* p = RingBuf_uint8_t_pop_DMABuf( &Uart0_Tx_RingBuf , &length );
//		if( length )
//		{
//			uDMAChannelTransferSet( UDMA_PRI_SELECT | UDMA_CH9_UART0TX , \
//				UDMA_MODE_BASIC , p , (void*)&UART0->DR , length );
//			uDMAChannelEnable( UDMA_CH9_UART0TX );
//		}
//	}
//	IntEnable( INT_UART0 );
//}
//static void UART0_Handler()
//{
//	UARTIntClear( UART0_BASE , UART_INT_OE );
//	UARTRxErrorClear( UART0_BASE );
//	while( ( UART0->FR & (1<<4) ) == false	)
//	{
//		//����
//		uint8_t rdata = UART0->DR;
//		RingBuf_uint8_t_push( &Uart0_Rx_RingBuf , rdata );
//	}
//	
//	if( uDMAChannelIsEnabled( UDMA_CH9_UART0TX ) == false )
//	{
//		uint16_t length;
//		uint8_t* p = RingBuf_uint8_t_pop_DMABuf( &Uart0_Tx_RingBuf , &length );
//		if( length )
//		{
//			uDMAChannelTransferSet( UDMA_PRI_SELECT | UDMA_CH9_UART0TX , \
//				UDMA_MODE_BASIC , p , (void*)&UART0->DR , length );
//			uDMAChannelEnable( UDMA_CH9_UART0TX );
//		}
//	}
//}
//uint16_t read_Uart0( uint8_t* data , uint16_t length )
//{
//	IntDisable( INT_UART0 );
//	uint8_t read_bytes = RingBuf_uint8_t_pop_length( &Uart0_Rx_RingBuf , data , length );
//	IntEnable( INT_UART0 );
//	return read_bytes;
//}
//uint16_t Uart0_DataAvailable()
//{
//	IntDisable( INT_UART0 );
//	uint16_t bytes2read = RingBuf_uint8_t_get_Bytes2read( &Uart0_Rx_RingBuf );
//	IntEnable( INT_UART0 );
//	return bytes2read;
//}