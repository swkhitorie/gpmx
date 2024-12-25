/**
 * low level driver for stm32f4 series, base on std periph library
 * all core and periph interrupt
*/

#include "lld_kernel.h"
#include "lld_uart.h"
#include "lld_exit.h"
#include "lld_i2c.h"
#include "lld_spi.h"
#include "lld_timer.h"

#include "ttscheduler.h"

void SysTick_Handler(void)
{
    lld_kernel_irq();
    ttscheduler_update();
}

//==============================================  EXTIx IRQHandler
void EXTI0_IRQHandler(void) 	  //----EXTI0 IRQ-----//
{   if(mcu_exit_list[0]) lld_exit_irq(mcu_exit_list[0]); }
void EXTI1_IRQHandler(void) 	  //----EXTI1 IRQ-----//
{	if(mcu_exit_list[1]) lld_exit_irq(mcu_exit_list[1]); }
void EXTI2_IRQHandler(void) 	  //----EXTI2 IRQ-----//
{	if(mcu_exit_list[2]) lld_exit_irq(mcu_exit_list[2]); }
void EXTI3_IRQHandler(void) 	  //----EXTI3 IRQ-----//
{	if(mcu_exit_list[3]) lld_exit_irq(mcu_exit_list[3]); }
void EXTI4_IRQHandler(void) 	  //----EXTI4 IRQ-----//
{	if(mcu_exit_list[4]) lld_exit_irq(mcu_exit_list[4]); }

void EXTI9_5_IRQHandler(void) 	//---EXTI9_5 IRQ----//
{
	if(EXTI_GetITStatus(EXTI_Line5))       //--EXTI5--//
	{		if(mcu_exit_list[5]) lld_exit_irq(mcu_exit_list[5]); }
	if(EXTI_GetITStatus(EXTI_Line6))       //--EXTI6--//
	{		if(mcu_exit_list[6]) lld_exit_irq(mcu_exit_list[6]); }
	if(EXTI_GetITStatus(EXTI_Line7))       //--EXTI7--//
	{		if(mcu_exit_list[7]) lld_exit_irq(mcu_exit_list[7]); }
	if(EXTI_GetITStatus(EXTI_Line8))       //--EXTI8--//
	{		if(mcu_exit_list[8]) lld_exit_irq(mcu_exit_list[8]); }
	if(EXTI_GetITStatus(EXTI_Line9))       //--EXTI9--//
	{		if(mcu_exit_list[9]) lld_exit_irq(mcu_exit_list[9]); }
}

void EXTI15_10_IRQHandler(void) //---EXTI15_10 IRQ--//
{
	if(EXTI_GetITStatus(EXTI_Line10))     //--EXTI10--//
	{		if(mcu_exit_list[10]) lld_exit_irq(mcu_exit_list[10]); }	
	if(EXTI_GetITStatus(EXTI_Line11))     //--EXTI11--//
	{		if(mcu_exit_list[11]) lld_exit_irq(mcu_exit_list[11]); }
	if(EXTI_GetITStatus(EXTI_Line12))     //--EXTI12--//
	{		if(mcu_exit_list[12]) lld_exit_irq(mcu_exit_list[12]); }
	if(EXTI_GetITStatus(EXTI_Line13))     //--EXTI13--//
	{		if(mcu_exit_list[13]) lld_exit_irq(mcu_exit_list[13]); }
	if(EXTI_GetITStatus(EXTI_Line14))     //--EXTI14--//
	{		if(mcu_exit_list[14]) lld_exit_irq(mcu_exit_list[14]); }
	if(EXTI_GetITStatus(EXTI_Line15))     //--EXTI15--//
	{		if(mcu_exit_list[15]) lld_exit_irq(mcu_exit_list[15]); }
}

//==============================================  UARTx_IRQHandler 

void USART1_IRQHandler(void)    
{	if(mcu_uart_list[0])  lld_uart_irq(mcu_uart_list[0]);}
void USART2_IRQHandler(void)    
{	if(mcu_uart_list[1])  lld_uart_irq(mcu_uart_list[1]);}
void USART3_IRQHandler(void)   
{	if(mcu_uart_list[2])  lld_uart_irq(mcu_uart_list[2]); }
void UART4_IRQHandler(void)   
{	if(mcu_uart_list[3])  lld_uart_irq(mcu_uart_list[3]); }
void UART5_IRQHandler(void)    
{	if(mcu_uart_list[4])  lld_uart_irq(mcu_uart_list[4]); }
void UART6_IRQHandler(void)    
{	if(mcu_uart_list[5])  lld_uart_irq(mcu_uart_list[5]); }


//==============================================  DMA  IRQHandler 

//=======================================  UART1 =======================================//
void DMA2_Stream7_IRQHandler(void)
{ if(mcu_uart_list[0])  lld_uart_irq_txdma(mcu_uart_list[0]); }

