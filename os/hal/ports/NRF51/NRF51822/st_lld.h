/*
    ChibiOS - Copyright (C) 2015 Fabio Utzig

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    st_lld.h
 * @brief   NRF51822 ST subsystem low level driver header.
 * @details This header is designed to be include-able without having to
 *          include other files from the HAL.
 *
 * @addtogroup ST
 * @{
 */

#ifndef _ST_LLD_H_
#define _ST_LLD_H_

#include "halconf.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   System ticks implementer as a timer.
 */
#define NRF51_SYSTEM_TICKS_AS_TIMER    1

/**
 * @brief   System ticks implementer as an rtc.
 */
#define NRF51_SYSTEM_TICKS_AS_RTC      2

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Select the method to generates system ticks
 * @details Possibles values are:
 *            NRF51_SYSTEM_TICKS_AS_TIMER for TIMER0
 *            NRF51_SYSTEM_TICKS_AS_RTC   for RTC0
 */
#if !defined(NRF51_SYSTEM_TICKS) || defined(__DOXYGEN__)
#define NRF51_SYSTEM_TICKS  NRF51_SYSTEM_TICKS_AS_RTC
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if ((NRF51_SYSTEM_TICKS != NRF51_SYSTEM_TICKS_AS_RTC) &&	\
     (NRF51_SYSTEM_TICKS != NRF51_SYSTEM_TICKS_AS_TIMER))
#error "NRF51_SYSTEM_TICKS illegal value"
#endif

#if defined(CH_CFG_ST_TIMEDELTA) && CH_CFG_ST_TIMEDELTA < 5
#error "CH_CFG_ST_TIMEDELTA is too low"
#endif

#if (OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING)
#error "Freeruning (tick-less) mode currently not working"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void st_lld_init(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Driver inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Returns the time counter value.
 *
 * @return              The counter value.
 *
 * @notapi
 */
static inline systime_t st_lld_get_counter(void) {
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_RTC)
  return (systime_t)NRF_RTC0->COUNTER;
#endif
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_TIMER)
  NRF_TIMER0->TASKS_CAPTURE[1] = 1;
  return (systime_t)NRF_TIMER0->CC[1];
#endif
}

/**
 * @brief   Starts the alarm.
 * @note    Makes sure that no spurious alarms are triggered after
 *          this call.
 *
 * @param[in] abstime   the time to be set for the first alarm
 *
 * @notapi
 */
static inline void st_lld_start_alarm(systime_t abstime) {
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_RTC)
  NRF_RTC0->CC[0]               = abstime;
  NRF_RTC0->EVENTS_COMPARE[0]   = 0;
  NRF_RTC0->EVTENSET            = RTC_EVTEN_COMPARE0_Msk;
#endif
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_TIMER)
  NRF_TIMER0->CC[0]             = abstime;
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  NRF_TIMER0->INTENSET          = TIMER_INTENSET_COMPARE0_Msk;
#endif
}

/**
 * @brief   Stops the alarm interrupt.
 *
 * @notapi
 */
static inline void st_lld_stop_alarm(void) {
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_RTC)
  NRF_RTC0->EVTENCLR            = RTC_EVTEN_COMPARE0_Msk;
  NRF_RTC0->EVENTS_COMPARE[0]   = 0;
#endif
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_TIMER)
  NRF_TIMER0->INTENCLR          = TIMER_INTENCLR_COMPARE0_Msk;
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
#endif
}

/**
 * @brief   Sets the alarm time.
 *
 * @param[in] abstime   the time to be set for the next alarm
 *
 * @notapi
 */
static inline void st_lld_set_alarm(systime_t abstime) {
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_RTC)
    NRF_RTC0->CC[0]             = abstime;
#endif
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_TIMER)
    NRF_TIMER0->CC[0]           = abstime;
#endif
}

/**
 * @brief   Returns the current alarm time.
 *
 * @return              The currently set alarm time.
 *
 * @notapi
 */
static inline systime_t st_lld_get_alarm(void) {
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_RTC)
  return (systime_t)NRF_RTC0->CC[0];
#endif
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_TIMER)
  return (systime_t)NRF_TIMER0->CC[0];
#endif
}

/**
 * @brief   Determines if the alarm is active.
 *
 * @return              The alarm status.
 * @retval false        if the alarm is not active.
 * @retval true         is the alarm is active
 *
 * @notapi
 */
static inline bool st_lld_is_alarm_active(void) {
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_RTC)
  return NRF_RTC0->EVTEN & RTC_INTENSET_COMPARE0_Msk;
#endif
#if (NRF51_SYSTEM_TICKS == NRF51_SYSTEM_TICKS_AS_TIMER)
  return false;
#endif
}

#endif /* _ST_LLD_H_ */

/** @} */
