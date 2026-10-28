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

#include "service.h"
#include "logservice.h"
#include "servicehandleractivityxpubxsub.h"
#include "servicehandleractivitysub.h"
#include "servicehandlerupstreamtimereq.h"
#include "configureexception.h"
#include "startexception.h"
#include "parameterconvert.h"
#include "simactivityservice.pb.h"
#include "upstreamtime.pb.h"

#include "simactivityservice/topics.h"
#include "statisticpublisher.h"

#include <ostatistic/service.h>

#include <zmq.h>
#include <cstring>

#include <sys/eventfd.h>

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>

namespace
{
  const uint64_t one{1};

  const char * pzSchema="\
 <xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>\
    <xs:element name='simactivityservice'>\
    <xs:complexType>\
      <xs:sequence>\
        <xs:element name='upstream-time' minOccurs='0'>\
          <xs:complexType>\
             <xs:attribute name='request' type='xs:string' use='required'/>\
          </xs:complexType>\
        </xs:element>\
        <xs:element name='participants'>\
          <xs:complexType>\
            <xs:sequence>\
              <xs:element name='participant' maxOccurs='unbounded'>\
                <xs:complexType>\
                  <xs:sequence>\
                    <xs:element name='actions' minOccurs='0'>\
                       <xs:complexType>\
                         <xs:sequence>\
                           <xs:element name='action' maxOccurs='unbounded'>\
                             <xs:complexType>\
                               <xs:attribute name='name' type='xs:string' use='required'/> \
                             </xs:complexType>\
                           </xs:element>\
                         </xs:sequence>\
                       </xs:complexType>\
                    </xs:element>\
                  </xs:sequence>\
                  <xs:attribute name='endpoint' type='xs:string' use='required'/>\
                  <xs:attribute name='tag' type='xs:string' use='required'/> \
                </xs:complexType>\
              </xs:element>\
            </xs:sequence>\
          </xs:complexType>\
        </xs:element>\
      </xs:sequence>\
      <xs:attribute name='publish' type='xs:string' use='required'/>\
      <xs:attribute name='nudge-microseconds' type='xs:unsignedLong' default='0'/>\
      <xs:attribute name='ostatistic' type='xs:string' default='0.0.0.0:47001'/>\
    </xs:complexType>\
  </xs:element>\
</xs:schema>";

  void createZMQSocket(SimActivityService::RAIIZMQContext & pContext,
                       SimActivityService::RAIIZMQSocket & pZMQSocket,
                       int type,
                       const std::string & sLabel)
  {
    pZMQSocket.reset(zmq_socket(pContext.get(),type));

    if(!pZMQSocket)
      {
        throw SimActivityService::StartException{"Error creating new socket %s: %s",
                                                 sLabel.c_str(),
                                                 zmq_strerror(errno)};
      }

    int iIPv6 = 1;

    if(zmq_setsockopt(pZMQSocket.get(),ZMQ_IPV6,&iIPv6,sizeof(iIPv6)))
      {
        throw SimActivityService::StartException{"Error setting socket %s opiton ZMQ_IPV6: %s",
                                                 sLabel.c_str(),
                                                 zmq_strerror(errno)};
      }
  }
}


SimActivityService::Service::Service(const uuid_t & uuid):
  iEventFd_{-1},
  iTimeoutMilliseconds_{1000}
{
  uuid_copy(uuid_,uuid);
  iEventFd_ = eventfd(0,0);
  ledgerManager_.setActivityTimeSender(this);
};

