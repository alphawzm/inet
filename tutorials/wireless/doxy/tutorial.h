/**
@mainpage Wireless Tutorial for the INET Framework -- Part 1

In this tutorial, we show you how to build wireless simulations in the INET
Framework. The tutorial contains a series of simulation models numbered from 1 through 14.
The models are of increasing complexity -- they start from the basics and
in each step, they introduce new INET features and concepts related to wireless communication
networks.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in @opp and  INET. If you aren't, you can check out
the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/"
target="_blank">TicToc Tutorial</a> to get started with using @opp. The <a
href="../../../doc/walkthrough/tutorial.html" target="_blank">INET Walkthrough</a>
is an introduction to INET and working with protocols.

If you need more information at any time, feel free to refer to the @opp and
INET documentation:

- <a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
- <a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- <a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

In the tutorial, each step is a separate configuration in the same omnetpp.ini file.
Steps build on each other, they extend the configuration of the previous step by adding
a few new lines. Consecutive steps mostly share the same network, defined in NED.

@section contents Contents

- @ref step1
- @ref step2
- @ref step3
- @ref step4
- @ref step5
- @ref step6
- @ref step7
- @ref step8
- @ref step9
- @ref step10
- @ref step11
- @ref step12
- @ref step13
- @ref step14

@nav{index,step1}
@fixupini

<!------------------------------------------------------------------------>

@page step1 Step 1 - Two hosts communicating wirelessly

@nav{index,step2}

@section s1goals Goals

In the first step, we want to create a network that contains two hosts,
with one host sending a UDP data stream wirelessly to the other. Our goal
is to keep the physical layer and lower layer protocol models as simple
and possible.

We'll make the model more realistic in later steps.

@section s1model The model

In this step we'll use the model depicted below.

<img src="wireless-step1.png">

Here is the NED source of the network:

@dontinclude WirelessA.ned
@skip network WirelessA
@until ####

We'll explain the above NED file below.

<b>The playground</b>

The model contains a playground of the size 500x650 meters, with two hosts
spaced 400 meters apart. (The distance will be relevant in later steps.)
These numbers are set via display strings.

The modules that are present in the network in addition to the hosts are
responsible for tasks like visualization, configuring the IP layer, and
modeling the physical radio channel. We'll return to them later.

<b>The hosts</b>

In INET, hosts are usually represented with the `StandardHost` NED type,
which is a generic template for TCP/IP hosts. It contains protocol
components like TCP, UDP and IP, slots for plugging in application models,
and various network interfaces (NICs). `StandardHost` has some variations
in INET, for example `WirelessHost`, which is basically a `StandardHost`
preconfigured for wireless scenarios.

As you can see, the hosts' type is parametric in this NED file (defined via
a `hostType` parameter and the `INetworkNode` module interface). This done
so that in later steps we can replace hosts with a different NED type. The
actual NED type here is `WirelessHost` (given near the top of the NED
file), and later steps will override this setting using `omnetpp.ini`.

The hosts have to know each others' MAC addresses to communicate, which, in
this model, is taken care of by using `GlobalARP` instead of a real ARP
protocol.

<b>Traffic model</b>

In the model, Host A generates UDP packets which are received by Host B. To
this end, Host A is configured to contain a `UDPBasicApp` module, which generates 1000-byte
UDP messages at random intervals with exponential distribution, the mean of
which is 10ms. Therefore the app is going to generate 100 kbyte/s (800
kbps) UDP traffic, not counting protocol overhead. Host B contains a
`UDPSink` application that just discards received packets.

The model also displays the number of packets received by Host B. The text
is added is added by the `@figure[rcvdPk]` line, and the subsequent few
lines arrange the figure to be updated during the simulation.

<b>Physical layer modeling</b>

Let us concentrate on the module called `radioMedium`.
All wireless simulations in INET need a radio medium module. This module
represents the shared physical medium where communication takes place. It
is responsible for taking signal propagation, attenuation, interference,
and other physical phenomena into account.

INET can model the wireless physical layer at at various levels of detail,
realized with different radio medium modules. In this step, we use
`IdealRadioMedium`, which is the simplest model. It implements a variation
of unit disc radio, meaning that physical phenomena like signal attenuation
are ignored, and the communication range is simply specified in meters.
Transmissions within range are always correctly received unless collisions
occur. Modeling collisions (overlapping transmissions causing reception
failure) and interference range (a range where the signal cannot be
received correctly, but still collides with other signals causing their
reception to fail) are optional.

@note Naturally, this model of the physical layer has little correspondence
to reality. However, it has its uses in the simulation. Its simplicity and
its consequent predictability are an advantage in scenarios where realistic
modeling of the physical layer is not a primary concern, for example in the
modeling of ad-hoc routing protocols. Simulations using `IdealRadioMedium`
also run faster than more realistic ones, due to the low computational
cost.

In hosts, network interface cards are represented by NIC modules. Radio is part of
wireless NIC modules. There are various radio modules, and one must always
use one that is compatible with the medium module. In this step, hosts contain
`IdealRadio` as part of `IdealWirelessNic`.

In this model, we configure the chosen physical layer model
(`IdealRadioMedium` and `IdealRadio`) as follows. The communication range
is set to 500m. Modeling packet losses due to collision (termed
"interference" in this model) is turned off, resulting in pairwise
independent duplex communication channels. The radio data rates are set to
1 Mbps. These values are set in `omnetpp.ini` with the
`communicationRange`, `ignoreInterference`, and `bitrate` parameters of
the appropriate modules.

<b>MAC layer</b>

NICs modules also contain an L2 (i.e. data link layer) protocol. The MAC
protocol in `IdealWirelessNic` is configurable, the default choice being
`IdealMac`. `IdealMac` implements a trivial MAC layer which only provides
encapsulation/decapsulation but no real medium access protocol. There is
virtually no medium access control: packets are transmitted as soon as the
previous packet has completed transmission. `IdealMac` also contains
an optional out-of-band acknowledgement mechanism which we turn off here.

The configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless01]
@until ####


@section s1results Results

When we run the simulation, here's what happens. Host A's `UDPBasicApp`
generates UDP packets at random intervals. These packets are sent down via
UDP and IPv4 to the network interface for transmission. The network
interface queues packets, and transmits them as soon as it can. As long as
there are packets in the network interface's transmission queue, packets
are transmitted back-to-back, with no gaps between subsequent packets.

These events can be followed on OMNeT++'s Qtenv runtime GUI. The following
image has been captured from Qtenv, and shows the inside of Host A during
the simulation. One can see a UDP packet being sent down from the
`udpApp` submodule, traversing the intermediate protocol layers, and being
transmitted by the wlan interface.

<img src="step1_1.gif"> <!-- TODO the animation is completely wrong in Qtenv, must be re-recorded with Tkenv! -->

The next animation shows the communication between the hosts, using
OMNeT++'s default "message sending" animation.

<img src="step1_8.gif">  <!-- TODO re-record so that it has no NAN in it! -->

When the simulation concludes at t=25s, the packet count meter indicates that
around 2000 packets were sent. A packet with overhead is 1028 bytes, which means
the transmission rate was around 660 kbps.

<b>Number of packets received by Host B: 2018</b>

Sources: @ref omnetpp.ini, @ref WirelessA.ned

@nav{index,step2}
@fixupini

<!------------------------------------------------------------------------>

@page step2 Step 2 - Setting up some animations

@nav{step1,step3}

@section s2goals Goals

To facilitate understanding - because a picture worth a thousand words - we will
visualize certain aspects of the simulation throughout this tutorial. In this step,
we will focus on the physical layer, most notably radio transmissions and signal
propagation.

@section s2model The model

Visualization support in INET is implemented as separate modules that
are optional parts of a network model. There are several kinds of visualizers
responsible for showing various aspects of the simulation. Visualizers are
parameterizable, and some visualizers are themselves composed of several
optional components.

The `visualizer` submodule in this network is an `IntegratedCanvasVisualizer`,
which is a compound module that contains all typically useful visualizers as submodules.
It can display physical objects in the physical environment, movement trail,
discovered network connectivity, discovered network routes, ongoing
transmissions, ongoing receptions, propagating radio signals, statistics, and more.

We enable several kinds of visualizations: communication range, signal propagation
and recent successful physical layer transmissions.

The visualization of communication range is enabled using the `displayCommunicationRange`
parameter of the radio module in Host A. It displays a circle around the host
which represents the maximum distance where successful transmission is still possible
with some hosts in the network.

The visualization of signal propagation is enabled with the
`displaySignals` parameter of `MediumCanvasVisualizer`. It displays
transmissions as colored rings emanating from hosts. Since this is
sufficient to represent radio signals visually, it is advisable to turn off
message animations in the Tkenv/Qtenv preferences dialog. The `signalPropagationUpdateInterval`
parameter tells the visualizer to periodically update the display when
there's at least one propagating radio signal on the medium.

The visualization of recent successful physical layer transmissions is
enabled with the `packetNameFilter` parameter of the `physicalLinkVisualizer` submodule.
Matching successful transmissions are displayed with grey arrows that fade with time.
When a packet is successfully received by the physical layer, the arrow between
the transmitter and receiver hosts is created or reinforced. The arrows
visible at any given time indicate recent successful communication patterns.

@dontinclude omnetpp.ini
@skipline

Configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless02]
@until ####

@section s2results Results

The most notable change is the bubble animations representing radio
signals. Each transmission starts with displaying a growing filled circle
centered at the transmitter. The outer edge of the circle indicates the
propagation of the radio signal's first bit. When the transmission ends,
the circle becomes a ring and the inner edge appears at the transmitter.
The growing inner edge of the ring indicates the propagation of the radio
signal's last bit. The reception starts when the outer edge reaches the
receiver, and it finishes when the inner edge arrives.

Note that when the inner edge appears, the outer edge is already far away
from the transmitter. The explanation for that is that in this network,
transmission durations are much longer than propagation times.

The UDP application generates packets at a rate so that there are
back-to-back transmissions. Back-to-back means the first bit of a
transmission immediately follows the last bit of the previous transmission.
This can be seen in the animation, as there are no gaps between the colored
transmission rings. Sometimes the transmission stops for a while,
indicating that the transmission queue became empty.

The blue circle around Host A depicts the communication range, and it clearly shows that
Host B is within the range, therefore successful communication is possible.

The arrow that goes from Host A to Host B indicates successful
communication at the physical layer. The arrow is created after a packet
reception is successfully completed, when the packet is passed up to the
link layer. The arrow is displayed when the reception of the first packet
at Host B is over.

<img src="step2_3.gif">

Frame exchanges may also be visualized using the Sequence Chart tool in the
OMNeT++ IDE. The following image was obtained by recording an event log
(`.elog`) file from the simulation, opening it in the IDE, and tweaking the
settings in the Sequence Chart tool.

The transmission of the packet `UDPData-0` starts at around 15ms, and
completes at around 23ms. The signal propagation takes a nonzero amount of
time, but it's such a small value compared to the transmission duration
that it's not visible in this image. (The arrow signifying the beginning of
the transmission appears to be vertical, one needs to zoom in along the
time axis to see that in fact it is not. In a later step, we will see that
it is possible to configure the Sequence Chart tool to represent time in a
non-linear way.) The chart also indicates that `UDPData-1` and `UDPData-2`
are transmitted back-to-back, because there's no gap between them.

<img src="wireless-step2-seq.png" width=900px>

<b>Number of packets received by Host B: 2018</b>

Sources: @ref omnetpp.ini, @ref WirelessA.ned

@nav{step1,step3}

@fixupini

<!------------------------------------------------------------------------>

@page step3 Step 3 - Adding more nodes and decreasing the communication range

@nav{step2,step4}

@section s3goals Goals

Later in this tutorial, we'll want to turn our model into an ad-hoc network
and experiment with routing. To this end, in this step we add three more
wireless nodes, and reduce the communication range so that our two original
hosts cannot reach one another directly. In later steps, we'll set up
routing and use the extra nodes as relays.

@section s3model The model

We need to add 3 more hosts. This could be done by copying end editing the
network used in the previous steps, but instead we extend WirelessA into
WirelessB using the inheritance feature of NED:

@dontinclude WirelessB.ned
@skip network WirelessB
@until ####

We decrease the communication range of the radios of all hosts to 250
meters. This will make direct communication between hosts A and B
impossible, because their distance is 400 meters. The recently added hosts
are in the correct positions to relay data between hosts A and B, but
routing is not yet configured. The result is that hosts A and B will not be
able to communicate at all.

The configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless03]
@until ####

@section s3results Results

As we run the simulation, we can see that hosts R1 and R2 are the only
hosts in the communication range of Host A. Therefore they are the only ones that
receive Host A's transmissions. This is indicated by the grey arrows
connecting Host A to R1 and R2, respectively, representing recent successful
receptions in the physical layer.

Host B is in the transmission range of Host R1, and R1 could potentially relay A's packets,
but it drops them, because routing is not configured yet (it will be configured
in a later step). Therefore no packets are received by Host B.

<img src="wireless-step3.png">

Host R1's MAC submodule logs indicate that it is discarding the received packets, as they are
not addressed to it:

<img src="wireless-step3-log.png">

<b>Number of packets received by Host B: 0</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step2,step4}

@fixupini

<!------------------------------------------------------------------------>

@page step4 Step 4 - Setting up static routing

@nav{step3,step5}

@section s4goals Goals

In this step, we set up routing so that packets can flow from Host A to B.
For this to happen, the intermediate nodes will need to act as a routers.
As we still want to keep things simple, we'll use statically added routes
that remain unchanged throughout the simulation.

We also configure visualization so that we can see the paths packets take
when traveling from Host A to B.

@section s4model The model

<b>Setting up routing</b>

For the recently added hosts to act as routers, IPv4 forwarding needs to be
enabled. This can be done by setting the `forwarding` parameter of
`StandardHost`.

We also need to set up static routing. Static configuration in the INET
Framework is often done by configurator modules. Static IPv4 configuration,
including address assignment and adding routes, is usually done using the
`IPv4NetworkConfigurator` module. The model already has an instance of this
module, the `configurator` submodule. The configurator can be configured
using an XML specification and some additional parameters. Here, the XML
specification is provided as a string constant inside the ini file.

Without going into details about the contents of the XML configuration
string and other configurator parameters, we tell the configurator to
assign IP addresses in the 10.0.0.x range, and to create routes based on
the estimated packet error rate of links between the nodes. (The
configurator looks at the wireless network as a full graph. Links with high
error rates will have high costs, and links with low error rates will have
low costs. Routes are formed such as to minimize their costs. In the case
of the `IdealRadio` model, the error rate is 1 for nodes that are out of
range, and a very small value for ones in range. The result will be that
nodes that are out of range of each other will send packets to intermediate
nodes that can forward them.)

<b>Visualization</b>

The `IntegratedCanvasVisualizer` we use as the `visualizer` submodule in
this network contains a `routeVisualizer` module which is able to render
packet paths. This module displays paths where a packet has been recently
sent between the network layers of the two end hosts. The path is displayed as
a colored arrow that goes through the hosts visited. The path continually
fades and then it disappears after a certain amount of time unless it is
reinforced by another packet.

The route visualizer is activated by specifying in its `packetNameFilter`
parameter which packets it should take into account. By default it is set
to the empty string, meaning *none*. Setting `*` would mean all packets.
Our UDP application generates packets with the name `UDPData-0`,
`UDPData-1`, etc, so we set the name filter to `UDPData*`
in order to filter out other types of packets that will appear in later
steps.


Configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless04]
@until ####

@section s4results Results

Routing tables are submodules of hosts. The routing table of Host A (10.0.0.1) can be seen in the following image.
It tells that Host B (10.0.0.2) can be reached via Host R1 (10.0.0.3), as specified by the gateway (gw) value.

<img src="wireless-step4-rt.png">

When the first packet sent by Host A arrives at Host R1, a grey arrow appears
between the two hosts indicating a successful physical layer exchange, as it was
noted earlier. A few events later but still at the same simulation time, a green
arrow appears on top of the grey one. The green arrow represents a successful
exchange between the two data link layers of the same hosts. As opposed to the
previous step, this happens because according to the routing table of Host A, a
packet destined to Host B, has to be sent to Host R1 (the gateway). As the packet
reaches the network layer of Host R1, it immediately gets routed according to the
routing table of this host directly towards Host B. So when the first packet arrives
at Host B, first a grey arrow appears, then a green arrow appears on top of that,
similarly to the Host R1 case. Still at the same simulation time the packet leaves
the network layer of Host B towards the UDP protocol. At this moment a new polyline
arrow appears between Host A and Host B going through Host R1. This blue arrow
represents the route the packet has taken from first entering the network layer
at Host A until it left the network layer at Host B.

Note that there are grey arrows leading to Host R2 and R3 even though they don't
transmit. This is because they receive the transmissions at the physical layer,
but they discard the packets at the link layer because it is not addressed to
them.

<img src="wireless-step4.png">

<! number of packets>

<b>Number of packets received by Host B: 2453</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step3,step5}

@fixupini

<!------------------------------------------------------------------------>

@page step5 Step 5 - Taking interference into account

@nav{step4,step6}

@section s5goals Goals

In this step, we make our model of the physical layer a little bit more
realistic. First, we turn on interference modeling in the unit disc radio.
By interference we mean that if two signals collide (arrive at the receiver
at the same time), they will become garbled and the reception will fail.
(Remember that so far we were essentially modeling pairwise duplex
communication.)

Second, we'll set the interference range of the unit disc radio to be 500m,
twice as much as the communication range. The interference range parameter
acknowledges the fact that radio signals become weaker with distance, and
there is a range where they can no longer be received correctly, but they
are still strong enough to interfere with other signals, that is, can cause
the reception to fail. (For completeness, there is a third range called
detection range, where signals are too weak to cause interference, but can
still be detected by the receiver.)

Of course, this change reduces the throughput of the communication
channel, so we expect the number of packets that go through to drop.

@section s5model The model

To turn on interference modeling, we set the `ignoreInterference` parameter
in the receiver part of `IdealRadio` to `false`. Interference range is the
`interferenceRange` parameter of `IdealRadio`'s transmitter part, so we set that to 500m.

We expect that although Host B will not be able to receive Host A's
transmissions, those transmission will still cause interference with other
(e.g. R1's) transmissions at Host B.

To make the animation simpler, we disable displaying successful exchanges at
the physical layer and the data link layer, but displaying the network routes
is still enabled.

@dontinclude omnetpp.ini
@skipline [Config Wireless05]
@until ####

@section s5results Results

Host A is transmitting packets generated by its UDP Application module. R1 is at the right
position to act as a relay between A and B, and it retransmits A's packets to B as soon as it receives them.
Most of the time, A and R1 are transmitting simultaneously, causing two signals to be
present at the receiver of Host B, which results in collisions. When the gap between Host A's
two successive transmissions is large enough, R1 can transmit a packet without collision at B.

When the first packet (UDPData-0) of Host A arrives at Host R1, it gets routed towards
Host B, so Host R1 immediately starts transmitting it. Unfortunately at the same
time, Host A already transmits the second packet (UDPData-1) back-to-back with the
first one. Thus, the reception of the first packet from Host R1 at Host B fails due
to a collision. The colliding packets are the second packet (UDPData-1) of Host A and
the first packet (UDPData-0) of Host R1. This is indicated by the lack of appearance
of a blue polyline arrow between Host A and Host B.

Luckily, the third packet (UDPData-2) of Host A is not part of a back-to-back transmission.
When it gets routed and retransmitted at Host R1, there's no other transmission from Host A
to collide with at Host B. Interestingly, the first bit of the transmission of Host R1 is just
after the last bit of the transmission of Host A. This happens because the processing, including
the routing decision, at Host R1 takes zero amount of time. The successful transmission is
indicated by the appearance of the blue polyline arrow between Host A and Host B.

This is shown in the animation below:

<img src="step5_2.gif">


As we expected, the number of packets received by Host B is low. The following
sequence chart illustrates packet traffic between Hosts A's, R1's and B's network layer.
The image indicates that Host B only occasionally receives packets successfully,
most packets sent by R1 do not make it Host B's IP submodule.
<img src="wireless-step5-seq.png" width=900px>

The image below shows Host R1's and Host A's signals overlapping at Host B.

<img src="wireless-step5-seq-2.png">

To minimize interference, some kind of media access protocol is needed to govern
which host can transmit and when.

<b>Number of packets received by Host B: 197</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step4,step6}
@fixupini

<!------------------------------------------------------------------------>

@page step6 Step 6 - Using CSMA to better utilize the medium

@nav{step5,step7}

@section s6goals Goals

In this step, we try to increase the utilization of the communication
channel by choosing a medium access control (MAC) protocol that is better
suited for wireless communication.

In the previous step, nodes transmitted on the channel immediately when
they had something to send, without first listening for ongoing
transmissions, and this resulted in a lot of collisions and lost packets.
We improve the communication by using the CSMA protocol, which is based
on the "sense before transmit" (or "listen before talk") principle.

CSMA (carrier sense multiple access) is a probabilistic MAC protocol in
which a node verifies the absence of other traffic before transmitting on
the shared transmission medium. In this protocol, a node that has data to
send first waits for the channel to become idle, and then it also waits for
random backoff period. If the channel was still idle during the backoff,
the node can actually start transmitting. Otherwise the procedure starts
over, possibly with an updated range for the backoff period.
We expect that the use of CSMA will improve throughput, as there will be
less collisions, and the medium will be utilized better.

@section s6model The model

To use CSMA, we need to replace `IdealWirelessNic` in the hosts with
`WirelessNic`. However, `WirelessNic` is configured for
IEEE 802.11 by default, so we need to replace both the radio and the MAC
protocol in it. This is done by specifying `IdealRadio` for `radioType`,
and `CSMA` for `macType`.

The `CSMA` module implements Carrier Sense Multiple Access with optional
acknowledgements and a retry mechanism. It has a number of parameters for
tweaking its operation. With the appropriate parameters, it can even
approximate basic 802.11b ad-hoc mode operation. Parameters include:

- bit rate (this is used for both data and ACK frames)
- protocol overhead: MAC header and ACK frame lengths
- acknowledgements on/off
- ACK timeout, maximum retry count
- backoff parameters: strategy (exponential, linear, constant),
  minimum/maximum contention window, backoff slot time
- interval before transmitting ACK frame
- hardware timing parameters such as CCA (Clear Channel Assessment) time
  and radio turnaround time (the time to swich from Rx to Tx state)

For now, we turn off acknowledgement (sending of ACK packets) in CSMA so we
can see purely the effect of "listen before talk" and waiting a random
backoff period before each transmission.


@dontinclude omnetpp.ini
@skipline [Config Wireless06]
@until ####

@section s6results Results

The effect of CSMA can be seen in the animation below. The first packet is sent by Host A,
and after waiting for a backup period, Host R1 retransmits the first packet. This time,
Host B receives it correctly, because only Host R1 is transmitting.

<img src="step6_2_1.gif">

The following sequence chart displays that after receiving the UDPData-0 packet,
Host R1 transmits it after the backoff period timer has expired.

<!--TODO: backoff time sequence chart, FIX CSMA first -->
<img src="wireless-step6-seq-2.png">

<b>Number of packets received by Host B: 1172</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step5,step7}
@fixupini

<!------------------------------------------------------------------------>

@page step7 Step 7 - Turning on ACKs in CSMA

@nav{step6,step8}

@section s7goals Goals

In this step, we try to make link layer communication more reliable by
adding acknowledgement to the MAC protocol.

TODO We not expect that the use of CSMA will improve throughput, but
packets will not be lost etc.

@section s7model The model

We turn on ACKs by setting the `useMACAcks` parameter of `CSMA`.

@dontinclude omnetpp.ini
@skipline [Config Wireless07]
@until ####

@section s7results Results

With ACKs enabled, each successfully received packet is acknowledged. The animation
below depicts a packet transmission from Host A, and the corresponding ACK
from Host R1.

<img src="step7_2_2.gif">

The UDPData + ACK sequences can can be seen in the sequence chart below:

<img src="wireless-step7-seq.png">

When the channel utilization is low, and collisions are infrequent, using ACKs
doesnt have a positive impact on the number of correctly received packets, as
they increase the overhead of transmissions. However, when the channel is operating
close to its capacity, and more collisions happen, ACKs can increase the number of
successfuly received packets by forcing hosts to retransmit lost ones. This implements
a kind of reliable packet transport, where lost packets are always retransmitted.

<b>Number of packets received by Host B: TODO</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step6,step8}
@fixupini

<!------------------------------------------------------------------------>

@page step8 Step 8 - Modeling energy consumption

@nav{step7,step9}

@section s8goals Goals

Wireless ad-hoc networks often operate in an energy-constrained
environment, and thus it is often useful to model the energy consumption of
the devices. Consider, for example, wireless motes that operate on battery.
The mote's activity has be planned so that the battery lasts until it can
be recharged or replaced.

In this step, we augment the nodes with components so that we can model
(and measure) their energy consumption. For simplicity, we ignore energy
constraints, and just install infinite energy sources into the nodes.

@section s8model The model

<b>Energy consumption model</b>

In a real system, there are energy consumers like the radio, and energy
sources like a battery or the mains. In the INET model of the radio, energy
consumption is represented by a separate component. This <i>energy
consumption model</i> maps the activity of the core parts of the radio (the
transmitter and the receiver) to power (energy) consumption. The core parts
of the radio themselves do not contain anything about power consumption,
they only expose their state variables. This allows one to switch to
arbitrarily complex (or simple) power consumption models, without affecting
the operation of the radio. The energy consumption model can be specified
in the `energyConsumerType` parameter of the `Radio` module.

Here, we set the energy consumption model in the node radios to
`StateBasedEnergyConsumer`. `StateBasedEnergyConsumer` models radio power
consumption based on states like radio mode, transmitter and receiver
state. Each state has a constant power consumption that can be set by a
parameter. Energy use depends on how much time the radio spends in a
particular state.

To go into a little bit more detail: the radio maintains two state
variables, _receive state_ and _transmit state_. At any given time, the
radio mode (one of _off_, _sleep_, _switching_, _receiver_, _transmitter_
and _transciever_) decides which of the two state variables are valid. The
receive state may be _idle_, _busy_, or _receiving_, the former two
referring to the channel state. When it is _receiving_, a sub-state stores
which part of the signal it is receiving: the _preamble_, the (physical
layer) _header_, the _data_, or _any_ (we don't know/care). Similarly, the
transmit state may be _idle_ or _transmitting_, and a sub-state stores
which part of the signal is being transmitted (if any).

`StateBasedEnergyConsumer` expects the consumption in various states to be
specified in watts in parameters like `sleep&shy;PowerConsumption`,
`receiverBusy&shy;PowerConsumption`,
`transmitterTransmitting&shy;PreamblePowerConsumption` and so on.

<b>Measuring energy consumption</b>

Hosts contain an energy storage component that basically models an energy
source like a battery or the mains. INET contains several <i>energy storage
models</i>, and the desired one can be selected in the `energyStorageType`
parameter of the host. Also, the radio's energy consumption model is
preconfigured to draw energy from the host's energy storage. (Hosts with
more than one energy storage component are also possible.)

In this model, we use `IdealEnergyStorage` in hosts. `IdealEnergyStorage`
provides an infinite amount of energy, can't be fully charged or depleted.
We use this because we want to concentrate on the power consumption, not
the storage.

The energy storage module contains an `energyBalance` watched variable that
can be used to track energy consumption. Also, the `residualCapacity`
signal can be used to display energy consumption over time.  (TODO how? why mentioned?)

Configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless08]
@until ####

@section s8results Results

The image below shows Host A's 'energyBalance' variable at the end of the simulation.
The negative energy value signifies the consumption of energy.

<img src="wireless-step8-energy-2.png">

The 'residualCapacity' signal of Hosts A, R1 and B is plotted in following diagram.
The diagram shows that Host A has the consumed the most power because it transmitted more than
the other nodes.

<! R1 should have transmitted the most because of ACKs>

<img src="wireless-step8.png">

<b>Number of packets received by Host B: 980</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step7,step9}
@fixupini

<!------------------------------------------------------------------------>

@page step9 Step 9 - Configuring node movements

@nav{step8,step10}

@section s9goals Goals

In this step, we make the model more interesting by adding node mobility.
Namely, we make the intermediate nodes travel north during simulation.
After a while, they will move out of the range of Host A (and B), breaking the
communication path.

@section s9model The model

In the INET Framework, node mobility is handled by the `mobility` submodule
of hosts. Several mobility module type exist that can be plugged into a
host. The movement trail may be deterministic (such as line, rectangle or
circle), probabilistic (e.g. random waypoint), scripted (e.g. a "turtle"
script) or trace-driven. There are also individual and group mobility
models.

Here we install `LinearMobility` into the intermediate nodes.
`LinearMobility` implements movement along a line, where the heading and
speed are parameters. We configure the nodes to move north at the speed of
12 m/s.

The visualization of radio signals as expanding bubbles is no longer needed,
so we turn it off.

@dontinclude omnetpp.ini
@skipline [Config Wireless09]
@until ####

@section s9results Results

It is advisable to run the simulation in Fast mode, because the nodes move very slowly if
viewed in Normal mode.

It can be seen in the animation below as Host R1 leaves Host A's communication range at around 11 seconds.
After that, the communication path is broken. Traffic could be routed through R2 and R3, but that does not happen because
the routing tables are static and have been configured according to the
initial positions of the nodes. The intermediate hosts are leaving behind a movement trail as they are traveling.
When the communication path breaks, the blue arrow that represents successful network layer communication paths fades away,
because there are no more packets to reinforce it.

<img src="step9_1.gif">

As mentioned before, a communication path could be established between Host A and B by routing traffic
through Hosts R2 and R3. To reconfigure routes according to the changing topology of the network, an
ad-hoc routing protocol is required.

<b>Number of packets received by Host B: 264</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step8,step10}
@fixupini

<!------------------------------------------------------------------------>

@page step10 Step 10 - Configuring ad-hoc routing (AODV)

@nav{step9,step11}

@section s10goals Goals

In this step, we configure a routing protocol that adapts to the changing
network topology, and will arrange packets to be routed through `R2` and `R3`
as `R1` departs.

We'll use AODV (ad hoc on-demand distance vector routing). It is a
reactive routing protocol, which means its maintenance of the routing
tables is driven by demand. This is in contrast to proactive routing
protocols which keep routing tables up to date all the time (or at least
try to).

@section s10model The model

Let's configure ad-hoc routing with AODV. As AODV will manage the routing
tables, we don't need the statically added routes any more. We only need
`IPv4NetworkConfigurator` to assign the IP addresses, and turn all other
functions off.

More important, we change the hosts to be instances of `AODVRouter`.
`AODVRouter` is like  `WirelessHost`, but with an added `AODVRouting`
submodule. This change turns each node into an AODV router.

AODV stands for Ad hoc On-Demand Distance Vector. In AODV, routes are
established as they are needed. Once established, a route is maintained
as long as it is needed.

In AODV, the network is silent until a connection is needed. At that point
the network node that needs a connection broadcasts a request for connection.
Other AODV nodes forward this message, and record the node that they heard
it from, creating an explosion of temporary routes back to the needy node.
When a node receives such a message and already has a route to the desired
node, it sends a message backwards through a temporary route to the requesting
node. The needy node then begins using the route that has the least number
of hops through other nodes. Unused entries in the routing tables are recycled
after a time.

The message types defined by AODV are Route Request (RREQ), Route Reply (RREP),
and Route Error (RERRs). We expect to see these messages at the start of the
simulation (when an initial route needs to be established), and later when
the topology of the network changes due to the movement of nodes.

The configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless10]
@until ####

@section s10results Results

Host R1 moves out of communication range or Hosts A and B. The route that was
established through R1 is broken. Hosts R2 and R3 are at the right position to
relay Host A's packets to Host B, and the AODV protocol reconfigures the route
to go through R2 and R3. The UDP stream is re-established using these
intermediate hosts to relay Host A's packets to Host B. This can be seen in the
animation below.

<img src="step10_4.gif">

AODV detects when a route is no longer able to pass packets. When the link is
broken, there are no ACKs arriving at Host A, so its AODV submodule triggers the
reconfiguration of the route. To detect possible new routes and establish one,
there is a burst of AODV transmissions between the hosts, and a new route is
established through Hosts R2 and R3. This detection and reconfiguration takes
very little time. After the short AODV packet burst, the arrows displaying it
quickly fade, and the UDP traffic continues.

<img src="step10_1.gif">

The AODV Route-discovery messages can be seen in the following packet log:

<img src="wireless-step10-packetlog.png">

The number of successfully received packets at Host B is about two times
compared to the previous step. This is because the flow of packets doesn't stop
when Host R1 gets out of communication range of Host A. Although the AODV
protocol adds come overhead, in this simulation it is not significant, the
number received packets still increase substantially.

<b>Number of packets received by Host B: 446</b>

Sources: @ref omnetpp.ini, @ref WirelessB.ned

@nav{step9,step11}
@fixupini

<!------------------------------------------------------------------------>

@page step11 Step 11 - Adding obstacles to the environment

@nav{step10,step12}

@section s11goals Goals

In an attempt to make our simulation both more realistic and more
interesting, we add some obstacles to the playground.

In the real world, objects like walls, trees, buildings and hills act as
obstacles to radio signal propagation. They absorb and reflect radio waves,
reducing signal quality and decreasing the chance of successful reception.

In this step, we add a concrete wall to the model that sits between Host A
and `R1`, and see what happens. Since our model still uses the ideal radio
and ideal wireless medium models that do not model physical phenomena,
obstacle modeling will be very simple: all obstacles completely absorb
radio signals, making reception behind them impossible.

@section s11model The model

First, we need to represent obstacles. In INET, obstacles are managed as
part of the `PhysicalEnvironment` module, so we need to add an instance to
the `WirelessB` network:

@dontinclude WirelessC.ned
@skip network WirelessC
@until ####

Obstacles are described in an XML file. An obstacle is defined by its
shape, location, orientation, and material. It may also have a name, and
one can define how it should be rendered (color, line width, opacity, etc.)
The XML format allows one to use predefined shapes like cuboid, prism,
polyhedron or sphere, and also to define new shapes that may be reused for
any number of obstacles. It is similar for materials: there are predefined
materials like concrete, brick, wood, glass, forest, and one can also
define new materials. A material is defined with its physical properties
like resistivity, relative permittivity and relative permeability. These
properties are used in the computations of dielectric loss tangent,
refractive index and signal propagation speed, and ultimately in the
computation of signal loss.

Our wall is defined in walls.xml, and the file name is given to
`PhysicalEnvironment` in its `config` parameter.

Having obstacles is not enough in itself, we also need to teach the model
of the wireless medium to take them into account. This is done by
specifying an <i>obstacle loss model</i>. Since our model contains
`IdealMedium`, we specify `IdealObstacleLoss`. With `IdealObstacleLoss`,
obstacles completely block radio signals, making reception behind them
impossible.

TODO Visualization

The configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless11]
@until ####

The walls.xml file:

@dontinclude walls.xml
@skip environment
@until /environment

@section s11results Results

At the beginning of the simulation, the initial route the was established in
previous steps (A-R1-B) cannot be established, because the wall is between Host
A and R1. The wall is completely blocking transmissions, therefore AODV
establishes the A-R2-R1-B route. Host R2's transmission is cut when R2 moves
behind the wall. This time, however, Host R1 is available to relay Host A's
transmissions to Host B. A new route is formed, and traffic continues to use
this route until Host R1 moves out of communication range. After that, the
A-R2-R3-B route is used, as seen in the previous steps.

<img src="step11_1.gif">

<b>Number of packets received by Host B: TODO</b>

Sources: omnetpp.ini, WirelessC.ned, walls.xml

@nav{step10,step12}
@fixupini

<!------------------------------------------------------------------------>

@page step12 Step 12 - Changing to a more realistic radio model

@nav{step11,step13}

@section s12goals Goals

After so many steps, we let go of the ideal radio model, and introduce a
more realistic one. Our new radio will use an APSK modulation scheme, but
still without other techniques like forward error correction, interleaving
or spreading. We also want our model of the radio channel to simulate
attenuation and obstacle loss.

@section s12model The model

<b>Switching to APSK radio</b>

In this step, we replace `IdealRadio` with `APSKScalarRadio`.
`APSKScalarRadio` models a radio with an APSK (amplitude and phase-shift
keying) modulation scheme. By default it uses BPSK, but QPSK, QAM-16,
QAM-64, QAM-256 and several other modulations can also be configured.
(Modulation is a parameter of the radio's transmitter component.)

Since we are moving away from the "unit disc radio" type of abstraction, we
need to specify the carrier frequency, signal bandwidth and transmission
power of the radios. Together with other parameters, they will allow the
radio channel and the receiver models to compute path loss, SNIR, bit error
rate and other values, and ultimately determine the success of reception.

`APSKScalarRadio` also adds realism in that it simulates that the data are
preceded by a preamble and a physical layer header. Their lengths are also
parameters (and may be set to zero when not needed.)

Physical parameters of the receiver are important, too. We configure the
following receiver parameters:
- sensitivity [dBm]: if the signal power is below this threshold, reception
  is not possible  (i.e. the receiver cannot go from the *channel busy*
  state to *receiving*)
- energy detection threshold [dBm]: if reception power is below this
  threshold, no signal is detected and the channel is sensed to be empty
  (this is significant for the "carrier sense" part of CSMA)
- SNIR threshold [dB]: reception is not successful if the SNIR is below
  this threshold

The concrete values in the inifile were chosen to approximately
reproduce the communication and interference ranges used in the
previous steps.

<b>Setting up the wireless channel</b>

Since we switched the radio to `APSKScalarRadio`, we also need to change
the medium to `APSKScalarRadioMedium`. In general, one always needs to use
a medium that is compatible with the given radio. (With `IdealRadio`, we
also used `IdealRadioMedium`.)

`APSKScalarRadioMedium` has "slots" to plug in various propagation
models, path loss models, obstacle loss models, analog models and
background noise models. Here we make use of the fact that the default
background noise model is homogeneous isotropic white noise, and set up the
noise level to a nonzero value (-110dBm).

Configuration:

@dontinclude omnetpp.ini
@skipline [Config Wireless12]
@until ####


@section s12results Results

What happens is about the same as in the previous step. At first, Host A's
packets are relayed by Host R2 until it moves so that the wall separates them.
The connection is re-established when Host R1 moves out from behind the wall.
Then it gets out of communication range, and the new route goes through Hosts R2
and R3.

In this model, there are more physical effects simulated than in previous steps.
There is radio signal attenuation, background noise and a more realistic radio
model. The blue circles representing communication range is an approximation.
There is no distinct distance where receptions fail, as in the case of
'IdealRadio'.

<b>Number of packets received by Host B: 285</b>

Sources: @ref omnetpp.ini, @ref WirelessC.ned

@nav{step11,step13}
@fixupini

<!------------------------------------------------------------------------>

@page step13 Step 13 - Configuring a more accurate pathloss model

@nav{step12,step14}

@section s13goals Goals

By default, the medium uses the free-space path loss model, which assumes
line-of-sight path, with no obstacles nearby to cause reflection or
diffraction. Since our wireless hosts move on the ground, a more accurate
path loss model would be the two-ray ground reflection model that
calculates with one reflection from the ground.

@section s13model The model

It has been mentioned that `APSKScalarRadioMedium` relies on various
subcomponents for computing path loss, obstacle loss, and background noise,
among others. Installing the two-ray ground reflection model is just a matter
of changing its `pathLossType` parameter from the default
`FreeSpacePathLoss` to `TwoRayGroundReflection`. (Further options include
`RayleighFading`, `RicianFading`, `LogNormalShadowing` and some others.)

The two-ray ground reflection model uses the altitudes of the transmitter
and the receiver antennas above the ground as inputs. To compute the
altitude, we need the hosts' (x,y,z) positions, and the ground's elevation
at those points. In INET, the ground's elevation is defined by the
<i>ground model</i>. which is part of the physical environment model.

In this model, we'll use `FlatGround` for ground model, and specify it to
the `physicalEnvironment` module. (Note that we added `physicalEnvironment`
to the network when we introduced obstacles.) The ground's elevation is the
`elevation` parameter of `FlatGround`. We set this parameter to -1m to have
antennas that are one meter above the ground. -1m may seem like an odd
choice, but it is actually very convenient, because the hosts' _z_
coordinates are zero. (That is the default when the hosts' positions are
initialized from display strings.)

@dontinclude omnetpp.ini
@skipline [Config Wireless13]
@until ####

@section s13results Results

TODO

The image below shows the bit error rate of Host R1's radio, as a function of
time. The bit error rate is shown when free space space propagation is used, and
when using two ray ground reflection. The interval shown here corresponds to the
time in the simulation when Host R1 is not cut off from Host A by the wall
anymore, and also still in communication range of Host A and B. In the interval
that is shown, from around 5 seconds to 11 seconds, the distance between Host R1
and Hosts A and B is increasing, which results in increase in the bit error rate
as well. There are is no significant difference between the free space
propagation path loss and the two ray ground reflection path loss at first. The
two curves separate towards the end of the displayed interval. As expected, in
the case of the two ray ground reflection model, the bit error rate is greater.

<img src="wireless-step13.png">

<b>Number of packets received by Host B: 243</b>

@nav{step12,step14}
@fixupini

<!------------------------------------------------------------------------>

@page step14 Step 14 - Introducing antenna gain

@nav{step13,index}

@section s14goals Goals

In the previous steps, we have assumed an isotropic antenna for the radio,
with a gain of 1 (0dB). Here we want to enhance the simulation by taking
antenna gain into account.

@section s14model The model

For simplicity, we configure the hosts to use `ConstantGainAntenna`.
`ConstantGainAntenna` is an abstraction: it models an antenna that has a
constant gain in the directions relevant for the simulation, regardless of
how such antenna could be implemented in real life. For example, if all
nodes of a simulated wireless network are on the same plane,
`ConstantGainAntenna` could correspond to an omnidirectional antenna such
as a vertical dipole. (INET contains support for directional antennas as
well.)


@dontinclude omnetpp.ini
@skipline [Config Wireless14]
@until ####

@section s14results Results

TODO

<b>Number of packets received by Host B: 942</b>

@nav{step13,index}
@fixupini

*/