void DMA2_Stream2_IRQHandler(void)
{ if(mcu_uart_list[0])  lld_uart_irq_rxdma(mcu_uart_list[0]); }

//=======================================  UART2 =======================================//
void DMA1_Stream6_IRQHandler(void)
{ if(mcu_uart_list[1])  lld_uart_irq_txdma(mcu_uart_list[1]); }

void DMA1_Stream5_IRQHandler(void)
{ if(mcu_uart_list[1])  lld_uart_irq_rxdma(mcu_uart_list[1]); }

//=======================================  UART3 =======================================//

void DMA1_Stream3_IRQHandler(void)
{ if(mcu_uart_list[2])  lld_uart_irq_txdma(mcu_uart_list[2]); }

void DMA1_Stream1_IRQHandler(void)
{ if(mcu_uart_list[2])  lld_uart_irq_rxdma(mcu_uart_list[2]); }

//=======================================  UART4 =======================================//
void DMA1_Stream4_IRQHandler(void)
{ if(mcu_uart_list[3])  lld_uart_irq_txdma(mcu_uart_list[3]); }

void DMA1_Stream2_IRQHandler(void)
{ if(mcu_uart_list[3])  lld_uart_irq_rxdma(mcu_uart_list[3]); }

//=======================================  UART5 =======================================//
void DMA1_Stream7_IRQHandler(void)
{ if(mcu_uart_list[4])  lld_uart_irq_txdma(mcu_uart_list[4]); }

void DMA1_Stream0_IRQHandler(void)
{ if(mcu_uart_list[4])  lld_uart_irq_rxdma(mcu_uart_list[4]); }

//=======================================  UART6 =======================================//
//void DMA2_Stream6_IRQHandler(void)
//{ if(mcu_uart_list[5])  lld_uart_irq_txdma(mcu_uart_list[5]); }

//void DMA2_Stream2_IRQHandler(void)
//{ if(mcu_uart_list[5])  lld_uart_irq_rxdma(mcu_uart_list[5]); }

////==============================================     I2Cx  IRQHandler 

void I2C1_EV_IRQHandler(void)   //----I2C1 EVENT----//
{	 if(mcu_i2c_list[0])  lld_i2c_irq_event(mcu_i2c_list[0]);  }
void I2C1_ER_IRQHandler(void)   //----I2C1 ERROR----//
{	 if(mcu_i2c_list[0])  lld_i2c_irq_error(mcu_i2c_list[0]);  }
void I2C2_EV_IRQHandler(void)   //----I2C2 EVENT----//
{	 if(mcu_i2c_list[1])  lld_i2c_irq_event(mcu_i2c_list[1]);  }
void I2C2_ER_IRQHandler(void)   //----I2C2 ERROR----//
{	 if(mcu_i2c_list[1])  lld_i2c_irq_error(mcu_i2c_list[1]);  }
void I2C3_EV_IRQHandler(void)   //----I2C2 EVENT----//
{	 if(mcu_i2c_list[2])  lld_i2c_irq_event(mcu_i2c_list[2]);  }
void I2C3_ER_IRQHandler(void)   //----I2C2 ERROR----//
{	 if(mcu_i2c_list[2])  lld_i2c_irq_error(mcu_i2c_list[2]);  }

void TIM1_UP_IRQHandler(void)   //----TIM1 Up-------//
{   if(mcu_timer_list[0]) lld_timer_irq(mcu_timer_list[0]);}
void TIM1_CC_IRQHandler(void)		//----TIM1 CC-------//
{	if(mcu_timer_list[0]) lld_timer_irq(mcu_timer_list[0]);}
void TIM2_IRQHandler(void)      //----TIM2 IRQ------// 
{   if(mcu_timer_list[1]) lld_timer_irq(mcu_timer_list[1]);}
void TIM3_IRQHandler(void)      //----TIM3 IRQ------// 
{	if(mcu_timer_list[2]) lld_timer_irq(mcu_timer_list[2]);}
void TIM4_IRQHandler(void)      //----TIM4 IRQ------// 
{	if(mcu_timer_list[3]) lld_timer_irq(mcu_timer_list[3]);}
void TIM5_IRQHandler(void)      //----TIM5 IRQ------// 
{	if(mcu_timer_list[4]) lld_timer_irq(mcu_timer_list[4]);}
void TIM6_IRQHandler(void)      //----TIM6 IRQ------// 
{	if(mcu_timer_list[5]) lld_timer_irq(mcu_timer_list[5]);	}

void TIM7_IRQHandler(void)      //----TIM7 IRQ------// 
{	if(mcu_timer_list[6]) lld_timer_irq(mcu_timer_list[6]);}
void TIM8_UP_IRQHandler(void)   //----TIM8 Up-------//
{   if(mcu_timer_list[7]) lld_timer_irq(mcu_timer_list[7]);}
void TIM8_CC_IRQHandler(void)		//----TIM8 CC-------//
{	if(mcu_timer_list[7]) lld_timer_irq(mcu_timer_list[7]);}










