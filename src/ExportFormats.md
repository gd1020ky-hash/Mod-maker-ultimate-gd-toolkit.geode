# Universal export design

All Pathfinder output first becomes the same internal `Route`:

frame -> button -> press/release

Only after verification does an exporter serialize it.

`.sm`
- Native SM format.
- Primary/master representation.

`.gdr`
- Dedicated exporter slot.

`.gdr2`
- Dedicated exporter slot.

`.echo`
- Dedicated exporter slot.

Important: these are exporter targets, not claims that a file is valid merely because
the extension was renamed. Each target needs its actual specification/encoder and a
round-trip or external compatibility test before being marked ready.
