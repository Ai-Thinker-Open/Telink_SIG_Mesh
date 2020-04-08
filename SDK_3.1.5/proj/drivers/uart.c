/********************************************************************************************************
 * @file     uart.c 
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#include "uart.h"
#include "proj/tl_common.h"
//#include "../mcu/watchdog_i.h"

#define     STARTTX         (reg_dma_tx_rdy0 |= BIT(1))                     //trigger dma1 channel to transfer.dma1 is the uart tx channel
#define     TXDONE          ((reg_uart_status1 & FLD_UART_TX_DONE) ? 1:0)   //1:uart module has send all data.0:still has data to send
#define     RXERRORCLR      (reg_uart_status0  |= FLD_UART_RX_ERR_CLR)      //if uart module occur error,this bit can clear error flag bit.
#define     RXERROR         ((reg_uart_status0 & FLD_UART_RX_ERR_FLAG)? 1:0)//uart module error status flag bit.


#if(MCU_CORE_TYPE == MCU_CORE_8266)
#define UART_CONTINUE_DELAY_EN          1
#else
#define UART_CONTINUE_DELAY_EN          0
#endif

volatile unsigned char uart_tx_busy_flag = 0;                   // must "volatile"
static unsigned char *tx_buff = NULL;
#if(UART_CONTINUE_DELAY_EN)
static volatile unsigned int uart_continue_delay_time = 0;      // must "volatile"
static unsigned int baudrate_set = 0;
#endif

/**********************************************************
*
*	@brief	reset uart module
*	
*	@param	none
*
*	@return	none
*/
void uart_Reset(void){

	write_reg8(0x800061,0x80);
	write_reg8(0x800061,0x00);
	
}
/**********************************************************
*	
*	@brief	clear error state of uart rx, maybe used when application detected UART not work
*
*	@parm	none
*
*	@return	'1' RX error flag rised and cleard success; '0' RX error flag not rised 
*
*/
unsigned char uart_ErrorCLR(void){
	if(RXERROR){
		RXERRORCLR;
		return 1;
	}
	return 0;
}

void uart_set_tx_busy_flag(){
    uart_tx_busy_flag = 1;
    #if(UART_CONTINUE_DELAY_EN)
    uart_continue_delay_time = 0;
    #endif
}

void uart_clr_tx_busy_flag(){
    #if(UART_CONTINUE_DELAY_EN)
    uart_continue_delay_time = clock_time() | 1; // make sure not zero
    #else
    uart_tx_busy_flag = 0;
    #endif
}

unsigned char uart_tx_is_busy(){
#if(UART_CONTINUE_DELAY_EN)
    return uart_tx_busy_flag;
#else
    return (!TXDONE);
#endif
}

/*******************************************************
*
*	@brief	uart initiate, set uart clock divider, bitwidth and the uart work mode
*
*	@param	uartCLKdiv - uart clock divider
*			bwpc - bitwidth, should be set to larger than 2
*			en_rx_irq - '1' enable rx irq; '0' disable.
*			en_tx_irq - enable tx irq; '0' disable.
*			hdwC - enum variable of hardware control functions
*
*	@return	'1' set success; '0' set error probably bwpc smaller than 3.
*
*		BaudRate = sclk/((uartCLKdiv+1)*(bwpc+1))  
*		SYCLK = 16Mhz
		115200		9			13
		9600		103			15
*
*		SYCLK = 32Mhz
*		115200		19			13
		9600		237			13	
*/

unsigned char uart_Init(unsigned short uartCLKdiv, unsigned char bwpc,unsigned char en_rx_irq,unsigned char en_tx_irq)
{

	if(bwpc<3)
		return 0;

    unsigned int  baudrate = (CLOCK_SYS_CLOCK_HZ / (uartCLKdiv + 1)) / (bwpc + 1);
	#if(UART_CONTINUE_DELAY_EN)
	baudrate_set = baudrate;
	#endif
	write_reg16(0x800094,(uartCLKdiv|0x8000));//set uart clk divider and enable clock divider
	write_reg8(0x800096,(0x30|bwpc));//set bit width and enable rx/tx DMA
	
	#if (UART_115200RX_TIMEOUT_2MS_EN)
	if(baudrate > 100000){          // 115200
    	write_reg8(0x80009a,0xff);
    	write_reg8(0x80009b,3);
	}else
	#endif
	{
    	write_reg8(0x80009a,(bwpc+1)*12);//one byte includes 12 bits at most
    	write_reg8(0x80009b,1);//For save
	}
	
	write_reg8(0x800097,0x00);//No clts and rts

	//receive DMA and buffer details
	write_reg8(0x800503,0x01);//set DMA 0 mode to 0x01 for receive
	write_reg8(0x800507,0x00);//DMA1 mode to send

	uart_IRQSourceGet();//clear uart irq
	if(en_rx_irq){
		reg_dma_chn_irq_msk |= 0x01;//open dma1 interrupt mask
		reg_irq_mask |= 0x10;//open dma interrupt mask
		//irq_enable(); // write_reg8(0x800643,0x01);//enable intterupt
	}
	if(en_tx_irq){
		reg_dma_chn_irq_msk |= 0x02;//open dma1 interrupt mask
		reg_irq_mask |= 0x10;//open dma interrupt mask
		//irq_enable(); // write_reg8(0x800643,0x01);//enable intterupt
	}
	return 1;
	
}

