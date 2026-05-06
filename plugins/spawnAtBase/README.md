# Spawn at Custom Base

[![Current Release](https://img.shields.io/badge/release-v1.0.0-orange.svg)](https://github.com/allejo/spawnAtBase/releases/tag/v1.0.0)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.5+-blue.svg)

This plug-in introduces a custom map object that allows you define spawn zones for players to spawn at **after** a flag capture, initial spawn, or an explicit call to [`bz_setPlayerSpawnAtBase`](https://wiki.bzflag.org/Bz_setPlayerSpawnAtBase); these spawn zones work exactly like a base with regards to respawning after a flag capture.

This is especially useful for maps with custom bases and require custom spawn locations without forcing the players to always spawn there with a normal zone.

## Map Object

The `BASESPAWNZONE` map object uses the [bz_CustomZoneObject](https://forums.bzflag.org/viewtopic.php?f=78&t=19031), which provides the [following syntax](https://forums.bzflag.org/viewtopic.php?f=40&t=19034).

```
basespawnzone
  position 0 100 0
  size 4 4 0.1
  color 1
end
```

### Custom Options

`color` - This option specifies the team color which should spawn at this location; the team colors are the same numbers as a base object. Only one color should be defined, if multiple are defined, the last definition will be the one used. Define multiple objects at the same location if multiple teams need to spawn here.

## License

MIT