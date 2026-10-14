# BestAmp Protocol Specification
### v1, Will Otterbein, 20261014

This document serves to (somewhat) formally define the structure of the application layer protocol of BestAmp.

## Assumptions
The application layer protocol should be built on top of the TCP transport layer protocol.

## Message Structure
A BestAmp message shall be comprised of three components
- type
- content length
- content

`[type, 4B] [content length, 4B] [content 0-nB]`

### Header
The header consists of the first 2 message components, the **type** & **content length**.

#### Type Values
Message types are differentiated as per the logical difference of request / reply. From a protocol perspective, any device *should* accept all message types. Practically, the client will not transmit replies & therefore BestAmp will not process replies, only emit them. Similarly, the client will not receive requests, only emit them (and therefore does not need to process them).

Incoming (Client -> BestAmp) message types shall be the following:
| Type (Decimal Value) | Meaning |
| :- | :- |
| 0 | trigger a graceful termination of the BestAmp process. |
| 1 | update preferences (chosen host API, in device idx, out device idx, etc). |
| 2 | update ampvalues (gain, power, bass, treble, voice) |
| 3 | query host APIs |
| 4 | query default host API |
| 5 | query devices for host API |

Outgoing (BestAmp -> Client) message types shall be the following:
| Type (Decimal Value) | Meaning |
| :- | :- |
| 1000 | termination reply |
| 1001 | update preferences reply |
| 1002 | update ampvalues reply |
| 1003 | query host APIs reply |
| 1004 | query default host API reply |
| 1005 | query devices for host API reply |

#### Content Length
Should describe the number of bytes that the message content contains.

### Content
There are no formal restrictions on the content part of a message.

### Messages
Definitions of the structures of BestAmp messages.

#### Update Preferences
Intention of this message: update host API, input device, output device, input channel number, output channel number, frames per buffer size, sample rate and sample format for the device.

The request shall be of the following byte-structure:
```
```

The reply shall be of the following byte-structure(s):

*on success*
```
```

*on fail*
```
```

> This request can fail if the messages supplies an invalid combination of the parameters, as determined by the underlying host API via BestAmps calls to PortAudio `Pa_IsFormatValid`.

#### Update Amp Values
Intention of this message: update amplifier values, gain, power, bass, treble, voice.

The request shall be of the following byte-structure:
```
```

The reply shall be of the following byte-structure:
```
```

#### Query Host APIs
Intention of this message: Let a client determine which host APIs are available on the current machine.

The request shall be of the following byte-structure:
```
```

The reply shall be of the following byte-structure:
```
```

> Replies for this message will be of a variable length, indicated by the header content length value.

#### Query Default Host API
Intention of this message: Let a client determine what the default Host API / Device & other preference values should be at the time of its startup.

The request shall be of the following byte-structure:
```
```

The reply shall be of the following byte-structure:
```
```

#### Query Devices for Host API
Intention of this message: Let a client determine, for a specific host API, which devices are available.

The request shall be of the following byte-structure:
```
```

The reply shall be of the following byte-structure(s):

*on success*
```
```

*on failure*
```
```

> Replies for this message will be of a variable length, indicated by the header content length value.

> This request can fail if the request message supplies an invalid host API index, the host API index should be determine either with the Query Host APIs message or the Query Default Host API message.
