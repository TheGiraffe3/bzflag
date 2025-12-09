# Mapped Death Zones

[![GitHub release](https://img.shields.io/github/release/allejo/mappedDeathZones.svg)](https://github.com/allejo/mappedDeathZones/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.20+-blue.svg)
[![License](https://img.shields.io/github/license/allejo/mappedDeathZones.svg)](LICENSE.md)

A BZFlag plug-in that introduces some basic logic to player spawning when they die. If a player dies inside of a `DEATHZONE` object, they will then automatically spawn at the provided `SPAWNZONE` object.

## Requirements

- BZFlag 2.4.20
- C++11

This plug-in follows [my standard instructions for compiling plug-ins](https://github.com/allejo/docs.allejo.io/wiki/BZFlag-Plug-in-Distribution).

## Usage

### Loading the plug-in

This plug-in does not take any configuration options at load time.

```
-loadplugin mappedDeathZones
```

### Custom Map Objects

This plug-in introduces the `DEATHZONE` and `SPAWNZONE` map objects which support the traditional `position`, `size`, and `rotation` attributes for rectangular objects and `position`, `height`, and `radius` for cylindrical objects.

```text
deathzone
  position 0 0 0
  size 5 5 5
  rotation 0

  name red_base
  spawnzone blue_spawn_zone
  team 3
end

spawnzone
  position 0 0 0
  size 5 5 5
  rotation 0

  name blue_spawn_zone
end
```

#### `deathzone` Object

When players die inside of this zone, their next spawn will be at a specified `spawnzone` object.

Properties:

- `name` (required) - This is a unique name amongst *all* `deathzones` in a map.
- `spawnzone` (required) - The name of the `spawnzone` object to send players to. This attribute may be used multiple times in a single object, when multiple possible spawn zones are given, the server will randomly pick one of these zones for the player's next spawn.
- `team` (optional) - The team color this death zone will be limited to. This attribute may be used multiple times in a single object and it will affect multiple teams; omit this property to have this zone affect all players. If a player is on a team that is not affected by this death zone, their next spawn will not be affected.

#### `spawnzone` Object

When players are affected by the effects of a `deathzone`, then their next spawn will be inside a `spawnzone` object.

Properties:

- `name` (required) - This is a unique name amongst *all* `spawnzone`s in a map.

> **Warning:** These objects should not be placed outside the world boundaries (partially or completely). Doing so will lead to unexpected/unwanted behavior with regards to player spawns.

## License

[MIT](LICENSE.md)
