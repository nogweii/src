Musca

- Musca

|@constellation_small.png right@|http://en.wikipedia.org/wiki/Musca| |controls|#controls| : |source|#source| : |author|#author| : |bugs|https://bugs.launchpad.net/musca| : |questions|https://answers.launchpad.net/musca| : |suggestions|https://blueprints.launchpad.net/musca| : |customize|#customize| : |settings|#settings| : |howto|#howto| : |changelog|#changelog| : |mailing list|http://mail.aerosuidae.net/mailman/listinfo/musca_aerosuidae.net| : |launchpad|https://launchpad.net/musca| : |0.9.2|#source|

*A simple |dynamic window manager|http://en.wikipedia.org/wiki/Dynamic_window_manager| for X, with features nicked from |ratpoison|http://www.nongnu.org/ratpoison/| and |dwm|http://www.suckless.org/dwm/|*:

* Musca operates as a |tiling window manager|http://en.wikipedia.org/wiki/Tiling_window_manager| by default.  It uses `manual tiling`, which means the user determines how the screen is divided into non-overlapping `frames`, with no restrictions on layout.  Application windows always fill their assigned frame, with the exception of transient windows and popup dialog boxes which float above their parent application at the appropriate size.  Once visible, applications do not change frames unless so instructed.

* Since not all applications suit tiling, a more traditional |stacking window manager|http://en.wikipedia.org/wiki/Stacking_window_manager| mode is also available, allowing windows to float at any screen location and overlap.

* There are no built in status bars, panels, tabs or window decorations to take up screen real estate.  If the user wants any of these things, there are plenty of external applications available to do the job.  Window decoration is limited to a slender border, which is coloured to indicate keyboard focus.

* |@musca1_small.png right@|musca1.png| Windows are placed in named `groups` which can be used in a similar fashion to virtual desktops.  Groups can be added and removed on the fly, and each group has its own frame layout.

* The excellent *dmenu* utility is used to execute commands and launch applications, and it can also act as a window and group switcher.

* Windows and frames are navigated and focused on any mouse button click, including rolling the wheel, or alternatively driven entirely by the keyboard.  Simple key combinations exist for window switching, group switching, frame control and screen switching.

* Frames can be `dedicated` to a single application window, preventing new windows usurping said frame.  One frame per group can also be flagged as a `catch-all` so that all new application windows open there.  The frame border colour changes to reflect these modes.

* |@musca2_small.png right@|musca2.png| Musca has multi-screen support out of the box, and will automatically create groups for every available screen.

*Thanks to ratpoison and dwm authors*.  Musca's code is actually written from scratch, but a lot of useful stuff was gleaned from reading the source code of those two excellent projects.

*Extra kudos to dwm authors for creating dmenu!*  A true sliced-bread-beating invention.

