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

#ifndef SIMACTIVITYSERVICE_LEDGERSTATE_HEADER_
#define SIMACTIVITYSERVICE_LEDGERSTATE_HEADER_

#include "types.h"
#include "activitytimesender.h"
#include "upstreamtimesender.h"

#include <string>
#include <map>
#include <uuid.h>

namespace SimActivityService
{
  using EarliestWorkTimeMap = std::map<std::string,
                                       std::uint64_t>;

  using ActionsTagMap = std::map<std::string,TagSet>;

  using Actions = std::set<std::string>;

  using TagActionsMap = std::map<std::string,Actions>;

  using ActivityLedger = std::map<std::string,TagSet>;

  using ActionActivityLedgerMap = std::map<std::string,ActivityLedger>;

  using ActivityLedger = std::map<std::string,TagSet>;

  using ActionActivityLedgerMap = std::map<std::string,ActivityLedger>;

  class LedgerStateUser;

  class LedgerState
  {
  public:
    virtual ~LedgerState();

    virtual
    void handleActivityStatus(LedgerStateUser * pLedgerStateUser,
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
                              std::uint64_t & u64LedgerSequenceNumber);

    virtual
    void handleActivityHello(LedgerStateUser * pLedgerStateUser,
                             ActivityTimeSender * pActivityTimeSender,
                             const TimePoint & simulationTime,
                             const TagSet & activityParticipantSet,
                             const uuid_t & uuid,
                             const std::string & sTag,
                             const TimePoint & now,
                             std::uint64_t u64SequenceNumber,
                             TagSet & helloSet,
                             std::uint64_t & u64LedgerSequenceNumber);

    virtual
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
                                         std::uint64_t & u64LedgerSequenceNumber);


    virtual
    void handleActivityTimeConfirmNow(LedgerStateUser * pLedgerStateUser,
                                      ActivityTimeSender * pActivityTimeSender,
                                      const TagSet & activityParticipantSet,
                                      const uuid_t & uuid,
                                      const std::string & sTag,
                                      const TimePoint & now,
                                      std::uint64_t u64SequenceNumber,
                                      const TimePoint & confirm,
                                      bool bReady,
                                      std::uint64_t u64ReferenceSequenceNumber,
                                      TagSet & confirmReadySet,
                                      EarliestWorkTimeMap & earliestWorkTimeMap,
                                      ActionActivityLedgerMap & actionActivityLedgerMap,
                                      TimePoint & simulationTime,
                                      TimePoint & candidateNextTime,
                                      std::uint64_t & u64LedgerSequenceNumber);



    virtual
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
                              TimePoint & candidateNextTime);

    virtual
    void handleUpstreamTimeNow(LedgerStateUser * pLedgerStateUser,
                               ActivityTimeSender * pActivityTimeSender,
                               UpstreamTimeSender * pUpstreamTimeSender,
                               const TimePoint & now,
                               TimePoint & simulationTime,
                               TimePoint & candidateNextTime,
                               TimePoint & upstreamTimeAskAfterTime,
                               std::uint64_t & u64LedgerSequenceNumber);

    virtual
    void handleUpstreamTimeNextResponse(LedgerStateUser * pLedgerStateUser,
                                        bool bSuccess);


    virtual const char * name() = 0;

  protected:
    LedgerState() = default;

    void changeState(LedgerStateUser * pLedgerStateUser,
                     LedgerState * pState);
  };
}

#endif // SIMACTIVITYSERVICE_LEDGERSTATE_HEADER_