void
SimActivityService::Service::configure(const std::string & sConfigurationXML)
{
  LIBXML_TEST_VERSION;

  xmlDocPtr pSchemaDoc{xmlReadMemory(pzSchema,
                                     strlen(pzSchema),
                                     "file:///simactivityservice.xsd",
                                     NULL,
                                     0)};

  if(!pSchemaDoc)
    {
      throw ConfigureException{"unable to open schema"};
    }

  xmlSchemaParserCtxtPtr pParserContext{xmlSchemaNewDocParserCtxt(pSchemaDoc)};

  if(!pParserContext)
    {
      throw  ConfigureException{"bad schema context"};
    }

  xmlSchemaPtr pSchema{xmlSchemaParse(pParserContext)};

  if(!pSchema)
    {
      throw  ConfigureException{"bad schema parser"};
    }

  xmlSchemaValidCtxtPtr pSchemaValidCtxtPtr{xmlSchemaNewValidCtxt(pSchema)};

  if(!pSchemaValidCtxtPtr)
    {
      throw  ConfigureException{"bad schema valid context"};
    }

  xmlSchemaSetValidOptions(pSchemaValidCtxtPtr,XML_SCHEMA_VAL_VC_I_CREATE);

  xmlDocPtr pDoc = xmlReadFile(sConfigurationXML.c_str(),nullptr,0);

  if(xmlSchemaValidateDoc(pSchemaValidCtxtPtr, pDoc))
    {
      throw ConfigureException{"invalid document"};
    }

  xmlNodePtr pRoot = xmlDocGetRootElement(pDoc);

  xmlChar * pPublishEndpoint = xmlGetProp(pRoot,BAD_CAST "publish");

  sActivityPublishEndpoint_ = reinterpret_cast<const char *>(pPublishEndpoint);

  LogServiceSingleton::instance()->log(INFO_LEVEL,
                                       "Service::%s %s: %s",
                                       __func__,
                                       "publish",
                                       sActivityPublishEndpoint_.c_str());

  xmlFree(pPublishEndpoint);

  xmlChar * pNudgeMicroseconds = xmlGetProp(pRoot,BAD_CAST "nudge-microseconds");

  Microseconds nudgeMicroseconds =
    Microseconds{ParameterConvert(reinterpret_cast<const char *>(pNudgeMicroseconds)).toUINT64()};

  LogServiceSingleton::instance()->log(INFO_LEVEL,
                                       "Service::%s %s: %zu",
                                       __func__,
                                       "nudge-microseconds",
                                       nudgeMicroseconds.count());

  xmlFree(pNudgeMicroseconds);

  xmlChar * pOpenStatisticEndpoint = xmlGetProp(pRoot,BAD_CAST "ostatistic");

  sOpenStatisticEndpoint_ = reinterpret_cast<const char *>(pOpenStatisticEndpoint);

  LogServiceSingleton::instance()->log(INFO_LEVEL,
                                       "Service::%s %s: %s",
                                       __func__,
                                       "ostatistic",
                                       sOpenStatisticEndpoint_.c_str());

  xmlFree(pOpenStatisticEndpoint);

  xmlXPathContextPtr pXPathCtxt{xmlXPathNewContext(pDoc)};

  xmlXPathObjectPtr
    pUpstreamTimeXPathObj{xmlXPathEvalExpression(BAD_CAST "/simactivityservice/upstream-time",
                                                 pXPathCtxt)};

  if(pUpstreamTimeXPathObj)
    {
      if(pUpstreamTimeXPathObj->nodesetval->nodeNr)
        {
          xmlChar * pUpstreamTimeRequest =
            xmlGetProp(pUpstreamTimeXPathObj->nodesetval->nodeTab[0],
                       BAD_CAST "request");

          sUpstreamTimeRequestEndpoint_ =
            reinterpret_cast<const char *>(pUpstreamTimeRequest);

          LogServiceSingleton::instance()->log(INFO_LEVEL,
                                               "Service::%s %s: %s",
                                               __func__,
                                               "upstream-time request",
                                               sUpstreamTimeRequestEndpoint_.c_str());

          xmlFree(pUpstreamTimeRequest);
        }

      xmlXPathFreeObject(pUpstreamTimeXPathObj);
    }

  xmlXPathObjectPtr
    pXPathObj{xmlXPathEvalExpression(BAD_CAST "/simactivityservice/participants/participant",
                                     pXPathCtxt)};

  if(!pXPathObj)
    {
      xmlXPathFreeContext(pXPathCtxt);
      xmlFreeDoc(pDoc);
      throw ConfigureException{"unable to evaluate xpath"};
    }

  int iSize = pXPathObj->nodesetval->nodeNr;

  if(!iSize)
    {
      throw ConfigureException{"no activity participants defined"};
    }

  for(int i = 0; i < iSize; ++i)
    {
      xmlChar * pEndpoint  = xmlGetProp(pXPathObj->nodesetval->nodeTab[i],BAD_CAST "endpoint");

      xmlChar * pTag  = xmlGetProp(pXPathObj->nodesetval->nodeTab[i],BAD_CAST "tag");

      xmlDocSetRootElement(pDoc,pXPathObj->nodesetval->nodeTab[i]);

      xmlXPathObjectPtr
        pActionXPathObj{xmlXPathEvalExpression(BAD_CAST "/participant/actions/action",
                                               pXPathCtxt)};

      Actions actions{};

      for(int i = 0; i < pActionXPathObj->nodesetval->nodeNr; ++i)
        {
          xmlChar * pName  = xmlGetProp(pActionXPathObj->nodesetval->nodeTab[i],
                                        BAD_CAST "name");

          actions.emplace(reinterpret_cast<const char *>(pName));

          xmlFree(pName);
        }

      xmlXPathFreeObject(pActionXPathObj);

      participants_.push_back(Participant(reinterpret_cast<const char *>(pTag),
                                          reinterpret_cast<const char *>(pEndpoint),
                                          actions));

      xmlFree(pEndpoint);

      xmlFree(pTag);
    }

  xmlDocSetRootElement(pDoc,pRoot);

  xmlXPathFreeObject(pXPathObj);

  xmlXPathFreeContext(pXPathCtxt);

  xmlFreeDoc(pDoc);

  for(const auto & participant : participants_)
    {
      LogServiceSingleton::instance()->logfn(INFO_LEVEL,
                                             [&participant]()
                                             {
                                               std::list<std::string> s;

                                               for(const auto & action : participant.actions_)
                                                 {
                                                   s.push_back(action);
                                                 }

                                               return s;
                                             },
                                             "Service::%s %s: endpoint: %s tag: %s actions:",
                                             __func__,
                                             "activity participant",
                                             participant.sEndpoint_.c_str(),
                                             participant.sTag_.c_str());
    }

  ledgerManager_.configure(participants_,nudgeMicroseconds);

  StatisticPublisher::instance()->initialize();
}