*But `why` do this when there are 17 million other window managers already swanning about the internet?*  Variety is the spice of life?  Actually, *ratpoison* is very good and I used it for many years; but, I always wanted it to be just a little bit more friendly to the mouse, and just a little bit more informative about frame focus and layout, and just a little bit less `modal` (I can't think of a better way to say that) everywhere.  Sleek little *dwm* is also great, and while it does focus-follow-mouse and has nice minimal yet informative frame borders, it can't do manual frame layouts and I couldn't add the feature to it satisfactorily (probably my fault).  Other options like *Ion3* and *Xmonad* were also fun, but ultimately had fluff of one sort or another.  So, here is *Musca*: the strange offspring of ratpoison and dwm, and very likely only suited to my preferences ;-)  Oh well.

*Why is it named after a star constellation?*  Firstly, so it didn't have "*wm*" in the name.  Secondly, why not?

-- |Controls\controls|

All Musca key bindings start with a modifier key ("M" below), which is bound to *Mod4* by default.  Mod4 is usually *Super_L* or the left hand "Windows" key.  This is most convenient as it leaves Ctrl/Alt free for application use.

See |howto|#howto_mod_key| for troubleshooting or changing the Modifier key.

--- Frame Control

Key bindings:

:table bindings
Keys	Action
M+h	split frame in half horizontally to form two frames.
M+v	split frame in half vertically to form two frames.
M+r	remove the current frame and resize others to fill the gap.
M+o	remove all other frames except the current one, resizing it to full screen.
M+u	revert the last frame layout change.
M+d	(toggle) dedicate the current frame to the current app.
M+a	(toggle) set the current frame as a `catch-all`, where all new windows will open.
M+Left	change focus to the frame on the left.
M+Right	change focus to the frame on the right.
M+Up	change focus to the frame above.
M+Down	change focus to the frame below.
M+Ctrl+Left and M+Ctrl+Right	resize current frame horizontally.
M+Ctrl+Up and M+Ctrl+Down	resize current frame vertically.

Musca commands:

:table commands
Command	Action
hsplit <relative\|pixel>	split frame horizontally.
vsplit <relative\|pixel>	split frame vertically.
width <relative\|pixel>	resize frame horizontally.
height <relative\|pixel>	resize frame vertically.
remove	remove the current frame and resize others to fill the gap.
only	remove all other frames except the current one, resizing it to full screen.
dedicate <on\|off>	(toggle) dedicate the current frame to the current app.
catchall <on\|off>	(toggle) set the current frame as a `catch-all`, where all new windows will open.
focus <left\|right\|up\|down> or lfocus rfocus ufocus dfocus	change focus to a frame in the specified direction.
undo	revert the last frame layout change.
resize <left\|right\|up\|down>	resize the current frame in the specified direction.

Relative values can be specified as a percentage or a fraction:

 hsplit 2/3
 hsplit 33%

*hsplit* and *vsplit* adjust frame size relative to `itself`.

*width* and *height* adjust frame size relative to the `screen size`, less any group padding.

--- |Window Control\controls_window|

Key bindings:

:table bindings
Keys	Action
M+t	launch a terminal.
M+x	launch an app via dmenu.
M+w	switch windows in the current frame, via dmenu.
M+k	politely close the window in the current frame via a close event.  Press again to forcibly kill it.
M+c	cycle a hidden window into the current frame.

Windows automatically receive the keyboard input when they are visible in a focussed frame.

Musca commands:

:table commands
Command	Action
swap <left\|right\|up\|down> or lswap rswap uswap dswap	swap current window with the contents of the frame to the left, right, up and down respectively.
move <name>	move the current window to the group called `name`.
kill	politely close the window in the current frame via a close event.  Press again to forcibly kill it.
cycle	cycle a hidden window into the current frame.
raise <number\|title>	raise and focus a window in the current group by number (order opened) or title.
manage <on\|off> <name>	(toggle) set whether the window class called `name` is managed or ignored.

--- Group Control

Key bindings:

:table bindings
Keys	Action
M+g	switch groups via dmenu.
M+PageUp	switch to the previous group.  (PageUp == X11 Prior)
M+PageDn	switch to the next group.  (PageDn == X11 Next)
M+s	(toggle) switch the current group between `tiling` and `stacking` window modes.

Musca commands:

:table commands
Command	Action
add <name>	create a new group called `name`, and switch to it.
drop <name>	delete a group by `name`.
name <name>	rename the current group.
dump <file>	export a description (group name and frame layout) of the current group to `file`.
load <file>	import a description from `file` into the current group.
use <name>	switch to the group called `name`.
stack <on\|off>	(toggle) switch the current group between `tiling` and `stacking` window modes.

In `stacking` mode, Windows can be moved using *M+Mouse1*, and resized using *M+Mouse3*.  |More detail|#howto_stacking|.

--- Screen Control

Key bindings:

:table bindings
Keys	Action
M+Tab	switch to the next available screen.

Musca Commands:

:table commands
Command	Action
screen <number>	switch to screen `number`.  This is zero based, and should match the order in which screens are defined in {xorg.conf}.

--- |General Controls\controls_general|

Key bindings:

:table bindings
Keys	Action
M+m	Run a Musca command via dmenu.

Musca commands:

:table commands
Command	Action
exec <command>	execute as shell command.
pad <left> <right> <top> <bottom>	set the current group screen padding in pixels.
bind <on\|off> <Modifier>+<Key> <command>	bind a Musca command to a key combination with `on`, and remove it again with `off`.  The `command` argument is only need for `on`.
set <setting> <value>	set a Musca variable.  See |settings|#settings| for a list of variable names.
quit	exit Musca.

-- |Source\source|

A Musca bazaar repository is available on |launchpad|https://launchpad.net/musca|.  It should always build, but it is a development tree so it may not be stable.

This is the latest dated snapshot considered stable: |musca-0.9.2.tgz|musca-0.9.2.tgz|

--- Build Dependencies:

* Xlib
* GNU C Library
* make
* gcc

Install the above for your system, grab the source, and run *make*.  Copy the resulting {musca} binary into your {$PATH} somewhere.

--- Runtime Dependencies:

* dmenu

-- |Author\author|

Feel free to email feedback:

sean dot pringle at gmail dot com

-- |Customize\customize|

Most |settings|#settings| can be changed on the fly using Musca commands, and applied each time using the startup file option.  Alternatively, to change the default settings, modify {config.h} and recompile.

-- |Settings\settings|

Musca has a list of settings that can be altered on the fly using the *set <name> <value>* command:

 set border_focus Orange

:table musca_settings
Name	Default	Description
border_focus	Blue	Border colour for focused frames in tiling mode, and focused windows in stacking mode.
border_unfocus	Dim Gray	Border colour for unfocused frames in tiling mode, and unfocused windows in stacking mode.
border_dedicate_focus	Red	Border colour for focused `dedicated` frames in tiling mode.
border_dedicate_unfocus	Dark Red	Border colour for unfocused `dedicated` frames in tiling mode.
border_catchall_focus	Green	Border colour for focused `catchall` frames in tiling mode.
border_catchall_unfocus	Dark Green	Border colour for unfocused `catchall` frames in tiling mode.
frame_min_wh	100	Minimum width and height in pixels of frames and managed windows.
frame_resize	20	Size in pixels of a frame resize step.  Setting this smaller will make resizing operations smoother, but also slower and increase load.
startup	.musca_start	(optional) Path to a file containing Musca commands to run at start up.  The default setting is relative to the working directory; ie, Musca will use $HOME/.musca_start `only if Musca is started from $HOME`, which is the usual method for login managers.  The file must contain one command per line.  Lines starting with hash *#* are comments and blank lines are acceptable.
dmenu	{dmenu -i -b}	Command to run to launch *dmenu* along with any customize appearance arguments.  This can be replaced by another launcher so long as it accepts a list of *\\n* terminated items on stdin and returns a single line on stdout.
switch_window	{xargs -I name $MUSCA -c "raise name"}	The command to run once the user has selected a window number and name from *dmenu*.
switch_group	{xargs -I name $MUSCA -c "use name"}	The command to run once the user has selected a group name from *dmenu*.
run_musca_command	{xargs -I command $MUSCA -c "command"}	The command to run when the user has entered a Musca command via *dmenu*.
run_shell_command	{xargs -I command $MUSCA -c "exec command"}	The command to run when the user has entered a shell command via *dmenu*.
notify	{echo %s}	The command to run to send the user a message.  By default Musca just writes to stdout.
stack_mouse_modifier	Mod4	The modifier key to use in `stacking` mode, along with mouse buttons 1 and 3, to move and resize windows respectively.

-- |Howto\howto|

* |Start Musca|#howto_start|
* |Use multi-windowed apps like the Gimp|#howto_multi_window_apps|
* |Change or troubleshoot the Modifier key|#howto_mod_key|
* |Change the default key combinations|#howto_key_combos|
* |Change the default border colours|#howto_borders|
* |Run a system tray|#howto_tray|
* |Set a desktop background|#howto_background|
* |Make Musca ignore windows|#howto_ignore|
* |Use a startup config file|#howto_startup|
* |Control Musca externally|#howto_control|
* |Get a list of windows in the current group|#howto_windows|
* |Use `stacking` window management mode|#howto_stacking|

--- |Start Musca\howto_start|

---- Using startx

Launch it from your *.xinitrc* file, using *startx*:

 exec /path/to/musca

---- Using a login manager like GDM or KDM

Create /usr/share/xsessions/musca.desktop, and select the Musca session at login:

 [Desktop Entry]
 Encoding=UTF-8
 Type=XSession
 Exec=/path/to/musca
 Name=musca

---- For debugging

Start X with a single xterm running, and launch Musca manually from the xterm, so you can see stdout/stderr (which is where any errors from your dmenu commands will appear).  So in {.xinitrc}:

 exec xterm

--- |Use multi-windowed apps like the Gimp\howto_multi_window_apps|

---- In Tiling Mode

People seem to think this is a huge problem with tiling window managers, but it really is not, particularly in Musca where any old tiling layout can be used and nothing moves around once visible.  |See|musca2.png|?

Start with a single full screen frame and a few small frames off to one side.  Switch to and dedicate the largest frame with *M+d*.  Open the app in the largest frame and the first (hopefully primary) app window will open there, while secondary windows distribute themselves among the smaller frames.  Adjust the number and layout of frames as required for the app in question.  Use the |swap|#controls_window| commands to shift windows around if needs be.

Afterward it may be worthwhile dedicating the smaller frames to ensure they only ever hold the dialog windows you want there, or just set the largest frame as a catch-all to achieve the same effect.

Note that some apps like OpenOffice have windows which are normal dialogs, but they automatically unmap themselves when the primary window is not focussed.  These may flick in and out of existence in the smaller frames.  Not much we can do about it, except use OOo full screen and dock everything with Ctrl+Shift+F10, or use stacking mode.

---- In Stacking Mode

Create a new window group and set it to `stacking` mode with *M+s*, then use the multi-windowed app just as you would in any stacking window manager.  See how the |stacking controls differ|#howto_stacking|.

--- |Change or troubleshoot the Modifier key\howto_mod_key|

Changing the Modifier key is possible by modifying {config.h} and recompiling.  Valid modifier keys are listed in {modmasks[]}, and default key combinations are in {keymaps[]}.  Alternatively, to prevent the need to recompile, you can customize key bindings on the fly using the |*bind* command|#controls_general|.

Mod4 is commonly bound to X11's *Super_L* key, which is usually the left Windows key on Linux PCs with a US or UK keyboard layout.  If you're on a different system and the default Modifier key does not work, then establish where mod4 (if it exists!) is pointing by using *xmodmap*:

 $ xmodmap | grep mod4
 mod4        Super_L (0x7f),  Hyper_L (0x80)

Or run the *xev* utility, press the left Win key, and watch xev's standard output while you do it.  Something like this should appear:

 KeyRelease event, serial 27, synthetic NO, window 0xe00002,
    root 0x259, subw 0x0, time 672433, (417,298), root:(418,299),
    state 0x40, keycode 115 (keysym 0xffeb, Super_L), same_screen YES,
    XLookupString gives 0 bytes:
    XFilterEvent returns: False

Using these two tools, find a suitable Modifier key for your system.

--- |Change the default key combinations\howto_key_combos|

Look at the {keymaps[]} structure in {config.h}.  X11 key names are in X11/keysymdef.h; just remove the 'XK_' from the constant definitions to get the names.  Either modify {keymaps{}} and recompile, or apply key bindings on the fly in {.musca_start} using the |*bind* command|#controls_general|.

--- |Change the default border colours\howto_borders|

Look at the *border_...* fields in {settings[]} in {config.h}.  X11 named colours are on |http://en.wikipedia.org/wiki/X11_color_names|http://en.wikipedia.org/wiki/X11_color_names|.  Either modify {settings[]} and recompile, or apply key bindings on the fly in {.musca_start} using the |*set* command|#controls_general|.

--- |Run a system tray\howto_tray|

Use the *trayer* utlity and set padding on a window group so as not to obscure it.  For example:

 #!/bin/bash
 trayer --edge bottom --align center --height 32 --SetDockType true --SetPartialStrut false \
  --transparent true --alpha 255 --tint 0x00ff00 --distance 0 --expand true >.trayer.log 2>&1

Whatever system tray application you choose will need to have its WM_CLASS |unmanaged|#howto_ignore|.  Trayer is already unmanaged by default.

--- |Set a desktop background\howto_background|

Musca doesn't touch the root window and frames are transparent, so go use something line *xsetroot* or *xv* or *imagemagick* to set the background.  For example, with imagemagick:

 $ display -window root <path_to_image>

--- |Make Musca ignore windows\howto_ignore|

In {config.h} there is an {unmanaged_windows[]} list of WM_CLASS names:

 char *unmanaged_windows[] = { "trayer", "Xmessage", "Conky" };

Check out the *xprop* utility to find class names.  Either change {unmanaged_windows[]} and recompile, or do on the fly in {.musca_start} with the |*manage* command|#controls_window|.  Note that any *manage* setting only takes effect for `subsequently` opened windows.

--- |Use a startup config file\howto_startup|

Set the `startup` setting in {config.h} to point to a text file of your choice (default is {.musca_start}).  It should contain Musca commands (exactly as would be launched with *M+m*), one per line.  Any comments must be on separate lines starting with hash *#*.  Blank lines are acceptable.  For example:

 manage off trayer
 manage off Conky

 screen 1
 name bling
 pad 0 0 0 32
 exec trayer
 exec conky

 screen 0
 add mail
 add net
 add work
 use mail
 exec firefox gmail.com
 exec evolution
 hsplit 1/2

 set notify notify-send -t 3000 Musca "%s"

Note that lanuching apps from the startup file is OK, but can be of limited use if you want to do it in more than one group.  The *exec* command runs asynchronously, and if apps are slow to create their windows, they may not necessarily appear in the group you expect.  In this case, try launching apps from {.xinitrc} and just moving them around here.

--- |Control Musca externally\howto_control|

Musca commands can be dispatched from an external script by calling Musca with the {-c <command>} command line argument:

 musca -c "hsplit 1/3"

In this case, the Musca binary will try to connect to an already running instance of Musca, deliver the command, and return once the command has executed.

--- |Get a list of windows in the current group\howto_windows|

Use the group *dump <file>* command and extract lines starting with the word `window`.  Each line is a series of tab delimited fields in this order:

* Number in the current group.
* Class name.
* Title.

For example:

 #!/bin/bash
 file=/tmp/group.txt
 musca -c "dump $file" && cat $file | grep -r '^window' | awk -F '\t' '{print "id: " $2 " class: " $3 " title: " $4}'

Mould into whatever form suits you.

--- |Use `stacking` window management mode\howto_stacking|

Stacking window management mode is available at the window group level, on a group by group basis.  Select the group you wish to make stacking, and press *M+s* or run the command *stack on*.  Any frames in the group will disapear.  Other groups will not be affected.

Floating windows can be moved using *M+Mouse1* -- that is: hold down the modifier key and click the left mouse button on the window -- and dragging the mouse.  Floating windows can be resized using *M+Mouse3* in the same fashion.  Click-to-focus still works.

Most of the frame manipulation related key combinations and commands are disabled in stacking mode.

No, there is no way to float specific windows while in tiling mode.

To switch the group back to tiling mode, press *M+f* again or run the command *stack off*.  The group frames will be recreated as they were before the mode change.

-- |*Changelog*\changelog|

* |0.9.2|#changelog_0.9.2| (|tgz|musca-0.9.2.tgz|)
* |0.9.1|#changelog_0.9.1| (|tgz|musca-0.9.1.tgz|)
* |0.9|#changelog_0.9| (|tgz|musca-0.9.tgz|)
* |20090313|#changelog_20090313| (|tgz|musca_20090313.tgz|)
* |20090312a|#changelog_20090312a| (|tgz|musca_20090312a.tgz|)
* |20090312|#changelog_20090312| (|tgz|musca_20090312.tgz|)
* |20090311|#changelog_20090311| (|tgz|musca_20090311.tgz|)
* |20090310|#changelog_20090310| (|tgz|musca_20090310.tgz|)
* |20090309|#changelog_20090309| (|tgz|musca_20090309.tgz|)
* |20090305|#changelog_20090305| (|tgz|musca_20090305.tgz|)
* |20090304|#changelog_20090304| (|tgz|musca_20090304.tgz|)
* |20090303|#changelog_20090303| (|tgz|musca_20090303.tgz|)
* |20090302|#changelog_20090302| (|tgz|musca_20090302.tgz|)
* |20090301|#changelog_20090301| (|tgz|musca_20090301.tgz|)

--- |0.9.2\changelog_0.9.2|

* Tuning: politely check PResizeInc for apps in both stacking and tiling modes.
* Tuning: changed window resize and drag in stacking mode to use a window outline, to better suit slow video.

--- |0.9.1\changelog_0.9.1|

* Tuning: prevent parent windows from hiding when transients popup.
* Bugfix: correctly resize bound keys structure when >31 combinations are bound.

--- |0.9\changelog_0.9|

No difference to |20090313|#changelog_20090313| release, expect that we are starting a more traditional versioning numbering scheme to better suit distro packaging practices.  We're starting at 0.9 because Musca now has all the major features I wanted when starting the project, and bugs seem minimal, but we still need to do extended stability testing.  There is a feature freeze now in effect.

`*Note* There was some discussion via email that this should be 1.0, and the 0.9 tarball was initially pushed as 1.0.  But after coffee, I think being feature complete and not having many bugs reported doesn't really justify that with a young code base.  So, 0.9 it is.`

--- |20090313\changelog_20090313|

* Windows now remember their floating position across stacking/tiling mode switches.
* Bugfix: better MotionNotify co-ordinate checking when resizing in stacking mode.
* Improved window stacking behavior in relation to unmapped windows, and reduced focus flicker of groups in stacking mode.
* Improved error checking converting colour names to pixel values for borders.
* Use {execlp()} instead of {execl()} for launching shell commands with *exec*, to mimic shell parsing and $PATH checking for commands without a full path.
* Improved key grabbing to prevent blocking some key combinations from the application when we don't need them.

--- |20090312a\changelog_20090312a|

* |Bugfix|https://bugs.launchpad.net/musca/+bug/341219|: using SIG_IGN for SIGCHLD exits annoys *dbus* autolaunch, so handle it normally with waitpid.
* Added additional error check to *bind* command, to ensure the supplied key symbol is valid.  Previously, it only verified key modifiers.

--- |20090312\changelog_20090312|

* Cleaned up {config.h}.
* Replaced various constants with a simple table {settings[]} holding variables that can be set dynamically.
* Converted {key_callbacks[]} to {keymaps[]} to simply map key strokes to Musca command strings.
* Added commands: resize, raise, bind, switch, command, shell, set.
* Added code to filter out NumLock and CapsLock from our key commands (too easy to leave one turned on and disable stuff). Thanks to Nikita Kanounnikov for pointing this bug out.

--- |20090311\changelog_20090311|

* Tweaked Musca's dmenu usage to execute in a child process.  This should help with the reports made by some people where both dmenu and Musca freeze when the mouse is clicked, or a window opens, while dmenu is running.  Now neither event affects dmenu.
* Added an {XGrabKeyboard()} check during the Musca startup process.  If it fails, it will throw a warning to {stderr} like:  `Could not temporarily grab keyboard. Something might be blocking key strokes.`  This might help with |this bug|https://bugs.launchpad.net/musca/+bug/336473|.
* Added the *raise <number\|title>* command, to raise a window.

--- |20090310\changelog_20090310|

* Added option to switch window groups between tiling and stacking modes.

--- |20090309\changelog_20090309|

* Added commands: remove, kill, cycle, only, focus, dedicate, catchall, undo, dump, load, use, exec, swap, screen, manage.  Mnay of these duplicate hot keys, but may be useful to external scripts.
* Added ability to dump and load group frame layouts to file with *dump <file>* and *load <file>* commands.
* Added the option of a startup script (which needs to be a list of Musca commands), defined by the {STARTUP} definition in {config.h}.
* Added frame layout `undo` tracking, so that up to 32 frame layout changes per group can be rolled back.
* Added client command interface for external control by calling {musca -c <command>}
* Rearranged Musca startup routine slightly to isolate |this bug|https://bugs.launchpad.net/musca/+bug/336473|.
* Applied a {FOR_RING()} macro to automate looping about head/group/frame/client doubly-linked rings.
* Improved `click-to-focus` behavior to reduce frame screen flicker.
* Added restrictions to ensure transient windows follow if their parent is moved between groups.
* Added logic to prevent a parent window being cycled into another frame when a transient takes focus above it.
* Added logic to ensure a parent window regains focus in the same frame with a transient window exits.
* Migrated old {client->kill_event_sent} to a {client->flags} bit.
* Added ability to manage and unmanage window classes on the fly.
* Changed {commands[]} struct in {config.h} to a list of command pointers, rather than one long hard to read \\n delimited string.

--- |20090305\changelog_20090305|

---- misc stuff

* Added TERMINAL to config.h to point to the perferred console app, defaulting to xterm.
* Added NOTIFY to config.h to point to an external notification app, like {dzen} or {notify-send}.
* Added example custom launcher functions to config.h, with M+t activated to launch a terminal.
* Convert {unmananged_windows} to use window class names instead of titles.
* Reduced default verbosity when logging.

--- |20090304\changelog_20090304|

---- bug fixes

* Stop frames on an unfocused screen taking the keyboard focus when their client window exited.
* Fix possible buffer overflow, due to an incorrect {realloc()}, when creating the list of window titles for dmenu.

--- |20090303\changelog_20090303|

---- `width` and `height` commands

Added *width* and *height* commands, and {com_frame_size()}, to resize a frame size relative to the screen size or to a specific pixel value.

--- |20090302\changelog_20090302|

---- move windows between groups

Added the *move <group_name>* command, and {com_window_to_group()}, to move the active window to another group.

---- define a `catch-all` frame

Added *M+a* key combination, and {frame_catchall()}, to define a frame per group in which *all* new non-transient windows will open.

--- |20090301\changelog_20090301|

---- key_modifiers

Added {key_modifiers[]} struct to config.h.  This lists the modifier key combinations we're interested in.  Any modifier used in {key_callbacks[]} must also appear in {key_modifiers[]}.

---- key combination logging

Added key combination logging.  Each matched modifier+key combination is logged, eg, a hsplit:

 keypress handling key: Mod4 h

..and each unmatched modifier+key (where modifier is one we're interested in) is logged:

 keypress unhandled key: Mod4 q

This makes it easy to find out X11 key names when modifying {key_callbacks[]}.
