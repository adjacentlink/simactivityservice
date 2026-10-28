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

#include "statisticpublisher.h"
#include "logservice.h"

#include <ostatistic/service.h>

SimActivityService::StatisticPublisher::StatisticPublisher():
  pOpenLedgerTable_{},
  pClosedLedgerTable_{},
  pActivityMessageTable_{}
{}

void
SimActivityService::StatisticPublisher::initialize()
{
  auto & registrar =
    OpenStatistic::Service::instance()->registrar();

  pOpenLedgerTable_ =
    registrar.registerTable<std::string>("table.ledegr.open",
                                         {"Action","Identifier","Participant","Status"},
                                         OpenStatistic::StatisticProperties::NONE,
                                         "Open ledger table.");

  pClosedLedgerTable_ =
    registrar.registerTable<std::string>("table.ledger.closed",
                                         {"Action","Identifier","Duration"},
                                         OpenStatistic::StatisticProperties::NONE,
                                         "Closed ledger table.");

  pActivityMessageTable_ =
    registrar.registerTable<std::string>("table.activity.message",
                                         {"Wall Time",
                                          "Participant",
                                          "Sequence Number",
                                          "Type",
                                          "Message Time"},
                                         OpenStatistic::StatisticProperties::NONE,
                                         "Activity Service receive message table.");

  pActivityCountTable_ =
    registrar.registerTable<std::string>("table.activity.count",
                                         {"Participant",
                                          "hello",
                                          "Confirm Notice",
                                          "Confirm Now",
                                          "Status",
                                          "Action"},
                                         OpenStatistic::StatisticProperties::NONE,
                                         "Activity Service receive message count table.");

  pTimeTable_ =
    registrar.registerTable<std::string>("table.time",
                                         {"Type",
                                          "Start",
                                          "Now",
                                          "Duration"},
                                         OpenStatistic::StatisticProperties::NONE,
                                         "Time table.");
  pTimeTable_->addRow("simulation",
                      {OpenStatistic::Any{"simulation"},
                       OpenStatistic::Any{"NA"},
                       OpenStatistic::Any{"NA"},
                       OpenStatistic::Any{"NA"}});

  pTimeTable_->addRow("wall",
                      {OpenStatistic::Any{"wall"},
                       OpenStatistic::Any{"NA"},
                       OpenStatistic::Any{"NA"},
                       OpenStatistic::Any{"NA"}});


}

void
SimActivityService::
StatisticPublisher::updateAction(const std::string & sAction,
                                 const std::string & sIdentifier,
                                 const std::string & sParticipant,
                                 const ActionsTagMap & actionsTagMap,
                                 [[maybe_unused]] const TimePoint & simulationTime)
{
  std::string sActionInstance{sAction + sIdentifier};
  std::string sRowKey{sAction + sIdentifier + sParticipant};

  auto iter = ledgerTableActionInstanceMap_.find(sActionInstance);

  if(iter == ledgerTableActionInstanceMap_.end())
    {
      iter =
        ledgerTableActionInstanceMap_.emplace(sActionInstance,
                                              std::make_tuple(TagSet{},Clock::now())).first;

      if(const auto iter = actionsTagMap.find(sAction);
         iter != actionsTagMap.end())
        {
          const TagSet & members{iter->second};

          for(const auto & sMember : members)
            {
              std::string sRowKey{sAction + sIdentifier + sMember};

              pOpenLedgerTable_->addRow(sRowKey,
                                        {OpenStatistic::Any{sAction},
                                         OpenStatistic::Any{sIdentifier},
                                         OpenStatistic::Any{sMember},
                                         OpenStatistic::Any{"open"}});
            }
        }
      else
        {
          LogServiceSingleton::instance()->
            log(ERROR_LEVEL,
                "StatisticPublisher::%s"
                " action: %s"
                " identifier: %s"
                " cannot find action in actions tag map",
                __func__,
                sAction.c_str(),
                sIdentifier.c_str());
        }
    }

  auto & [repondents,startTime] = iter->second;

  repondents.emplace(sParticipant);

  pOpenLedgerTable_->setCell(sRowKey,3,OpenStatistic::Any{"closed"});

  if(const auto iter = actionsTagMap.find(sAction);
     iter != actionsTagMap.end())
    {
      const TagSet & members{iter->second};

      if(repondents.size() == members.size())
        {
          for(const auto & sMember : members)
            {
              std::string sRowKey{sAction + sIdentifier + sMember};

              pOpenLedgerTable_->deleteRow(sRowKey);
            }

          Microseconds duration{std::chrono::duration_cast<Microseconds>(Clock::now() - startTime)};

          pClosedLedgerTable_->addRow(sRowKey,
                                      {OpenStatistic::Any{sAction},
                                       OpenStatistic::Any{sIdentifier},
                                       OpenStatistic::Any{duration.count()}});

          ledgerTableActionInstanceMap_.erase(sActionInstance);
        }
    }
  else
    {
      LogServiceSingleton::instance()->
        log(ERROR_LEVEL,
            "StatisticPublisher::%s"
            " action: %s"
            " identifier: %s"
            " cannot find action in actions tag map",
            __func__,
            sAction.c_str(),
            sIdentifier.c_str());
    }
}