void
SimActivityService::Service::start()
{
  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s",
                                       __func__);

  OpenStatistic::Service::instance()->start(sOpenStatisticEndpoint_);

  pContext_.reset(zmq_ctx_new());

  if(!pContext_)
    {
      throw StartException{"Error creating messaging context: %s",
                           zmq_strerror(errno)};
    }

  createZMQSocket(pContext_,pActivityPubSocket_,ZMQ_PUB,"activity pub");

  if(zmq_bind(pActivityPubSocket_.get(),"tcp://lo:*"))
    {
      throw StartException{"Error binding activity pub socket: %s",
                           zmq_strerror(errno)};
    }

  createZMQSocket(pContext_,pActivitySubSocket_,ZMQ_SUB,"activity sub");

  if(zmq_connect(pActivitySubSocket_.get(),
                 std::string{"tcp://"}.append(sActivityPublishEndpoint_).c_str()))
    {
      throw StartException{"Error connecting activity sub socket: %s",
                           zmq_strerror(errno)};
    }

  std::vector<std::string> topics{Topics::ACTIVITY_SERVICE_TOPIC_TIME_CONFIRM_NOTICE,
                                  Topics::ACTIVITY_SERVICE_TOPIC_TIME_CONFIRM_NOW,
                                  Topics::ACTIVITY_SERVICE_TOPIC_STATUS,
                                  Topics::ACTIVITY_SERVICE_TOPIC_HELLO_ACKNOWLEDGE,
                                  Topics::ACTIVITY_SERVICE_TOPIC_ACTION};

  for(const auto & topic : topics)
    {
      if(zmq_setsockopt(pActivitySubSocket_.get(),
                        ZMQ_SUBSCRIBE,
                        topic.c_str(),
                        topic.size()))
        {
          throw StartException{"Error subscribing activity sub"
                               " socket topic: %s: %s",
                               topic.c_str(),
                               zmq_strerror(errno)};
        }
    }

  createZMQSocket(pContext_,pActivityXPubSocket_,ZMQ_XPUB,"activity xpub");

  if(zmq_bind(pActivityXPubSocket_.get(),
              std::string{"tcp://"}.append(sActivityPublishEndpoint_).c_str()))
    {
      throw StartException{"Error binding activity xpub socket: %s",
                           zmq_strerror(errno)};
    }
  else
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "Service::%s"
                                           "bind xpub endpoint: %s",
                                           __func__,
                                           sActivityPublishEndpoint_.c_str());
    }


  createZMQSocket(pContext_,pActivityXSubSocket_,ZMQ_XSUB,"activity xsub");

  char buff[1024];
  size_t sizeBuff{sizeof(buff)};

  if(zmq_getsockopt(pActivityPubSocket_.get(),ZMQ_LAST_ENDPOINT,&buff,&sizeBuff))
    {
      throw StartException{"Unable to get activity pub endpoint"
                           " via ZMQ_LAST_ENDPOINT: %s",
                           zmq_strerror(errno)};

    }

  if(zmq_connect(pActivityXSubSocket_.get(),buff))
    {
      throw StartException{"Error connecting activity xsub socket: %s",
                           zmq_strerror(errno)};
    }
  else
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "Service::%s"
                                           " xsub connected to"
                                           " %s",
                                           __func__,
                                           buff);
    }

  for(const auto & participant : participants_)
    {
      if(zmq_connect(pActivityXSubSocket_.get(),
                     std::string{"tcp://"}.append(participant.sEndpoint_).c_str()))
        {
          throw StartException{"Error connecting activity xsub"
                               " socket to %s: %s",
                               participant.sEndpoint_.c_str(),
                               zmq_strerror(errno)};
        }
      else
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "Service::%s"
                                               " xsub connected to"
                                               " %s",
                                               __func__,
                                               participant.sEndpoint_.c_str());
        }
    }

  if(!sUpstreamTimeRequestEndpoint_.empty())
    {
      createZMQSocket(pContext_,
                      pUpstreamTimeRequestSocket_,
                      ZMQ_REQ,
                      "upstream time req");

      int iLingerMilliseconds = 5000;

      if(zmq_setsockopt(pUpstreamTimeRequestSocket_.get(),
                        ZMQ_LINGER,
                        &iLingerMilliseconds,
                        sizeof(iLingerMilliseconds)))
        {
          throw StartException{"Error setting pstream time req"
                               " socket opiton ZMQ_LINGER: %s",
                               zmq_strerror(errno)};
        }


      if(zmq_connect(pUpstreamTimeRequestSocket_.get(),
                     std::string{"tcp://"}.append(sUpstreamTimeRequestEndpoint_).c_str()))
        {
          throw StartException{"Error connecting upstream time req"
                               " socket to %s: %s",
                               sUpstreamTimeRequestEndpoint_.c_str(),
                               zmq_strerror(errno)};
        }
      else
        {
          LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                               "Service::%s"
                                               " upstream time request connected to"
                                               " %s",
                                               __func__,
                                               sUpstreamTimeRequestEndpoint_.c_str());
        }

      ledgerManager_.setUpstreamTimeSender(this);
    }

  thread_ = std::move(std::thread(&Service::process,this));
}

