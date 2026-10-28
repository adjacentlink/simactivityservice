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

#include "initialledgerstate.h"
#include "logservice.h"
#include "initialtimenoticeconfirmledgerstate.h"

#include <algorithm>

void
SimActivityService::
InitialLedgerState::handleActivityHello(LedgerStateUser * pLedgerStateUser,
                                        ActivityTimeSender * pActivityTimeSender,
                                        [[maybe_unused]] const TimePoint & simulationTime,
                                        const TagSet & activityParticipantSet,
                                        const uuid_t & uuid,
                                        const std::string & sTag,
                                        const TimePoint & now,
                                        std::uint64_t u64SequenceNumber,
                                        TagSet & helloSet,
                                        std::uint64_t & u64LedgerSequenceNumber)
{
  char pzUUID[37];
  uuid_unparse(uuid,pzUUID);

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "InitialLedgerState::%s"
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


  helloSet.emplace(sTag);

  TagSet missingSet{};

  std::set_difference(activityParticipantSet.begin(),
                      activityParticipantSet.end(),
                      helloSet.begin(),
                      helloSet.end(),
                      std::inserter(missingSet,
                                    missingSet.begin()));

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
                                         "InitialLedgerState::%s missing hello %zu of %zu,"
                                         " missing:",
                                         __func__,
                                         missingSet.size(),
                                         activityParticipantSet.size());

  if(missingSet.empty())
    {
      pActivityTimeSender->sendActivityTimeNotice(TimePoint{},
                                                  ++u64LedgerSequenceNumber);

      changeState(pLedgerStateUser,
                  InitialTimeNoticeConfirmLedgerState::instance());
    }
}

const char *
SimActivityService::InitialLedgerState::name()
{
  return "InitialLedgerState";
}
