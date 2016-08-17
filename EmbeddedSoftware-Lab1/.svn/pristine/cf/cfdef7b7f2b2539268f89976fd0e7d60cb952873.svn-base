/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author ANGZE LI, LIN TAO
 *  @date 2016-08-07
 */

// new types
#include "types.h"
#include "UART.h"
#include "MK70F12.h"
#include "FIFO.h"

static TFIFO TxFIFO, RxFIFO;

BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  uint16_t ubd, brfa;
  uint8_t temp;

  //enable the clock gate to UART2 and PORTE
  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

  // PORTE_PCR17: MUX=3
  PORTE_PCR17 = PORT_PCR_MUX(3);
  // PORTE_PCR16: MUX=3
  PORTE_PCR16 = PORT_PCR_MUX(3);

  //disable the transmitter and receiver while changing the configuration
  UART2_C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);

  /* Configure the UART for 8-bit mode, no parity */
  /* We need all default settings, so entire register is cleared */
  UART2_C1 = 0;


  /* Calculate baud settings */
  ubd = (uint16_t)((moduleClk)/(baudRate * 16));
  /* Save off the current value of the UARTx_BDH except for the SBR */
  temp = UART2_BDH & ~(UART_BDH_SBR(0x1F));
  UART2_BDH = temp |  UART_BDH_SBR(((ubd & 0x1F00) >> 8));
  UART2_BDL = (uint8_t)(ubd & UART_BDL_SBR_MASK);
  /* Determine if a fractional divider is needed to get closer to the baud rate */
  brfa = (((moduleClk*32)/(baudRate * 16)) - (ubd * 32));
  /* Save off the current value of the UARTx_C4 register except for the BRFA */
  temp = UART2_C4 & ~(UART_C4_BRFA(0x1F));
  UART2_C4 = temp |  UART_C4_BRFA(brfa);

  //enable the transmitter and receiver
  UART2_C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK);

  // Initialize FIFOs
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);

  return bTRUE;
}



BOOL UART_InChar(uint8_t * const dataPtr)
{
  return FIFO_Get(&RxFIFO, dataPtr);
}


BOOL UART_OutChar(const uint8_t data)
{
  return FIFO_Put(&TxFIFO, data);
}


void UART_Poll(void)
{
  //Receive data
  if (UART2_S1 & UART_S1_RDRF_MASK)
    FIFO_Put(&RxFIFO, UART2_D);

  //transmit data
  if (UART2_S1 & UART_S1_TDRE_MASK)
    FIFO_Get(&TxFIFO, &UART2_D); //warning?????????????????????(uint8_t *) &UART2_D
}