/********************************************************************************
*	@brief	uart send data function, this  function tell the DMA to get data from the RAM and start 
*			the DMA send function
*
*	@param	sendBuff - send data buffer
*
*	@return	'1' send success; '0' DMA busy
*/

unsigned char uart_Send(unsigned char* data, unsigned int len){
    extern const u8 UART_TX_LEN_MAX;
    if(len > UART_TX_LEN_MAX){
        return 1;	// skip valid length data, avoid dead loop
    }

	if(tx_buff && TXDONE){
		memcpy(tx_buff, &len, 4);
	    memcpy(tx_buff + 4, data, len);
		reg_dma1_addr = (unsigned short)(unsigned int)tx_buff;//packet data, start address is sendBuff+1
		STARTTX;
		
		return 1;
	}
	return 0;

}

unsigned char uart_tx_busy_check(){
    #if(UART_CONTINUE_DELAY_EN)
    if(uart_tx_is_busy()){      
        if(uart_continue_delay_time && clock_time_exceed(uart_continue_delay_time, (baudrate_set > 100000 ? 800 : 9000))){
            uart_continue_delay_time = 0;    // minimum delay :  115200 delay 600us;  9600 delay 7200us
            uart_tx_busy_flag = 0;
        }else{
            return 1;
        }
    }
    return 0;
    
    #else
	return uart_tx_is_busy();
	#endif
}

void uart_tx_busy_clear(){
}

/****************************************************************************************
*
*	@brief	data receive buffer initiate function. DMA would move received uart data to the address space, uart packet length
*			needs to be no larger than (recBuffLen - 4).
*
*	@param	*recAddr:	receive buffer's address info.
*			recBuffLen:	receive buffer's length, the maximum uart packet length should be smaller than (recBuffLen - 4)
*
*	@return	none
*/

void uart_BuffInit(unsigned char *recAddr, unsigned short recBuffLen, unsigned char *txAddr){
	unsigned char bufLen;
	bufLen = recBuffLen/16;

	write_reg16(0x800500,(unsigned short)((unsigned int)(recAddr)));//set receive buffer address
	write_reg8(0x800502,bufLen);//set receive buffer size

	if(txAddr){
    	tx_buff = txAddr;
    }
}

/******************************************************************************
*
*	@brief		get the uart IRQ source and clear the IRQ status, need to be called in the irq process function
*
*	@return		uart_irq_src- enum variable of uart IRQ source, 'UARTRXIRQ' or 'UARTTXIRQ'
*
*/
u8 uart_IRQSourceGet(void){
	unsigned char irqS;
	irqS = reg_dma_rx_rdy0;
	reg_dma_rx_rdy0 = irqS;
#if 0
	if(irqS & 0x01)
		return UARTRXIRQ;
	
	if(irqS & 0x02)
		return UARTTXIRQ;

    return UARTNOIRQ;
#else
    return (irqS & UARTIRQ_MASK);
#endif    
}

void uart_io_init(unsigned char uart_io_sel){
#if(MCU_CORE_TYPE == MCU_CORE_8266)
	uart_io_sel = uart_io_sel;
	gpio_set_func(GPIO_UTX, AS_UART);
	gpio_set_func(GPIO_URX, AS_UART);
#elif((MCU_CORE_TYPE == MCU_CORE_8267) || (MCU_CORE_TYPE == MCU_CORE_8269))
    if(UART_GPIO_8267_PA6_PA7 == uart_io_sel){
        UART_GPIO_CFG_PA6_PA7();
    }else if(UART_GPIO_8267_PC2_PC3 == uart_io_sel){
        UART_GPIO_CFG_PC2_PC3();
		gpio_setup_up_down_resistor(GPIO_PC2,PM_PIN_PULLUP_10K);
		gpio_setup_up_down_resistor(GPIO_PC3,PM_PIN_PULLUP_10K);
    }else if(UART_GPIO_8267_PB2_PB3 == uart_io_sel){
        UART_GPIO_CFG_PB2_PB3();
    }	 
#endif
}

// ------------------kma----
/********************************************************************************
*	@brief	uart send data function, this  function tell the DMA to get data from the RAM and start
*			the DMA send function
*
*	@param	sendBuff - send data buffer
*
*	@return	'1' send success; '0' DMA busy
*/
unsigned char uart_Send_kma(unsigned char* addr){
    unsigned long len = *((unsigned long *)addr);

    if(len > 252){
        return 0;
    }

    if (uart_tx_is_busy ())
    {
    	return 0;
    }

    uart_set_tx_busy_flag();

    write_reg16(0x800504, (u16)(u32)addr);//packet data, start address is sendBuff+1

	STARTTX;

	return 1;
}

