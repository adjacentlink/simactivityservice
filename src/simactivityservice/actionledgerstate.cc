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

#include "actionledgerstate.h"
#include "logservice.h"
#include "errorledgerstate.h"
#include "timenoticeconfirmledgerstate.h"
#include "statisticpublisher.h"

#include <algorithm>

void
SimActivityService::
ActionLedgerState::handleActivityAction(LedgerStateUser * pLedgerStateUser,
                                        [[maybe_unused]] ActivityTimeSender * pActivityTimeSender,
                                        const TimePoint & simulationTime,
                                        [[maybe_unused]] const TagSet & activityParticipantSet,
                                        const uuid_t & uuid,
                                        const std::string & sTag,
                                        const TimePoint & now,
                                        std::uint64_t u64SequenceNumber,
                                        const std::string & sAction,
                                        const std::string & sIdentifier,
                                        [[maybe_unused]] TagSet & confirmReadySet,
                                        [[maybe_unused]] TagSet & confirmNotReadySet,
                                        EarliestWorkTimeMap & earliestWorkTimeMap,
                                        ActionActivityLedgerMap & actionActivityLedgerMap,
                                        ActionsTagMap & actionsTagMap,
                                        [[maybe_unused]] TimePoint & candidateNextTime)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  if(now == simulationTime)
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "ActionLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " now: %zu"
                                           " sequence: %zu"
                                           " action: %s"
                                           " identifier: %s",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           u64SequenceNumber,
                                           sAction.c_str(),
                                           sIdentifier.c_str());

    }
  else
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "ActionLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " now: %zu"
                                           " sequence: %zu"
                                           " action: %s"
                                           " identifier: %s"
                                           " now is not current simuation time:"
                                           " %zu, unrecoverable",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           u64SequenceNumber,
                                           sAction.c_str(),
                                           sIdentifier.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (simulationTime.time_since_epoch()).count());

      changeState(pLedgerStateUser,ErrorLedgerState::instance());

      return;
    }

  if(auto iter = actionActivityLedgerMap.find(sAction);
     iter != actionActivityLedgerMap.end())
    {
      auto & activityLedger = iter->second;

      auto activityIter = activityLedger.find(sIdentifier);

      if(activityIter == activityLedger.end())
        {
          activityIter = activityLedger.emplace(sIdentifier,
                                                TagSet{}).first;

          // invalidate earliest work time for all participants in the
          // action group
          if(auto iter = actionsTagMap.find(sAction);
             iter != actionsTagMap.end())
            {
              for(const auto & sMemebrTag : iter->second)
                {
                  if(sMemebrTag != sTag)
                    {
                      LogServiceSingleton::instance()->
                        log(DEBUG_LEVEL,
                            "ActionLedgerState::%s"
                            " action: %s"
                            " identifier: %s"
                            " dropping tag %s"
                            " from earliest work set due to new identifier",
                            __func__,
                            sAction.c_str(),
                            sIdentifier.c_str(),
                            sMemebrTag.c_str());

                      earliestWorkTimeMap.erase(sMemebrTag);
                    }
                }
            }
          else
            {
              LogServiceSingleton::instance()->
                log(ERROR_LEVEL,
                    "ActionLedgerState::%s"
                    " action: %s"
                    " identifier: %s"
                    " cannot find action in actions tag map, unrecoverable",
                    __func__,
                    sAction.c_str(),
                    sIdentifier.c_str());

              changeState(pLedgerStateUser,ErrorLedgerState::instance());
            }
        }

      auto & acknowledgeSet = activityIter->second;

      acknowledgeSet.emplace(sTag);

      earliestWorkTimeMap.erase(sTag);

      StatisticPublisher::instance()->updateAction(sAction,
                                                   sIdentifier,
                                                   sTag,
                                                   actionsTagMap,
                                                   simulationTime);
    }
  else
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "ActionLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " now: %zu"
                                           " sequence: %zu"
                                           " action: %s"
                                           " identifier: %s"
                                           " action unknown, unrecoverable",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           u64SequenceNumber,
                                           sAction.c_str(),
                                           sIdentifier.c_str());

      changeState(pLedgerStateUser,ErrorLedgerState::instance());
    }
}

