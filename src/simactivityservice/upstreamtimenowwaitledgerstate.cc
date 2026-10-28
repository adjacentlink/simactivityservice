/*
 * Copyright (c) 2025=2026 - Adjacent Link LLC, Bridgewater, New Jersey
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of Adjacent Link LLC nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "upstreamtimenowwaitledgerstate.h"
#include "timenowconfirmledgerstate.h"
#include "errorledgerstate.h"
#include "logservice.h"
#include "statisticpublisher.h"

void
SimActivityService::
UpstreamTimeNowWaitLedgerState::handleUpstreamTimeNow(LedgerStateUser * pLedgerStateUser,
                                                      ActivityTimeSender * pActivityTimeSender,
                                                      UpstreamTimeSender * pUpstreamTimeSender,
                                                      const TimePoint & now,
                                                      TimePoint & simulationTime,
                                                      TimePoint & candidateNextTime,
                                                      TimePoint & upstreamTimeAskAfterTime,
                                                      std::uint64_t & u64LedgerSequenceNumber)
{
  const static TimePoint MIN_TIME{Microseconds{0}};

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "UpstreamTimeNowWaitLedgerState::%s"
                                       " now: %zu"
                                       " next: %zu",
                                       __func__,
                                       std::chrono::duration_cast<Microseconds>
                                       (now.time_since_epoch()).count(),
                                       std::chrono::duration_cast<Microseconds>
                                       (candidateNextTime.time_since_epoch()).count());

  if(candidateNextTime == MIN_TIME)
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "UpstreamTimeNowWaitLedgerState::%s"
                                           " first update time now. Adjusting"
                                           " candidate next time to now",
                                           __func__);

      candidateNextTime = now;
    }

  // sanity check, now cannot be in the past
  if(now < simulationTime)
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "UpstreamTimeNowWaitLedgerState::%s"
                                           " now: %zu"
                                           " next: %zu"
                                           " simulation: %zu"
                                           " now < simulation, unrecoverable",
                                           __func__,
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           std::chrono::duration_cast<Microseconds>
                                           (candidateNextTime.time_since_epoch()).count(),
                                           std::chrono::duration_cast<Microseconds>
                                           (simulationTime.time_since_epoch()).count());

      changeState(pLedgerStateUser,ErrorLedgerState::instance());

      return;
    }

  if(now < candidateNextTime)
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "UpstreamTimeNowWaitLedgerState::%s"
                                           " now: %zu"
                                           " next: %zu now < next, request"
                                           " upstream time next again",
                                           __func__,
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           std::chrono::duration_cast<Microseconds>
                                           (candidateNextTime.time_since_epoch()).count());

      if(pUpstreamTimeSender)
        {
          // send another upstream time next
          pUpstreamTimeSender->sendUpstreamTimeNext(candidateNextTime);
        }
      else
        {
          LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                               "UpstreamTimeNowWaitLedgerState::%s"
                                               " cannot send upstream time next"
                                               " with an invalid upstream time"
                                               " sender, unrecoverable",
                                               __func__);

          changeState(pLedgerStateUser,ErrorLedgerState::instance());

          return;
        }
    }
  else
    {
      if(now > candidateNextTime)
        {
          LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                               "UpstreamTimeNowWaitLedgerState::%s"
                                               " now: %zu"
                                               " next: %zu now > next, will not ask"
                                               " again until then",
                                               __func__,
                                               std::chrono::duration_cast<Microseconds>
                                               (now.time_since_epoch()).count(),
                                               std::chrono::duration_cast<Microseconds>
                                               (candidateNextTime.time_since_epoch()).count());
        }

      upstreamTimeAskAfterTime = now;

      simulationTime = candidateNextTime;

      candidateNextTime = TimePoint{};

      StatisticPublisher::instance()->updateTime(simulationTime);

      pActivityTimeSender->sendActivityTimeNow(simulationTime,
                                               ++u64LedgerSequenceNumber);

      changeState(pLedgerStateUser,TimeNowConfirmLedgerState::instance());
    }
}

const char *
SimActivityService::UpstreamTimeNowWaitLedgerState::name()
{
  return "UpstreamTimeNowWaitLedgerState";
}
