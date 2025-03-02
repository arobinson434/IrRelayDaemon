# IR Relay Daemon
This project houses a simple IR relay service that is intended to run on a
Raspberry Pi. The daemon listens to the network for incoming, IR transmission
requests. When received, the daemon with transmit the IR signal described in
the request. The daemon will also listen for a button push on the Raspberry Pi,
and once triggered, will listen for an IR signal on it's IR receiver. If a
signal is received, it will publish that signal to the network.

This daemon is intended to be used by/with my [PiMote](https://github.com/arobinson434/PiMote)
phone app, which can 'learn' IR commands published by this daemon, and issue
transmission requests to this daemon.

## Build Environment
I am cross-compiling this with buildroot.

In the near future, I plan to build up a buildroot external config repo, which
will pull and build this project by default. Once I am done with that effort,
I will link that repo here.

In the mean time, you too can build this with buildroot's tool-chain assuming
you have ProtoBuf and Boost in that environment.

## Structure
This program runs four services across three threads (including the main
thread).
* `PresenceNotifier` - This service periodically publishes it's name and
description to the network. This is how the phone app discovers the device.
* `StatusLedMgr` - This service manages the status LED for the daemon. It is
thread safe and is accessed by other services.
* `LearningService` - This service listens for a button press, indicating that
we want to record an IR signal. Once recorded, it will publish the details of
that signal to the network.
* `ListeningService` - This service listens for IR transmission requests from
the network, and issues those commands.

The `PresenceNotifier` and `StatusLedMgr` services run on the main thread, and
the `LearningService` and `ListeningService` each run on their own threads.

## Status LED
The status LED conveys the following:
| LED                   | Meaning                                                      |
|:---------             |:--------                                                     |
|2 x Green+Blue Flashes | The daemon has started                                       |
|1 x Green Flashes      | Network request received and issued over IR                  |
|Continued Blue Flashes | The board is in learning mode, and waiting for an IR signal  |
|2 x Green Flashes      | An IR signal has been recorded, and published to the network |
|1 x Red Flash          | Learning mode timed out waiting for an IR signal             |
|2 x Red Flash          | Learning mode received an invalid IR signal                  |
|3 x Red Flash          | Learning mode failed to publish the recorded IR signal       |
