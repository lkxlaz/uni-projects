/*! @file
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the periodic interrupt timer (PIT).
 *
 *  @author LIN TAO ANGZE LI
 *  @date 2016-09-18
 */
/*!
**  @addtogroup PIT_module PIT module documentation
**  @{
*/
/* MODULE PIT */
// new types
#include "types.h"
#include "PIT.h"
#include "MK70F12.h"
#include "OS.h"

void (*userFunctionPIT)(void*);
void* userArgumentsPIT;
uint32_t ModuleCLK;

OS_ECB *PITCallbackSemaphore;


BOOL PIT_Init(const uint32_t moduleClk)
{
  //Enable the clock gate to the PIT module
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

  //Activate the PIT before any other steps is done
  PIT_MCR &= ~PIT_MCR_MDIS_MASK;
  //freezes the timer when debugging
  PIT_MCR |= PIT_MCR_FRZ_MASK;

  //clear the flag
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
  //Enable the interrupt
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;

  //setup NVIC
  //IRQ = 68, location = IRQ mod 32 = 4;
  //NVIC non-IPR=2 IPR=17
  //clear pending interrupts
  NVICICPR2 = NVIC_ICPR_CLRPEND(1 << 4);
  //Enable interrupts
  NVICISER2 = NVIC_ISER_SETENA(1 << 4);

  //userFunctionPIT = userFunction;
  //userArgumentsPIT = userArguments;
  ModuleCLK = moduleClk;

  PITCallbackSemaphore = OS_SemaphoreCreate(0);

  return bTRUE;
}


void PIT_Set(const uint32_t period, const BOOL restart)
{
  if (restart)
  {
    //disable the PIT
    PIT_Enable(bFALSE);

    //set up the start time value
    //period * moduleClk - 1 = LDVAL
    PIT_LDVAL0 = period * 0.001 * ModuleCLK- 1;

    //enable the PIT
    PIT_Enable(bTRUE);
  }
  else
    PIT_LDVAL0 = period * 0.001 * ModuleCLK- 1;
}


void PIT_Enable(const BOOL enable)
{
  if (enable)
    PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
  else
    PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
}


void __attribute__ ((interrupt)) PIT_ISR(void)
{

  //clear the flag
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
  OS_ISREnter();
  OS_SemaphoreSignal(PITCallbackSemaphore);
  OS_ISRExit();

}
/* END PIT */
/*!
** @}
*/


