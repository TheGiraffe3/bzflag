# Neutral Zones

[![GitHub release](https://img.shields.io/github/release/allejo/neutralZones.svg)](https://github.com/allejo/neutralZones/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.4+-blue.svg)
[![License](https://img.shields.io/github/license/allejo/neutralZones.svg)](LICENSE.md)

A BZFlag plug-in for adding zones to a map where flags are disallowed, meaning players who enter these zones with specific flags will have it removed.

## Requirements

- BZFS 2.4.4+

This plug-in follows [my standard instructions for compiling plug-ins](https://github.com/allejo/docs.allejo.io/wiki/BZFlag-Plug-in-Distribution).

## Usage

### Loading the plug-in

This plugin loads without any configuration options.

```
-loadplugin neutralZones
```

### Custom Map Objects

This plug-in introduces the `NEUTRALZONE` map object which supports the traditional `position`, `size`, and `rotation` attributes for rectangular objects and `position`, `height`, and `radius` for cylindrical objects.

```text
neutralzone
  position 0 0 0
  size 5 5 5
  rotation 0

  # Repeatable custom properties
  team <0 | 1 | 2 | 3 | 4 | 5 | 6>
  flag <* | abbv>
end
```

#### Affecting multiple teams or multiple flags

The `team` property can be used multiple times in an object allowing you to specify this zone to affect more than one team. For example, a neutral zone that affects both the red team and the green team, your zone would look like the following,

```text
neutralzone
  position 0 0 0
  size 5 5 5
  rotation 0
  team 1
  team 2
  flag SW
  flag GM
  flag L
end
```

#### Affecting all teams

For a neutral zone to affect all teams, simply omit the `team` property from the zone. For example, disallowing the Genocide flag from a specific area of the map for all teams would look like the following,

```text
neutralzone
  position 0 0 0
  size 5 5 5
  rotation 0
  flag G
end
```

#### Disallowing all flags

In order to disallow all flags from being carried, the special `*` directive can be used to affect all flags.

```text
neutralzone
  position 0 0 0
  size 5 5 5
  rotation 0
  flag *
end
```

## Known Issues

Determining when a player has entered the given zone carries some latency and will not result in the player's flag being removed immediately. These may be a 1-2 second delay before the flag removal happens depending on player/server lag.

## License

[MIT](LICENSE.md)
