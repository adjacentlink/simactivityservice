/*
 * Copyright (c) 2013-2014,2025 - Adjacent Link LLC, Bridgewater,
 *  New Jersey
 * Copyright (c) 2008 - DRS CenGen, LLC, Columbia, Maryland
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
 * * Neither the name of DRS CenGen, LLC nor the names of its
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

#ifndef SIMACTIVITYSERVICE_EXCEPTION_HEADER_
#define SIMACTIVITYSERVICE_EXCEPTION_HEADER_

#include <exception>
#include <string>
#include <cstdarg>

namespace SimActivityService
{
  class Exception : public std::exception
  {
  public:
    virtual ~Exception() noexcept;

    const char* what() const noexcept;

    const char * type() const noexcept;

  protected:
    Exception(const std::string & sType,
              const std::string & sDescription = {}):
      sType_{sType},
      sDescription_{sDescription}{}

    void setDescription(const std::string & sDescription)
    {
      sDescription_ = sDescription;
    }

  private:
    const std::string sType_;
    std::string sDescription_;
  };
}

#define SIMACTIVITYSERVICE_EXCEPTION_DEFINE(x)  \
  namespace SimActivityService                  \
  {                                             \
    class x : public Exception                  \
    {                                           \
    public:                                     \
    x(const char * fmt,...):                    \
      Exception{#x}                             \
      {                                         \
        char buf[1024];                         \
        va_list ap;                             \
        va_start(ap,fmt);                       \
        vsnprintf(buf,sizeof(buf),fmt,ap);      \
        va_end(ap);                             \
        setDescription(buf);                    \
      }                                         \
      x(const std::string & s):                 \
        Exception{#x}                           \
      {                                         \
        setDescription(s);                      \
      }                                         \
      ~x() noexcept {}                          \
    };                                          \
  }

#include "exception.inl"

#endif // SIMACTIVITYSERVICE_EXCEPTION_HEADER_
