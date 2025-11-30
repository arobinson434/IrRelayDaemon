# IR Relay Daemon
This project houses a simple IR relay service that is intended to run on a
Raspberry Pi. The daemon listens to the network for incoming, IR transmission
requests. When received, the daemon will transmit the IR signal described in
the request. The daemon will also listen for an IR signal on it's IR receiver.
If a signal is received, it will publish that signal to the network.

This daemon is intended to be used by/with my [PiMote](https://github.com/arobinson434/PiMote)
phone app, which can 'learn' IR commands published by this daemon, and issue
transmission requests to this daemon.

## Build Environment
I am cross-compiling this with buildroot.

If you wish to build a minimal buildroot image with this package, my
[PiIrRelay](https://github.com/arobinson434/PiIrRelay) project should have you
covered! Just head there, and follow the instructions.

If you want to build this daemon standalone, you can do that here, provided that
you have `boost` (with `boost-program_options`), `protobuf`, and `libgpiod2`.
Additionally, you may need to modify `constants/hardware.h`, depending on your
GPIO setup.

## Structure
This program runs four services across three threads (including the main
thread).
* `PresenceNotifier` - This service periodically publishes it's name and
description to the network. This is how the phone app discovers the device.
* `StatusLedMgr` - This service manages the status LED for the daemon. It is
thread safe and is accessed by other services.
* `LearningService` - This service listens for  an IR signal. Once recorded,
it will publish the details of that signal to the network.
* `ListeningService` - This service listens for IR transmission requests from
the network, and issues those commands.

The `PresenceNotifier` and `StatusLedMgr` services run on the main thread, and
the `LearningService` and `ListeningService` each run on their own threads.

## Status LED
The status LED conveys the following:
| LED      | Meaning                                                |
|:-------- |:--------                                               |
|5 Flashes | The daemon has started                                 |
|1 Flash   | Network request received and issued over IR            |
|2 Flashes | IR signal received and published to the network        |
|3 Flashes | Learning mode failed to publish the received IR signal |

## Hardware
The current config of this project assumes the following hardware setup:
![Circuit Overview](https://github.com/arobinson434/IrRelayDaemon/blob/main/docs/overview.png)