void
SimActivityService::
ActionLedgerState::handleActivityStatus(LedgerStateUser * pLedgerStateUser,
                                        ActivityTimeSender * pActivityTimeSender,
                                        const TimePoint & simulationTime,
                                        const TagSet & activityParticipantSet,
                                        const uuid_t & uuid,
                                        const std::string & sTag,
                                        const TimePoint & now,
                                        std::uint64_t u64SequenceNumber,
                                        const TimePoint & next,
                                        const Microseconds & nudgeMicroseconds,
                                        EarliestWorkTimeMap & earliestWorkTimeMap,
                                        ActionActivityLedgerMap actionActivityLedgerMap,
                                        TagActionsMap & tagActionsMap,
                                        TimePoint & candidateNextTime,
                                        std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  if(now == simulationTime)
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "ActionLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " now: %zu"
                                           " sequence: %zu"
                                           " next: %zu",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           u64SequenceNumber,
                                           std::chrono::duration_cast<Microseconds>
                                           (next.time_since_epoch()).count());
    }
  else
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "ActionLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " now: %zu"
                                           " sequence: %zu"
                                           " next: %zu"
                                           " now is not current simuation time:"
                                           " %zu, unrecoverable",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (now.time_since_epoch()).count(),
                                           std::chrono::duration_cast<Microseconds>
                                           (next.time_since_epoch()).count(),
                                           u64SequenceNumber,
                                           std::chrono::duration_cast<Microseconds>
                                           (simulationTime.time_since_epoch()).count());

      changeState(pLedgerStateUser,ErrorLedgerState::instance());

      return;
    }

  bool bIgnore{};

  if(const auto & iter = tagActionsMap.find(sTag);
     iter !=  tagActionsMap.end())
    {
      const auto  & actions = iter->second;

      for(const auto & action : actions)
        {
          if(const auto & iter =
             actionActivityLedgerMap.find(action);
             iter != actionActivityLedgerMap.end())
            {
              const auto & activityLedger = iter->second;

              for(const auto & entry : activityLedger)
                {
                  const auto & identifier = entry.first;
                  const auto & acknowledgeSet = entry.second;

                  if(!acknowledgeSet.count(sTag))
                    {
                      LogServiceSingleton::instance()
                        ->log(DEBUG_LEVEL,
                              "ActionLedgerState::%s"
                              " uuid: %s"
                              " tag: %s"
                              " next: %zu"
                              " action: %s"
                              " still open"
                              " identifier: %s, ignoring",
                              __func__,
                              pzUUID,
                              sTag.c_str(),
                              std::chrono::duration_cast<Microseconds>
                              (next.time_since_epoch()).count(),
                              action.c_str(),
                              identifier.c_str());

                      bIgnore = true;

                      break;
                    }
                }
            }
          else
            {
              LogServiceSingleton::instance()
                ->log(ERROR_LEVEL,
                      "ActionLedgerState::%s"
                      " uuid: %s"
                      " tag: %s"
                      " next: %zu"
                      " action: %s"
                      " cannot find action in action activity ledger map,"
                      " unrecoverable",
                      __func__,
                      pzUUID,
                      sTag.c_str(),
                      std::chrono::duration_cast<Microseconds>
                      (next.time_since_epoch()).count(),
                      action.c_str());

              bIgnore = true;

              break;

            }
        }
    }
  else
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "ActionLedgerState::%s"
                                           " uuid: %s"
                                           " tag: %s"
                                           " next: %zu"
                                           " cannot find nem id for tag,"
                                           " ignoring",
                                           __func__,
                                           pzUUID,
                                           sTag.c_str(),
                                           std::chrono::duration_cast<Microseconds>
                                           (next.time_since_epoch()).count());

      bIgnore = true;
    }

  static const auto NO_WORK_TIMPOINT = TimePoint{Microseconds{0}};

  if(!bIgnore)
    {
      if(next == NO_WORK_TIMPOINT || next > simulationTime)
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "ActionLedgerState::%s"
                                               " uuid: %s"
                                               " tag: %s"
                                               " now: %zu"
                                               " setting next: %zu",
                                               __func__,
                                               pzUUID,
                                               sTag.c_str(),
                                               std::chrono::duration_cast<Microseconds>
                                               (now.time_since_epoch()).count(),
                                               std::chrono::duration_cast<Microseconds>
                                               (next.time_since_epoch()).count());

          earliestWorkTimeMap[sTag] =
            std::chrono::duration_cast<Microseconds>(next.time_since_epoch()).count();
        }
      else
        {
          LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                               "ActionLedgerState::%s"
                                               " uuid: %s"
                                               " tag: %s"
                                               " now: %zu"
                                               " next time %zu"
                                               " but simulation time: %zu,"
                                               " ignoring",
                                               __func__,
                                               pzUUID,
                                               sTag.c_str(),
                                               std::chrono::duration_cast<Microseconds>
                                               (now.time_since_epoch()).count(),
                                               std::chrono::duration_cast<Microseconds>
                                               (next.time_since_epoch()).count(),
                                               std::chrono::duration_cast<Microseconds>
                                               (simulationTime.time_since_epoch()).count());
        }
    }
  else
    {
      // if there was an earliest work time for this reporter, remove it
      if(earliestWorkTimeMap.erase(sTag))
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "ActionLedgerState::%s"
                                               " uuid: %s"
                                               " tag: %s"
                                               " now: %zu"
                                               " invalidated next time due to"
                                               " new activity",
                                               __func__,
                                               pzUUID,
                                               sTag.c_str(),
                                               std::chrono::duration_cast<Microseconds>
                                               (simulationTime.time_since_epoch()).count());
        }
    }



  std::uint64_t u64EarliestWorkTime{};
  bool bHasValue{false};
  TagSet missingSet{};

  if(earliestWorkTimeMap.size() == activityParticipantSet.size())
    {
      for(const auto & entry : earliestWorkTimeMap)
        {
          if(entry.second &&
             (!bHasValue || entry.second < u64EarliestWorkTime))
            {
              u64EarliestWorkTime = entry.second;
              bHasValue = true;
            }
        }
    }
  else
    {
      missingSet = activityParticipantSet;

      for(const auto & entry : earliestWorkTimeMap)
        {
          missingSet.erase(entry.first);
        }
    }

  if(missingSet.empty() && bHasValue && u64EarliestWorkTime)
    {
      candidateNextTime = TimePoint{Microseconds{u64EarliestWorkTime}};

      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "ActionLedgerState::%s"
                                           " %zu of %zu ready earliest: %zu"
                                           " sending time notice",
                                           __func__,
                                           earliestWorkTimeMap.size(),
                                           activityParticipantSet.size(),
                                           std::chrono::duration_cast<Microseconds>
                                           (candidateNextTime.time_since_epoch()).count());

      changeState(pLedgerStateUser,TimeNoticeConfirmLedgerState::instance());

      pActivityTimeSender->sendActivityTimeNotice(candidateNextTime,
                                                  ++u64LedgerSequenceNumber);
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
                                             "ActionLedgerState::%s"
                                             " %zu of %zu ready  missing:",
                                             __func__,
                                             earliestWorkTimeMap.size(),
                                             activityParticipantSet.size());

      if(missingSet.empty() && !u64EarliestWorkTime)
        {
          if(nudgeMicroseconds != Microseconds::zero())
            {
              candidateNextTime = simulationTime + nudgeMicroseconds;

              LogServiceSingleton::instance()->
                log(DEBUG_LEVEL,
                    "ActionLedgerState::%s all participants have reported no next"
                    " time, so nudge time forward by %zu microseconds, "
                    " simulation time: %zu"
                    " candidate time: %zu",
                    __func__,
                    nudgeMicroseconds.count(),
                    std::chrono::duration_cast<Microseconds>
                    (simulationTime.time_since_epoch()).count(),
                    std::chrono::duration_cast<Microseconds>
                    (candidateNextTime.time_since_epoch()).count());

              changeState(pLedgerStateUser,TimeNoticeConfirmLedgerState::instance());

              pActivityTimeSender->sendActivityTimeNotice(candidateNextTime,
                                                          ++u64LedgerSequenceNumber);
            }
          else
            {
              LogServiceSingleton::instance()->
                log(INFO_LEVEL,
                    "ActionLedgerState::%s all participants have reported no next"
                    " time, no nudge time configured, simulation effectively"
                    " complete, "
                    " simulation time: %zu",
                    __func__,
                    std::chrono::duration_cast<Microseconds>
                    (simulationTime.time_since_epoch()).count());
            }
        }
    }
}

