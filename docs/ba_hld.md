---
version: 1
---

# BestAmp
### High Level Design

## Goals for BestAmp
BestAmp is to be a software guitar amplifier, one that should be simple to use compared to the sometimes cumbersome existing amplifier softwares.

Unlike other software, which attempts to be all things to all people, BestAmp will be the most stripped down version of an amplifier giving users the following inputs: Pre-amplification (henceforth referred to as Gain), Power-amplification (henceforth referred to as Volume) as well as the two most common EQ inputs Bass & Treble.

In addition to the amplifier inputs, the user will also be able to select the drivers and input / output device settings for said drivers, in order to take full advantage of whatever interface.

BestAmp will target Windows only, we are not experienced enough with cross-compilation nor do we think we have enough time to attempt a cross-platform release during the semester.

## Architecture
BestAmp will be implemented as two separate programs:
- An AudioEngine "backend" Application
- A Client "frontend" Application

The AudioEngine application is implemented in C using PortAudio. We chose this approach for the AudioEngine because the signal processing must be done with the lowest latency possible, & we can eliminate the overhead of a runtime/JIT by having the AudioEngine be a native binary. The PortAudio library allows BestAmp to support the broadest surface of operating systems, drivers and therefore devices (even though our target OS is only Windows, we want to support drivers & devices).

The Client application is implemented with Web technology (Blazor WASM), with the App serving the GUI using Windows Forms with WebView2 to give a native appearance. Our decision to write the GUI with Web technology comes down to the development team's preference for Web technology w.r.t. GUIs (it's also the easiest technology to create GUIs with), the alternative would be to implement the GUI also in C using Win32 APIs, but those APIs are cumbersome (to say the least). Serving the GUI with WebView2 on windows will let BestAmp provide it's users with a native feeling experience in the UI, while also allowing the project to remain feasible.

```mermaid
--- 
config:
    layout: dagre
---
flowchart
    Users
    subgraph BestAmp.App
        UI["UI Thread (BestAmp.GUI)"]
        APP[Message Translation Thread<br/>WebSocket & TCP Peer]
    end
    subgraph BestAmp.AudioEngine
        MT[Main Thread</br>TCP Peer]
        PA[Audio Processing Thread]
    end

    Users <--> UI

    APP <-. "Websocket" .-> UI
    APP <-. "TCP" .-> MT
    MT -. "Amplifier state updates" .-> PA
    MT -. "Driver / device state updates" .-> PA
```


### AudioEngine State

The Client will control the AudioEngine, which will exist in different states according to the following diagram:

```mermaid
---
title: Audio Engine State Diagram
---
stateDiagram-v2
    [*] --> Closed: Initial stream state
    Closed --> Open: Update amp values request validated
    Note left of Closed: BestAmp does not open nor start the PortAudio stream by default
    Open --> Started: PortAudio stream open succeeds
    Started --> Stopped: Update amp values request or Program Termination
    Note left of Started: Start or resume the audio thread (bestAmpCB)
    Stopped --> Closed: PortAudio stream close succeeds
    Closed --> [*]: Program Termination

```

### Message Definitions

In order for the Client application to control the AudioEngine, a standard for communication messages is defined in the [BestAmp Protocol Specification](./ba_proto_spec.md). This document dictates the byte-structure of messages & the semantics of communication: the messages the AudioEngine will accept as requests & the messages it will emit as replies.

For each of the protocol messages, the Client application and AudioEngine will interact according to the following diagrams:

**Query Host APIs**
```mermaid
sequenceDiagram
    actor U as User

    participant C   as BestAmp.GUI
    participant B   as BestAmp.App
    participant A1  as BestAmp
    participant A2  as BestAmp (PortAudio)


    %% start
    U ->> C: Opens Application
    activate C
    C ->> B: `send(HostApiRequest.BuildMsg())`
    activate B
    Note right of B: forward websocket to tcp
    B ->> A1: 
    activate A1
    Note right of A1: process message type `3`
    A1 ->> A2: `Pa_GetHostApiCount`
    A2 -->> A1: `n`
    loop n times
        A1 ->> A2: `Pa_GetHostApiInfo`
        A2 -->> A1: `PaHostApiInfo*`
    end
    A1 -->> B: response type 1003, host api list (variable length)
    Note right of B: forward tcp to websocket
    deactivate A1
    B -->> C: response type 1003, host api list (variable length)
    deactivate B
    C -->> U: Sees default values in settings
    deactivate C

    %% end
```

**Query Devices**
```mermaid
sequenceDiagram
    actor U as User

    participant C   as BestAmp.GUI
    participant B   as BestAmp.App
    participant A1  as BestAmp
    participant A2  as BestAmp (PortAudio)


    %% start

    U ->> C: Opens Application or Selects New Host API
    activate C
    C ->> B: `send(DeviceIdsRequest.BuildMsg())`
    Note right of C: query for the selected or default host API<br/>ie. supplying the host ID message param
    activate B
    Note right of B: forward websocket to tcp
    B ->> A1: 
    activate A1
    critical process message type `5`
    option PaHostApiInfo* is NULL (invalid)
        A1 ->> A2: `Pa_GetHostApiInfo(index)`
        activate A2
        A2 -->> A1: `NULL`
        deactivate A2
        A1 -->> B: response type 1005, code `1`
        B -->> C: response type 1005, code `1`
        C -->> U: Sees the failed response
        Note right of U: unless for some reason the client<br/>sends arbitrary HostApi index<br/>this sort of situation should be impossible

    option PaHostApiInfo* not NULL (valid)
        A1 ->> A2: `Pa_GetHostApiInfo(index)`
        activate A2
        A2 -->> A1: `PaHostApiInfo*`
        Note right of A1: Use this to get device count
        loop `deviceCount` times
            A1 ->> A2: `Pa_HostApiDeviceIndexToDeviceIndex`
            A2 -->> A1: Global device index value
            A1 ->> A2: `Pa_GetDeviceInfo`
            A2 -->> A1: `PaDeviceInfo*`
        end
        deactivate A2
        A1 -->> B: response type 1005, device list (variable length)
        B -->> C: response type 1005, device list (variable length)
        Note right of B: forward tcp to websocket
        C -->> U: Sees device values for selected or default host API
    end
    deactivate A1
    deactivate B
    deactivate C

    %% end
```

