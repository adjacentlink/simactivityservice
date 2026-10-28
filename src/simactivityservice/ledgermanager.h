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

#ifndef SIMACTIVITYSERVICE_LEDGERMANAGER_HEADER_
#define SIMACTIVITYSERVICE_LEDGERMANAGER_HEADER_

#include "ledgerstateuser.h"
#include "activitytimesender.h"
#include "participant.h"
#include "types.h"

#include <string>
#include <set>
#include <map>
#include <tuple>
#include <optional>

#include <uuid.h>

namespace SimActivityService
{
  class LedgerManager : LedgerStateUser
  {
  public:
    LedgerManager();

    void setActivityTimeSender(ActivityTimeSender * pActivityTimeSender);

    void setUpstreamTimeSender(UpstreamTimeSender * pUpstreamTimeSender);

    void configure(const Participants & participants,
                   const Microseconds & nudgeMicroseconds);


    void handleActivityStatus(const uuid_t & uuid,
                              const std::string & sTag,
                              const TimePoint & now,
                              std::uint64_t u64SequenceNumber,
                              const TimePoint & next);


    void handleActivityHello(const uuid_t & uuid,
                             const std::string & sTag,
                             const TimePoint & now,
                             std::uint64_t u64SequenceNumber);

    void handleActivityTimeConfirmNotice(const uuid_t & uuid,
                                         const std::string & sTag,
                                         const TimePoint & now,
                                         std::uint64_t u64SequenceNumber,
                                         const TimePoint & confirm,
                                         bool bReady,
                                         std::uint64_t u64ReferenceSequenceNumber);

    void handleActivityTimeConfirmNow(const uuid_t & uuid,
                                      const std::string & sTag,
                                      const TimePoint & now,
                                      std::uint64_t u64SequenceNumber,
                                      const TimePoint & confirm,
                                      bool bReady,
                                      std::uint64_t u64ReferenceSequenceNumber);

    void handleActivityAction(const uuid_t & uuid,
                              const std::string & sTag,
                              const TimePoint & now,
                              std::uint64_t u64SequenceNumber,
                              const std::string & sAction,
                              const std::string & sIdentifier);

    void handleUpstreamTimeNow(const TimePoint & now);

    void handleUpstreamTimeNextResponse(bool bSuccess);

    void discover();

    const TimePoint & now();

  private:
    ActivityTimeSender * pActivityTimeSender_;
    UpstreamTimeSender * pUpstreamTimeSender_;

    using ActionKey = std::string;

    using TagActionsMap = std::map<std::string,Actions>;
    TagActionsMap tagActionsMap_;

    using ActionsTagMap = std::map<std::string,TagSet>;
    ActionsTagMap actionsTagMap_;

    TagSet activityParticipantSet_;
    TagSet helloSet_;
    TagSet confirmReadySet_;
    TagSet confirmNotReadySet_;

    using EarliestWorkTimeMap = std::map<std::string,
                                         std::uint64_t>;

    EarliestWorkTimeMap earliestWorkTimeMap_;

    using ActivityKey = std::string;
    using ActivityLedger = std::map<ActivityKey,TagSet>;

    using ActionActivityLedgerMap = std::map<ActionKey,ActivityLedger>;

    ActionActivityLedgerMap actionActivityLedgerMap_;

    TimePoint simulationTime_;

    TimePoint candidateNextTime_;

    TimePoint upstreamTimeAskAfterTime_;

    std::uint64_t u64SequenceNumber_;

    Microseconds  nudgeMicroseconds_;
  };
}

#endif // SIMACTIVITYSERVICE_LEDGERMANAGER_HEADER_