void
SimActivityService::
StatisticPublisher::updateTime(const TimePoint & simulationTime)
{
  auto now = Clock::now();

  pClosedLedgerTable_->clear();

  if(!ledgerTableActionInstanceMap_.empty())
    {
      LogServiceSingleton::instance()->
        log(ERROR_LEVEL,
            "StatisticPublisher::%s"
            " simulaiton time update to %zu but there is still open ledger"
            " entries in statistic table, clearing table",
            __func__,
            std::chrono::duration_cast<Microseconds>
            (simulationTime.time_since_epoch()).count());

      ledgerTableActionInstanceMap_.clear();
    }

  pOpenLedgerTable_->clear();
  pActivityMessageTable_->clear();

  static bool once{};


  if(!once)
    {
      startSimulationTime_ = simulationTime;
      startWallTime_ = now;

      once = true;

      pTimeTable_->
        setCell("simulation",
                1,
                OpenStatistic::Any{std::chrono::duration_cast<Microseconds>
                                   (startSimulationTime_.time_since_epoch()).count()});

      pTimeTable_->
        setCell("wall",
                1,
                OpenStatistic::Any{std::chrono::duration_cast<Microseconds>
                                   (startWallTime_.time_since_epoch()).count()});
    }

  currentSimulationTime_ = simulationTime;
  currentWallTime_ = now;

  pTimeTable_->
    setCell("simulation",
            2,
            OpenStatistic::Any{std::chrono::duration_cast<Microseconds>
                               (currentSimulationTime_.time_since_epoch()).count()});

  pTimeTable_->
    setCell("simulation",
            3,
            OpenStatistic::Any{std::chrono::duration_cast<DoubleSeconds>
                               (currentSimulationTime_ - startSimulationTime_).count()});


  pTimeTable_->
    setCell("wall",
            2,
            OpenStatistic::Any{std::chrono::duration_cast<Microseconds>
                               (currentWallTime_.time_since_epoch()).count()});

  pTimeTable_->
    setCell("wall",
            3,
            OpenStatistic::Any{std::chrono::duration_cast<DoubleSeconds>
                               (currentWallTime_ - startWallTime_).count()});

}

void
SimActivityService::
StatisticPublisher::updateActivity(const std::string sParticipant,
                                   std::uint64_t u64SequenceNumber,
                                   Activity::Type type,
                                   const TimePoint & timeFromMessage,
                                   const TimePoint & now)
{
  std::string sKey{std::to_string(std::chrono::duration_cast<Microseconds>
                                  (now.time_since_epoch()).count()) + sParticipant};

  std::string sType{};
  int index{};

  switch(type)
    {
    case Activity::TYPE_ACTIVITY_HELLO:
      sType = "hello";
      index= 1;
      break;

    case Activity::TYPE_ACTIVITY_TIME_CONFIRM_NOTICE:
      sType = "notice";
      index= 2;
      break;

    case Activity::TYPE_ACTIVITY_TIME_CONFIRM_NOW:
      sType = "now";
      index= 3;
      break;

    case Activity::TYPE_ACTIVITY_STATUS:
      sType = "status";
      index= 4;
      break;

    case Activity::TYPE_ACTIVITY_ACTION:
      sType = "action";
      index= 5;
      break;

    default:
      sType = "unknown";
    };

  pActivityMessageTable_->addRow(sKey,
                                 { OpenStatistic::Any{std::chrono::duration_cast<Microseconds>
                                                      (now.time_since_epoch()).count()},
                                   OpenStatistic::Any{sParticipant},
                                   OpenStatistic::Any{u64SequenceNumber},
                                   OpenStatistic::Any{sType},
                                   OpenStatistic::Any{std::chrono::duration_cast<Microseconds>
                                     (timeFromMessage.time_since_epoch()).count()}});

  auto iter = particpantMessageCountMap_.find(sParticipant);

  if(iter == particpantMessageCountMap_.end())
    {
      iter = particpantMessageCountMap_.
        emplace(sParticipant,
                MessageCountMap{{Activity::TYPE_ACTIVITY_HELLO,0},
                                {Activity::TYPE_ACTIVITY_TIME_CONFIRM_NOTICE,0},
                                {Activity::TYPE_ACTIVITY_TIME_CONFIRM_NOW,0},
                                {Activity::TYPE_ACTIVITY_STATUS,0},
                                {Activity::TYPE_ACTIVITY_ACTION,0}}).first;

      pActivityCountTable_->addRow(sParticipant,
                                   {OpenStatistic::Any{sParticipant},
                                    OpenStatistic::Any{std::uint64_t{0}},
                                    OpenStatistic::Any{std::uint64_t{0}},
                                    OpenStatistic::Any{std::uint64_t{0}},
                                    OpenStatistic::Any{std::uint64_t{0}},
                                    OpenStatistic::Any{std::uint64_t{0}}});
    }

  auto & messageCountMap = iter->second;

  if(auto iter = messageCountMap.find(type);
     iter != messageCountMap.end())
    {
      auto & count = iter->second;

      ++count;

      pActivityCountTable_->setCell(sParticipant,index,OpenStatistic::Any{count});
    }
  else
    {
      LogServiceSingleton::instance()->
        log(ERROR_LEVEL,
            "StatisticPublisher::%s"
            " cannot find particpant %s message count entry for %s",
            __func__,
            sParticipant.c_str(),
            sType.c_str());
    }
}
