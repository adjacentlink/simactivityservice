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

#include "ledgerstate.h"
#include "ledgerstateuser.h"
#include "errorledgerstate.h"
#include "logservice.h"

SimActivityService::LedgerState::~LedgerState(){}

void
SimActivityService::
LedgerState::handleActivityStatus(LedgerStateUser * pLedgerStateUser,
                                  [[maybe_unused]] ActivityTimeSender * pActivityTimeSender,
                                  [[maybe_unused]] const TimePoint & simulationTime,
                                  [[maybe_unused]] const TagSet & activityParticipantSet,
                                  const uuid_t & uuid,
                                  const std::string & sTag,
                                  const TimePoint & now,
                                  std::uint64_t u64SequenceNumber,
                                  const TimePoint & next,
                                  [[maybe_unused]] const Microseconds & nudgeMicroseconds,
                                  [[maybe_unused]] EarliestWorkTimeMap & earliestWorkTimeMap,
                                  [[maybe_unused]] ActionActivityLedgerMap actionActivityLedgerMap,
                                  [[maybe_unused]] TagActionsMap & tagActionsMap,
                                  [[maybe_unused]] TimePoint & candidateNextTime,
                                  [[maybe_unused]] std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                       "LedgerState::%s"
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

  pLedgerStateUser->changeState(ErrorLedgerState::instance());
}

void
SimActivityService::
LedgerState::handleActivityHello(LedgerStateUser * pLedgerStateUser,
                                 [[maybe_unused]] ActivityTimeSender * pActivityTimeSender,
                                 [[maybe_unused]] const TimePoint & simulationTime,
                                 [[maybe_unused]] const TagSet & activityParticipantSet,
                                 const uuid_t & uuid,
                                 const std::string & sTag,
                                 const TimePoint & now,
                                 std::uint64_t u64SequenceNumber,
                                 [[maybe_unused]] TagSet & helloSet,
                                 [[maybe_unused]] std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                       "LedgerState::%s"
                                       " uuid: %s"
                                       " tag: %s"
                                       " now: %zu"
                                       " sequence: %zu",
                                       __func__,
                                       pzUUID,
                                       sTag.c_str(),
                                       std::chrono::duration_cast<Microseconds>
                                       (now.time_since_epoch()).count(),
                                       u64SequenceNumber);

  pLedgerStateUser->changeState(ErrorLedgerState::instance());
}

void
SimActivityService::
LedgerState::handleActivityTimeConfirmNotice(LedgerStateUser * pLedgerStateUser,
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
                                             [[maybe_unused]] EarliestWorkTimeMap & earliestWorkTimeMap,
                                             [[maybe_unused]] ActionActivityLedgerMap & actionActivityLedgerMap,
                                             [[maybe_unused]] TimePoint & simulationTime,
                                             [[maybe_unused]] TimePoint & candidateNextTime,
                                             [[maybe_unused]] TimePoint & upstreamTimeAskAfterTime,
                                             [[maybe_unused]] std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                       "LedgerState::%s"
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

  pLedgerStateUser->changeState(ErrorLedgerState::instance());
}

void
SimActivityService::
LedgerState::handleActivityTimeConfirmNow(LedgerStateUser * pLedgerStateUser,
                                          [[maybe_unused]] ActivityTimeSender * pActivityTimeSender,
                                          [[maybe_unused]] const TagSet & activityParticipantSet,
                                          const uuid_t & uuid,
                                          const std::string & sTag,
                                          const TimePoint & now,
                                          std::uint64_t u64SequenceNumber,
                                          const TimePoint & confirm,
                                          bool bReady,
                                          std::uint64_t u64ReferenceSequenceNumber,
                                          [[maybe_unused]] TagSet & confirmReadySet,
                                          [[maybe_unused]] EarliestWorkTimeMap & earliestWorkTimeMap,
                                          [[maybe_unused]] ActionActivityLedgerMap & actionActivityLedgerMap,
                                          [[maybe_unused]] TimePoint & simulationTime,
                                          [[maybe_unused]] TimePoint & candidateNextTime,
                                          [[maybe_unused]] std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                       "LedgerState::%s"
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

  pLedgerStateUser->changeState(ErrorLedgerState::instance());
}

void
SimActivityService::
LedgerState::handleActivityAction(LedgerStateUser * pLedgerStateUser,
                                  [[maybe_unused]] ActivityTimeSender * pActivityTimeSender,
                                  [[maybe_unused]] const TimePoint & simulationTime,
                                  [[maybe_unused]] const TagSet & activityParticipantSet,
                                  const uuid_t & uuid,
                                  const std::string & sTag,
                                  const TimePoint & now,
                                  std::uint64_t u64SequenceNumber,
                                  const std::string & sAction,
                                  const std::string & sIdentifier,
                                  [[maybe_unused]] TagSet & confirmReadySet,
                                  [[maybe_unused]] TagSet & confirmNotReadySet,
                                  [[maybe_unused]] EarliestWorkTimeMap & earliestWorkTimeMap,
                                  [[maybe_unused]] ActionActivityLedgerMap & actionActivityLedgerMap,
                                  [[maybe_unused]] ActionsTagMap & actionsTagMap,
                                  [[maybe_unused]] TimePoint & candidateNextTime)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                       "LedgerState::%s"
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

  pLedgerStateUser->changeState(ErrorLedgerState::instance());
}

void
SimActivityService::
LedgerState::handleUpstreamTimeNow(LedgerStateUser * pLedgerStateUser,
                                   [[maybe_unused]] ActivityTimeSender * pActivityTimeSender,
                                   [[maybe_unused]] UpstreamTimeSender * pUpstreamTimeSender,
                                   const TimePoint & now,
                                   [[maybe_unused]] TimePoint & simulationTime,
                                   TimePoint & candidateNextTime,
                                   TimePoint & upstreamTimeAskAfterTime,
                                   [[maybe_unused]] std::uint64_t & u64LedgerSequenceNumber)
{
  LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                       "LedgerState::%s"
                                       " now: %zu"
                                       " next: %zu"
                                       " ask after: %zu",
                                       __func__,
                                       std::chrono::duration_cast<Microseconds>
                                       (now.time_since_epoch()).count(),
                                       std::chrono::duration_cast<Microseconds>
                                       (candidateNextTime.time_since_epoch()).count(),
                                       std::chrono::duration_cast<Microseconds>
                                       (upstreamTimeAskAfterTime.time_since_epoch()).count());

  pLedgerStateUser->changeState(ErrorLedgerState::instance());
}


void
SimActivityService::
LedgerState::handleUpstreamTimeNextResponse(LedgerStateUser * pLedgerStateUser,
                                            bool bSuccess)
{
  LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                       "LedgerState::%s"
                                       " success: %s",
                                       __func__,
                                       bSuccess ? "yes" : "no");

  pLedgerStateUser->changeState(ErrorLedgerState::instance());
}


void
SimActivityService::
LedgerState::changeState(LedgerStateUser * pLedgerStateUser,
                         LedgerState * pLedgerState)
{
  pLedgerStateUser->changeState(pLedgerState);
}