/****************************************************************************************
*
*	@brief	data receive buffer initiate function. DMA would move received uart data to the address space, uart packet length
*			needs to be no larger than (recBuffLen - 4).
*
*	@param	*recAddr:	receive buffer's address info.
*			recBuffLen:	receive buffer's length, the maximum uart packet length should be smaller than (recBuffLen - 4)
*
*	@return	none
*/

void uart_RecBuffInit(unsigned char *recAddr, unsigned short recBuffLen){
	uart_BuffInit(recAddr, recBuffLen, 0);
}

/************************Application Example******************
unsigned char recBuff[128];//Declare a receive buffer
void uart_useExample(void ){
	CLK32M_UART9600;
	uart_RecBuffInit(&recBuff,128);
	//Initial IO,UART rx & tx declare
	write_reg8(0x800596,0xC3);
	write_reg8(0x8005B2,0x3C);
}
*/

enum UARTIRQSOURCE uart_IRQSourceGet_kma(void){
	unsigned char irqS;
	irqS = read_reg8(0x800526);
	write_reg8(0x800526,irqS);//CLR irq source
#if(!UART_CONTINUE_DELAY_EN)
	if(irqS & 0x01)	return UARTRXIRQ;
	if(irqS & 0x02)	return UARTTXIRQ;

	return UARTRXIRQ;
#else
	return (irqS & UARTIRQ_MASK);
#endif
}

// --------end

/**
 * @brief UART hardware flow control configuration. Configure RTS pin.
 * @param[in]   enable: enable or disable RTS function.
 * @param[in]   mode: set the mode of RTS(auto or manual).
 * @param[in]   thrsh: threshold of trig RTS pin's level toggle(only for auto mode),
 *                     it means the number of bytes that has arrived in Rx buf.
 * @param[in]   invert: whether invert the output of RTS pin(only for auto mode)
 * @return none
 */
void uart_RTSCfg(unsigned char enable, unsigned char mode, unsigned char thrsh, unsigned char invert)
{
    if (enable) {
        write_reg8(0x800596, read_reg8(0x800596) & (~BIT(4))); //disable GPIOC_GP4 Pin's GPIO function
        write_reg8(0x8005b2, read_reg8(0x8005b2) | BIT(4)); //enable GPIOC_GP4 Pin as RTS Pin
        write_reg8(0x800098, read_reg8(0x800098) | BIT(7)); //enable RTS function
    }
    else {
        write_reg8(0x800596, read_reg8(0x800596) | BIT(4)); //enable GPIOC_GP4 Pin's GPIO function
        write_reg8(0x8005b2, read_reg8(0x8005b2) & (~BIT(4))); //disable GPIOC_GP4 Pin as RTS Pin
        write_reg8(0x800098, read_reg8(0x800098) & (~BIT(7))); //disable RTS function
    }

    if (mode) {
        write_reg8(0x800098, read_reg8(0x800098) | BIT(6));
    }
    else {
        write_reg8(0x800098, read_reg8(0x800098) & (~BIT(6)));
    }

    if (invert) {
        write_reg8(0x800098, read_reg8(0x800098) | BIT(4));
    }
    else {
        write_reg8(0x800098, read_reg8(0x800098) & (~BIT(4)));
    }

    //set threshold
    write_reg8(0x800098, read_reg8(0x800098) & 0xf0);
    write_reg8(0x800098, read_reg8(0x800098) | (thrsh & 0x0f));
}

/**
 * @brief This function sets the RTS pin's level manually
 * @param[in]   polarity: set the output of RTS pin(only for manual mode)
 * @return none
 */
void uart_RTSLvlSet(unsigned char polarity)
{
    if (polarity) {
        write_reg8(0x800098, read_reg8(0x800098) | BIT(5));
    }
    else {
        write_reg8(0x800098, read_reg8(0x800098) & (~BIT(5)));
    }
}

/**
 * @brief UART hardware flow control configuration. Configure CTS pin.
 * @param[in]   enable: enable or disable CTS function.
 * @param[in]   select: when CTS's input equals to select, tx will be stopped
 * @return none
 */
void uart_CTSCfg(unsigned char enable, unsigned char select)
{
    if (enable) {
        write_reg8(0x800596, read_reg8(0x800596) & (~BIT(5))); //disable GPIOC_GP5 Pin's GPIO function
        write_reg8(0x8005b2, read_reg8(0x8005b2) | BIT(5)); //enable GPIOC_GP5 Pin as CTS Pin
        write_reg8(0x800097, read_reg8(0x800097) | BIT(1)); //enable CTS function
    }
    else {
        write_reg8(0x800596, read_reg8(0x800596) | BIT(5)); //enable GPIOC_GP5 Pin's GPIO function
        write_reg8(0x8005b2, read_reg8(0x8005b2) & (~BIT(5))); //disable GPIOC_GP5 Pin as CTS Pin
        write_reg8(0x800097, read_reg8(0x800097) & (~BIT(1))); //disable CTS function
    }

    if (select) {
        write_reg8(0x800097, read_reg8(0x800097) | BIT(0));
    }
    else {
        write_reg8(0x800097, read_reg8(0x800097) & (~BIT(0)));
    }
}