void SimActivityService::Service::stop()
{
  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s",
                                       __func__);


  eventfd_write(iEventFd_,one);
  thread_.join();
}


void SimActivityService::Service::process()
{
  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s",
                                       __func__);

  try
    {
      bool bRun{true};

      while(bRun)
        {
          std::vector<zmq_pollitem_t> items =
            {
              {nullptr,iEventFd_,ZMQ_POLLIN,0},
              {pActivityXPubSocket_.get(),0,ZMQ_POLLIN,0},
              {pActivityXSubSocket_.get(),0,ZMQ_POLLIN,0},
              {pActivitySubSocket_.get(),0,ZMQ_POLLIN,0},
            };

          // if we are using an upstream time manager, add the
          // response socket
          if(pUpstreamTimeRequestSocket_.get())
            {
              items.push_back({pUpstreamTimeRequestSocket_.get(),
                  0,
                  ZMQ_POLLIN,
                  0});
            }

          int rc = zmq_poll(&items[0],items.size(),iTimeoutMilliseconds_);

          if(rc == -1)
            {
              continue;
            }
          else if(rc == 0)
            {
              ledgerManager_.discover();
            }

          for(const auto & item : items)
            {
              if(item.revents & ZMQ_POLLIN)
                {
                  if(item.fd == iEventFd_)
                    {
                      bRun = false;

                      break;
                    }
                  else if(item.socket == pActivityXPubSocket_.get())
                    {
                      ServiceHandlerActivityXPubXSub::handle(pActivityXPubSocket_.get(),
                                                             pActivityXSubSocket_.get());
                    }
                  else if(item.socket == pActivityXSubSocket_.get())
                    {
                      ServiceHandlerActivityXPubXSub::handle(pActivityXSubSocket_.get(),
                                                             pActivityXPubSocket_.get());
                    }
                  else if(item.socket == pActivitySubSocket_.get())
                    {
                      ServiceHandlerActivitySub::handle(pActivitySubSocket_.get(),
                                                        ledgerManager_);
                    }
                  else if(pUpstreamTimeRequestSocket_.get() &&
                          item.socket == pUpstreamTimeRequestSocket_.get())
                    {
                      ServiceHandlerUpstreamTimeReq::handle(pUpstreamTimeRequestSocket_.get(),
                                                            ledgerManager_);
                    }
                }
            }
        }
    }
  catch(std::exception & exp)
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "Service::%s %s",
                                           __func__,
                                           exp.what());
    }

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s fin",
                                       __func__);
}


