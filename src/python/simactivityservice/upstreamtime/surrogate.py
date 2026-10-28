# Copyright (c) 2025 - Adjacent Link LLC, Bridgewater, New Jersey
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
# * Neither the name of Adjacent Link LLC nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# See toplevel COPYING for more information.
#

import zmq
import os
import logging
import uuid
import ctypes
import struct
import secrets
import sys
import traceback

from threading import Thread

import simactivityservice.interface.upstreamtime_pb2 as upstreamtime_pb2

class Surrogate(object):
    def __init__(self,
                 upstreamtime_response_endpoint,
                 nth,
                 change_nth_by,
                 stop_callback):
        logging.info('upstreamtime_response_endpoint: {}'.
                     format(upstreamtime_response_endpoint))

        logging.info('nth: {}'.format(nth))

        logging.info('change_nth_by: {}'.
                     format(change_nth_by))

        self._thread = None

        self._libc = ctypes.CDLL(None)

        self._eventfd = \
            self._libc.eventfd(ctypes.c_uint(0),
                               ctypes.c_uint(0))

        self._context = zmq.Context()

        self._response = self._context.socket(zmq.REP)

        self._response.setsockopt(zmq.IPV6,1)

        self._response.bind('tcp://{}'.format(upstreamtime_response_endpoint))

        self._stop_callback = stop_callback

        self._nth = nth

        self._change_nth_by = change_nth_by

        self._time_next_count = 0

    def start(self):
        logging.info('starting')
        self._thread = Thread(target=self.__process)
        self._thread.start()

    def __process(self):
        simulation_time_start_microseconds = None

        poller = zmq.Poller()

        poller.register(self._eventfd,zmq.POLLIN)
        poller.register(self._response,zmq.POLLIN)

        while True:

            try:
                socks = dict(poller.poll())

                if self._eventfd in socks and \
                   socks[self._eventfd] == zmq.POLLIN:
                    break

                elif self._response in socks and \
                    socks[self._response] == zmq.POLLIN:

                    msg = self._response.recv()

                    request = upstreamtime_pb2.UpstreamTime()

                    request.ParseFromString(msg)

                    if request.type == upstreamtime_pb2.UpstreamTime.TYPE_REQUEST:

                        if request.HasField('request'):
                            if request.request.type == \
                               upstreamtime_pb2.UpstreamTime.Request.TYPE_REQUEST_TIME_NEXT:

                                if request.request.HasField('time_next'):
                                    self._time_next_count += 1

                                    next_time = \
                                        request.request.time_next.time_microseconds_since_epoch

                                    logging.debug('received time next {}'.format(next_time))

                                    response = upstreamtime_pb2.UpstreamTime()

                                    response.type = upstreamtime_pb2.UpstreamTime.TYPE_RESPONSE

                                    response.response.type = upstreamtime_pb2.UpstreamTime.Response.TYPE_RESPONSE_TIME_NOW

                                    if self._nth and not self._time_next_count % self._nth:
                                        logging.debug('changing {} to {}'.format(next_time,
                                                                                 next_time + self._change_nth_by))
                                        next_time += self._change_nth_by

                                    response.response.time_now.time_microseconds_since_epoch = next_time

                                    logging.debug('sending time now {}'.format(next_time))

                                    self._response.send(response.SerializeToString())

                                else:
                                    logging.error('mailformed UpstreamTime Request missing: next_time')

                            else:
                                logging.error('invalid request type')

                        else:
                            logging.error('mailformed UpstreamTime missing: request')

            except:
                logging.error(traceback.format_exc())



    def stop(self):
        if self._thread != None:
            os.write(self._eventfd,struct.pack('Q',1))
            self._thread.join()
            self._thread = None
