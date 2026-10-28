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

#ifndef SIMACTIVITYSERVICE_TOPICS_HEADER_
#define SIMACTIVITYSERVICE_TOPICS_HEADER_

namespace SimActivityService
{
  namespace Topics
  {
    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_TIME_NOTICE =
      "simactivityservice.time.notice";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_TIME_CONFIRM_NOTICE =
      "simactivityservice.time.confirm.notice";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_TIME_NOW =
      "simactivityservice.time.now";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_TIME_CONFIRM_NOW =
      "simactivityservice.time.confirm.now";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_TIME_SIGNAL =
      "simactivityservice.time.signal";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_STATUS =
      "simactivityservice.status";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_HELLO_ANNOUNCE =
      "simactivityservice.hello.announce";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_HELLO_ACKNOWLEDGE =
      "simactivityservice.hello.acknowledge";

    static constexpr
    const char * ACTIVITY_SERVICE_TOPIC_ACTION =
      "simactivityservice.action";
  }
}

#endif // SIMACTIVITYSERVICE_TOPICS_HEADER_
