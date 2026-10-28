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

#ifndef SIMACTIVITYSERVICE_SERVICE_HEADER_
#define SIMACTIVITYSERVICE_SERVICE_HEADER_

#include "activitytimesender.h"
#include "upstreamtimesender.h"
#include "ledgermanager.h"
#include "participant.h"
#include "raiizmq.h"

#include "simactivityservice.pb.h"

#include <thread>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

#include <uuid.h>

namespace SimActivityService
{
  class Service: public ActivityTimeSender,
                 public UpstreamTimeSender
  {
  public:
    Service(const uuid_t & uuid);

    void configure(const std::string & sConfigurationXML);

    void start();

    void stop();

  private:
    uuid_t uuid_;
    std::string sActivityPublishEndpoint_;
    std::string sUpstreamTimeRequestEndpoint_;

    RAIIZMQContext pContext_;
    RAIIZMQSocket pActivityXPubSocket_;
    RAIIZMQSocket pActivityXSubSocket_;
    RAIIZMQSocket pActivityPubSocket_;
    RAIIZMQSocket pActivitySubSocket_;
    RAIIZMQSocket pUpstreamTimeRequestSocket_;

    std::thread thread_;
    int iEventFd_;
    std::string sOpenStatisticEndpoint_;
    Participants participants_;
    LedgerManager ledgerManager_;
    int iTimeoutMilliseconds_;

    void process();

    void sendActivityTimeNotice(const TimePoint & activityTime,
                                std::uint64_t u64SequenceNumber) override;

    void sendActivityTimeNow(const TimePoint & activityTime,
                             std::uint64_t u64SequenceNumber) override;

    void sendActivityTimeSignal(std::uint64_t u64SequenceNumber) override;

    void sendActivityHello(std::uint64_t u64SequenceNumber) override;

    void sendUpstreamTimeNext(const TimePoint & upstreamTime) override;

    void sendActivity(const TimePoint & activityTime,
                      std::uint64_t u64SequenceNumber,
                      const std::string & sTopic,
                      SimActivityService::Activity & msg);
  };
}

#endif // SIMACTIVITYSERVICE_SERVICE_HEADER_
