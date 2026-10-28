/*
 * Copyright (c) 2025-2026 - Adjacent Link LLC, Bridgewater, New Jersey
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

#include "timenoticeconfirmledgerstate.h"
#include "logservice.h"
#include "timenoticecancelledgerstate.h"
#include "timenowconfirmledgerstate.h"
#include "actionledgerstate.h"
#include "errorledgerstate.h"
#include "upstreamtimenowwaitledgerstate.h"
#include "statisticpublisher.h"

#include <algorithm>

void
SimActivityService::
TimeNoticeConfirmLedgerState::
handleActivityTimeConfirmNotice(LedgerStateUser * pLedgerStateUser,
                                ActivityTimeSender * pActivityTimeSender,
                                UpstreamTimeSender * pUpstreamTimeSender,
                                const TagSet & activityParticipantSet,
                                const uuid_t & uuid,
                                const std::string & sTag,
                                const TimePoint & now,
                                std::uint64_t u64SequenceNumber,
                                const TimePoint & confirm,
                                bool bReady,
                                std::uint64_t u64ReferenceSequenceNumber,
                                TagSet & confirmReadySet,
                                TagSet & confirmNotReadySet,
                                [[maybe_unused]] const Microseconds & nudgeMicroseconds,
                                EarliestWorkTimeMap & earliestWorkTimeMap,
                                ActionActivityLedgerMap & actionActivityLedgerMap,
                                TimePoint & simulationTime,
                                TimePoint & candidateNextTime,
                                TimePoint & upstreamTimeAskAfterTime,
                                std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  if(now == simulationTime)
    {
      if(u64LedgerSequenceNumber != u64ReferenceSequenceNumber)
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "TimeNoticeConfirmLedgerState::%s"
                                               " uuid: %s"
                                               " tag: %s"
                                               " now: %zu"
                                               " sequence: %zu"
                                               " confirm: %zu"
                                               " ready: %s"
                                               " reference: %zu"
                                               " current confirm sequence: %zu mismatch,"
                                               " ignoring",
                                               __func__,
                                               pzUUID,
                                               sTag.c_str(),
                                               std::chrono::duration_cast<Microseconds>
                                               (now.time_since_epoch()).count(),
                                               u64SequenceNumber,
                                               std::chrono::duration_cast<Microseconds>
                                               (confirm.time_since_epoch()).count(),
                                               bReady ? "yes" : "no",
                                               u64ReferenceSequenceNumber,
                                               u64LedgerSequenceNumber);

          return;
        }

      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "TimeNoticeConfirmLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " now: %zu"
                                           " sequence: %zu"
                                           " confirm: %zu"
                                           " ready: %s"
                                           " reference: %zu",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           u64SequenceNumber,
                                           std::chrono::duration_cast<Microseconds>
                                           (confirm.time_since_epoch()).count(),
                                           bReady ? "yes" : "no",
                                           u64ReferenceSequenceNumber);

      if(bReady)
        {
          confirmReadySet.emplace(sTag);
        }
      else
        {
          confirmNotReadySet.emplace(sTag);

          earliestWorkTimeMap.erase(sTag);

          TagSet confirmSet{};

          std::set_union(confirmReadySet.begin(),
                         confirmReadySet.end(),
                         confirmNotReadySet.begin(),
                         confirmNotReadySet.end(),
                         std::inserter(confirmSet,
                                       confirmSet.begin()));

          TagSet missingSet{};

          std::set_difference(activityParticipantSet.begin(),
                              activityParticipantSet.end(),
                              confirmSet.begin(),
                              confirmSet.end(),
                              std::inserter(missingSet,
                                            missingSet.begin()));


          if(missingSet.empty())
            {
              // no more confirms so we go to ActionLedgerState
              confirmReadySet.clear();
              confirmNotReadySet.clear();

              changeState(pLedgerStateUser,
                          ActionLedgerState::instance());
            }
          else
            {
              // still waiting for confirms so go to
              // TimeNoticeCancelLedgerState

              LogServiceSingleton::instance()->logfn(DEBUG_LEVEL,
                                                     [&missingSet]()
                                                     {
                                                       std::list<std::string> s;

                                                       for(const auto & sTag : missingSet)
                                                         {
                                                           s.push_back(sTag);
                                                         }

                                                       return s;
                                                     },
                                                     "TimeNoticeConfirmLedgerState::%s"
                                                     " missing confirm %zu of %zu,"
                                                     " missing:",
                                                     __func__,
                                                     missingSet.size(),
                                                     activityParticipantSet.size());

              changeState(pLedgerStateUser,
                          TimeNoticeCancelLedgerState::instance());
            }

          return;
        }

      TagSet missingSet{};

      std::set_difference(activityParticipantSet.begin(),
                          activityParticipantSet.end(),
                          confirmReadySet.begin(),
                          confirmReadySet.end(),
                          std::inserter(missingSet,
                                        missingSet.begin()));

      if(missingSet.empty())
        {
          confirmReadySet.clear();

          earliestWorkTimeMap.clear();

          for(auto & entry : actionActivityLedgerMap)
            {
              entry.second.clear();
            }

          if(pUpstreamTimeSender)
            {
              // we have to ask for upstream time unless were were
              // given permission to move forward until a certain
              // point in time by receiving an upstream time now
              // greater than what we asked for in our last request
              if(candidateNextTime > upstreamTimeAskAfterTime)
                {
                  pUpstreamTimeSender->sendUpstreamTimeNext(candidateNextTime);

                  changeState(pLedgerStateUser,
                              UpstreamTimeNowWaitLedgerState::instance());

                  // now we wait
                  return;
                }
            }

          simulationTime = candidateNextTime;

          candidateNextTime = TimePoint{};

          StatisticPublisher::instance()->updateTime(simulationTime);

          pActivityTimeSender->sendActivityTimeNow(simulationTime,
                                                   ++u64LedgerSequenceNumber);

          changeState(pLedgerStateUser,TimeNowConfirmLedgerState::instance());
        }
      else
        {
          LogServiceSingleton::instance()->logfn(DEBUG_LEVEL,
                                                 [&missingSet]()
                                                 {
                                                   std::list<std::string> s;

                                                   for(const auto & sTag : missingSet)
                                                     {
                                                       s.push_back(sTag);
                                                     }

                                                   return s;
                                                 },
                                                 "TimeNoticeConfirmLedgerState::%s"
                                                 " missing confirm %zu of %zu,"
                                                 " missing:",
                                                 __func__,
                                                 missingSet.size(),
                                                 activityParticipantSet.size());
        }
    }
  else
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "TimeNoticeConfirmLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " now: %zu"
                                           " sequence: %zu"
                                           " confirm: %zu"
                                           " ready: %s"
                                           " reference: %zu"
                                           " simulation: %zu time mismatch,"
                                           " unrecoverable",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           u64SequenceNumber,
                                           std::chrono::duration_cast<Microseconds>
                                           (confirm.time_since_epoch()).count(),
                                           bReady ? "yes" : "no",
                                           u64ReferenceSequenceNumber,
                                           std::chrono::duration_cast<Microseconds>
                                           (simulationTime.time_since_epoch()).count());

      changeState(pLedgerStateUser,ErrorLedgerState::instance());
    }
}

void
SimActivityService::
TimeNoticeConfirmLedgerState::handleActivityAction(LedgerStateUser * pLedgerStateUser,
                                                   ActivityTimeSender * pActivityTimeSender,
                                                   const TimePoint & simulationTime,
                                                   const TagSet & activityParticipantSet,
                                                   const uuid_t & uuid,
                                                   const std::string & sTag,
                                                   const TimePoint & now,
                                                   std::uint64_t u64SequenceNumber,
                                                   const std::string & sAction,
                                                   const std::string & sIdentifier,
                                                   TagSet & confirmReadySet,
                                                   TagSet & confirmNotReadySet,
                                                   EarliestWorkTimeMap & earliestWorkTimeMap,
                                                   ActionActivityLedgerMap & actionActivityLedgerMap,
                                                   ActionsTagMap & actionsTagMap,
                                                   TimePoint & candidateNextTime)
{
  // an action message prior to confirm means we need to return to action
  candidateNextTime = TimePoint{};

  changeState(pLedgerStateUser,TimeNoticeCancelLedgerState::instance());

  TimeNoticeCancelLedgerState::instance()->handleActivityAction(pLedgerStateUser,
                                                                pActivityTimeSender,
                                                                simulationTime,
                                                                activityParticipantSet,
                                                                uuid,
                                                                sTag,
                                                                now,
                                                                u64SequenceNumber,
                                                                sAction,
                                                                sIdentifier,
                                                                confirmReadySet,
                                                                confirmNotReadySet,
                                                                earliestWorkTimeMap,
                                                                actionActivityLedgerMap,
                                                                actionsTagMap,
                                                                candidateNextTime);
}

const char *
SimActivityService::TimeNoticeConfirmLedgerState::name()
{
  return "TimeNoticeConfirmLedgerState";
}
