/*
 * Copyright (c) 2025 - Adjacent Link LLC, Bridgewater, New Jersey
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

#ifndef SIMACTIVITYSERVICE_TIMENOTICECONFIRMLEDGERSTATE_HEADER_
#define SIMACTIVITYSERVICE_TIMENOTICECONFIRMLEDGERSTATE_HEADER_

#include "ledgerstate.h"
#include "ledgerstateuser.h"
#include "singleton.h"

namespace SimActivityService
{
  class TimeNoticeConfirmLedgerState : public LedgerState,
                                       public Singleton<TimeNoticeConfirmLedgerState>
  {
  public:
    void handleActivityTimeConfirmNotice(LedgerStateUser * pLedgerStateUser,
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
                                         const Microseconds & nudgeMicroseconds,
                                         EarliestWorkTimeMap & earliestWorkTimeMap,
                                         ActionActivityLedgerMap & actionActivityLedgerMap,
                                         TimePoint & simulationTime,
                                         TimePoint & candidateNextTime,
                                         TimePoint & upstreamTimeAskAfterTime,
                                         std::uint64_t & u64LedgerSequenceNumber) override;

    void handleActivityAction(LedgerStateUser * pLedgerStateUser,
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
                              TimePoint & candidateNextTime) override;

    const char * name() override;
  };
}

#endif // SIMACTIVITYSERVICE_TIMENOTICECONFIRMLEDGERSTATE_HEADER_
