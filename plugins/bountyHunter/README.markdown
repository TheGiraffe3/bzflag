Bounty Hunter
=============

Bounty Hunter is a BZFlag plugin that will award points to players who stop the rampage of another player. The longer the rampage a player has been on, the more the killer will be rewarded.

Authors
------

- Vladimir "allejo" Jimenez

Compiling
---------

### Requirements

- BZFlag 2.4.0

### How to Compile

1.  Check out the BZFlag source code.

    `git clone -b v2_4_x https://github.com/BZFlag-Dev/bzflag-import-3.git bzflag`

2.  Go into the newly checked out source code and then the plugins directory.

    `cd bzflag/plugins`

3.  Create a plugin using the `newplug.sh` script.

    `sh newplug.sh bountyHunter`

4.  Delete the newly create bountyHunter directory.

    `rm -rf bountyHunter`

5.  Run a git clone of this repository from within the plugins directory. This should have created a new bountyHunter directory within the plugins directory.

    `git clone https://github.com/allejo/bountyHunter.git`

6.  Instruct the build system to generate a Makefile and then compile and install the plugin.

    `cd ..; ./autogen.sh; ./configure; make; make install;`

### Updating the Plugin

1.  Go into the bountyHunter folder located in your plugins folder.

2.  Pull the changes from Git.

    `git pull origin master`

3.  (Optional) If you have made local changes to any of the files from this project, you may receive conflict errors where you may resolve the conflicts yourself or you may simply overwrite your changes with whatever is in the repository, which is recommended. *If you have a conflict every time you update because of your local change, submit a pull request and it will be accepted, provided it's a reasonable change.*

    `git reset --hard origin/master; git pull`

4.  Compile the changes.

    `make; make install;`

Server Details
--------------

### How to Use

To use this plugin after it has been compiled, simply load the plugin via the configuration file.

`-loadplugin /path/to/bountyHunter.so`

License
-------

[GNU General Public License Version 3.0](https://github.com/allejo/bountyHunter/blob/master/LICENSE.markdown)
