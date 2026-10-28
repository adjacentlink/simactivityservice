/*
 * Copyright (c) 2026 - Adjacent Link LLC, Bridgewater, New Jersey
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

#ifndef SIMACTIVITYSERVICE_STATISTICPUBLISHER_HEADER_
#define SIMACTIVITYSERVICE_STATISTICPUBLISHER_HEADER_

#include "ledgerstate.h"
#include "singleton.h"

#include "simactivityservice.pb.h"

#include <ostatistic/table.h>

#include <tuple>

namespace SimActivityService
{
  class StatisticPublisher : public Singleton<StatisticPublisher>
  {
  public:
    void initialize();

    void updateTime(const TimePoint & simulationTime);

    void updateAction(const std::string & sAction,
                      const std::string & sIdentifier,
                      const std::string & sParticipant,
                      const ActionsTagMap & actionsTagMap,
                      const TimePoint & simulationTime);

    void updateActivity(const std::string sParticipant,
                        std::uint64_t u64SequenceNumber,
                        Activity::Type type,
                        const TimePoint & timeFromMessage,
                        const TimePoint & now);

  protected:
    StatisticPublisher();

  private:
    OpenStatistic::Table<std::string> * pOpenLedgerTable_;
    OpenStatistic::Table<std::string> * pClosedLedgerTable_;
    OpenStatistic::Table<std::string> * pActivityMessageTable_;
    OpenStatistic::Table<std::string> * pActivityCountTable_;
    OpenStatistic::Table<std::string> * pTimeTable_;

    using LedgerTableActionInstanceMap = std::map<std::string,
                                                  std::tuple<TagSet,
                                                             TimePoint>>;

    LedgerTableActionInstanceMap ledgerTableActionInstanceMap_;

    using MessageCountMap = std::map<Activity::Type,std::uint64_t>;
    using ParticpantMessageCountMap = std::map<std::string,MessageCountMap>;
    ParticpantMessageCountMap particpantMessageCountMap_;

    TimePoint startSimulationTime_;
    TimePoint startWallTime_;
    TimePoint currentSimulationTime_;
    TimePoint currentWallTime_;
  };
};

#endif // SIMACTIVITYSERVICE_STATISTICPUBLISHER_HEADER_
