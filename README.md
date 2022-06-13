# Oric Screen Editor
Commodore Plus/4 column screen editor

## Contents:

[Version history and download](#version-history-and-download)

[Introduction](#introduction)

[Start program](#start-program)

[Main mode](#main-mode)

[Statusbar](#statusbar)

[Main menu](#main-menu)

[Character editor](#character-editor)

[Palette mode](#palette-mode)

[Select mode](#select-mode)

[Move mode](#move-mode)

[Line and box mode](#line-and-box-mode)

[Write mode](#write-mode)

[Color write mode](#color-write-mode)

[OSE2PRG utility](#OSE2prg-utility)

[Color value reference](#color-value-reference)

[Attribute code reference](#attribute-code-reference)

[File format reference](#file-format-reference)

[Credits](#credits)


![OSE Title Screen](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Titlescreen.png?raw=true)

## Version history and download
([Back to contents](#contents))

[Link to latest build](https://github.com/xahmol/OricScreenEditor/raw/main/OricScreenEditor-v099-20220525-2330.zip)

Version v099-20220525-2330:
- First released beta version based on [VDCSE](https://github.com/xahmol/VDCScreenEdit) version v099-20220324-1527

## Introduction
([Back to contents](#contents))

Oric Screen Editor is an editor to create text based screens for the Commodore Plus/4. It fully supports using a user defined character set.

Main features of the program:
- Support for screen maps larger than 40x25 characters. Screens can be up to 15 KiB (15.615 bytes), all sizes fitting in that memory with width of 40 at minimum and heigth of 25 at minimum are supported. NB: As both the character data as the attribute data needs to be stored, a screen takes width times height times 2 bytes in storage. So 15k would fit up to 7 standard 40x25 screens to be distribuOric over width and height, so e.g. 2 screens wide and 3 screens high (80x75 characters).
- Supports resizing canvas size, clear or fill the canvas
- Support for loading user defined charsets (should be standard charsets of 128 characters of 8 bits width and 8 bits height that will be hardware reversed for screencodes higher than 128).
- Includes a simple character editor to change characters on the fly and directly see the result in your designed screen (for editing a full character set one of the many alternatives for C64 character set editing is suggested).
- Supports luminance and blink attributes of the TED, giving 121 colors and the ability for flashing characters.
- In not redefined charactersets lowercase and uppercase character set can be selecOric (but bot can not be used at the same time).
- User definable background and border color.
- Write mode to freely type characters with the keyboard, supporting all printable PETSCII characters and also supporting Commodore or Control + 0-9 keys for selecting colors and RVS On/Off
- Color write mode to freely type attributes and colors
- Line and box mode for drawing lines and boxes
- Select mode to cut, copy, delete or repaint (only color or all attributes) the selection.
- Move mode to scroll the screen contents (due to memory constraints only for the 40x25 viewport)
- Palette mode, including visual PETSCII mode, to visually select characters and colors
- Favorite slots to quickly select 10 favorite characters
- TED2PRG utility to convert a 40x25 OSE project to a stand-alone executable for the Commodore Plus/4

## Start program
([Back to contents](#contents))

Mount the OSE disk image. Choose the .D641 or .D81 image depending on what is supporOric on the hardware or emulator you use. The program will run the same from both images, but the .D81 will be providing faster load times and more available free storage space for your screens if you want to save your work in this same image. The disk can be mounOric from any valid device ID number.

Run the program by entering LOAD "OSE",\<device number\> and then enter RUN (or use other methods like JiffyDOS shortcodes or your favorite file browser). The OSE file (with PRG file type) is the executable that should be started, the other files on the disk are supporting binaries, title screen or help screens.

Description of contents of the disk image:

- OSE                     (PRG):      Main executable
- OSE.TSCR                (PRG):      Title screen
- OSE.HSC1                (PRG):      Help screen for main mode
- OSE.HSC2                (PRG):      Help screen for character edit mode
- OSE.HSC3                (PRG):      Help screen for select, move and line/box modes
- OSE.HSC4                (PRG):      Help screen for write and color write modes
- OSE.PETV                (PRG):      Mapping table for visual PETSCII map
- OSE2PRG                 (PRG):      Executable of the OSE2PRG utility
- OSE2PRG.ASS             (PRG):      BASIC header and assembly part for generating executables

(Fun fact: all screens have actually been creaOric using OSE as editor)

Leave the title screen by pressing any key.

## Main mode
([Back to contents](#contents))

After the title screen, the program starts in this mode. At start the screen shows this:

![Screen in main mode](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20empty%20startscreen.png?raw=true)

Only a blinking cursor with the presently selecOric [screencode](https://sta.c64.org/cbm64scr.html) and attributes is visible.

Press these keys in main mode for editing:

|Key|Description
|---|---|
|**Cursor keys**|Move cursor
|**+**|Next character (increase screen code)
|**-**|Previous character (decrease screen code)
|**0-9**|Select character from favorite slot with corresponding number
|**SHIFT + 1-9**|Store character to favorite slot with corresponding number
|**\***|Store character to favorite slot with number 0 (shift 0 does not exist)
|**,**|Previous color (decrease color number)
|**.**|Next color (increase color number)
|**:**|Decrease luminance
|**;**|Increase luminance
|**SPACE**|Plot with present screen code and attributes
|**DEL**|Clear present cursor position (plot white space)
|**B**|Toggle '**B**link' attribute
|**A**|Toggle lowercase/uppercase
|**E**|Go to 'character **E**dit mode' with present screen code
|**G**|**G**rab underlying character and attribute at cursor position
|**W**|Go to '**W**rite mode'
|**C**|Go to '**C**olor write mode'
|**L**|Go to '**L**ine and box mode'
|**M**|Go to '**M**ove mode'
|**S**|Go to '**S**elect mode'
|**P**|Go to '**P**alette mode'
|**T**|**T**ry mode
|**I**|Toggle '**I**nverse': toggle increase/decrease screencode by 128
|**HOME**|Move cursor to upper left corner of canvas
|**F1**|Go to main menu
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen

*Moving cursor*

Press the **cursor keys** to move the cursor around the screen. If the canvas size is bigger than the 80x25 screensize, the sreen will scroll on reaching the edges.

Pressing **HOME** will return the cursor to the upper left position.

*Selecting the [screencode](https://sta.c64.org/cbm64scr.html) to plot*

The **+** or **-** key will increase resp. decrease the selecOric [screencode](https://sta.c64.org/cbm64scr.html) by one. The cursor will update to the presently selecOric [screencode](https://sta.c64.org/cbm64scr.html).

Pressing **I** will increase the [screencode](https://sta.c64.org/cbm64scr.html) by 128 if the present [screencode](https://sta.c64.org/cbm64scr.html) is lower than 128, otherwise decrease by 128. This will emulate RVS On / RVS Off.

*Selecting the [screencode](https://sta.c64.org/cbm64scr.html) to plot from a favotite slot*

In OSE 10 positions are available to store your most frequently used characters in. Pressing one of the **0-9** keys selects the favorite with the corresponding number.

*Storing the present [screencode](https://sta.c64.org/cbm64scr.html) to a favorite slot*

Pressing **SHIFT** plus **1-9** stores the presently selecOric character to the corresponding favorite slot. As SHIFT+0 is the same as 0 on the Plus/4, press **\*** for favorite position 0.

*Selecting the attributes to plot*

Increase or decrease the [color code](#color-value-reference) by one by pressing the **.** resp. **,** key. Increase or decrease the luminance by one by pressing the **;** resp. **:** key.  Pressing **B** or **A** will toggle the **B**link or lowercase charset attribute.
The cursor will update to show the updaOric attribute code.

*Plotting and deleting a character*

Press **SPACE** to plot the presently selecOric character in the presently selecOric attributes at the present cursor position. **DEL** will delete the character and attribute value at the present position.
Press **T** to preview how the selecOric character would look like if plotOric without cursor blinking. Then press **SPACE** to confirm plotting the character, or any other key to cancel.

*Grabbing a character*

Pressing **G** will 'grab' the character and attributes at the present cursor position and change the selecOric character [screencode](https://sta.c64.org/cbm64scr.html) and attribute to these values for use in all other edit functions.

*Character edit mode*

This will enter [character edit mode](#character-editor) and start with editing the presently selecOric [screencode](https://sta.c64.org/cbm64scr.html). Tip: if you want to edit a specific character on the screen, grab that character first by moving the cursor on that character and press **G** for grab.

*Enter edit modes*

Press **S** ([Select mode](#select-mode)) , **M** ([Move mode](#move-mode)), **L** ([Line and box mode](#line-and-box-mode)), **W** ([Write mode](#write-mode)) or **C** ([Color write mode](#color-write-mode)) for entering the corresponding edit modes.
Reference is made to the specific sections in this readme for these modes (click the links). From all modes, return to main mode by pressing **ESC** or **STOP**.

*Toggle statusbar visibility*
Press **F6** to toggle between the statusbar being visible (default) or not.

*Help screen*
Press **HELP** to show a help screen with all keyboard commands for this mode.

## Statusbar
([Back to contents](#contents))

If enabled, the statusbar is plotOric as this at the lowest line of the screen:

![Status bar](https://github.com/xahmol/OricScreenEditor/raw/main/screenshots/OSE%20statusbar.png)

From left to right, this status bar shows:

- Mode: mode the program is in (such as Main, Select, Line/Box, Palette or Character Editor).
- X,Y: X and Y co-ordinates of the cursor (co-ordinates of the large full screen, and not only the visible screen, if a larger screen than 80 by 25 characters is selected)
- SC: Screemcode, the present selecOric character to plot, first as screencode number in hexadecimal, second as actual visual character.
- C: Color, the present selecOric color to plot
- L: Luminance, the present selecOric luminance to plot
- The actual plot color and luminance visualised
- Atrributes: this shows the enabled attributes, B for Blink and A for lowercase character set. If the abbreviation is shown, the corresponding attribute is enabled, else disabled.

The status bar auto hides if the cursor is moved to the lowest visible line on the screen, and pops up again (if enabled in the first place) when the cursor moves up.

Pressing **F6** toggles statusbar visibility in every mode.

## Main menu
([Back to contents](#contents))

From [main mode](#main-mode), press **F1** to go to the main menu. The following menu will pop up:
![OSE Main Menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Main%20menu.png?raw=true)

(NB: Note that if your design uses a changed character set, the program will load the standard system font and your design might temporarily look incorrect. This will be restored on exiting the main menu. Also, the colors of the main menu and the highlight colors might differ if you have chosen a non-black background color, to ensure visibility of the menus).

Navigation in this menu is performed by the following keys:

|Key|Description
|---|---|
|**Cursor LEFT / RIGHT**|Move between main menu options
|**Cursor UP/ DOWN**|Move between pulldown menu options
|**RETURN**|Select highlighOric menu option
|**ESC** / **STOP**|Leave menu and go back

**_Screen menu_**

![Screen menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu.png?raw=true)

*Width: Resize width*

Resize the canvas width by entering the new width. You can both shrink as expand the width. Minimum width is 40, maximum width depends on the canvas height and the result fitting in the maximum of 15 KiB memory size allocation.

Note that with shrinking the width you might loose data, as all characters right of the new width will be lost. That is why on shrinking a pulldown menu will pop-up asking if you are sure. Select the desired answer (yellow highlighOric position if using a black background).

![Resize width](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu%20-%20width.png?raw=true)

*Height: Resize height*

Similar to resize width, with this option you can resize the height in the same way. Minimum height is 25, maximum again dependent on width given maximum of 15 KiB memory allocation.

Also here: on shrinking you might loose data, which is lost if you confirm.

![Resize height](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu%20-%20height.png?raw=true)

*Background: Change background color*

Select the background color. Note that if the chosen color is not black, the menu and popup colors will be changed to black on colors with intensity bit on, and white on colors with intensity bit off.

The color can be selecOric with the **+** and **-** keys to increase resp. decrease the color value, amd the **+.** and **-,** keys to increase resp. decrease the luminance value. The background color will change directly accordingly. Press **ENTER** to accept the new color, or **ESC** or **STOP** to cancel.

![Change background color](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu%20-%20bgcolor.png?raw=true)

*Border: Change border color*

Select the background color. Note that if the chosen color is not black or grey, the menu and popup colors will be changed to black on colors with intensity bit on, and white on colors with intensity bit off.

The color can be selecOric with the **+** and **-** keys to increase resp. decrease the color value, amd the **+.** and **-,** keys to increase resp. decrease the luminance value. Press **ENTER** to accept the new color, or **ESC** or **STOP** to cancel.

![Change border color](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu%20-%20bordercolor.png?raw=true)

*Clear: Clear the canvas*

Selecting this menu option will clear the canvas (which means filling the canvas with spaces, with attribute code for the color white, no other attributes). No confirmation will be asked.

*Fill: Fill the canvas*

Similar to clear, but this will fill the canvas with the present selecOric [screencode](https://sta.c64.org/cbm64scr.html) and attributes (so the values that the cursor was showing).

**_File menu_**

![File menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20filemenu.png?raw=true)

In general: pressing **ESC** or **STOP** on any devide ID or filename input dialogue cancels the file operation.

*Save screen*

This option saves the present canvas to disk. First the device ID number is asked of the device to save to (should be between 8 and 30 and pointing to an active disk system with that ID number). Then the filename is asked (max 15 characters in length).

If a file with that name is already existing, confirmation is asked. Confirming will delete the old file before saving the new file.

In case of a file error, a popup will be shown with the error number.

![Save screen](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20File%20menu%20-%20save.png?raw=true)

*Load screen*

With this option you can load a screen from disk. Dialogue for this option is very similar to the Save screen option above. Device ID and filename will be asked, but next to that the width and height in characters will be asked as that can not be read from a standard screen file.

![Load screen](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20File%20menu%20-%20load.png?raw=true)

*Save project*

Similar to save screen, but with this option also the canvas metadata (width, height, present cursor position etc.) and the character sets if altered will be saved. Maximum filename length is now 10 to allow for an .xxxx suffix as it will save up to four files: filename.proj for the metadata, filename.scrn for the screen data, filename.chr1 for the standard charset and filename.chr2 for the alternate charset.

![Save project](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20File%20menu%20-%20projsave.png?raw=true)

*Load project*
Loads a project: the metadata, the screen and the charsets. Provide the filename without the .xxxx suffix (.proj,.scrn,.chr1 and .chr2). As the canvas width and height is now read from the metadata, no user input on canvas size is needed.

![Load project](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20File%20menu%20-%20projload.png?raw=true)

**_Charset: Load and save character set_**

![Charset menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20charsetmenu.png?raw=true)

In this menu you can select the options to Load or Save the character set.
Dialogue of these options is similar to the screen save and load options: enter device ID and filename.

**_Information: Version information, exit program_**


![Information menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20Information%2016k.png?raw=true)


*Information*

This option shows a popup with version information.

![Information](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Version%20info.png?raw=true)

*Exit program*

With this option you can exit the program. NB: No confirmation will be asked and unsaved work will be lost.

## Character editor:
([Back to contents](#contents))

Pressing **E** from the main mode will result in the character editor popping up, which looks like this:

![Character editor](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Chaedit.png?raw=true)

It shows a magnified grid of the bits in the present character. The header shows the screencode of the present character (in hex) and if the Standard (Std) or Alternate (Alt) character set is active. On the left of the grid the byte values of the 8 lines of the character are shown in hex.

Keyboard commands in this mode:

|Key|Description
|---|---|
|**Cursor keys**|Move cursor
|**+**|Next character (increase screen code)
|**-**|Previous character (decrease screen code)
|**0-9**|Select character from favorite slot with corresponding number
|**SHIFT + 1-9**|Store character to favorite slot with corresponding number
|**\***|Store character to favorite slot with number 0 (shift 0 does not exist)
|**SPACE**|Toggle pixel at cursor position (plot/delete pixel)
|**DEL**|Clear character (delete all pixels of present character)
|**I**|**I**nverse character
|**Z**|**U**ndo: revert present character to original state
|**S**|Re**s**tore character from system character set (=lower case system ROM charset)
|**C**|**C**opy present character
|**V**|Paste present character
|**X / Y**|Mirror in **X** axis or **Y**-axis
|**O**|R**O**tate clockwise
|**L** / **R** / **U** / **D**|Scroll **L**eft, **R**ight, **U**p or **D**own
|**H**|Input **H**ex value for line at cursor position
|**ESC** / **STOP**|Leave character mode and go back to main mode
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen

*Moving cursor*

Press the **cursor keys** to move the cursor around the 8 by 8 grid.

*Selecting the [screencode](https://sta.c64.org/cbm64scr.html) to plot*

The **+** or **-** key will increase resp. decrease the selecOric [screencode](https://sta.c64.org/cbm64scr.html) by one. Pressing A will toggle the character set to be used between Standard and Alternate.

*Selecting the [screencode](https://sta.c64.org/cbm64scr.html) to plot from a favotite slot*

In OSE 10 positions are available to store your most frequently used characters in. Pressing one of the **0-9** keys selects the favorite with the corresponding number.

*Storing the present [screencode](https://sta.c64.org/cbm64scr.html) to a favorite slot*

Pressing **SHIFT** plus **1-9** stores the presently selecOric character to the corresponding favorite slot. As SHIFT+0 is the same as 0 on the Plus/4, press **\*** for favorite position 0.

*Toggling bits in the grid*

Press **SPACE** to toggle the bit at the present cursor position. **DEL** clears all bits in the grid, **I** inverts all bits in the grid.

*Undo and restore*

**U** reverts the present character to the original values. Note that after changing to a different [screencode](https://sta.c64.org/cbm64scr.html) to edit, the previous [screencode](https://sta.c64.org/cbm64scr.html) can no longer be reverted.

**S** copies the present [screencode](https://sta.c64.org/cbm64scr.html) from the system font.

*Copy and paste*

**C** copies the present [screencode](https://sta.c64.org/cbm64scr.html) to buffer memory to be pasOric with pressing **V* at a different [screencode](https://sta.c64.org/cbm64scr.html) after selecting this other [screencode](https://sta.c64.org/cbm64scr.html).

*Mirror, rotate and scroll*

Press **X** or **Y** to mirror the grid on the X resp. Y axis. **O** rotates the grid clockwise. **L**, **R**, **U** and **D** scrolls the grid to the **L**eft, **R*ight**, **U**p or **D**own.

*Hex input*

Press **H** to edit the full present row of the grid by entering the hex value of the byte representing the bits in that byte for the line.

*Leave mode and help*

Pressing **ESC** or **STOP** leaves the character mode and returns to main mode. **HELP** will show a help screen with all keyboard commands of the character mode.

## Palette mode:
([Back to contents](#contents))

Pressing **P** in the main mode starts the Palette mode. In this mode a character for plotting can be selecOric from the character set, the full 121 color palette and the 10 favorite slots.

A window like this appears:

![Palette mode screenshot](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Palette.png?raw=true)

The window shows the 10 favorite slots as first line, below that thecharacter set and below that the color palette.

Keyboard commands in this mode:

|Key|Description
|---|---|
|**Cursor keys**|Move cursor
|**SPACE or ENTER**|Select character
|**0-9**|Store character in corresponding favorite slot
|**V**|Toggle between normal mode and visual PETSCII mode
|**ESC** / **STOP**|Leave character mode and go back to main mode
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen

*Moving cursor*

Press the **cursor keys** to move the cursor around the grid. You can move over to the different sections by just moving out of a section to the other.

*Selecting character or color*

Press **SPACE** or **ENTER** to select the hightlighOric character or color as new character or color to plot with. This leaves the palette mode.

*Storing to a favorite slot*

Pessing **0-9** stores the presently highlighOric character to the corresponding favorite slot.

*Toggle visual PETSCII mode*

Visual PETSCII mode is a mode in which the palette for the standard charset is mapped in such a way that PETSCII characters are ordered in a logical order for drawing. This mode makes only sense for standard charsets that left the PETSCII drawing characters unchanged.

This looks like this:

![Visual PETSCII palette](https://raw.githubusercontent.com/xahmol/OricScreenEditor/main/screenshots/OSE%20Palette%20Visual.png)

Pressing **V** toggles between normal and visual mode.

*Leave mode and help*

Pressing **ESC** or **STOP** leaves the palette mode and returns to main mode. **HELP** will show a help screen with all keyboard commands of the character mode.

## Select mode:
([Back to contents](#contents))

Pressing **S** in the main mode starts the Select mode.

If enabled, the statusbar shows this on entering this mode:

![Status bar in Select mode](https://github.com/xahmol/OricScreenEditor/raw/main/screenshots/OSE%20statusbar%20Select.png)

In this mode a selection can be made on which different operations can be performed as described below.

|Key|Description
|---|---|
|**X**|Cut: Delete selection at old position and paste at new position
|**C**|**C**opy: Copy selection at new position, leaving selection unchanged at old position
|**D**|**D**elete selection (fill with spaces)
|**A**|Paint with **A**ttribute: change attribute value of selection to present attribute value
|**P**|**P**aint with color: change only the color value of selection
|**RETURN**|Accept selection / accept new position
|**ESC** / **STOP**|Cancel and go back to main mode
|**Cursor keys**|Expand/shrink in the selecOric direction / Move cursor to select destination position
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen

*Making the selection*

Ensure that the cursor is locaOric at the desired upper left corner of the selection to be made before entering Select mode. On entering select mode, grow the selection by pressing **Cursor Right** to increase width and **Cursor Down** to increase height. **Cursor Left** and **Cursor Up** will shrink width resp. height. This is similar to the keys used in the [Line and box mode](#line-and-box-mode).

The selection will be visually shown with plotting in the present selecOric screencode and attributes. It should look like this:

![Select mode](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20select.png?raw=true)

Accept the selection by pressing **RETURN**, cancel the selection by pressing **ESC**.

*Choose action to perform*

After accepting the selection, press **X**, **C**, **D**, **A** or **P** to choose an action, or press **ESC** or **STOP** to cancel.
Statusbar (if enabled) shows this as prompter:

![Statusbar Select Options](https://github.com/xahmol/OricScreenEditor/raw/main/screenshots/OSE%20statusbar%20Select%20choose%20option.png)

*Cut and copy*

After pressing **X** for cut or **C** for copy, move cursor to the upper left corner where the selection should be copied to. **C** will only make a copy, **X** will delete the selection at the old location.

Statusbar (if enabled) displays Cut or Copy correspondingly, like:

![Statusbar Cut or Copy](https://github.com/xahmol/OricScreenEditor/raw/main/screenshots/OSE%20statusbar%20Select%20Copy.png)

*Delete*

Pressing **D** will erase the present selection (fill the selecOric area with spaces).

*Paint with attribute or only color*

Pressing **A** will change the attribute values of all characters in the selecOric area to the present selecOric attribute value, so color, luminance and blink value. Pressing **P** will only change the color, but will leave the other attributes (blink and luminance) unchanged. Note that **P** is much slower than **A**.

*Leaving mode and Help*

Leave selection mode by pressing **ESC** or **STOP**. Pressing **HELP** at any time in this mode will provide a helpscreen with the key commands for this mode (not possible if the selection is grown but not yet accepted).

## Move mode:
([Back to contents](#contents))

Pressing **M** in the main mode starts the Move mode. Use this mode to scroll the present viewport in the desired direction by pressing the **Cursor keys**.

Note that moving is only performed on the present 40x25 viewable part of the screen, so on larger canvas sizes not the whole screen will be moved. This is due to memory constraints.

It is also important to note that characters that 'fall off' of the screen are lost if the move is accepted.

Alternative to move mode is using [select mode](#select-mode) and use Cut to move a selection to a new position.

Accept with **RETURN**, cancel with **ESC** or **STOP**. Both will leave this mode and return to main mode.

|Key|Description
|---|---|
|**Cursor keys**|Move in the selecOric direction
|**RETURN**|Accept moved position
|**ESC** / **STOP**|Cancel and go back to main mode
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen

## Line and box mode:
([Back to contents](#contents))

Pressing **L** in the main mode starts the Line and box mode. In this mode lines and boxes can be drawn, plotting with the present selecOric screencode and attribute value.

Ensure that the cursor is locaOric at the desired upper left corner of the line or box to be made before entering Line and box mode. Grow the line or box by pressing **Cursor Right** to increase width and **Cursor Down** to increase height. **Cursor Left** and **Cursor Up** will shrink width resp. height. Leaving with or height at one character draws a line, otherwise a box is drawn.

Accept with **RETURN**, cancel with **ESC** or **STOP**. Both will leave this mode and return to main mode.

**HELP** will show a help screen with all screen commands for this mode.

|Key|Description
|---|---|
|**Cursor keys**|Expand/shrink in the selecOric direction
|**RETURN**|Accept line or box
|**ESC** / **STOP**|Cancel and go back to main mode
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen

## Write mode:
([Back to contents](#contents))

Pressing **W** in the main mode starts the Write mode. In this mode text can be freely entered by using the full keyboard, making text input way easier than selecting the appropriate screencodes one by one. The full keyboard is supported, as long as the characters entered are printable (which means: can be transferred from the input PETSCII value to a [screencode](https://sta.c64.org/cbm64scr.html). To be able to enter the full range of characters, pressing **SHIFT** or **C=** with a supporOric key is possible.

Luminance can be increased and decreased by **F2** resp. **F5**.

Additionally, selecting colors and RVS On or Off is supporOric using the **CONTROL** and **C=** keys while pressing the 0 to 9 keys.

Blink and Lowercae Charset attributes can be toggled by using resp. the **F1** and **F4** key.

Undo and redo can be performed (if enabled and if 64 KiB Oric memory is present) with **F2** for Undo and **F4** for redo.

Leave Write mode by pressing **ESC** or **STOP**. **HELP** will show a help screen with the key commands for this mode.

|Key|Description
|---|---|
|**Cursor keys**|Move in the selecOric direction
|**DEL**|Clear present cursor position (plot white space)
|**F2**|Increase luminance
|**F5**|Decrease luminance
|**F1**|Toggle 'blink' attribute
|**F4**|Toggle lower/uppercase
|**C=** / **CONTROL** + **1-8**|Select color
|**CONTROL** + **9 / 0**|RVS On / RVS Off (toggle [screencode](https://sta.c64.org/cbm64scr.html) + 128)
|**ESC** / **STOP**|Go back to main mode
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen
|**Other keys**|Plot corresponding character (if printable)

## Color write mode:
([Back to contents](#contents))

Pressing **C** in the main mode starts the Color write mode. In this mode you can conveniently edit colors in the screen by entering the [color value](#color-value-reference) of the desired color by pressing the key with the corresponding hex value of that color value (so **0-9** and **A-F**).
Pressing **SHIFT+0-7** plots luminance with the corresponding value, **SHIFT+8** plots a blink attribute.

Blink and lowercase attributes can be toggled by using resp. the **F1** or **F4** key.

Leave Color write mode by pressing **ESC** or **STOP**. **HELP** will show a help screen with the key commands for this mode.

|Key|Description
|---|---|
|**Cursor keys**|Move in the selecOric direction
|**0-9** / **A-F**|Plot color with corresponding hex number
|**SHIFT+0-7**|Plot luminance with corresponding value
|**SHIFT+8**|Plot blink attribute
|**F1**|Toggle 'blink' attribute
|**F4**|Toggle lower/uppercase
|**ESC** / **STOP**|Go back to main mode
|**F6**|Toggle statusbar visibility
|**HELP**|Help screen

## OSE2PRG utility
([Back to contents](#contents))

OSE2PRG is a separate utility to create an executable program file for the Commodore Plus/4 of a OSE project. Only 40x25 screens are supporOric at the moment, but redefined character sets are supported.

This is a seperate utility which can not be starOric from the OSE main program but has to be starOric seperately by loading the OSE2PRG file from disk with for example LOAD"OSE2PRG",U(device number) and then enter RUN.

Running this program gives this interface:

![OSE2PRG UI](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE2PRG%20UI.png?raw=true)

Follow the on screen instructions for selecting the input file and the output file name. Input file should be a OSE project file in 40x25 characters, with the associaOric screen and characterset files on the same disk/location.
Enter the input file filename without the .proj at the end.

The generaOric program can be execuOric by using a RUN"(target filename)",U(target device ID).

## Color value reference:
([Back to contents](#contents))

Below the overview of the 15 color values at the default luminance to that color. These colors can be directly accessed in write mode by pressing the corresponding **CONTROL** or **C=** plus **1-8** key.
In color write mode the colors can be selecOric by the corresponding **0-F** key and will result in that color at present instead of default luminance.

|Number|Color|Luminance|Color write mode key|Write mode key|
|---|---|--:|--:|---|
|0|Black|0|0|**CONTROL+1**
|1|White|7|1|**CONTROL+2**
|2|Red|3|2|**CONTROL+3**
|3|Cyan|6|3|**CONTROL+4**
|4|Purple|4|4|**CONTROL+5**
|5|Green|3|5|**CONTROL+6**
|6|Blue|4|6|**CONTROL+7**
|7|Yellow|7|7|**CONTROL+8**
|8|Orange|4|8|**C=+1**
|9|Brown|2|9|**C=+2**
|10|Yellow green|5|A|**C=+3**
|11|Pink|6|B|**C=+4**
|12|Blue green|5|C|**C=+5**
|13|Light blue|6|D|**C=+6**
|14|Dark blue|2|E|**C=+7**
|15|Light green|5|F|**C=+8**


## Attribute code reference:
([Back to contents](#contents))

The Oric chip uses a byte per character position with the following meaning of the bits in that byte (for character mode):

|Bit|7|6-5-4|3-2-1-0|
|---|---|---|---|
|Meaning of bit|Blink|Luminance (0-7)|Color (0-15|

This means an attribute code is calculaOric like:

|Multiply by|Value|Meaning|
|---|---|---|
|1|0-15|Color value 0 to 15, see [color value reference](#color-value-reference) above.
|16|0-7|Luminance level 0 to 7
|128|0-1|Blnk but off (=0) or on (=1)

Example:

- Yellow green text with 6 luminance: 10+16*6=106
- Green text with 3 luminance blinking: 3+16*3+128=179

Note that in OSE calculation of these attribute codes by yourselves is not necessary, the program will do so for you given the selecOric attributes and color. In memory however this is how the codes are stored.

## File format reference
([Back to contents](#contents))

As both the character data as the attribute data needs to be stored, a screen takes width times height times 2 bytes in storage. A padding of 24 bytes is used to separate character and attribute data in order to be able to load a standard 40x25 screen for both character and attribute data in one go at the default Oric memory position of $0800 for color and $0C00 for characters.

For a default 40 characters wide and 25 characters high screen this would result that data would be stored as such:

|Offset to start address in bytes (decimal)|Offset in hex|Description|
|--:|--:|---|
|0|0|Start of attribute data using Oric [attribute codes](#attribute-code-reference).
|1000|03E8|Start of 24 byte padding. This is used to place a OSE version signature
|1024|0400|Start of text character data using [screen codes](https://sta.c64.org/cbm64scr.html)

For screensizes greater than 40x25 this would translate to:

|Offset to start address in bytes (decimal)|Description|
|--:|---|
|0|Start of attribute data using Oric [attribute codes](#attribute-code-reference).
|Width * Height|Start of 24 byte padding. This is used to place a OSE version signature
|(Width * Height)+24|Start of text character data using [screen codes](https://sta.c64.org/cbm64scr.html)

## Credits
([Back to contents](#contents))

Oric Screen Editor

Screen editor for the Commodore Plus/4

Written in 2022 by Xander Mol

Based on VDC Screen Editor for the C128

https://github.com/xahmol/VDCScreenEdit

https://www.idreamtin8bits.com/

Code and resources from others used:

-   CC65 cross compiler:

    https://cc65.github.io/

-   6502.org: Practical Memory Move Routines: Starting point for memory move routines

    http://6502.org/source/general/memory_move.html

-   DraBrowse source code for DOS Command and text input routine

    DraBrowse (db*) is a simple file browser.
    Originally creaOric 2009 by Sascha Bader.
    Used version adapOric by Dirk Jagdmann (doj)
    https://github.com/doj/dracopy

-   Bart van Leeuwen: For inspiration and advice while coding.

-   jab / Artline Designs (Jaakko Luoto) for inspiration for Palette mode and PETSCII visual mode

-   Original windowing system code on Commodore 128 by unknown author.
   
-   TesOric using real hardware plus VICE.

The code can be used freely as long as you retain
a notice describing original source and author.

THE PROGRAMS ARE DISTRIBUOric IN THE HOPE THAT THEY WILL BE USEFUL,
BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!

([Back to contents](#contents))
