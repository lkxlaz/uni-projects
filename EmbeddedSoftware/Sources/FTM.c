/*! @file
 *
 *  @brief Routines for setting up the flexible timer module (FTM) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the flexible timer module (FTM).
 *
 *  @author LIN TAO, ANGZE LI
 *  @date 2016-09-19
 */
/*!
**  @addtogroup FTM_module FTM module documentation
**  @{
*/
/* MODULE FTM */
// new types
#include "types.h"
#include "MK70F12.h"
#include "FTM.h"
#include "OS.h"

#define CHANNELNB 8

BOOL FTM_Init()
{
  //Enable the clock gate to the FTM module
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

  if (FTM0_FMS | FTM_FMS_WPEN_MASK)
    // Disable write protection
    FTM0_MODE |= FTM_MODE_WPDIS_MASK;

  //set initial value to the counter
  FTM0_CNTIN   &= ~FTM_CNTIN_INIT_MASK;

  //set modulo value
  FTM0_MOD     |= FTM_MOD_MOD_MASK;

  //set counter
  FTM0_CNT     |= FTM_CNT_COUNT(1);

  //select the fixed frequency clock(10)
  FTM0_SC |= FTM_SC_CLKS(2);

  // Setup for only input capture / output compare
  // Up counting mode
  FTM0_SC      &= ~FTM_SC_CPWMS_MASK;
  FTM0_QDCTRL  &= ~FTM_QDCTRL_QUADEN_MASK;
  // No CHANNELNB are linked
  FTM0_COMBINE  = 0;

  // Enable FTM
  FTM0_MODE    |= FTM_MODE_FTMEN_MASK;

  FTM0_FMS     |= FTM_FMS_WPEN_MASK;

  //setup NVIC
  //IRQ = 62, location = IRQ mod 32 = 30;
  //NVIC non-IPR=1 IPR=15
  //clear pending interrupts
  NVICICPR1    |= (1 << 30);
  NVICISER1    |= (1 << 30);

  FTMCh0Semaphore = OS_SemaphoreCreate(0);
  FTMCh1Semaphore = OS_SemaphoreCreate(0);

  return bTRUE;

}


BOOL FTM_Set(const TFTMChannel* const aTimer)
{
  if (aTimer->timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
  {
    if (aTimer->ioType.outputAction == TIMER_OUTPUT_DISCONNECT)
    {
      // Disable write protection
      if (FTM0_FMS | FTM_FMS_WPEN_MASK)
        FTM0_MODE |= FTM_MODE_WPDIS_MASK;

      FTM0_CnSC(aTimer->channelNb) &= ~FTM_CnSC_MSB_MASK;
      FTM0_CnSC(aTimer->channelNb) |= FTM_CnSC_MSA_MASK;
      FTM0_CnSC(aTimer->channelNb) &= ~FTM_CnSC_ELSB_MASK;
      FTM0_CnSC(aTimer->channelNb) &= ~FTM_CnSC_ELSA_MASK;

      // Enable write protection
      FTM0_FMS |= FTM_FMS_WPEN_MASK;
	  
      return bTRUE;
    }
  }
  else
    return bFALSE;
}



BOOL FTM_StartTimer(const TFTMChannel* const aTimer)
{

  if (aTimer -> timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
  {
    // set channel to generate 1 sec interrupt
    FTM0_CnV (aTimer -> channelNb ) =   FTM0_CNT + aTimer->delayCount;
	
    // Clear status flag of channel
    FTM0_STATUS &= ~(1 << aTimer->channelNb);

    // Enable interrupt
    FTM0_CnSC(aTimer->channelNb) |= FTM_CnSC_CHIE_MASK;

    return bTRUE;
  }
  return bFALSE;

}



void __attribute__ ((interrupt)) FTM0_ISR(void)
{
  OS_ISREnter();

  // figure out which channel has been interrupted
  uint8_t channelNb;
  for (channelNb = 0; channelNb < CHANNELNB; channelNb++)
  {
    if ((FTM0_CnSC(channelNb) & FTM_CnSC_CHIE_MASK) && (FTM0_CnSC(channelNb) & FTM_CnSC_CHF_MASK))
    {
       // clear channel interrupt flag
       FTM0_CnSC (channelNb) &=~ FTM_CnSC_CHF_MASK;

       //check which channel is to be signaled
       switch (channelNb)
       {
	 case 0:
	   OS_SemaphoreSignal(FTMCh0Semaphore);
	   break;
	 case 1:
	   OS_SemaphoreSignal(FTMCh1Semaphore);
	   break;
	 default:
	   break;
       }
    }
  }

  OS_ISRExit();
}
/* END FTM */
/*!
** @}
*/
