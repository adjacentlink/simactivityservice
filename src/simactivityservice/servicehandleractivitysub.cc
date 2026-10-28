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

#include "servicehandleractivitysub.h"
#include "logservice.h"
#include "simactivityservice/topics.h"
#include "statisticpublisher.h"

#include "simactivityservice.pb.h"

#include <zmq.h>
#include <uuid.h>

void
SimActivityService::
ServiceHandlerActivitySub::handle(void * pActivitySubSocket,
                                  LedgerManager & ledgerManager)
{
  std::string sMessageTopic{};

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "ServiceHandlerActivitySub");
  while(1)
    {
      zmq_msg_t message;

      zmq_msg_init(&message);

      zmq_msg_recv(&message,pActivitySubSocket,0);

      int iMore{};

      size_t sizeMore{sizeof(iMore)};

      zmq_getsockopt(pActivitySubSocket,ZMQ_RCVMORE,&iMore,&sizeMore);

      if(iMore)
        {
          sMessageTopic = std::string{reinterpret_cast<char *>(zmq_msg_data(&message)),
                                      zmq_msg_size(&message)};

          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "ServiceHandlerActivitySub::%s"
                                               " topic %s",
                                               __func__,
                                               sMessageTopic.c_str());
        }
      else
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "ServiceHandlerActivitySub::%s"
                                               " topic %s data length: %zu",
                                               __func__,
                                               sMessageTopic.c_str(),
                                               zmq_msg_size(&message));
          SimActivityService::Activity m{};

          if(m.ParseFromArray(zmq_msg_data(&message),
                              zmq_msg_size(&message)))
            {
              uuid_t uuid;

              uuid_copy(uuid,
                        reinterpret_cast<const unsigned char *>(m.uuid().data()));

              TimePoint messageNow =
                Clock::time_point{Microseconds{m.time_microseconds_since_epoch()}};

              StatisticPublisher::instance()->updateActivity(m.tag(),
                                                             m.sequence_number(),
                                                             m.type(),
                                                             messageNow,
                                                             Clock::now());


              if(sMessageTopic == Topics::ACTIVITY_SERVICE_TOPIC_STATUS)
                {
                  if(m.type() == SimActivityService::Activity::TYPE_ACTIVITY_STATUS)
                    {
                      if(m.has_status())
                        {
                          const auto pStatus = m.mutable_status();

                          auto timeFromMessage =
                            Clock::time_point{Microseconds{pStatus->next_time_microseconds_since_epoch()}};

                          ledgerManager.handleActivityStatus(uuid,
                                                             m.tag(),
                                                             messageNow,
                                                             m.sequence_number(),
                                                             timeFromMessage);
                        }
                      else
                        {
                          LogServiceSingleton::instance()->
                            log(ERROR_LEVEL,
                                "ServiceHandlerActivitySub:%s"
                                " activity message missing status"
                                " field topic: %s, ignoring",
                                __func__,
                                sMessageTopic.c_str());
                        }
                    }
                  else
                    {
                      LogServiceSingleton::instance()->
                        log(ERROR_LEVEL,
                            "ServiceHandlerActivitySub:%s"
                            " activity message type does"
                            " not match field topic: %s,"
                            " ignoring",
                            __func__,
                            sMessageTopic.c_str());
                    }
                }
              else if(sMessageTopic == Topics::ACTIVITY_SERVICE_TOPIC_HELLO_ACKNOWLEDGE)
                {
                  if(m.type() == SimActivityService::Activity::TYPE_ACTIVITY_HELLO)
                    {
                      ledgerManager.handleActivityHello(uuid,
                                                        m.tag(),
                                                        messageNow,
                                                        m.sequence_number());
                    }
                  else
                    {
                      LogServiceSingleton::instance()->
                        log(ERROR_LEVEL,
                            "ServiceHandlerActivitySub:%s"
                            " activity message type does"
                            " not match field topic: %s,"
                            " ignoring",
                            __func__,
                            sMessageTopic.c_str());
                    }
                }
              else if(sMessageTopic == Topics::ACTIVITY_SERVICE_TOPIC_TIME_CONFIRM_NOTICE)
                {
                  if(m.type() == SimActivityService::Activity::TYPE_ACTIVITY_TIME_CONFIRM_NOTICE)
                    {
                      const auto pTimeConfirmNotice = m.mutable_time_confirm_notice();

                      auto confirm =
                        Clock::time_point{Microseconds{pTimeConfirmNotice->time_microseconds_since_epoch()}};

                      ledgerManager.handleActivityTimeConfirmNotice(uuid,
                                                                    m.tag(),
                                                                    messageNow,
                                                                    m.sequence_number(),
                                                                    confirm,
                                                                    pTimeConfirmNotice->ready(),
                                                                    pTimeConfirmNotice->reference_sequence_number());
                    }
                  else
                    {
                      LogServiceSingleton::instance()->
                        log(ERROR_LEVEL,
                            "ServiceHandlerActivitySub:%s"
                            " activity message type does"
                            " not match field topic: %s,"
                            " ignoring",
                            __func__,
                            sMessageTopic.c_str());
                    }
                }
              else if(sMessageTopic == Topics::ACTIVITY_SERVICE_TOPIC_TIME_CONFIRM_NOW)
                {
                  if(m.type() == SimActivityService::Activity::TYPE_ACTIVITY_TIME_CONFIRM_NOW)
                    {
                      const auto pTimeConfirmNow = m.mutable_time_confirm_now();

                      auto confirm =
                        Clock::time_point{Microseconds{pTimeConfirmNow->time_microseconds_since_epoch()}};

                      ledgerManager.handleActivityTimeConfirmNow(uuid,
                                                                 m.tag(),
                                                                 messageNow,
                                                                 m.sequence_number(),
                                                                 confirm,
                                                                 pTimeConfirmNow->ready(),
                                                                 pTimeConfirmNow->reference_sequence_number());
                    }
                  else
                    {
                      LogServiceSingleton::instance()->
                        log(ERROR_LEVEL,
                            "ServiceHandlerActivitySub:%s"
                            " activity message type does"
                            " not match field topic: %s,"
                            " ignoring",
                            __func__,
                            sMessageTopic.c_str());
                    }
                }
              else if(sMessageTopic == Topics::ACTIVITY_SERVICE_TOPIC_ACTION)
                {
                  if(m.type() == SimActivityService::Activity::TYPE_ACTIVITY_ACTION)
                    {
                      if(m.has_action())
                        {
                          const auto pAction = m.mutable_action();


                          ledgerManager.handleActivityAction(uuid,
                                                             m.tag(),
                                                             messageNow,
                                                             m.sequence_number(),
                                                             pAction->name(),
                                                             pAction->unique_identifier());
                        }
                      else
                        {
                          LogServiceSingleton::instance()->
                            log(ERROR_LEVEL,
                                "ServiceHandlerActivitySub:%s"
                                " activity message missing action"
                                " field topic: %s, ignoring",
                                __func__,
                                sMessageTopic.c_str());
                        }
                    }
                  else
                    {
                      LogServiceSingleton::instance()->
                        log(ERROR_LEVEL,
                            "ServiceHandlerActivitySub:%s"
                            " activity message type does"
                            " not match field topic: %s,"
                            " ignoring",
                            __func__,
                            sMessageTopic.c_str());
                    }
                }
              else
                {
                  LogServiceSingleton::instance()->
                    log(ERROR_LEVEL,
                        "ServiceHandlerActivitySub:%s activity message topic is"
                        " not handled: %s, ignoring",
                        __func__,
                        sMessageTopic.c_str());
                }
            }
          else
            {
              LogServiceSingleton::instance()
                ->log(ERROR_LEVEL,
                      "ServiceHandlerActivitySub::%s"
                      " unable to parse simulation message"
                      " topic: %s, ignoring",
                      __func__,
                      sMessageTopic.c_str());
            }
        }

      zmq_msg_close(&message);

      if(!iMore)
        {
          break;
        }
    }
}
