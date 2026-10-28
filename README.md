simactivityservice
==

The *Simulation Activity Service* (*SAS*) provides simulation
experiment time coordination using [Google Protocol
Buffers](https://protobuf.dev/) defined *activity messages* exchanged
via [ZeroMQ](https://zeromq.org/) PUB/SUB. Applications relying on
*SAS* time are referred to as *participants*. Participants who perform
activities that interact with other participants, such as sending
control and/or data messages, are assigned to *action groups* in order
to guarantee that all participants have reached an idle work state
prior to *SAS* time progression.
