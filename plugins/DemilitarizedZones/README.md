# Demilitarized Zones

[![GitHub release](https://img.shields.io/github/release/allejo/DemilitarizedZones.svg)](https://github.com/allejo/DemilitarizedZones/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.14+-blue.svg)
[![License](https://img.shields.io/github/license/allejo/DemilitarizedZones.svg)](LICENSE.md)

> :rotating_light: :warning: **WARNING** :rotating_light: :warning:
>
> The BZFlag 2.4.x protocol does not *correctly* support change shot types through a plug-in. A shot type *can* be changed by a plug-in and other players on the server will see that new shot type. However, the tank who made the shot will **not** see the change; this means that if the shot ricochets off any objects, the shot path the player sees is entirely different from the shot path other players on the server will see.
>
> This plug-in converts shots into Phantom Zoned shots, rendering them harmless to *most* tanks. Despite the name, these shots can and will kill any phantomed tanks.
>
> Use at your own risk!

A BZFlag plug-in that introduces the `DMZ` map object. Any tank that shoots inside this zone will have their shots converted into Phantom Zoned shots, rendering them harmless to *most* tanks.

## Requirements

- C++11
- BZFlag 2.4.14+

## Usage

### Loading the plug-in

This plug-in does not take any special configuration options at load time.

```
-loadplugin DemilitarizedZones
```

### Custom Map Objects

```text
# Rectangle
dmz
  position <x> <y> <z>
  size <x> <y> <z>
  rotation <rot>
  team <team>
end
```

```text
# Cylinder
dmz
  position <x> <y> <z>
  height <height>
  radius <radius>
  team <team>
end
```

- `team` (optional) - This option restricts the DMZ to only affect tanks of a certain color. If this option is left out, it'll apply to all teams. You may use this option multiple times to specify multiple teams. Supported values:
  - 0 - Rogue Team
  - 1 - Red Team
  - 2 - Green Team
  - 3 - Blue Team
  - 4 - Purple Team
  - 5 - Rabbit Team
  - 6 - Hunter Team

## License

[MIT](LICENSE.md)
