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

#include "ledgermanager.h"
#include "logservice.h"
#include "configureexception.h"
#include "initialledgerstate.h"
#include "errorledgerstate.h"

SimActivityService::LedgerManager::LedgerManager():
  LedgerStateUser{InitialLedgerState::instance()},
  pActivityTimeSender_{},
  pUpstreamTimeSender_{},
  u64SequenceNumber_{}
{}

void
SimActivityService::
LedgerManager::setActivityTimeSender(ActivityTimeSender * pActivityTimeSender)
{
  pActivityTimeSender_ = pActivityTimeSender;
}

void
SimActivityService::
LedgerManager::setUpstreamTimeSender(UpstreamTimeSender * pUpstreamTimeSender)
{
  pUpstreamTimeSender_ = pUpstreamTimeSender;
}

void
SimActivityService::LedgerManager::configure(const Participants & participants,
                                             const Microseconds & nudgeMicroseconds)
{
  nudgeMicroseconds_ = nudgeMicroseconds;

  for(const auto & participant : participants)
    {
      for(const auto & action : participant.actions_)
        {
          auto iter = actionActivityLedgerMap_.find(action);

          if(iter == actionActivityLedgerMap_.end())
            {
              iter =
                actionActivityLedgerMap_.emplace(action,
                                                 ActivityLedger{}).first;

              actionsTagMap_.emplace(action,TagSet{});
            }

          actionsTagMap_[action].emplace(participant.sTag_);
        }

      if(!tagActionsMap_.emplace(participant.sTag_,
                                 participant.actions_).second)
        {
          throw ConfigureException{"Error activity participant duplicate"
                                   " tag: %s",
                                   participant.sTag_.c_str()};
        }

      activityParticipantSet_.emplace(participant.sTag_);


    }
}


void
SimActivityService::LedgerManager::handleActivityAction(const uuid_t & uuid,
                                                        const std::string & sTag,
                                                        const TimePoint & now,
                                                        std::uint64_t u64SequenceNumber,
                                                        const std::string & sAction,
                                                        const std::string & sIdentifier)
{
  pLedgerState_->handleActivityAction(this,
                                      pActivityTimeSender_,
                                      simulationTime_,
                                      activityParticipantSet_,
                                      uuid,
                                      sTag,
                                      now,
                                      u64SequenceNumber,
                                      sAction,
                                      sIdentifier,
                                      confirmReadySet_,
                                      confirmNotReadySet_,
                                      earliestWorkTimeMap_,
                                      actionActivityLedgerMap_,
                                      actionsTagMap_,
                                      candidateNextTime_);
}


void
SimActivityService::LedgerManager::handleActivityStatus(const uuid_t & uuid,
                                                        const std::string & sTag,
                                                        const TimePoint & now,
                                                        std::uint64_t u64SequenceNumber,
                                                        const TimePoint & next)
{
  pLedgerState_->handleActivityStatus(this,
                                      pActivityTimeSender_,
                                      simulationTime_,
                                      activityParticipantSet_,
                                      uuid,
                                      sTag,
                                      now,
                                      u64SequenceNumber,
                                      next,
                                      nudgeMicroseconds_,
                                      earliestWorkTimeMap_,
                                      actionActivityLedgerMap_,
                                      tagActionsMap_,
                                      candidateNextTime_,
                                      u64SequenceNumber_);
}



void
SimActivityService::LedgerManager::handleActivityHello(const uuid_t & uuid,
                                                       const std::string & sTag,
                                                       const TimePoint & now,
                                                       std::uint64_t u64SequenceNumber)
{
  pLedgerState_->handleActivityHello(this,
                                     pActivityTimeSender_,
                                     simulationTime_,
                                     activityParticipantSet_,
                                     uuid,
                                     sTag,
                                     now,
                                     u64SequenceNumber,
                                     helloSet_,
                                     u64SequenceNumber_);
}


void
SimActivityService::
LedgerManager::handleActivityTimeConfirmNotice(const uuid_t & uuid,
                                               const std::string & sTag,
                                               const TimePoint & now,
                                               std::uint64_t u64SequenceNumber,
                                               const TimePoint & confirm,
                                               bool bReady,
                                               std::uint64_t u64ReferenceSequenceNumber )
{
  pLedgerState_->handleActivityTimeConfirmNotice(this,
                                                 pActivityTimeSender_,
                                                 pUpstreamTimeSender_,
                                                 activityParticipantSet_,
                                                 uuid,
                                                 sTag,
                                                 now,
                                                 u64SequenceNumber,
                                                 confirm,
                                                 bReady,
                                                 u64ReferenceSequenceNumber,
                                                 confirmReadySet_,
                                                 confirmNotReadySet_,
                                                 nudgeMicroseconds_,
                                                 earliestWorkTimeMap_,
                                                 actionActivityLedgerMap_,
                                                 simulationTime_,
                                                 candidateNextTime_,
                                                 upstreamTimeAskAfterTime_,
                                                 u64SequenceNumber_);
}

void
SimActivityService::
LedgerManager::handleActivityTimeConfirmNow(const uuid_t & uuid,
                                            const std::string & sTag,
                                            const TimePoint & now,
                                            std::uint64_t u64SequenceNumber,
                                            const TimePoint & confirm,
                                            bool bReady,
                                            std::uint64_t u64ReferenceSequenceNumber)
{
  pLedgerState_->handleActivityTimeConfirmNow(this,
                                              pActivityTimeSender_,
                                              activityParticipantSet_,
                                              uuid,
                                              sTag,
                                              now,
                                              u64SequenceNumber,
                                              confirm,
                                              bReady,
                                              u64ReferenceSequenceNumber,
                                              confirmReadySet_,
                                              earliestWorkTimeMap_,
                                              actionActivityLedgerMap_,
                                              simulationTime_,
                                              candidateNextTime_,
                                              u64SequenceNumber_);
}

void
SimActivityService::
LedgerManager::handleUpstreamTimeNow(const TimePoint & now)
{
  pLedgerState_->handleUpstreamTimeNow(this,
                                       pActivityTimeSender_,
                                       pUpstreamTimeSender_,
                                       now,
                                       simulationTime_,
                                       candidateNextTime_,
                                       upstreamTimeAskAfterTime_,
                                       u64SequenceNumber_);
}

void
SimActivityService::
LedgerManager::handleUpstreamTimeNextResponse(bool bSuccess)
{
  pLedgerState_->handleUpstreamTimeNextResponse(this,bSuccess);
}

const SimActivityService::TimePoint &
SimActivityService::LedgerManager::now()
{
  return simulationTime_;
}

void
SimActivityService::LedgerManager::discover()
{
  pActivityTimeSender_->sendActivityHello(u64SequenceNumber_++);
}
