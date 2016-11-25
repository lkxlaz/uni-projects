/*
/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the real time clock (RTC).
 *
 *  @author Angze Li, Lin Tao
 *  @date 2016-09-11
 */
/*!
**  @addtogroup RTC_module RTC module documentation
**  @{
*/
/* MODULE RTC */
// new types
#include "types.h"
#include "RTC.h"
#include "MK70F12.h"

OS_ECB *RTCCallbackSemaphore;

BOOL RTC_Init()
{
  //enable the clock gate to the RTC module
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  //configure the required load capacitance (18pf)
  RTC_CR |= (RTC_CR_SC2P_MASK | RTC_CR_SC16P_MASK);

  //enable the 32.768 KHZ crystal oscillator
  RTC_CR |= RTC_CR_OSCE_MASK;

  //lock the control register
  RTC_LR |= RTC_LR_CRL_MASK;

  //enable the time counter and set the time second register start from 0
  RTC_SR |= RTC_SR_TCE_MASK;
 // RTC_TSR = 0;

  //enable the time second interrupt
  RTC_IER |= RTC_IER_TSIE_MASK;

  // initialize NVIC
  // Vector 0x53, IRQ = 67, location = IRQ mod 32 = 3
  // NVIC non-IPR=2 IPR=16
  // clear pending interrupts
  NVICICPR2 = NVIC_ICPR_CLRPEND(1 << 3);
  //Enable interrupts
  NVICISER2 = NVIC_ISER_SETENA(1 << 3);

  RTCCallbackSemaphore = OS_SemaphoreCreate(0);

  return bTRUE;
}


void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
  uint32_t initSeconds;
  initSeconds = (uint32_t)(hours * 3600 + minutes * 60 + seconds);

  //disable the time counter before writing to TSR
  //clear SR[TOF] SR[TIF] bit
  RTC_SR &= ~RTC_SR_TCE_MASK;
  RTC_SR &= ~RTC_SR_TOF_MASK;
  RTC_SR &= ~RTC_SR_TIF_MASK;

  //write the initial seconds to time second register
  RTC_TSR = initSeconds;

  //enable the time counter when finished writing
  RTC_SR |= RTC_SR_TCE_MASK;

}


void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  uint32_t currentTimeinSeconds;
  currentTimeinSeconds = RTC_TSR;
  *minutes = currentTimeinSeconds % 3600 / 60;
  *hours = currentTimeinSeconds / 3600;
  *seconds = currentTimeinSeconds % 60;
}


void __attribute__ ((interrupt)) RTC_ISR(void)
{
  OS_ISREnter();
  OS_SemaphoreSignal(RTCCallbackSemaphore);
  OS_ISRExit();
}
/* END RTC */
/*!
** @}
*/