**Update Amp Values**
```mermaid
sequenceDiagram
    actor U as User

    participant C   as BestAmp.GUI
    participant B   as BestAmp.App
    participant A1  as BestAmp


    %% start

    U ->> C: Updates dial value
    activate C
    C ->> B: `send(Dials.BuildMsg())`
    activate B
    B ->> A1: 
    Note right of B: forward websocket to tcp
    activate A1
    Note right of A1: process message type `2`
    A1 ->> A1: recalculate biQuadFilter values
    A1 -->> B: response type 9998, OK
    Note right of A1: as of 20261022 this message cannot fail
    deactivate A1
    B -->> C: response type 9998, OK
    deactivate B
    C -->> U: Sees the message response
    deactivate C

    %% end
```

**Update Preferences**
```mermaid
sequenceDiagram
    actor U as User

    participant C   as BestAmp.GUI
    participant B   as BestAmp.App
    participant A1  as BestAmp
    participant A2  as BestAmp (PortAudio)


    %% start
    U ->> C: Clicks "Send Settings Msg"
    activate C
    Note right of C: Client is connected<br/>to the Bridge via websocket
    C ->> B: `send(Settings.BuildMsg())`
    activate B
    B ->> A1: `ForwardBrowserToTcpAsync`
    activate A1
    Note right of B: Bridge receives websocket message<br/>& forwards the message to `BestAmp` w/  raw TCP

    critical    Process Message Type 1
    A1 ->> A2: `Pa_IsFormatSupported`
    activate A2
    option      Valid Message Type 1
        A2 -->> A1: `true`
        Note right of A1: The existing PortAudio stream must be closed & reopened w/<br/>the users newly select host API / device preferences
        A1 ->> A2: Stop Stream
        A2 -->> A1: Stream Stopped
        A1 ->> A2: Close Stream
        A2 -->> A1: Stream Closed
        A1 ->> A2: Open Stream (new host API & device preferences)
        A2 -->> A1: Stream Opened
        A1 ->> A2: Start Stream
        A2 -->> A1: Stream Started
        A1 -->> B: response type 1001, code 0
        B -->> C: response type 1001, code 0
        C -->> U: Sees the message
    option      Invalid Message Type 1
        A2 -->> A1: `false`
        Note right of A1: The user supplied a combination of Host API & device options<br/>Which PortAudio rejected, do nothing & let them know that<br/>their request was not valid
        %% [WO] annoyingly I have to put this in here
        deactivate A2
        A1 -->> B: response type 1001, code 1
        B -->> C: response type 1001, code 1
        C -->> U: Sees the message
    end
    deactivate A1
    deactivate B
    deactivate C
    %% end
```

### Client Application Class Hierarchy

The Client application will organize it's state according to the following class hierarchy:

```mermaid
---
title: BestAmpUI Class Diagram
config:
  layout: elk
---
classDiagram
direction LR
    class Request {
	    #List~byte~ msg
	    #BuildMsgContent()*
	    #GetOpCode() int*
	    #GetContentLength() int*
	    #AppendToMessage()
	    +BuildMsg() byte[]
    }

    class SettingsData {
	    +int HostApi
	    +int DeviceIn
	    +int ChannelIn
	    +int DeviceOut
	    +int ChannelOut
	    +int SampleRate
	    +int FramesPerBuffer
	    +int SampleFormat
    }

    class DialKnobsData {
	    +int Voice
	    +int Gain
	    +int Volume
	    +int Bass
	    +int Treble
    }

    class DeviceIdsRequest {
	    +int HostApi
    }

    class DefaultHostApiRequest {
    }

    class HostApisRequest {
    }

    class Response {
	    #AsHostOrderBytes(byte[], int, int) Span~byte~$
    }

    class DeviceIdsResponse {
	    +Dictionary~int, DeviceInfo~ Devices
	    +DeviceIdsResponse(byte[]) DeviceIdsResponse
    }

    class DefaultHostApiResponse {
	    +int HostApiId
	    +String HostApiName
	    +DefaultHostApiResponse(byte[]) DefaultHostApiResponse
    }

    class HostApisResponse {
	    +Dictionary~int, string~ HostApis
	    +HostApisResponse(byte[]) HostApisResponse
    }

    class DeviceInfo {
	    +String name
	    +int MaxInChannel
	    +int MaxOutChannel
	    +int SampleRate
    }

	<<Abstract>> Request
	<<Abstract>> Response

    Request <|-- SettingsData
    Request <|-- DialKnobsData
    Request <|-- DeviceIdsRequest
    Request <|-- DefaultHostApiRequest
    Request <|-- HostApisRequest
    Response <|-- DeviceIdsResponse
    Response <|-- DefaultHostApiResponse
    Response <|-- HostApisResponse
    DeviceInfo o-- DeviceIdsResponse
```
