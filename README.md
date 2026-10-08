# BestAmp

Software amplifier by Rodrigo & Will (BCIT).

## Dependencies

[External Dir](./external)

> anything that's not a system dependency.

- [PortAudio 19](./external/portaudio)
- Winsock 2

## Interfaces

[Include Dir](./include)

## Implementation

[Src Dir](./src)

## Testing

[Test Dir](./test)

20261008

entirely manual.

`xxd` to convert .hex messages into .bin files

write you message in .hex then convert it to binary via

```bash
xxd -r ./my-hex.hex > ./my-bin.bin
```

`message-sender.py` basic repl, prompts for .bin file path

```bash
python ./message-sender
```

## Additional Documentation

- [BestAmp protocol specification](./docs/ba_proto_spec.md)

