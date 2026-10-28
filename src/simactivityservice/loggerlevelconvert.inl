/*
 * Copyright (c) 2013-2014,2025 - Adjacent Link LLC, Bridgewater,
 *  New Jersey
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

inline SimActivityServiceMessage::LoggerMessage_LogLevel
SimActivityService::convertLogLevel(LogLevel level)
{
  switch(level)
    {
    case ABORT_LEVEL:
      return SimActivityServiceMessage::LoggerMessage_LogLevel_ABORT_LEVEL;

    case ERROR_LEVEL:
      return SimActivityServiceMessage::LoggerMessage_LogLevel_ERROR_LEVEL;

    case INFO_LEVEL:
      return SimActivityServiceMessage::LoggerMessage_LogLevel_INFO_LEVEL;

    default:
      return SimActivityServiceMessage::LoggerMessage_LogLevel_DEBUG_LEVEL;
    }
}


inline SimActivityService::LogLevel
SimActivityService::convertLogLevel(SimActivityServiceMessage::LoggerMessage_LogLevel level)
{
  switch(level)
    {
    case SimActivityServiceMessage::LoggerMessage_LogLevel_ABORT_LEVEL:
      return ABORT_LEVEL;

    case SimActivityServiceMessage::LoggerMessage_LogLevel_ERROR_LEVEL:
      return ERROR_LEVEL;

    case SimActivityServiceMessage::LoggerMessage_LogLevel_INFO_LEVEL:
      return INFO_LEVEL;

    default:
      return DEBUG_LEVEL;
    }
}
