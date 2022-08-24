# Oric Screen Editor
Screen editor for the Oric Atmos

## Contents:

[Version history and download](#version-history-and-download)

[Introduction](#introduction)

[Known issues](#known-issues)

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

[Color value reference](#color-value-reference)

[Serial attribute code reference](#serial-attribute-code-reference)

[File format reference](#file-format-reference)

[Credits](#credits)


![OSE Title Screen](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Titlescreen.png?raw=true)

## Version history and download
([Back to contents](#contents))

Link to latest builds:
- [.DSK disk image](https://github.com/xahmol/OricScreenEditor/raw/main/BUILD/OSE.dsk)
- [.HFE disk image](https://github.com/xahmol/OricScreenEditor/raw/main/BUILD/OSE.hfe)

Version v099-20220824-1345:
- File picker added
- Minor other tweaks and fixes

Version v099-20220615-1454:
- First released beta version based on [VDCSE](https://github.com/xahmol/VDCScreenEdit) version v099-20220324-1527

## Introduction
([Back to contents](#contents))

Oric Screen Editor is an editor to create text based screens for the Oric Atmos. It fully supports using user defined character sets.

Main features of the program:
- Support for screen maps larger than 40x25 characters. Screens can be up to 8 KiB (8.192 bytes), all sizes fitting in that memory with width of 40 at minimum and heigth of 27 at minimum are supported.
- Supports resizing canvas size, clear or fill the canvas
- Support for loading user defined charsets (should be standard charsets of 96 characters of 6 bits width and 8 bits height, alternate charsets of 80 characters or combined charsets of 176 characters).
- Includes a simple character editor to change characters on the fly and directly see the result in your designed screen.
- Supports the Oric serial atribites for ink, paper and character modifiers (standard vs alternate charset, double height, blink).
- Write mode to freely type characters with the keyboard
- Line and box mode for drawing lines and boxes
- Select mode to cut, copy, delete or repaint (only color or all attributes) the selection.
- Move mode to scroll the screen contents (due to memory constraints only for the 40x27 viewport)
- Palette mode, including visual charmap mode, to visually select characters and colors
- Favorite slots to quickly select 10 favorite characters

## Known issues
- Filepicker routine only is properly working with SEDORIC3 disks created by the TAP2DSK tool from OSDK. If you want to import screens or charsets from other disks, please copy them to a TAP2DSK created image first.

## Start program
([Back to contents](#contents))

Mount the OSE disk image. Choose the .DSK or .HFE image depending on what is supported on the hardware or emulator you use (in general: .HFE for Cumana Reborn, .DSK for emulator).

The OSE executable will autostart on mounting the disk. If not, just type OSE\<ENTER\>.

Description of contents of the disk image:

|Filename|Extension|Description|
|---|---|---|
|OSE|.COM|Main executable
|OSEHS1|.BIN|Help screen for main mode
|OSEHS2|.BIN|Help screen for character edit and palette modes
|OSEHS3|.BIN|Help screen for select, move and line/box modes
|OSEHS4|.BIN|Help screen for write and color write modes
|OSETSC|.BIN|Title screen


(Fun fact: all screens have actually been created using OSE as editor)

Leave the title screen by pressing any key.

## Main mode
([Back to contents](#contents))

After the title screen, the program starts in this mode. At start the screen shows this:

![Screen in main mode](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20empty%20startscreen.png?raw=true)

Only a inversed cursor with the presently selected screencode is visible.

Press these keys in main mode for editing:

|Key|Description
|---|---|
|**Cursor keys**|Move cursor
|**+** or **=**|Next character (increase screen code)
|**-**|Previous character (decrease screen code)
|**0-9**|Select character from favorite slot with corresponding number
|**SHIFT + 0-9**|Store character to favorite slot with corresponding number
|**,**|Previous ink (decrease color number)
|**.**|Next ink (increase color number)
|**;**|Decrease paper (decrease color number)
|**\'**|Increase paper (increase color number)
|**SPACE**|Plot with present screen code and attributes
|**DEL**|Clear present cursor position (plot white space)
|**B**|Toggle '**B**link' attribute
|**A**|Toggle **A**lternate or Standard character set
|**D**|Toggle **D**ouble height attribute
|**I**|Plot a change **I**nk modifier attribute for the present selected ink color
|**O**|Plot a change Paper modifier attribute for the present selected paper color
|**U**|Plot a characterset modifier attribute for the present selected charset selection, double size and blink attributes
|**E**|Go to 'character **E**dit mode' with present screen code
|**G**|**G**rab underlying character and attribute at cursor position
|**W**|Go to '**W**rite mode'
|**L**|Go to '**L**ine and box mode'
|**M**|Go to '**M**ove mode'
|**S**|Go to '**S**elect mode'
|**P**|Go to '**P**alette mode'
|**T**|**T**ry mode
|**R**|Toggle '**R**everse': toggle increase/decrease screencode by 128
|**FUNCT+1**|Go to main menu
|**FUNCT+6**|Toggle statusbar visibility
|**FUNT+8**|Help screen

**NB**: Within Oricutron using the default keyboard mapping, the FUNCT key is assigned to the ALT key on the PC keyboard.

*Moving cursor*

Press the **cursor keys** to move the cursor around the screen. If the canvas size is bigger than the 40x27 screensize, the sreen will scroll on reaching the edges.

*Selecting the screencode to plot*

The **+** or **-** key will increase resp. decrease the selected screencode by one. The cursor will update to the presently selected screencode.

Pressing **R** will increase the screencode by 128 if the present screencode is lower than 128, otherwise decrease by 128. This will enable/disable the reverse bit for the hardware reversing of the character.

*Selecting the screencode to plot from a favorite slot*

In OSE 10 positions are available to store your most frequently used characters in. Pressing one of the **0-9** keys selects the favorite with the corresponding number.

*Storing the present screencode to a favorite slot*

Pressing **SHIFT** plus **0-9** stores the presently selected character to the corresponding favorite slot.

*Selecting the attributes to plot*

Increase or decrease the [color code](#color-value-reference) by one by pressing the **.** resp. **,** key for the Ink color, or **;** resp. **\'** for the Paper color. Pressing **B**, **D** or **A** will toggle the **B**link, **D**ouble size or **A**lternate charset attribute.

*Plotting and deleting a character*

Press **SPACE** to plot the presently selected character at the present cursor position. **DEL** will delete the character or attribute value at the present position and replace it with a SPACE as character.
Press **T** to preview how the selected character would look like if plotted without cursor blinking. Then press **SPACE** to confirm plotting the character, or any other key to cancel.

*Plotting serial attributes*

Due to the way the Oric handles [color and attribute changes](#serial-attribute-code-reference), every screen position is either an attribute modifier or a character, but never both at the same time. For this reason, on plotting a character no atrribute is plotted. The user of OSE needs to plot the serial attributes seperately by its own desire.
To do so, press **I** for plotting an **I**nk modifier for the present ink color, **P** for plotting a **P**aper modifier for the present paper color and **U** for plotting a characterset modifier for the present character atrributes (the alternate, double and blink toggles).

*Grabbing a character*

Pressing **G** will 'grab' the character or attributes at the present cursor position and change the selected character screencode or attribute to these values for use in all other edit functions.

*Character edit mode*

This will enter [character edit mode](#character-editor) and start with editing the presently selected screencode. Tip: if you want to edit a specific character on the screen, grab that character first by moving the cursor on that character and press **G** for grab.

*Enter edit modes*

Press **S** ([Select mode](#select-mode)) , **M** ([Move mode](#move-mode)), **L** ([Line and box mode](#line-and-box-mode)) or **W** ([Write mode](#write-mode)) for entering the corresponding edit modes.
Reference is made to the specific sections in this readme for these modes (click the links). From all modes, return to main mode by pressing **ESC**.

*Toggle statusbar visibility*
Press **FUNCT+6** to toggle between the statusbar being visible (default) or not.

*Help screen*
Press **FUNCT+8** to show a help screen with all keyboard commands for this mode.

## Statusbar
([Back to contents](#contents))

If enabled, the statusbar is plotted as this at the lowest line of the screen:

![Status bar](https://github.com/xahmol/OricScreenEditor/raw/main/screenshots/OSE%20statusbar.png)

From left to right, this status bar shows:

- Mode: mode the program is in (such as Main, Select, Line/Box, Palette or Character Editor).
- X,Y: X and Y co-ordinates of the cursor (co-ordinates of the large full screen, and not only the visible screen, if a larger screen than 40 by 27 characters is selected. Normally shows in decimal, but shows in hexadecimal if X or Y maximum size is higher than 99)
- C: Screemcode, the present selected character to plot, first as screencode number in hexadecimal, second as actual visual character.
- S: The screencode in memory underneath the cursor position. Will show either the character code (bigger than $20) or the attribute code (smaller than $20) in hexadecimal
- I: Ink, the present selected color for the ink. First as number 0-7, then as visual color.
- P: Paper, the present selected color for the paper. First as number 0-7, then as visual color.
- The last three positions show the character set modifier attributes that are enabled: A if the alternate charset is enabled, D for double size and B for blink.

The status bar auto hides if the cursor is moved to the lowest visible line on the screen, and pops up again (if enabled in the first place) when the cursor moves up.

Pressing **FUNCT+6** toggles statusbar visibility in every mode.

## Main menu
([Back to contents](#contents))

From [main mode](#main-mode), press **FUNCT+1*** to go to the main menu. The following menu will pop up:
![OSE Main Menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Main%20menu.png?raw=true)

(NB: Note that if your design uses a changed character set, the program will load the standard system font and your design might temporarily look incorrect. This will be restored on exiting the main menu. Also, the pulldown menus will show a black shade to the right due to Oric system limitations).

Navigation in this menu is performed by the following keys:

|Key|Description
|---|---|
|**Cursor LEFT / RIGHT**|Move between main menu options
|**Cursor UP/ DOWN**|Move between pulldown menu options
|**RETURN**|Select highlighted menu option
|**ESC** |Leave menu and go back

**_Screen menu_**

![Screen menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu.png?raw=true)

*Width: Resize width*

Resize the canvas width by entering the new width. You can both shrink as expand the width. Minimum width is 40, maximum width depends on the canvas height and the result fitting in the maximum of 8 KiB memory size allocation.

Note that with shrinking the width you might loose data, as all characters right of the new width will be lost. That is why on shrinking a pulldown menu will pop-up asking if you are sure. Select the desired answer (yellow highlighted position if using a black background).

![Resize width](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu%20-%20width.png?raw=true)

*Height: Resize height*

Similar to resize width, with this option you can resize the height in the same way. Minimum height is 27, maximum again dependent on width given maximum of 8 KiB memory allocation.

Also here: on shrinking you might loose data, which is lost if you confirm.

![Resize height](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20menu%20-%20height.png?raw=true)

*Clear: Clear the canvas*

Selecting this menu option will clear the canvas (which means filling the canvas with spaces, with attribute code for the selected color and selected paper plotted in the first two columns of the screen). No confirmation will be asked.

*Fill: Fill the canvas*

Similar to clear, but this will fill the canvas with the present selected screencode (so the values that the cursor was showing).

**_File menu_**

![File menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20filemenu.png?raw=true)

In general: pressing **ESC** on a filename input dialogue cancels the file operation. Wildcards can be used in filenames for loading: **\*** for allowing anything afterwards, **?** to allow any character for the given position.

**NB: In the present version, error handling of file opeations is ppor, for example entering incorrect / inexisting filenames exit the program to BASIC with poor recovery possibility of unsaved work. Advice is to save often on work in progress versions.**

*Save screen*

This option saves the present canvas to disk. The filename is asked (max 9 characters in length).

In case of a file error, a popup will be shown with the error number.

![Save screen](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20File%20menu%20-%20save.png?raw=true)

*Load screen*

With this option you can load a screen from disk. The filename to be loaded can be selected with the file picker showing the files on the disc. Next to the disk name, if the name convention is recognized, filetype is shown for reference. After selecting the file, the width and height in characters will be asked as that can not be read from a standard screen file.

![Load screen](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20File%20menu%20-%20filepicker.png?raw=true)

*Save project*

Similar to save screen, but with this option also the canvas metadata (width, height, present cursor position etc.) and the character sets if altered will be saved. Maximum filename length is now 10 to allow for an .xxxx suffix as it will save up to four files: filename.proj for the metadata, filename.scrn for the screen data, filename.chr1 for the standard charset and filename.chr2 for the alternate charset.

![Save project](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20File%20menu%20-%20projsave.png?raw=true)

*Load project*
Loads a project: the metadata, the screen and the charsets. Provide the filename with the filepicker (same as with Load Screen). A pop-up will be shown if the filename selected does not meet the name convention for a project (filename has to end with PJ). As the canvas width and height is now read from the metadata, no user input on canvas size is needed.

*Save combined*

This option saves the present canvas and charsets to disk in a single file (First 768 bytes of the standard charset, 256 bytes of empty space (the first 32 non-visible positions of the alternate charset), then 640 bytes of the visble alternate charset, finally the screenmap). Only makes sense for 40x27 standard screens, with the benefit that they can be loaded in one go with $B500 as base address. For the rest exactly similar in dialogue as Save Screen.

*Load combined*

This option loads the present canvas and charsets to disk in a single file (First 768 bytes of the standard charset, 256 bytes of empty space (the first 32 non-visible positions of the alternate charset), then 640 bytes of the visble alternate charset, finally the screenmap). For the rest exactly similar in dialogue as Save Screen.

**_Charset: Load and save character set_**

![Charset menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20charsetmenu.png?raw=true)

In this menu you can select the options to Load or Save the character sets for either the standard or alternate charsets separately (768 bytes resp. 640 bytes files) or as a combined file for both character sets (768 standard charset, 256 bytes empty space, 640 bytes alternate charset).
Dialogue of these options is similar to the screen save and load options: enter device ID and filename.

**_Information: Version information, exit program_**


![Information menu](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Screen%20Information.png?raw=true)


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
|**SHIFT + 0-9**|Store character to favorite slot with corresponding number
|**SPACE**|Toggle pixel at cursor position (plot/delete pixel)
|**DEL**|Clear character (delete all pixels of present character)
|**I**|**I**nverse character
|**Z**|**U**ndo: revert present character to original state
|**S**|Re**s**tore character from system character set (=lower case system ROM charset)
|**C**|**C**opy present character
|**V**|Paste present character
|**X / Y**|Mirror in **X** axis or **Y**-axis
|**L** / **R** / **U** / **D**|Scroll **L**eft, **R**ight, **U**p or **D**own
|**H**|Input **H**ex value for line at cursor position
|**ESC**|Leave character mode and go back to main mode
|**FUNCT+6**|Toggle statusbar visibility
|**FUNCT+8**|Help screen

*Moving cursor*

Press the **cursor keys** to move the cursor around the 8 by 8 grid.

*Selecting the screencode to plot*

The **+** or **-** key will increase resp. decrease the selected screencode by one. Pressing A will toggle the character set to be used between Standard and Alternate.

*Selecting the screencode to plot from a favotite slot*

In OSE 10 positions are available to store your most frequently used characters in. Pressing one of the **0-9** keys selects the favorite with the corresponding number.

*Storing the present screencode to a favorite slot*

Pressing **SHIFT** plus **0-9** stores the presently selected character to the corresponding favorite slot.

*Toggling bits in the grid*

Press **SPACE** to toggle the bit at the present cursor position. **DEL** clears all bits in the grid, **I** inverts all bits in the grid.

*Undo and restore*

**U** reverts the present character to the original values. Note that after changing to a different screencode to edit, the previous screencode can no longer be reverted.

**S** copies the present screencode from the system font.

*Copy and paste*

**C** copies the present screencode to buffer memory to be pasted with pressing **V** at a different screencode after selecting this other screencode.

*Mirror and scroll*

Press **X** or **Y** to mirror the grid on the X resp. Y axis. **L**, **R**, **U** and **D** scrolls the grid to the **L**eft, **R**ight, **U**p or **D**own.

*Hex input*

Press **H** to edit the full present row of the grid by entering the hex value of the byte representing the bits in that byte for the line.

*Leave mode and help*

Pressing **ESC** leaves the character mode and returns to main mode. **FUNCT+8** will show a help screen with all keyboard commands of the character mode.

## Palette mode:
([Back to contents](#contents))

Pressing **P** in the main mode starts the Palette mode. In this mode a character for plotting can be selected from the character set, the full 121 color palette and the 10 favorite slots.

A window like this appears:

![Palette mode screenshot](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Palette.png?raw=true)

The window shows the 10 favorite slots as first line, below that the standard character set and below that the alternate character set.

Keyboard commands in this mode:

|Key|Description
|---|---|
|**Cursor keys**|Move cursor
|**SPACE or ENTER**|Select character
|**0-9**|Store character in corresponding favorite slot
|**V**|Toggle between normal mode and visual charmap mode
|**ESC** |Leave character mode and go back to main mode
|**FUNCT+6**|Toggle statusbar visibility
|**FUNCT+8**|Help screen

*Moving cursor*

Press the **cursor keys** to move the cursor around the grid. You can move over to the different sections by just moving out of a section to the other.

*Selecting character or color*

Press **SPACE** or **ENTER** to select the highlighted character or color as new character or color to plot with. This leaves the palette mode.

*Storing to a favorite slot*

Pessing **0-9** stores the presently highlighted character to the corresponding favorite slot.

*Toggle visual charmap mode*

Visual charmap mode is a mode in which the palette for the alternate charset is mapped in such a way that characters are ordered in a logical order for drawing. This mode makes only sense for unchanged alternate charsets.

This looks like this:

![Visual PETSCII palette](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20Palette%20Visual.png?raw=true)

Pressing **V** toggles between normal and visual mode.

*Leave mode and help*

Pressing **ESC** leaves the palette mode and returns to main mode. **FUNCT+8** will show a help screen with all keyboard commands of the character mode.

## Select mode:
([Back to contents](#contents))

Pressing **S** in the main mode starts the Select mode.

If enabled, the statusbar shows this on entering this mode:

![Status bar in Select mode](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20statusbar%20Select.png?raw=true)

In this mode a selection can be made on which different operations can be performed as described below.

|Key|Description
|---|---|
|**X**|Cut: Delete selection at old position and paste at new position
|**C**|**C**opy: Copy selection at new position, leaving selection unchanged at old position
|**D**|**D**elete selection (fill with spaces)
|**I**|Paint with **I**nk attribute: plot ink modifier in presently selected color
|**P**|Paint with **P**aper attribute: plot paper modifier in presently selected color
|**M**|Paint with Character **M**odifier attribute: plot character modifier in presently selected A,D and B attributes.
|**RETURN**|Accept selection / accept new position
|**ESC** |Cancel and go back to main mode
|**Cursor keys**|Expand/shrink in the selected direction / Move cursor to select destination position
|**FUNCT+6**|Toggle statusbar visibility
|**FUNCT+8**|Help screen

*Making the selection*

Ensure that the cursor is located at a corner of the selection to be made before entering Select mode. On entering select mode, grow the selection by pressing the **Cursor keys** to increase or decrease width and height in the desired direction from the origin. This is similar to the keys used in the [Line and box mode](#line-and-box-mode).

The selection will be visually shown with plotting in the present selected screencode and attributes. It should look like this:

![Select mode](https://github.com/xahmol/OricScreenEditor/blob/main/screenshots/OSE%20select.png?raw=true)

Accept the selection by pressing **RETURN**, cancel the selection by pressing **ESC**.

*Choose action to perform*

After accepting the selection, press **X**, **C**, **D**, **A** or **P** to choose an action, or press **ESC**  to cancel.
Statusbar (if enabled) shows this as prompter:

![Statusbar Select Options](https://github.com/xahmol/OricScreenEditor/raw/main/screenshots/OSE%20statusbar%20Select%20choose%20option.png)

*Cut and copy*

After pressing **X** for cut or **C** for copy, move cursor to the upper left corner where the selection should be copied to. **C** will only make a copy, **X** will delete the selection at the old location.

Statusbar (if enabled) displays Cut or Copy correspondingly, like:

![Statusbar Cut or Copy](https://github.com/xahmol/OricScreenEditor/raw/main/screenshots/OSE%20statusbar%20Select%20Copy.png)

*Delete*

Pressing **D** will erase the present selection (fill the selected area with spaces).

*Paint with attribute or only color*

Pressing **I**, **P** or **M** will fill the area with resp. ink color, paper color or character set modifiers with the presently selected values. Makes most sense in selecting a vertical length of one character wide to change attributes for all lines right of that line.

*Leaving mode and Help*

Leave selection mode by pressing **ESC** . Pressing **FUNCT+8** at any time in this mode will provide a helpscreen with the key commands for this mode (not possible if the selection is grown but not yet accepted).

## Move mode:
([Back to contents](#contents))

Pressing **M** in the main mode starts the Move mode. Use this mode to scroll the present viewport in the desired direction by pressing the **Cursor keys**.

Note that moving is only performed on the present 40x25 viewable part of the screen, so on larger canvas sizes not the whole screen will be moved. This is due to memory constraints.

It is also important to note that characters that 'fall off' of the screen are lost if the move is accepted.

Alternative to move mode is using [select mode](#select-mode) and use Cut to move a selection to a new position.

Accept with **RETURN**, cancel with **ESC**. Both will leave this mode and return to main mode.

|Key|Description
|---|---|
|**Cursor keys**|Move in the selected direction
|**RETURN**|Accept moved position
|**ESC**|Cancel and go back to main mode
|**FUNCT+6**|Toggle statusbar visibility
|**FUNCT+8**|Help screen

## Line and box mode:
([Back to contents](#contents))

Pressing **L** in the main mode starts the Line and box mode. In this mode lines and boxes can be drawn, plotting with the present selected screencode and attribute value.

Ensure that the cursor is located at a corner of the selection to be made before entering Select mode. On entering select mode, grow the selection by pressing the **Cursor keys** to increase or decrease width and height in the desired direction from the origin. Leaving with or height at one character draws a line, otherwise a box is drawn.

Accept with **RETURN**, cancel with **ESC**. Both will leave this mode and return to main mode.

**FUNCT+8** will show a help screen with all screen commands for this mode.

|Key|Description
|---|---|
|**Cursor keys**|Expand/shrink in the selected direction
|**RETURN**|Accept line or box
|**ESC**|Cancel and go back to main mode
|**FUNCT+6**|Toggle statusbar visibility
|**FUNCT+8**|Help screen

## Write mode:
([Back to contents](#contents))

Pressing **W** in the main mode starts the Write mode. In this mode text can be freely entered by using the full keyboard, making text input way easier than selecting the appropriate screencodes one by one. The full keyboard is supported, as long as the characters entered are printable (screencodes higher than 32).

Colors and attribute can be plotted while in write mode by first modifying the attributes desired, then by plotting an attribute code:
- Pressing **CTRL+Z** or **CTRL+X** decreases resp. increases selected ink color
- Pressing ***CTRL+C** or **CTRL+V** decreases resp. increases selected paper color
- Pressing **CTRL+B**, **CTRL+A** or **CTRL+D** toggles alternate charset, double resp. blink attribute
- Pressing **CTRL+R** toggles reverse mode
- Pressing **FUNCT+1** plots ink
- Pressing **FUNCT+2** plots paper
- Pressing **FUNCT+3** plots character modifier.

Leave Write mode by pressing **ESC**. **FUNCT+8** will show a help screen with the key commands for this mode.

|Key|Description
|---|---|
|**Cursor keys**|Move in the selecOric direction
|**DEL**|Clear present cursor position (plot white space)
|**CTRL+A**|Toggles alternate charset attribute
|**CTRL+B**|Toggles blink attribute
|**CTRL+D**|Toggles double attribute
|**CTRL+R**|Toggles reverse
|**CTRL+Z**|Decreases ink color
|**CTRL+X**|increases ink color
|**CTRL+C**|Decreases paper color
|**CTRL+V**|increases paper color
|**FUNCT+1**|Plots ink
|**FUNCT+2**|Plots paper
|**FUNCT+3**|Plots character modifier
|**ESC** |Go back to main mode
|**FUNCT+6**|Toggle statusbar visibility
|**FUNCT+8**|Help screen
|**Other keys**|Plot corresponding character (if printable)

## Color value reference:
([Back to contents](#contents))

Below the overview of the 8 color values.

The values are calculated from the 3 bits used for the Red, Green and Blue (RGB) bits:
- +1 for Red
- +2 for Green
- +4 for Blue

|Number|Color|B-G-R|
|---|---|---|
|0|Black|0-0-0|
|1|Red|0-0-1|
|2|Green|0-1-0|
|3|Yellow|0-1-1|
|4|Blue|1-0-0|
|5|Magenta|1-0-1|
|6|Cyan|1-1-0|
|7|White|1-1-1|

## Serial attribute code reference:
([Back to contents](#contents))

The Oric does not have a seperate attribute memory space, changing any attribute is done by plotting an attribute code where normally a character would go, with the effect of that attribute valid for the rest of the line until another attribute code overrides it in the rest of the line to the right.
Also, an attribute code can either change the ink, change the paper, change character set modifiers or change video control attributes, not any combination of these four at the same time. If you want to change two or more of these four categories, you have to plot the same number of attribute code after each other. That is why they are called serial attributes.
This rather complicates screen design in multi color as every color change does cost a spot where no normal character can go.

In Oric Screen Editor, all attributes but the video control attributes are supported. But OSE is not aware of atributes you have placed in the line, so proper attribute placement is something you as user should take care of in the design.

Attributes codes are all plot codes from 0 to 31, codes from 32 to 127 are the printable characters according to standard ASCII codes, codes from 128 and up are the same but in reverse video.

See for full background and reference:
https://osdk.org/index.php?page=articles&ref=ART9

Overview of possible attribute codes:

*Codes 0-7: Change ink*

To change the ink color, the codes are the basic color numbers as mentioned above in the [Color value reference](#color-value-reference), so basically just setting bits 0,1 and 2 for the RGB value, bit 3-7 at zero.

Bitpattern:

|Bit|7-6-5|4|3|2|1|0|
|---|---|---|---|---|---|---|
|Meaning|0|0|0|Blue|Green|Red|


|Code|Hex|0-0-0-P-C-B-G-R|Ink Color|
|---|---|---|---|
|00|00|0-0-0-0-0-0-0-0|Black|
|01|01|0-0-0-0-0-0-0-1|Red|
|02|02|0-0-0-0-0-0-1-0|Green|
|03|03|0-0-0-0-0-0-1-1|Yellow|
|04|04|0-0-0-0-0-1-0-0|Blue|
|05|05|0-0-0-0-0-1-0-1|Magenta|
|06|06|0-0-0-0-0-1-1-0|Cyan|
|07|07|0-0-0-0-0-1-1-1|White|

*Codes 8-15: Character set modifier*

With bit 3 enabled (so add 8) bits 0,1 and 2 are used to modify charset behavior. The different charset behaviors can be set at one time with one code.

Bitpattern:

|Bit|7-6-5|4|3|2|1|0|
|---|---|---|---|---|---|---|
|Meaning|0|0|Charset modifier on|Blink on|Double size on|Alternate on|

|Code|Hex|0-0-0-P-C-B-D-A|Effect on charset|
|---|---|---|---|
|08|08|0-0-0-1-0-0-0-0|Use standard charset|
|09|09|0-0-0-1-0-0-0-1|Use alternate charset|
|10|0A|0-0-0-1-0-0-1-0|Use double size standard charset|
|11|0B|0-0-0-1-0-0-1-1|Use double size alternate charset|
|12|0C|0-0-0-1-0-1-0-0|Use blinking standard charset|
|13|0D|0-0-0-1-0-1-0-1|Use blinking alternate charset|
|14|0E|0-0-0-1-0-1-1-0|Use double size blinking standard charset|
|15|0F|0-0-0-1-0-1-1-1|Use double size blinking alternate charset|

*Codes 16-23: Change paper*

To change the paper color, the codes are the basic color numbers as mentioned above in the [Color value reference](#color-value-reference), so basically just setting bits 0,1 and 2 for the RGB value, together with setting bit 4 (so adding 16). Bit 3, and bits 5,6 and 7 should be 0.

Bitpattern:

|Bit|7-6-5|4|3|2|1|0|
|---|---|---|---|---|---|---|
|Meaning|0|Paper modify on|0|Blue|Green|Red|

|Code|Hex|0-0-0-P-C-B-G-R|Paper Color|
|---|---|---|---|
|16|10|0-0-0-1-0-0-0-0|Black|
|17|11|0-0-0-1-0-0-0-1|Red|
|18|12|0-0-0-1-0-0-1-0|Green|
|19|13|0-0-0-1-0-0-1-1|Yellow|
|20|14|0-0-0-1-0-1-0-0|Blue|
|21|15|0-0-0-1-0-1-0-1|Magenta|
|22|16|0-0-0-1-0-1-1-0|Cyan|
|23|17|0-0-0-1-0-1-1-1|White|

Note that in OSE calculation of these attribute codes by yourselves is not necessary, the program will do so for you given the selected attributes and color. In memory however this is how the codes are stored.

## File format reference
([Back to contents](#contents))

As the Oric does not have seperate attribute memory, screendata is basically just a width*heigth dump of screen codes. The screen file is a flat data file with these screencodes, length is calculated as width*height.
So a standard 40x27 screen would be 1.080 bytes.

## Credits
([Back to contents](#contents))

Oric Screen Editor

Screen editor for the Oric Atmos

Written in 2022 by Xander Mol

Based on VDC Screen Editor for the C128

https://github.com/xahmol/OricScreenEditor

https://www.idreamtin8bits.com/

Code and resources from others used:

-   CC65 cross compiler:

    https://cc65.github.io/

-   6502.org: Practical Memory Move Routines: Starting point for memory move routines

    http://6502.org/source/general/memory_move.html

-   DraBrowse source code for DOS Command and text input routine

    DraBrowse (db*) is a simple file browser.
    Originally created 2009 by Sascha Bader.
    Used version adapted by Dirk Jagdmann (doj)
    https://github.com/doj/dracopy

-   lib-sedoric from oricOpenLibrary (for SEDORIC file operations
    By Raxiss, (c) 2021
    https://github.com/iss000/oricOpenLibrary/blob/main/lib-sedoric/libsedoric.s

-   Bart van Leeuwen and forum.defence-force.org: For inspiration and advice while coding.

-   jab / Artline Designs (Jaakko Luoto) for inspiration for Palette mode and PETSCII visual mode

-   Original windowing system code on Commodore 128 by unknown author.
   
-   Tested using real hardware Oric Atmos plus Cumana Reborn, and Oricutron for Windows and Linux

The code can be used freely as long as you retain
a notice describing original source and author.

THE PROGRAMS ARE DISTRIBUTED IN THE HOPE THAT THEY WILL BE USEFUL,
BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!

([Back to contents](#contents))