void
SimActivityService::Service::sendActivityTimeNotice(const TimePoint & activityTime,
                                                    std::uint64_t u64SequenceNumber)
{
  // once we send the first time notice we no longer need to discover
  // (send hellos)
  iTimeoutMilliseconds_ = -1;

  std::uint64_t u64TimeNoticeMicroseconds =
    std::chrono::duration_cast<Microseconds>
    (activityTime.time_since_epoch()).count();

  std::uint64_t u64TimeNowMicroseconds =
    std::chrono::duration_cast<Microseconds>
    (ledgerManager_.now().time_since_epoch()).count();

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s"
                                       " now: %zu"
                                       " sequence: %zu"
                                       " time notice: %zu",
                                       __func__,
                                       u64TimeNowMicroseconds,
                                       u64SequenceNumber,
                                       u64TimeNoticeMicroseconds);


  SimActivityService::Activity msg{};

  msg.set_type(SimActivityService::Activity::TYPE_ACTIVITY_TIME_NOTICE);

  const auto pTimeNotice = msg.mutable_time_notice();

  pTimeNotice->set_time_microseconds_since_epoch(u64TimeNoticeMicroseconds);

  sendActivity(ledgerManager_.now(),
               u64SequenceNumber,
               Topics::ACTIVITY_SERVICE_TOPIC_TIME_NOTICE,
               msg);
}

void
SimActivityService::Service::sendActivityTimeSignal(std::uint64_t u64SequenceNumber)
{
  std::uint64_t u64TimeNowMicroseconds =
    std::chrono::duration_cast<Microseconds>
    (ledgerManager_.now().time_since_epoch()).count();

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s now: %zu sequence: %zu",
                                       __func__,
                                       u64TimeNowMicroseconds,
                                       u64SequenceNumber);

  SimActivityService::Activity msg{};

  msg.set_type(SimActivityService::Activity::TYPE_ACTIVITY_TIME_SIGNAL);

  sendActivity(ledgerManager_.now(),
               u64SequenceNumber,
               Topics::ACTIVITY_SERVICE_TOPIC_TIME_SIGNAL,
               msg);
}


void
SimActivityService::Service::sendActivityTimeNow(const TimePoint & activityTime,
                                                 std::uint64_t u64SequenceNumber)
{
  std::uint64_t u64TimeMicroseconds =
    std::chrono::duration_cast<Microseconds>
    (activityTime.time_since_epoch()).count();

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s now: %zu sequence: %zu",
                                       __func__,
                                       u64TimeMicroseconds,
                                       u64SequenceNumber);

  SimActivityService::Activity msg{};

  msg.set_type(SimActivityService::Activity::TYPE_ACTIVITY_TIME_NOW);

  const auto pTimeNow = msg.mutable_time_now();

  pTimeNow->set_time_microseconds_since_epoch(u64TimeMicroseconds);

  sendActivity(ledgerManager_.now(),
               u64SequenceNumber,
               Topics::ACTIVITY_SERVICE_TOPIC_TIME_NOW,
               msg);
}


