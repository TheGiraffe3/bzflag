# CascadeFlag
 Modified plugin from [Bzflag plug-ins for dummies chapter 9](https://allejo.io/blog/bzflag-plug-ins-for-dummies-chapter-9/)

 This plugin adds a flag "Cascade" When a player carrying this flag shoots another tank, the victim will die and produce a shockwave at their position. If that shockwave hits any other tanks nearby, those tanks will be credited to the original flag carrier and produce more shockwaves that have the same effect. Hence "Cascade."

 ## Usage
 ### Loading the plugin
 The plugin can be loaded in the server config using:

```
-loadplugin /path/to/plugin/CascadeFlag
```

or

```
-loadplugin CascadeFlag
```
### Custom Flag
| Name           | Abbv | Description                     |
| -------------- | :--: |-------------------------------- |
| Cascade        |  CA  | "All of your victims will trigger a shockwave at their death location." |

## Credits and License
Licensed under [MIT](/LICENSE.md)

Credit to Allejo who wrote the plugin.

I only added the code to cascade the effect on to other tanks. See [Bzflag plug-ins for dummies chapter 9](https://allejo.io/blog/bzflag-plug-ins-for-dummies-chapter-9/) for his original version.