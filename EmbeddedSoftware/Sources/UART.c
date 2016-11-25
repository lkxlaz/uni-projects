/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author ANGZE LI, LIN TAO
 *  @date 2016-08-07
 */
/*!
 **  @addtogroup UART_module UART module documentation
 **  @{
 */
/* MODULE UART */
// new types
#include "types.h"
#include "UART.h"
#include "MK70F12.h"
#include "FIFO.h"
#include "packet.h"
#include "OS.h"
#include "Cpu.h"

/*! @brief A thread to transmit data
 *
 *  @param parameters used for a thread
 */
static void UART_TxThread(void *pData)
{
  uint8_t xData;
  for (;;)
  {
    OS_SemaphoreWait(TxSemaphore, 0);

    FIFO_Get(&TxFIFO, &xData);
    UART2_D = xData;
    UART2_C2 |= UART_C2_TCIE_MASK;
  }
}

/*! @brief A thread to receive data
 *
 *  @param parameters used for a thread
 */
static void UART_RxThread(void *pData)
{
  for (;;)
  {
    OS_SemaphoreWait(RxSemaphore, 0);
    FIFO_Put(&RxFIFO, UART2_D);
    UART2_C2 |= UART_C2_RIE_MASK;
  }
}



BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //userCallbackFunction = userFunction;
  uint16_t ubd;   /*!< baudrate divisor */
  uint16_t brfa;  /*!< The fractional part of the baudrate divisor */
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
  //enable the interrupts of the receiver
  UART2_C2 |= UART_C2_RIE_MASK;

  // initialize NVIC
  // Vector 0x41 = 65, IRQ = 49, location = IRQ mod 32 = 17
  // NVIC non-IPR=1 IPR=12
  // clear pending interrupts on UART2
  NVICICPR1 = NVIC_ICPR_CLRPEND(1 << 17);
  //Enable interrupts
  NVICISER1 = NVIC_ISER_SETENA(1 << 17);

  // Initialize FIFOs
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);

  //create semaphores
  TxSemaphore = OS_SemaphoreCreate(0);
  RxSemaphore = OS_SemaphoreCreate(0);

  return bTRUE;
}



void UART_InChar(uint8_t * const dataPtr)
{
  FIFO_Get(&RxFIFO, dataPtr);

}


void UART_OutChar(const uint8_t data)
{
  FIFO_Put(&TxFIFO, data);
  UART2_C2 |= UART_C2_TCIE_MASK;
}


void __attribute__ ((interrupt)) UART_ISR(void)
{

  OS_ISREnter();

  // Receive a character
  if (UART2_C2 & UART_C2_RIE_MASK)
  {
    // Clear RDRF flag by reading the status register
    if (UART2_S1 & UART_S1_RDRF_MASK)
    {
      // Semaphore signals the receive
      OS_SemaphoreSignal(RxSemaphore);
      // Disable RIE after bytes have been received
      UART2_C2 &= ~UART_C2_RIE_MASK;
    }
  }

  // Transmit a character
  if (UART2_C2 & UART_C2_TCIE_MASK)
  {
      // Clear TDRE flag by reading the status register
    if (UART2_S1 & UART_S1_TC_MASK)
    {
      // Semaphore signals the receive
      OS_SemaphoreSignal(TxSemaphore);
      // Disable TCIE after bytes have been transmitted
      UART2_C2 &= ~UART_C2_TCIE_MASK;
    }
  }

  OS_ISRExit();
}


/*END UART*/
/*!
 ** @}
 */