void
SimActivityService::
Service::sendUpstreamTimeNext(const TimePoint & upstreamTime)

{
  std::uint64_t u64TimeMicroseconds =
    std::chrono::duration_cast<Microseconds>
    (upstreamTime.time_since_epoch()).count();

  SimActivityService::UpstreamTime msg{};

  msg.set_type(UpstreamTime::TYPE_REQUEST);

  auto pRequest = msg.mutable_request();

  pRequest->set_type(UpstreamTime::Request::TYPE_REQUEST_TIME_NEXT);

  auto pTimeNext = pRequest->mutable_time_next();

  pTimeNext->set_time_microseconds_since_epoch(u64TimeMicroseconds);

  std::string sSerialization{};

  if(!msg.SerializeToString(&sSerialization))
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "Service::%s"
                                           " unable to serialize upstream time"
                                           " next message",
                                           __func__);
    }
  else
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "Service::%s"
                                           " sending upstream time next: %zu",
                                           __func__,
                                           u64TimeMicroseconds);

      if(zmq_send(pUpstreamTimeRequestSocket_.get(),
                  sSerialization.c_str(),
                  sSerialization.length(),
                  0) < 0)
        {
          LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                               "Service::%s zmq"
                                               " sending upstream time next error: %s",
                                               __func__,
                                               zmq_strerror(errno));
        }
    }
}


void
SimActivityService::Service::sendActivityHello(std::uint64_t u64SequenceNumber)
{
  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s",
                                       __func__);

  SimActivityService::Activity msg{};

  msg.set_type(SimActivityService::Activity::TYPE_ACTIVITY_HELLO);

  sendActivity(TimePoint{},
               u64SequenceNumber,
               Topics::ACTIVITY_SERVICE_TOPIC_HELLO_ANNOUNCE,
               msg);
}


void
SimActivityService::Service::sendActivity(const TimePoint & activityTime,
                                          std::uint64_t u64SequenceNumber,
                                          const std::string & sTopic,
                                          SimActivityService::Activity & msg)
{
  std::uint64_t u64TimeMicroseconds =
    std::chrono::duration_cast<Microseconds>
    (activityTime.time_since_epoch()).count();

  LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                       "Service::%s"
                                       " topic: %s"
                                       " sequence number: %zu"
                                       " activity time: %zu",
                                       __func__,
                                       sTopic.c_str(),
                                       u64SequenceNumber,
                                       u64TimeMicroseconds);


  msg.set_tag("simactivityservice");

  msg.set_uuid(reinterpret_cast<const char *>(uuid_),sizeof(uuid_));

  msg.set_time_microseconds_since_epoch(u64TimeMicroseconds);

  msg.set_sequence_number(u64SequenceNumber);

  std::string sSerialization{};

  if(!msg.SerializeToString(&sSerialization))
    {
      LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                           "Service::%s"
                                           " unable to serialize message: %s",
                                           __func__,
                                           sTopic.c_str());
    }
  else
    {
      LogServiceSingleton::instance()->log(DEBUG_LEVEL,
                                           "Service::%s"
                                           " sending %s",
                                           __func__,
                                           sTopic.c_str());

      if((zmq_send(pActivityPubSocket_.get(),
                   sTopic.c_str(),
                   sTopic.size(),
                   ZMQ_SNDMORE) < 0) ||
         zmq_send(pActivityPubSocket_.get(),
                  sSerialization.c_str(),
                  sSerialization.length(),
                  0) < 0)
        {
          LogServiceSingleton::instance()->log(ERROR_LEVEL,
                                               "Service::%s zmq"
                                               " send %s error %s",
                                               __func__,
                                               sTopic.c_str(),
                                               zmq_strerror(errno));
        }
    }
}