void
SimActivityService::
ActionLedgerState::handleActivityTimeConfirmNotice(LedgerStateUser * pLedgerStateUser,
                                                   [[maybe_unused]] ActivityTimeSender * pActivityTimeSender,
                                                   [[maybe_unused]] UpstreamTimeSender * pUpstreamTimeSender,
                                                   [[maybe_unused]] const TagSet & activityParticipantSet,
                                                   const uuid_t & uuid,
                                                   const std::string & sTag,
                                                   const TimePoint & now,
                                                   std::uint64_t u64SequenceNumber,
                                                   const TimePoint & confirm,
                                                   bool bReady,
                                                   std::uint64_t u64ReferenceSequenceNumber,
                                                   [[maybe_unused]] TagSet & confirmReadySet,
                                                   [[maybe_unused]] TagSet & confirmNotReadySet,
                                                   [[maybe_unused]] const Microseconds & nudgeMicroseconds,
                                                   EarliestWorkTimeMap & earliestWorkTimeMap,
                                                   [[maybe_unused]] ActionActivityLedgerMap & actionActivityLedgerMap,
                                                   TimePoint & simulationTime,
                                                   [[maybe_unused]] TimePoint & candidateNextTime,
                                                   [[maybe_unused]] TimePoint & upstreamTimeAskAfterTime,
                                                   std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  if(now == simulationTime)
    {
      if(u64LedgerSequenceNumber != u64ReferenceSequenceNumber)
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "ActionLedgerState::%s"
                                               " uuid: %s"
                                               " tag: %s"
                                               " now: %zu"
                                               " sequence: %zu"
                                               " confirm: %zu"
                                               " ready: %s,"
                                               " reference: %zu"
                                               " current confirm sequence: %zu"
                                               " mismatch, ignoring",
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
      else
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "ActionLedgerState::%s"
                                               " uuid: %s"
                                               " tag: %s"
                                               " now: %zu"
                                               " sequence: %zu"
                                               " confirm: %zu"
                                               " ready: %s,"
                                               " reference: %zu"
                                               " removing earliest work time: %s",
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
                                               bReady ? "no" : "yes" );
          if(!bReady)
            {
              earliestWorkTimeMap.erase(sTag);
            }
        }
    }
  else
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "ActionLedgerState::%s"
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


const char *
SimActivityService::ActionLedgerState::name()
{
  return "ActionLedgerState";
}
