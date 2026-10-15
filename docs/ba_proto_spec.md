# BestAmp Protocol Specification
### v1, Will Otterbein, 20261014

This document serves to (somewhat) formally define the structure of the application layer protocol of BestAmp.

## Assumptions
The application layer protocol should be built on top of the TCP transport layer protocol.

B -> Bytes

## Message Structure
A BestAmp message shall be comprised of three components
- type
- content length
- content

```
0xFFFFFFFF  0xFFFFFFFF            0xFFFFFFFFFFFFFFFF ...
type, 4B    content length, 4B    content, 0-(2^32-1)B
```

### Header
The header consists of the first 2 message components, the **type** & **content length**.


```
0xFFFFFFFF  0xFFFFFFFF            ...
type, 4B    content length, 4B
```

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

```
0x00000001    0xFFFFFFFF            0xFFFFFFFFFFFFFFFF ...
type (1), 4B  content length, 4B    content, 0-(2^32-1)B
```
```
0x00000002    0xFFFFFFFF            0xFFFFFFFFFFFFFFFF ...
type (2), 4B  content length, 4B    content, 0-(2^32-1)B
```
```
0x00000003    0xFFFFFFFF            0xFFFFFFFFFFFFFFFF ...
type (3), 4B  content length, 4B    content, 0-(2^32-1)B
```

etc.

#### Content Length
Should describe the number of bytes that the message content contains.

```
0xFFFFFFFF  0x00000002            0xFFFF
type, 4B    content length, 4B    content, 2B in this case
```

### Content
There are no formal restrictions on the content part of a message (so long as the content length accurately reflects the number of bytes in the content).

```
...    0xFFFFFFFFFFFFFFFF ...
       content, 0-(2^32-1)B
```

### Messages
Definitions of the structures of BestAmp messages.

#### Update Preferences
Intention of this message: update host API, input device, output device, input channel number, output channel number, frames per buffer size, sample rate and sample format for the device.

The request shall be of the following byte-structure:
```
0x00000001  0x00000020
type, 4B    content length (32B), 4B    


0xFFFFFFFF    0xFFFFFFFF  0xFFFFFFFF   0xFFFFFFFF 
host idx, 4B  in idx, 4B  out idx, 4B  in ch n, 4B

0xFFFFFFFF    0xFFFFFFFF    0xFFFFFFFF    0xFFFFFFFF
out ch n, 4B  buf size, 4B  sample r, 4B  sample f, 4B
```

Where `host idx`, `in idx`, `out idx`, `in ch n`, `out ch n`, `buf size`, and `sample f` are type `int`.

& `sample r` is type `float`.

> The aforementioned types are how these bytes are interpreted by BestAmp.

> The indices are PortAudio host API and device indices, it is expected that the client will use the query messages to determine these (alongside human readable names for display purposes).

The reply shall be of the following byte-structure(s):

*on success*
```
0x000003E9       0x00000001
type (1001), 4B  content length, 4B

0x00000000
```

*on fail*
```
0x000003E9       0x00000001
type (1001), 4B  content length, 4B

0x00000001
```

> This request can fail if the messages supplies an invalid combination of the parameters, as determined by the underlying host API via BestAmps calls to PortAudio `Pa_IsFormatSupported`. If the error reply is encountered by the client, inform the user that their selected combination of preferences is not valid.

#### Update Amp Values
Intention of this message: update amplifier values, gain, power, bass, treble, voice.

The request shall be of the following byte-structure:
```
0x00000002  0x00000014
type, 4B    content length (20B), 4B    


0xFFFFFFFF  0xFFFFFFFF  0xFFFFFFFF  0xFFFFFFFF 
gain, 4B    bass, 4B    treble, 4B  power, 4B

0xFFFFFFFF
voice, 4B
```

Where the content bytes are of type `int`.

> The aforementioned types are how these bytes are interpreted by BestAmp.

The reply shall be of the following byte-structure:
```
0x000003EA       0x00000000
type (1002), 4B  content length, 4B
```

#### Query Host APIs
Intention of this message: Let a client determine which host APIs are available on the current machine.

The request shall be of the following byte-structure:
```
0x00000003  0x00000000
type, 4B    content length, 4B
```

The reply shall be of the following byte-structure:
```
0x000003EB       0xFFFFFFFF
type (1003), 4B  content length (variable), 4B

0xFFFFFFFF    0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
host idx, 4B  host human name, 32B
```
Each host occupies 36B, where the first 4B is the host idx, type `int`, and the 32B proceeding B are the human name, the `char` (ASCII).

> Replies for this message will be of a variable length, indicated by the header content length value. Content length will be 36 * host count B.

#### Query Default Host API
Intention of this message: Let a client determine what the default Host API / Device & other preference values should be at the time of its startup.

The request shall be of the following byte-structure:
```
0x00000004  0x00000000
type, 4B    content length, 4B
```

The reply shall be of the following byte-structure:
```
0x000003EC       0x00000024
type (1004), 4B  content length, 4B

0xFFFFFFFF    0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
host idx, 4B  host human name, 32B
```

One host, first 4B is host idx, type `int`, and 32 proceeding B are human name, type `char` (ASCII).

> It is debatable if this should even be its own reply type or a special case of `1003` that just returns w/ a content length of 36B.

#### Query Devices for Host API
Intention of this message: Let a client determine, for a specific host API, which devices are available.

The request shall be of the following byte-structure:
```
0x00000005  0x00000001
type, 4B    content length, 4B

0xFFFFFFFF
host idx, 4B
```

The reply shall be of the following byte-structure(s):

*on success*
```
0x000003ED       0xFFFFFFFF
type (1005), 4B  content length (variable), 4B


0xFFFFFFFF   0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
dev idx, 4B  dev human name, 32B
```
Each device occupies 36B, where the first 4B is the device idx, type `int`, and the 32B proceeding B are the human name, the `char` (ASCII).

> Successful replies for this message will be of a variable length, indicated by the header content length value. Content length will be 36 * device count B.

> Those familiar with PortAudio may wonder if this device idx is host specific or system wide. Its the system wide device idx, most PortAudio calls rely on the system device idx so that is what is returned by this message.

*on failure*
```
0x000003ED       0x00000001
type (1005), 4B  content length, 4B

0x00000001
```

To ensure a consistent content of n > 0 for error code.

> This request can fail if the request message supplies an invalid host API index, the host API index should be determine either with the Query Host APIs message or the Query Default Host API message.
