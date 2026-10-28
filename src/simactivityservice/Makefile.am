bin_PROGRAMS = simactivityservice

simactivityservice_CPPFLAGS = \
 -I@top_srcdir@/include \
 $(AM_CPPFLAGS) \
 $(simactivityservice_CFLAGS)

simactivityservice_LDADD = \
 $(simactivityservice_LIBS)

nodist_simactivityservice_SOURCES = \
 loggermessage.pb.cc \
 loggermessage.pb.h \
 simactivityservice.pb.cc \
 simactivityservice.pb.h \
 upstreamtime.pb.cc \
 upstreamtime.pb.h

simactivityservice_SOURCES = \
 actionledgerstate.cc \
 actionledgerstate.h \
 activitytimesender.h \
 configureexception.h \
 datagramsocket.cc \
 datagramsocket.h \
 errorledgerstate.cc \
 errorledgerstate.h \
 exception.h \
 exception.inl \
 inetaddr.cc \
 inetaddrexception.h \
 inetaddr.h \
 initialledgerstate.cc \
 initialledgerstate.h \
 initialtimenoticeconfirmledgerstate.cc \
 initialtimenoticeconfirmledgerstate.h \
 ledgermanager.cc \
 ledgermanager.h \
 ledgerstate.cc \
 ledgerstate.h \
 ledgerstateuser.cc \
 ledgerstateuser.h \
 loggerlevelconvert.h \
 loggerlevelconvert.inl \
 loggerrecordmessage.cc \
 loggerrecordmessage.h \
 loglevel.h \
 logservice.cc \
 logservice.h \
 logservice.inl \
 parameterconvertexception.h \
 parameterconvert.h \
 parameterconvert.inl \
 participant.h \
 raiiobject.h \
 raiizmq.h \
 serializationexception.h \
 service.cc \
 service.h \
 servicehandleractivitysub.cc \
 servicehandleractivitysub.h \
 servicehandleractivityxpubxsub.cc \
 servicehandleractivityxpubxsub.h \
 servicehandlerupstreamtimereq.cc \
 servicehandlerupstreamtimereq.h \
 simactivityservice.cc \
 singleton.h \
 socketexception.h \
 startexception.h \
 statisticpublisher.cc \
 statisticpublisher.h \
 timenoticecancelledgerstate.cc \
 timenoticecancelledgerstate.h \
 timenoticeconfirmledgerstate.cc \
 timenoticeconfirmledgerstate.h \
 timenowconfirmledgerstate.cc \
 timenowconfirmledgerstate.h \
 types.h \
 upstreamtimenowwaitledgerstate.cc \
 upstreamtimenowwaitledgerstate.h \
 upstreamtimesender.h \
 vectorio.h

BUILT_SOURCES = \
 $(nodist_simactivityservice_SOURCES)

EXTRA_DIST = \
 loggermessage.proto

clean-local:
	rm -f $(BUILT_SOURCES)

simactivityservice.pb.cc simactivityservice.pb.h: @top_srcdir@/include/simactivityservice/proto/simactivityservice.proto
	protoc -I=@top_srcdir@/include/simactivityservice/proto --cpp_out=. $<

upstreamtime.pb.cc upstreamtime.pb.h: @top_srcdir@/include/simactivityservice/proto/upstreamtime.proto
	protoc -I=@top_srcdir@/include/simactivityservice/proto --cpp_out=. $<

loggermessage.pb.cc loggermessage.pb.h: loggermessage.proto
	protoc -I=. --cpp_out=. $<
