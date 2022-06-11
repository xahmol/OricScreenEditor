/*
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

-   Bart van Leeuwen: For inspiration and advice while coding.

-   jab / Artline Designs (Jaakko Luoto) for inspiration for Palette mode and PETSCII visual mode

-   Original windowing system code on Commodore 128 by unknown author.
   
-   Tested using real hardware plus Oricutron.

The code can be used freely as long as you retain
a notice describing original source and author.

THE PROGRAMS ARE DISTRIBUTED IN THE HOPE THAT THEY WILL BE USEFUL,
BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!
*/

//Includes
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <peekpoke.h>
#include <ctype.h>
#include <atmos.h>
#include "defines.h"
#include "osdklib.h"
#include "defines.h"
#include "oric_core.h"

// Overlay data
unsigned int overlaydata[4];
unsigned char overlay_active = 0;

//Window data
struct WindowStruct Window[9];
unsigned int windowaddress = WINDOWBASEADDRESS;
unsigned char windownumber = 0;

//Menu data
unsigned char menubaroptions = 4;
unsigned char pulldownmenunumber = 8;
char menubartitles[4][12] = {"Screen","File","Charset","Information"};
unsigned char menubarcoords[4] = {2,9,14,22};
unsigned char pulldownmenuoptions[5] = {4,4,4,2,2};
char pulldownmenutitles[5][4][16] = {
    {"Width:      40 ",
     "Height:     27 ",
     "Clear          ",
     "Fill           "},
    {"Save screen    ",
     "Load screen    ",
     "Save project   ",
     "Load project   "},
    {"Load standard  ",
     "Load alternate ",
     "Save standard  ",
     "Save alternate "},
    {"Version/credits",
     "Exit program   "},
    {"Yes",\
     "No "}
};

// Global variables
unsigned char charsetchanged[2];
unsigned char appexit;
char filename[10];
char programmode[11];
unsigned char showbar;

unsigned char screen_col;
unsigned char screen_row;
unsigned int xoffset;
unsigned int yoffset;
unsigned int screenwidth;
unsigned int screenheight;
unsigned int screentotal;
unsigned char plotscreencode;
unsigned char plotink;
unsigned char plotpaper;
unsigned char plotblink;
unsigned char plotdouble;
unsigned char plotaltchar;
unsigned int select_startx, select_starty, select_endx, select_endy, select_width, select_height, select_accept;
unsigned char rowsel = 0;
unsigned char colsel = 0;
unsigned char palettechar;
unsigned char visualmap = 0;
unsigned char favourites[10] = {33,33,33,33,33,33,33,33,33,33};

char buffer[81];
char version[22];

// Visual charset data
unsigned char visualchar[80] =
{
    0x37,0x4B,0x33,0x43,0x3F,0x4F,0x27,0x2B,0x3C,0x4C,0x40,0x30,0x48,0x34,0x24,0x28,
    0x55,0x5A,0x51,0x52,0x5D,0x5E,0x54,0x58,0x2D,0x2E,0x22,0x21,0x2A,0x25,0x53,0x5F,
    0x36,0x49,0x26,0x29,0x56,0x59,0x57,0x5B,0x3D,0x4E,0x32,0x41,0x50,0x5C,0x2C,0x2F,
    0x45,0x3A,0x44,0x38,0x47,0x3B,0x46,0x39,0x3E,0x4D,0x31,0x42,0x23,0x35,0x4A,0x20,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F
};

/* General screen functions */
void cspaces(unsigned char number)
{
    /* Function to print specified number of spaces, cursor set by conio.h functions */

    unsigned char x;

    for(x=0;x<number;x++) { cputc(CH_SPACE); }
}

void printcentered(char* text, unsigned char xpos, unsigned char ypos, unsigned char width)
{
    /* Function to print a text centered
       Input:
       - Text:  Text to be printed
       - Color: Color for text to be printed
       - Width: Width of window to align to    */

    gotoxy(xpos,ypos);
    cspaces(width);
    gotoxy(xpos,ypos);
    if(strlen(text)<width)
    {
        cspaces((width-strlen(text))/2-1);
    }
    cputs(text);
}

void charset_swap(unsigned char sysorred)
{
    /*  Function to swap the system and redefined standard charset
        Inout: sysorred = flag to select system (0) or redefined (1) charset */

    if(!sysorred)
    {
        ORIC_RestoreStandardCharset();
    }
    else
    {
        memcpy((void*)CHARSET_STD,(void*)CHARSET_SWAP,768);
    }
}

unsigned int screenmap_screenaddr(unsigned char row, unsigned char col, unsigned int width)
{
    // Function to calculate screenmap address for the character space
    // Input: row, col, width and height for screenmap

    return SCREENMAPBASE+(row*width)+col;
}


// Generic routines
int textInput(unsigned char xpos, unsigned char ypos, char* str, unsigned char size,unsigned char validation)
{

    /**
    * input/modify a string.
    * based on version DraCopy 1.0e, then modified.
    * Created 2009 by Sascha Bader.
    * @param[in] xpos screen x where input starts.
    * @param[in] ypos screen y where input starts.
    * @param[in,out] str string that is edited, it can have content and must have at least @p size + 1 bytes. Maximum size     if 255 bytes.
    * @param[in] size maximum length of @p str in bytes.
    * @return -1 if input was aborted.
    * @return >= 0 length of edited string @p str.
    */
    // Added input: Validation  =   0   All printable characters allowed
    //                          +   1   Numbers allowed only
    //                          +   2   Also alphabet allowed (lower and uppercase)
    //                          +   4   Also wildcards * and ? allowed
    //                          Add numbers to combine or 0 for no validation

    register unsigned char c;
    register unsigned char idx = strlen(str);
    register unsigned char valid=0;;

    cputsxy(xpos,ypos,str);
    cputc(CH_INVSPACE);
    
    while(1)
    {
        c = cgetc();
        switch (c)
        {
        case CH_ESC:
            return -1;

        case CH_ENTER:
            idx = strlen(str);
            str[idx] = 0;
            gotoxy(xpos,ypos);
            cspaces(size+1);
            cputsxy(xpos, ypos, str);
            return idx;

        case CH_DEL:
            if (idx)
            {
                --idx;
                cputcxy(xpos + idx,ypos,CH_SPACE);
                for(c = idx; 1; ++c)
                {
                    unsigned char b = str[c+1];
                    str[c] = b;
                    cputcxy(xpos+c,ypos,b? b : CH_SPACE);
                    if (b == 0) { break; }
                }
                gotoxy(xpos+idx, ypos);
                cputc(str[idx] ? 128+str[idx] : CH_INVSPACE);
                cputc(str[idx+1] ? str[idx+1] : CH_SPACE);
                gotoxy(xpos+idx, ypos);
            }
            break;

        case CH_CURS_LEFT:
            if (idx)
            {
                --idx;
                gotoxy(xpos+idx, ypos);
                cputc(str[idx] ? 128+str[idx] : CH_INVSPACE);
                cputc(str[idx+1] ? str[idx+1] : CH_SPACE);
                gotoxy(xpos+idx, ypos);

            }
            break;

        case CH_CURS_RIGHT:
            if (idx < strlen(str) && idx < size)
            {
                ++idx;
                gotoxy(xpos+idx-1, ypos);
                cputc(str[idx-1]);
                cputc(str[idx] ? 128+str[idx] : CH_INVSPACE);
                gotoxy(xpos + idx, ypos);
            }
            break;

        default:
            valid=0;
            if(!validation) { valid = 1; }
            if((validation&1) && c>47 && c<58) { valid = 1; }
            if((validation&2) && c>64 && c<91) { valid = 1; }
            if((validation&2) && c>96 && c<123) { valid = 1; }
            if((validation&4) && (c==42 || c==63)) { valid = 1; }
            if (isprint(c) && idx < size && valid)
            {
                unsigned char flag = (str[idx] == 0);
                str[idx] = c;
                cputcxy(xpos+idx++,ypos,c);
                cputc(CH_INVSPACE);
                if (flag) { str[idx+1] = 0; }
                break;
            }
            break;
        }
    }
    return 0;
}

// Status bar functions

void printstatusbar()
{
    if(screen_row==26) { return; }

    sprintf(buffer,"%-10s",programmode);
    cputsxy(2,26,buffer);
    if(screenwidth>99 || screenheight>99)
    {
        sprintf(buffer,"%2x,%2x",screen_col+xoffset,screen_row+yoffset);
    }
    else
    {
        sprintf(buffer,"%2u,%2u",screen_col+xoffset,screen_row+yoffset);
    }
    cputsxy(14,26,buffer);
    sprintf(buffer,"%2x",plotscreencode);
    cputsxy(21,26,buffer);
    cputcxy(23,26,plotscreencode);
    sprintf(buffer,"%2x",PEEK(screenmap_screenaddr(screen_row+yoffset,screen_col+xoffset,screenwidth)));
    cputsxy(26,26,buffer);
    sprintf(buffer,"%1u",plotink);
    cputsxy(30,26,buffer);
    cputc(16+plotink);
    cputc(A_BGWHITE);
    sprintf(buffer,"%1u",plotpaper);
    cputsxy(34,26,buffer);
    cputc(16+plotpaper);
    cputc(A_BGWHITE);
    if(plotaltchar)
    {
        cputsxy(37,26,"A");
    }
    else
    {
        cputsxy(37,26,"S");
    }
    if(plotdouble)
    {
        cputsxy(38,26,"D");
    }
    else
    {
        cputsxy(38,26," ");
    }
    if(plotblink)
    {
        cputsxy(39,26,"B");
    }
    else
    {
        cputsxy(39,26," ");
    }
}

void initstatusbar()
{
    if(screen_row==26) { return; }

    gotoxy(0,26);
    cputc(A_FWBLACK);
    cputc(A_BGWHITE);
    ORIC_FillArea(26,2,CH_SPACE,38,1);
    cputsxy(12,26,"XY");
    cputsxy(20,26,"C");
    cputsxy(25,26,"S");
    cputsxy(29,26,"I");
    cputsxy(33,26,"P");
    printstatusbar();
}

void hidestatusbar()
{
    ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset+26,0,26,40,1);
}

void togglestatusbar()
{
    if(screen_row==26) { return; }

    if(showbar)
    {
        showbar=0;
        hidestatusbar();
    }
    else
    {
        showbar=1;
        initstatusbar();
    }
}

// Functions for windowing and menu system

void windowsave(unsigned char ypos, unsigned char height, unsigned char loadsyscharset)
{
    /* Function to save a window
       Input:
       - ypos: startline of window
       - height: height of window    
       - loadsyscharset: load syscharset if userdefined charset is loaded enabled (1) or not (0) */

    unsigned int baseaddress = SCREENMEMORY + (ypos*40);
    int length =height*40;

    Window[windownumber].address = windowaddress;
    Window[windownumber].ypos = ypos;
    Window[windownumber].height = height;

    // Copy screen
    memcpy((int*)windowaddress,(int*)baseaddress,length);
    windowaddress += length;

    windownumber++;

    // Load system charset if needed
    if(loadsyscharset == 1 && charsetchanged[0] == 1)
    {
        charset_swap(0);
    }
}

void windowrestore(unsigned char restorealtcharset)
{
    /* Function to restore a window
       Input: restorealtcharset: request to restore user defined charset if needed enabled (1) or not (0) */

    unsigned int baseaddress = SCREENMEMORY + (Window[--windownumber].ypos*40);
    int length = Window[windownumber].height*40;

    windowaddress = Window[windownumber].address;

    // Restore screem
    memcpy((int*)baseaddress,(int*)windowaddress,length); 

    // Restore custom charset if needed
    if(restorealtcharset == 1 && charsetchanged[0] == 1)
    {
        charset_swap(1);
    }
}

void windownew(unsigned char xpos, unsigned char ypos, unsigned char height, unsigned char width, unsigned char loadsyscharset)
{
    /* Function to make menu border
       Input:
       - xpos: x-coordinate of left upper corner
       - ypos: y-coordinate of right upper corner
       - height: number of rows in window
       - width: window width in characters
        - loadsyscharset: load syscharset if userdefined charset is loaded enabled (1) or not (0) */
 
    windowsave(ypos, height,loadsyscharset);

    ORIC_FillArea(ypos,xpos,CH_SPACE,width,height);
    ORIC_VChar(ypos,xpos-1,A_STD,height);
    ORIC_VChar(ypos,xpos,A_BGWHITE,height);
    ORIC_VChar(ypos,xpos+1,A_FWBLACK,height);
}

void menuplacebar()
{
    /* Function to print menu bar */

    unsigned char x;

    gotoxy(0,0);
    cputc(A_FWBLACK);
    cputc(A_BGGREEN);
    ORIC_FillArea(0,2,CH_SPACE,38,1);
    for(x=0;x<menubaroptions;x++)
    { 
        cputsxy(menubarcoords[x],0,menubartitles[x]);
    }
}

unsigned char menupulldown(unsigned char xpos, unsigned char ypos, unsigned char menunumber, unsigned char escapable)
{
    /* Function for pull down menu
       Input:
       - xpos = x-coordinate of upper left corner
       - ypos = y-coordinate of upper left corner
       - menunumber = 
         number of the menu as defined in pulldownmenuoptions array 
       - espacable: ability to escape with escape key enabled (1) or not (0)  */

    unsigned char x;
    unsigned char key;
    unsigned char exit = 0;
    unsigned char menuchoice = 1;
    unsigned char endcolor = (menunumber>menubaroptions)?A_BGWHITE:A_BGBLACK;

    windowsave(ypos, pulldownmenuoptions[menunumber-1],0);
    for(x=0;x<pulldownmenuoptions[menunumber-1];x++)
    {
        gotoxy(xpos-1,ypos+x);
        cputc(A_STD);
        cputc(A_BGCYAN);
        cputc(A_FWBLACK);
        cprintf(" %s",pulldownmenutitles[menunumber-1][x]);
        cputc(endcolor);
    }
  
    do
    {
        gotoxy(xpos-1,ypos+menuchoice-1);
        cputc(A_STD);
        cputc(A_BGYELLOW);
        cputc(A_FWBLACK);
        cprintf("-%s",pulldownmenutitles[menunumber-1][menuchoice-1]);
        cputc(endcolor);
        
        do
        {
            key = cgetc();
        } while (key != CH_ENTER && key != CH_CURS_LEFT && key != CH_CURS_RIGHT && key != CH_CURS_UP && key != CH_CURS_DOWN && key != CH_ESC && key != CH_STOP );

        switch (key)
        {
        case CH_ESC:
        case CH_STOP:
            if(escapable == 1) { exit = 1; menuchoice = 0; }
            break;

        case CH_ENTER:
            exit = 1;
            break;
        
        case CH_CURS_LEFT:
            exit = 1;
            menuchoice = 18;
            break;
        
        case CH_CURS_RIGHT:
            exit = 1;
            menuchoice = 19;
            break;

        case CH_CURS_DOWN:
        case CH_CURS_UP:
            gotoxy(xpos-1,ypos+menuchoice-1);
            cputc(A_STD);
            cputc(A_BGCYAN);
            cputc(A_FWBLACK);
            cprintf(" %s",pulldownmenutitles[menunumber-1][menuchoice-1]);
            cputc(endcolor);
            if(key==CH_CURS_UP)
            {
                menuchoice--;
                if(menuchoice<1)
                {
                    menuchoice=pulldownmenuoptions[menunumber-1];
                }
            }
            else
            {
                menuchoice++;
                if(menuchoice>pulldownmenuoptions[menunumber-1])
                {
                    menuchoice = 1;
                }
            }
            break;

        default:
            break;
        }
    } while (exit==0);
    windowrestore(0);    
    return menuchoice;
}

unsigned char menumain()
{
    /* Function for main menu selection */

    unsigned char menubarchoice = 1;
    unsigned char menuoptionchoice = 0;
    unsigned char key;
    unsigned char xpos;

    menuplacebar();

    do
    {
        do
        {
            gotoxy(menubarcoords[menubarchoice-1]-1,0);
            cputc(A_BGWHITE);
            cprintf("%s",menubartitles[menubarchoice-1]);
            cputc(A_BGGREEN);

            do
            {
                key = cgetc();
            } while (key != CH_ENTER && key != CH_CURS_LEFT && key != CH_CURS_RIGHT && key != CH_ESC && key != CH_STOP);

            gotoxy(menubarcoords[menubarchoice-1]-1,0);
            cputc(A_BGGREEN);
            cprintf("%s",menubartitles[menubarchoice-1]);

            if(key==CH_CURS_LEFT)
            {
                menubarchoice--;
                if(menubarchoice<1)
                {
                    menubarchoice = menubaroptions;
                }
            }
            else if (key==CH_CURS_RIGHT)
            {
                menubarchoice++;
                if(menubarchoice>menubaroptions)
                {
                    menubarchoice = 1;
                }
            }
        } while (key!=CH_ENTER && key != CH_ESC && key != CH_STOP);
        if (key != CH_ESC && key != CH_STOP)
            {
            xpos=menubarcoords[menubarchoice-1]-1;
            if(xpos+strlen(pulldownmenutitles[menubarchoice-1][0])>38)
            {
                xpos=menubarcoords[menubarchoice-1]+strlen(menubartitles[menubarchoice-1])-strlen(pulldownmenutitles  [menubarchoice-1][0]);
            }
            menuoptionchoice = menupulldown(xpos,1,menubarchoice,1);
            if(menuoptionchoice==18)
            {
                menuoptionchoice=0;
                menubarchoice--;
                if(menubarchoice<1)
                {
                    menubarchoice = menubaroptions;
                }
            }
            if(menuoptionchoice==19)
            {
                menuoptionchoice=0;
                menubarchoice++;
                if(menubarchoice>menubaroptions)
                {
                    menubarchoice = 1;
                }
            }
        }
        else
        {
            menuoptionchoice = 99;
        }
    } while (menuoptionchoice==0);

    return menubarchoice*10+menuoptionchoice;    
}

unsigned char areyousure(char* message, unsigned char syscharset)
{
    /* Pull down menu to verify if player is sure */
    unsigned char choice;

    windownew(5,8,6,35,syscharset);
    cputsxy(7,9,message);
    cputsxy(7,10,"Are you sure?");
    choice = menupulldown(20,11,5,0);
    windowrestore(syscharset);
    return choice;
}

void fileerrormessage(unsigned char error, unsigned char syscharset)
{
    /* Show message for file error encountered */

    windownew(5,8,6,35,syscharset);
    cputsxy(7,9,"File error!");
    if(error<255)
    {
        sprintf(buffer,"Error nr.: %2X",error);
        cputsxy(7,11,buffer);
    }
    cputsxy(7,13,"Press key.");
    cgetc();
    windowrestore(syscharset);   
}

void messagepopup(char* message, unsigned char syscharset)
{
    // Show popup with a message

    windownew(5,8,6,35,syscharset);
    cputsxy(7,9,message);
    cputsxy(7,11,"Press key.");
    cgetc();
    windowrestore(syscharset); 
}

// Generic screen map routines
void screenmapplot(unsigned char row, unsigned char col, unsigned char screencode)
{
    // Function to plot a screencode
	// Input: row and column, screencode to plot

    POKE(screenmap_screenaddr(row,col,screenwidth),screencode);
}

void cursormove(unsigned char left, unsigned char right, unsigned char up, unsigned char down)
{
    // Move cursor and scroll screen if needed
    // Input: flags to enable (1) or disable (0) move in the different directions

    if(left == 1 )
    {
        if(screen_col==0)
        {
            if(xoffset>0)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,screenwidth,xoffset--,yoffset,0,0,40,27,2);
                initstatusbar();
            }
        }
        else
        {
            screen_col--;
        }
    }
    if(right == 1 )
    {
        if(screen_col==39)
        {
            if(xoffset+screen_col<screenwidth-1)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,screenwidth,xoffset++,yoffset,0,0,40,27,1);
                initstatusbar();
            }
        }
        else
        {
            screen_col++;
        }
    }
    if(up == 1 )
    {
        if(screen_row==0)
        {
            if(yoffset>0)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,screenwidth,xoffset,yoffset--,0,0,40,27,4);
                initstatusbar();
            }
        }
        else
        {
            screen_row--;
            if(showbar && screen_row==25) { initstatusbar(); }
        }
    }
    if(down == 1 )
    {
        if(screen_row==25) { hidestatusbar(); }
        if(screen_row==26)
        {
            if(yoffset+screen_row<screenheight-1)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,screenwidth,xoffset,yoffset++,0,0,40,27,8);
                initstatusbar();
            }
        }
        else
        {
            screen_row++;
        }
    }
}

// Help screens
void helpscreen_load(unsigned char screennumber)
{
    // Function to show selected help screen
    // Input: screennumber: 1-Main mode, 2-Character editor, 3-SelectMoveLinebox, 4-Write/colorwrite mode

    int rc;
    int len = 0;

    // Load system charset if needed
    if(charsetchanged[0] == 1)
    {
        charset_swap(0);
    }

    // Load selected help screen
    sprintf(buffer,"osehs%u.bin",screennumber);
    rc = loadfile(buffer,(void*)SCREENMEMORY,&len);

    if(!len)
    {
        messagepopup("Insert application disk.",0);
    }
    
    cgetc();

    // Restore screen
    ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
    if(showbar) { initstatusbar(); }
    if(screennumber!=2)
    {
        cputcxy(screen_col,screen_row,plotscreencode);
    }

    // Restore custom charset if needed
    if(charsetchanged[0] == 1)
    {
        charset_swap(1);
    }
}

// Application routines
void plotmove(unsigned char direction)
{
    // Drive cursor move
    // Input: ASCII code of cursor key pressed

    cputcxy(screen_col,screen_row,PEEK(screenmap_screenaddr(yoffset+screen_row,xoffset+screen_col,screenwidth)));

    switch (direction)
    {
    case CH_CURS_LEFT:
        cursormove(1,0,0,0);
        break;
    
    case CH_CURS_RIGHT:
        cursormove(0,1,0,0);
        break;

    case CH_CURS_UP:
        cursormove(0,0,1,0);
        break;

    case CH_CURS_DOWN:
        cursormove(0,0,0,1);
        break;
    
    default:
        break;
    }

    cputcxy(screen_col,screen_row,plotscreencode+128);
}

void plot_try()
{
    unsigned char key;

    strcpy(programmode,"Try");
    if(showbar) { printstatusbar(); }
    cputcxy(screen_col,screen_row,plotscreencode);
    key = cgetc();
    if(key==CH_SPACE)
    {
        screenmapplot(screen_row+yoffset,screen_col+xoffset,plotscreencode);
    }
    strcpy(programmode,"Main");
}

void writemode()
{
    // Write mode with screencodes

    unsigned char key, screencode;
    unsigned char rvs = 0;

    strcpy(programmode,"Write");

    do
    {
        if(showbar) { printstatusbar(); }
        key = cgetc();

        switch (key)
        {
        // Cursor move
        case CH_CURS_LEFT:
        case CH_CURS_RIGHT:
        case CH_CURS_UP:
        case CH_CURS_DOWN:
            plotmove(key);
            break;

        // Toggle blink
        case CH_CTRL_B:
            plotblink = (plotblink==0)? 1:0;
            break;

        // Toggle alternate character set
        case CH_CTRL_A:
            plotaltchar = (plotaltchar==0)? 1:0;
            break;

        // Toggle double
        case CH_CTRL_D:
            plotdouble = (plotdouble==0)? 1:0;
            break;

        // Decrease ink
        case CH_CTRL_Z:
            if(plotink==0) { plotink = 7; } else { plotink--; }
            break;

        // Increase ink
        case CH_CTRL_X:
            if(plotink==7) { plotink = 0; } else { plotink++; }
            break;

        // Decrease paper
        case CH_CTRL_C:
            if(plotpaper==0) { plotpaper = 7; } else { plotpaper--; }
            break;

        // Increase paper
        case CH_CTRL_V:
            if(plotpaper==7) { plotpaper = 0; } else { plotpaper++; }
            break;

        // Plot present ink
        case CH_F1:
            screenmapplot(screen_row+yoffset,screen_col+xoffset,plotink);
            plotmove(CH_CURS_RIGHT);
            break;

        // Plot present paper
        case CH_F2:
            screenmapplot(screen_row+yoffset,screen_col+xoffset,16+plotpaper);
            plotmove(CH_CURS_RIGHT);
            break;

        // Plot present character modifier
        case CH_F3:
            screenmapplot(screen_row+yoffset,screen_col+xoffset,ORIC_CharAttribute(plotaltchar,plotdouble,plotblink));
            plotmove(CH_CURS_RIGHT);
            break;

        // Delete present screencode and attributes
        case CH_DEL:
            screenmapplot(screen_row+yoffset,screen_col+xoffset,CH_SPACE);
            cputcxy(screen_row,screen_col,CH_SPACE);
            break;

        // Toggle statusbar
        case CH_F6:
            togglestatusbar();
            break;

        case CH_F8:
            helpscreen_load(4);
            break;

        // Toggle RVS with the RVS ON and RVS OFF keys (control 9 and control 0)
        case CH_CTRL_R:
            rvs = (rvs==1)?0:1;
            break;

        // Write printable character                
        default:
            if(isprint(key))
            {
                screencode = key+(rvs*128);
                screenmapplot(screen_row+yoffset,screen_col+xoffset,screencode);
                plotmove(CH_CURS_RIGHT);
            }
            break;
        }
    } while (key != CH_ESC && key != CH_STOP);
    strcpy(programmode,"Main");
}

void plotvisible(unsigned char row, unsigned char col, unsigned char setorrestore)
{
    // Plot or erase part of line or box if in visible viewport
    // Input: row, column, and flag setorrestore to plot new value (1) or restore old value (0)


    if(row>=yoffset && row<=yoffset+26 && col>=xoffset && col<=xoffset+39)
    {
        if(setorrestore==1)
        {
            cputcxy(col-xoffset,row-yoffset,plotscreencode+128);
        }
        else
        {
            cputcxy(col-xoffset,row-yoffset, PEEK(screenmap_screenaddr(row,col,screenwidth)));
        }
    }
}

void lineandbox(unsigned char draworselect)
{
    // Select line or box from upper left corner using cursor keys, ESC for cancel and ENTER for accept
    // Input: draworselect: Choose select mode (0) or draw mode (1)

    unsigned char key;
    unsigned char x,y;

    select_startx = screen_col + xoffset;
    select_starty = screen_row + yoffset;
    select_endx = select_startx;
    select_endy = select_starty;
    select_accept = 0;

    if(draworselect)
    {
        strcpy(programmode,"Line/Box");
    }
    cputcxy(screen_col,screen_row,plotscreencode+128);

    do
    {
        if(showbar) { printstatusbar(); }
        key = cgetc();

        switch (key)
        {
        case CH_CURS_RIGHT:
            cursormove(0,1,0,0);
            select_endx = screen_col + xoffset;
            for(y=select_starty;y<select_endy+1;y++)
            {
                plotvisible(y,select_endx,1);
            }
            break;

        case CH_CURS_LEFT:
            if(select_endx>select_startx)
            {
                cursormove(1,0,0,0);
                for(y=select_starty;y<select_endy+1;y++)
                {
                    plotvisible(y,select_endx,0);
                }
                select_endx = screen_col + xoffset;
            }
            break;

        case CH_CURS_UP:
            if(select_endy>select_starty)
            {
                cursormove(0,0,1,0);
                for(x=select_startx;x<select_endx+1;x++)
                {
                    plotvisible(select_endy,x,0);
                }
                select_endy = screen_row + yoffset;
            }
            break;

        case CH_CURS_DOWN:
            cursormove(0,0,0,1);
            select_endy = screen_row + yoffset;
            for(x=select_startx;x<select_endx+1;x++)
            {
                plotvisible(select_endy,x,1);
            }
            break;

        // Toggle statusbar
        case CH_F6:
            togglestatusbar();
            break;
        
        case CH_F8:
            if(select_startx==select_endx && select_starty==select_endy) helpscreen_load(3);
            break;
        
        default:
            break;
        }
    } while (key!=CH_ESC && key != CH_STOP && key != CH_ENTER);

    if(key==CH_ENTER)
    {
        select_width = select_endx-select_startx+1;
        select_height = select_endy-select_starty+1;
    }  

    if(key==CH_ENTER && draworselect ==1)
    {
        for(y=select_starty;y<select_endy+1;y++)
        {
            memset((void*)screenmap_screenaddr(y,select_startx,screenwidth),plotscreencode,select_width);
        }
        ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
        if(showbar) { initstatusbar(); }
        cputcxy(screen_col,screen_row,plotscreencode+128);
    }
    else
    {
        ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
        if(showbar) { initstatusbar(); }
        if(key==CH_ENTER) { select_accept=1; }
        cputcxy(screen_col,screen_row,plotscreencode+128);
    }
    if(draworselect ||key!=CH_ESC || key != CH_STOP )
    {
        strcpy(programmode,"Main");
    }
}

void movemode()
{
    // Function to move the 80x25 viewport

    unsigned char key,y;
    unsigned char moved = 0;

    strcpy(programmode,"Move");

    cputcxy(screen_col,screen_row,PEEK(screenmap_screenaddr(yoffset+screen_row,xoffset+screen_col,screenwidth)));
    
    if(showbar) { hidestatusbar(); }

    do
    {
        key = cgetc();

        switch (key)
        {
        case CH_CURS_RIGHT:
            ORIC_ScrollMove(0,0,40,27,2,1);
            ORIC_VChar(0,0,CH_SPACE,27);
            moved=1;
            break;
        
        case CH_CURS_LEFT:
            ORIC_ScrollMove(0,0,40,27,1,1);
            ORIC_VChar(0,39,CH_SPACE,27);
            moved=1;
            break;

        case CH_CURS_UP:
            ORIC_ScrollMove(0,0,40,27,8,1);
            ORIC_HChar(24,0,CH_SPACE,40);
            moved=1;
            break;
        
        case CH_CURS_DOWN:
            ORIC_ScrollMove(0,0,40,27,4,1);
            ORIC_HChar(0,0,CH_SPACE,40);
            moved=1;
            break;

        case CH_F8:
            helpscreen_load(3);
            break;
        
        default:
            break;
        }
    } while (key != CH_ENTER && key != CH_ESC && key != CH_STOP);

    if(moved==1)
    {
        if(key==CH_ENTER)
        {
            for(y=0;y<25;y++)
            {
                memcpy((void*)screenmap_screenaddr(y+yoffset,xoffset,screenwidth),(void*)(SCREENMEMORY+(y*40)),40);
            }
        }
        ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
        if(showbar) { initstatusbar(); }
    }

    cputcxy(screen_col,screen_row,plotscreencode+128);
    strcpy(programmode,"Main");
    if(showbar) { printstatusbar(); }
}

void selectmode()
{
    // Function to select a screen area to delete, cut, copy or paint

    unsigned char key,movekey,y,ycount;

    strcpy(programmode,"Select");

    movekey = 0;
    lineandbox(0);
    if(select_accept == 0) { return; }

    strcpy(programmode,"x/c/d/ipm?");

    do
    {
        if(showbar) { printstatusbar(); }
        key=cgetc();

        // Toggle statusbar
        if(key==CH_F6)
        {
            togglestatusbar();
        }

        if(key==CH_F8) { helpscreen_load(3); }

    } while (key !='d' && key !='x' && key !='c' && key != 'p' && key !='i' && key !='m' && key != CH_ESC && key != CH_STOP );

    if(key!=CH_ESC && key != CH_STOP)
    {
        if((key=='x' || key=='c')&&(select_width>1024))
        {
            messagepopup("Selection too big.",1);
            return;
        }

        if(key=='x' || key=='c')
        {
            if(key=='x')
            {
                strcpy(programmode,"Cut");
            }
            else
            {
                strcpy(programmode,"Copy");
            }
            do
            {
                if(showbar) { printstatusbar(); }
                movekey = cgetc();

                switch (movekey)
                {
                // Cursor move
                case CH_CURS_LEFT:
                case CH_CURS_RIGHT:
                case CH_CURS_UP:
                case CH_CURS_DOWN:
                    plotmove(movekey);
                    break;

                case CH_F8:
                    helpscreen_load(3);
                    break;

                default:
                    break;
                }
            } while (movekey != CH_ESC && movekey != CH_STOP && movekey != CH_ENTER);

            if(movekey==CH_ENTER)
            {
                if((screen_col+xoffset+select_width>screenwidth) || (screen_row+yoffset+select_height>screenheight))
                {
                    messagepopup("Selection does not fit.",1);
                    return;
                }

                for(ycount=0;ycount<select_height;ycount++)
                {
                    y=(screen_row+yoffset>=select_starty)? select_height-ycount-1 : ycount;
                    memcpy((void*)SCREENMEMORY,(void*)screenmap_screenaddr(select_starty+y,select_startx,screenwidth),select_width);
                    if(key=='x') {memset((void*)screenmap_screenaddr(select_starty+y,select_startx,screenwidth),CH_SPACE,select_width); }
                    memcpy((void*)screenmap_screenaddr(screen_row+yoffset+y,screen_col+xoffset,screenwidth),(void*)SCREENMEMORY,select_width);
                }
            }
        }

        if( key=='d')
        {
            for(y=0;y<select_height;y++)
            {
                memset((void*)screenmap_screenaddr(select_starty+y,select_startx,screenwidth),CH_SPACE,select_width);
            }
        }

        if(key=='i')
        {
            for(y=0;y<select_height;y++)
            {
                memset((void*)screenmap_screenaddr(select_starty+y,select_startx,screenwidth),plotink,select_width);
            }
        }

        if(key=='p')
        {
            for(y=0;y<select_height;y++)
            {
                memset((void*)screenmap_screenaddr(select_starty+y,select_startx,screenwidth),16+plotpaper,select_width);
            }
        }

        if(key=='m')
        {
            for(y=0;y<select_height;y++)
            {
                memset((void*)screenmap_screenaddr(select_starty+y,select_startx,screenwidth),ORIC_CharAttribute(plotaltchar,plotdouble,plotblink),select_width);
            }
        }

        ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
        if(showbar) { initstatusbar(); }
        cputcxy(screen_col,screen_row,plotscreencode+128);
    }
    strcpy(programmode,"Main");
}

void palette_draw()
{
    // Draw window for character palette
    unsigned char x,y;
    unsigned char counter = 0;

    windowsave(0,25,0);
    ORIC_FillArea(0,3,CH_SPACE,37,25);
    ORIC_VChar(0,0,A_STD,25);
    cputcxy(7,2,A_ALT);
    ORIC_VChar(15,7,A_ALT,10);
    ORIC_VChar(0,1,A_BGWHITE,25);
    ORIC_VChar(0,2,A_FWBLACK,25);
    cputsxy(3,1,"Fav:");
    cputsxy(3,3,"Std:");
    cputsxy(3,15,"Alt:");

    // Set coordinate of present char if no visual map
    rowsel = palettechar/16+1;
    colsel = palettechar%16;

    // Favourites palette
    for(x=0;x<10;x++)
    {
        cputcxy(8+x*2,1,favourites[x]);
        cputcxy(8+x*2,2,favourites[x]);
    }

    // Full charsets
    for(y=0;y<6;y++)
    {
        for(x=0;x<16;x++)
        {
            cputcxy(8+x*2,3+y*2,counter+32);
            if(y<5)
            {
                cputcxy(8+x*2,15+y*2,(visualmap)?visualchar[counter]:counter+32);
            }
            counter++;
        }
    }
}

unsigned char palette_returnscreencode()
{
    // Get screencode from palette position
    unsigned char palpos;

    if(rowsel==0)
    {
        palpos = favourites[colsel];
    }
    if(rowsel>0 && rowsel<7)
    {
        palpos = 32 + colsel + (rowsel-1)*16;
    }
    if(rowsel>6)
    {
        if(visualmap)
        {
            palpos = visualchar[colsel+(rowsel-7)*16];
        }
        else
        {
            palpos = 32 + colsel + (rowsel-7)*16;
        }
    }

    return palpos;
}

void palette()
{
    // Show character set palette

    unsigned char key;

    palettechar = plotscreencode-32;

    strcpy(programmode,"Palette");

    palette_draw();

    // Get key loop
    do
    {
        cputcxy(8+colsel*2,1+rowsel*2,palette_returnscreencode()+128);
        if(showbar) { printstatusbar(); }
        key=cgetc();

        switch (key)
        {
        // Cursor movemeny
        case CH_CURS_RIGHT:       
        case CH_CURS_LEFT:
        case CH_CURS_DOWN:
        case CH_CURS_UP:
            cputcxy(8+colsel*2,1+rowsel*2,palette_returnscreencode());
            if(key==CH_CURS_RIGHT) { colsel++; }
            if(key==CH_CURS_LEFT)
            {
                if(colsel>0)
                {
                    colsel--;
                }
                else
                {
                    colsel=15;
                    if(rowsel>0)
                    {
                        rowsel--;
                        if(rowsel==0) { colsel=9; }
                    }
                    else
                    {
                        rowsel=11;
                    }
                }
            }
            if(key==CH_CURS_DOWN) { rowsel++; }
            if(key==CH_CURS_UP)
            {
                if(rowsel>0)
                {
                    rowsel--;
                }
                else
                {
                    rowsel=11;
                }
            }
            if(colsel>9 && rowsel==0) { colsel=0; rowsel=1; }
            if(colsel>15) { colsel=0; rowsel++; }
            if(rowsel>11) { rowsel=0; }
            break;

        // Select character
        case CH_SPACE:
        case CH_ENTER:
            plotscreencode = palette_returnscreencode();;
            key = CH_ESC;
            break;

        // Toggle visual charset map
        case 'v':
            windowrestore(0);
            visualmap = (visualmap)?0:1;
            palette_draw();
            break;

        // Toggle statusbar
        case CH_F6:
            togglestatusbar();
            break;

        // Help screen
        case CH_F8:
            windowrestore(0);
            helpscreen_load(2);
            palette_draw();
            break;
        
        default:
            // Store in favourites
            if(key>47 && key<58 && rowsel>0)
            {
                palettechar=palette_returnscreencode()-32;
                favourites[key-48] = palettechar+32;
                cputcxy(8+(key-48)*2,1,favourites[key-48]);
                cputcxy(8+(key-48)*2,2,favourites[key-48]);
            }
            break;
        }
    } while (key != CH_ESC && key != CH_STOP);

    windowrestore(0);
    cputcxy(screen_col,screen_row,plotscreencode+128);
    strcpy(programmode,"Main");
}

void showchareditfield()
{
    // Function to draw char editor background field
    // Input: Flag for which charset is edited, standard (0) or alternate (1)

    windowsave(0,12,0);
    ORIC_FillArea(0,30,CH_SPACE,10,12);
    ORIC_VChar(0,27,A_STD,12);
    ORIC_VChar(0,28,A_BGWHITE,12);
    ORIC_VChar(0,29,A_FWBLACK,12);
}

unsigned int charaddress(unsigned char screencode, unsigned char stdoralt, unsigned char swap)
{
    // Function to calculate address of character to edit
    // Input:   screencode to edit, flag for standard (0) or alternate (1) charset,
    //          flag for swap memory (1) or normal memort for standard charset

    unsigned int address;

    if(swap && !stdoralt)
    {
        address = CHARSET_SWAP;
    }
    else
    {
        address = (stdoralt==0)? CHARSET_STD:CHARSET_ALT;
    }
    address += (screencode-32)*8;
    return address;
}

void showchareditgrid(unsigned int screencode, unsigned char stdoralt)
{
    // Function to draw grid with present char to edit

    unsigned char x,y,char_byte;
    unsigned int address;

    address = charaddress(screencode,stdoralt,0);

    gotoxy(30,1);
    cprintf("Ch %2X %s",screencode,(stdoralt==0)? "Std":"Alt");

    for(y=0;y<8;y++)
    {
        char_byte = PEEK(address+y);
        gotoxy(30,y+3);
        cprintf("%2X",char_byte);
        for(x=0;x<6;x++)
        {
            if(char_byte & (1<<(5-x)))
            {
                cputcxy(x+33,y+3,CH_INVSPACE);
            }
            else
            {
                cputcxy(x+33,y+3,CH_SPACE);
            }
        }
    }
}

void chareditor()
{
    unsigned char x,y,char_altorstd,char_screencode,key;
    unsigned char xpos=0;
    unsigned char ypos=0;
    unsigned char char_present[8];
    unsigned char char_copy[8];
    unsigned char char_undo[8];
    unsigned char char_buffer[8];
    unsigned int char_address;
    unsigned char charchanged = 0;
    unsigned char bitset;
    char* ptrend;

    char_altorstd = plotaltchar;
    char_screencode = plotscreencode;
    char_address = charaddress(char_screencode, char_altorstd,0);
    charsetchanged[plotaltchar]=1;
    strcpy(programmode,"Charedit");

    for(y=0;y<8;y++)
    {
        char_present[y]=PEEK(char_address+y);
        char_undo[y]=char_present[y];
    }

    showchareditfield();
    showchareditgrid(char_screencode, char_altorstd);
    do
    {
        bitset = (char_present[ypos] & (1<<(5-xpos)))?1:0;
        cputcxy(xpos+33,ypos+3,'*'+bitset*128);
        if(showbar) { printstatusbar(); }
        key = cgetc();
        cputcxy(xpos+33,ypos+3,CH_SPACE+bitset*128);

        switch (key)
        {
        // Movement
        case CH_CURS_RIGHT:
            if(xpos<5) {xpos++; }
            break;
        
        case CH_CURS_LEFT:
            if(xpos>0) {xpos--; }
            break;
        
        case CH_CURS_DOWN:
            if(ypos<7) {ypos++; }
            break;

        case CH_CURS_UP:
            if(ypos>0) {ypos--; }
            break;

        // Next or previous character
        case '+':
        case '-':
        case '=':
            if(key=='+' || key=='=')
            {
                char_screencode++;
                if(char_screencode>127 || (char_altorstd && char_screencode>11)) { char_screencode=32; }
            }
            else
            {
                if(char_screencode>32)
                {
                    char_screencode--;
                }
                else
                {
                    char_screencode = (char_altorstd)?111:127;
                }
                
            }
            charchanged=1;
            break;

        // Toggle bit
        case CH_SPACE:
            char_present[ypos] ^= 1 << (5-xpos);
            POKE(char_address+ypos,char_present[ypos]);
            POKE(charaddress(char_screencode,char_altorstd,1)+ypos,char_present[ypos]);
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Inverse
        case 'i':
            for(y=0;y<8;y++)
            {
                char_present[y] ^= 0x3F;
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Delete
        case CH_DEL:
            for(y=0;y<8;y++)
            {
                char_present[y] = 0;
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Undo
        case 'z':
            for(y=0;y<8;y++)
            {
                char_present[y] = char_undo[y];
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Restore from system font
        case 's':
            if(!char_altorstd)
            {
                for(y=0;y<8;y++)
                {
                    char_present[y] = PEEK(CHARSETROM+y+(char_screencode-32)*8);
                    POKE(char_address+y,char_present[y]);
                    POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
                }
                showchareditgrid(char_screencode, char_altorstd);
            }
            break;

        // Copy
        case 'c':
            for(y=0;y<8;y++)
            {
                char_copy[y] = char_present[y];
            }
            break;

        // Paste
        case 'v':
            for(y=0;y<8;y++)
            {
                char_present[y] = char_copy[y];
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Switch charset
        case 'a':
            char_altorstd = (char_altorstd==0)? 1:0;
            if(char_altorstd && char_screencode>111) { char_screencode=32; }
            charchanged=1;
            break;

        // Mirror y axis
        case 'y':
            for(y=0;y<8;y++)
            {
                POKE(char_address+y,char_present[7-y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[7-y]);
            }
            for(y=0;y<8;y++)
            {
                char_present[y]=PEEK(char_address+y);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Mirror x axis
        case 'x':
            for(y=0;y<8;y++)
            {
                for(x=0;x<6;x++)
                {
                    if(char_present[y] & (1<<(5-x)))
                    {
                        char_buffer[y] |= (1<<x);
                    }
                    else
                    {
                        char_buffer[y] &= ~(1<<x);
                    }
                }
            }
            for(y=0;y<8;y++)
            {
                char_present[y]=char_buffer[y];
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Scroll up
        case 'u':
            for(y=1;y<8;y++)
            {
                char_buffer[y-1]=char_present[y];
            }
            char_buffer[7]=char_present[0];
            for(y=0;y<8;y++)
            {
                char_present[y]=char_buffer[y];
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Scroll down
        case 'd':
            for(y=1;y<8;y++)
            {
                char_buffer[y]=char_present[y-1];
            }
            char_buffer[0]=char_present[7];
            for(y=0;y<8;y++)
            {
                char_present[y]=char_buffer[y];
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Scroll right
        case 'r':
            for(y=0;y<8;y++)
            {
                char_buffer[y]=char_present[y]>>1;
                if(char_present[y]&0x01) { char_buffer[y]+=0x20; }
            }
            for(y=0;y<8;y++)
            {
                char_present[y]=char_buffer[y];
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;
        
        // Scroll left
        case 'l':
            for(y=0;y<8;y++)
            {
                char_buffer[y]=char_present[y]<<1;
                if(char_present[y]&0x20) { char_buffer[y]+=0x01; }
                char_buffer[y] &= 0x3F;
            }
            for(y=0;y<8;y++)
            {
                char_present[y]=char_buffer[y];
                POKE(char_address+y,char_present[y]);
                POKE(charaddress(char_screencode,char_altorstd,1)+y,char_present[y]);
            }
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Hex edit
        case 'h':
            sprintf(buffer,"%2x",char_present[ypos]);
            textInput(30,ypos+3,buffer,2,3);
            char_present[ypos] = (unsigned char)strtol(buffer,&ptrend,16);
            POKE(char_address+ypos,char_present[ypos]);
            POKE(charaddress(char_screencode,char_altorstd,1)+ypos,char_present[ypos]);
            showchareditgrid(char_screencode, char_altorstd);
            break;

        // Toggle statusbar
        case CH_F6:
            togglestatusbar();
            break;

        // Help screen
        case CH_F8:
            windowrestore(0);
            helpscreen_load(2);
            if(charsetchanged[0] ==1)
            {
                charset_swap(1);
            }
            showchareditfield();
            showchareditgrid(char_screencode,char_altorstd);
            break;

        // Store to favourites with SHIFT+0-9
        case 33:
            favourites[1] = char_screencode;
            break;

        case 64:
            favourites[2] = char_screencode;
            break;

        case 35:
            favourites[3] = char_screencode;
            break;

        case 36:
            favourites[4] = char_screencode;
            break;

        case 37:
            favourites[5] = char_screencode;
            break;

        case 94:
            favourites[6] = char_screencode;
            break;

        case 38:
            favourites[7] = char_screencode;
            break;

        case 42:
            favourites[8] = char_screencode;
            break;

        case 40:
            favourites[9] = char_screencode;
            break;

        case 41:
            favourites[0] = char_screencode;
            break;

        default:
            // 0-9: Favourites select
            if(key>47 && key<58)
            {
                char_screencode = favourites[key-48];
                charchanged=1;
            }
            break;
        }

        if(charchanged)
        {
            charchanged=0;
            char_address = charaddress(char_screencode,char_altorstd,0);
            for(y=0;y<8;y++)
            {
                char_present[y]=PEEK(char_address+y);
                char_undo[y]=char_present[y];
            }
            showchareditgrid(char_screencode, char_altorstd);
        }
    } while (key != CH_ESC && key != CH_STOP);

    windowrestore(0);

    plotscreencode = char_screencode;
    plotaltchar = char_altorstd;
    cputcxy(screen_col,screen_row,plotscreencode+128);
    strcpy(programmode,"Main");
}

void resizewidth()
{
    // Function to resize screen canvas width

    unsigned int newwidth = screenwidth;
    unsigned int maxsize = MEMORYLIMIT - SCREENMAPBASE;
    unsigned char areyousure = 0;
    unsigned char sizechanged = 0;
    unsigned int y;
    char* ptrend;

    windownew(2,5,12,38,0);

    cputsxy(4,6,"Resize canvas width");
    cputsxy(4,8,"Enter new width:");

    sprintf(buffer,"%i",screenwidth);
    textInput(4,9,buffer,4,1);
    newwidth = (unsigned int)strtol(buffer,&ptrend,10);

    if((newwidth*screenheight)  > maxsize || newwidth<40 )
    {
        cputsxy(4,11,"New size unsupported. Press key.");
        cgetc();
    }
    else
    {
        if(newwidth < screenwidth)
        {
            cputsxy(4,11,"Shrinking might delete data.");
            cputsxy(4,12,"Are you sure?");
            areyousure = menupulldown(20,13,5,0);
            if(areyousure==1)
            {
                for(y=0;y<screenheight;y++)
                {
                    memcpy((void*)SCREENMEMORY,(void*)screenmap_screenaddr(y,0,screenwidth),newwidth);
                    memcpy((void*)screenmap_screenaddr(y,0,newwidth),(void*)SCREENMEMORY,newwidth);
                }
                if(screen_col>newwidth-1) { screen_col=newwidth-1; }
                sizechanged = 1;
            }
        }
        if(newwidth > screenwidth)
        {
            for(y=0;y<screenheight;y++)
            {
                memcpy((void*)SCREENMEMORY,(void*)screenmap_screenaddr(screenheight-y-1,0,screenwidth),screenwidth);
                memcpy((void*)screenmap_screenaddr(screenheight-y-1,0,newwidth),(void*)SCREENMEMORY,screenwidth);
                memset((void*)screenmap_screenaddr(screenheight-y-1,screenwidth,newwidth),CH_SPACE,newwidth-screenwidth);
            }
            sizechanged = 1;
        }
    }

    windowrestore(0);

    if(sizechanged==1)
    {
        screenwidth = newwidth;
        screentotal = screenwidth * screenheight;
        xoffset = 0;
        ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
        sprintf(pulldownmenutitles[0][0],"Width:   %5i ",screenwidth);
        menuplacebar();
        if(showbar) { initstatusbar(); }
        cputcxy(screen_col,screen_row,plotscreencode+128);
    }
}

void resizeheight()
{
    // Function to resize screen camvas height

    unsigned int newheight = screenheight;
    unsigned int maxsize = MEMORYLIMIT - SCREENMAPBASE;
    unsigned char areyousure = 0;
    unsigned char sizechanged = 0;
    unsigned char y;
    char* ptrend;

    windownew(2,5,12,38,0);

    cputsxy(4,6,"Resize canvas height");
    cputsxy(4,8,"Enter new height:");

    sprintf(buffer,"%i",screenheight);
    textInput(4,9,buffer,4,1);
    newheight = (unsigned int)strtol(buffer,&ptrend,10);

    if((newheight*screenwidth)  > maxsize || newheight < 27)
    {
        cputsxy(4,11,"New size unsupported. press key.");
        cgetc();
    }
    else
    {
        if(newheight < screenheight)
        {
            cputsxy(4,11,"Shrinking might delete data.");
            cputsxy(4,12,"Are you sure?");
            areyousure = menupulldown(20,13,5,0);
            if(areyousure==1)
            {
                memcpy((void*)screenmap_screenaddr(0,0,screenwidth),(void*)screenmap_screenaddr(0,0,screenwidth),screenheight*screenwidth);
                if(screen_row>newheight-1) { screen_row=newheight-1; }
                sizechanged = 1;
            }
        }
        if(newheight > screenheight)
        {
            for(y=0;y<screenheight;y++)
            {
                memcpy((void*)screenmap_screenaddr(screenheight-y-1,0,screenwidth),(void*)screenmap_screenaddr(screenheight-y-1,0,screenwidth),screenwidth);
            }
            memset((void*)screenmap_screenaddr(screenheight,0,screenwidth),CH_SPACE,(newheight-screenheight)*screenwidth);
            sizechanged = 1;
        }
    }

    windowrestore(0);

    if(sizechanged==1)
    {
        screenheight = newheight;
        screentotal = screenwidth * screenheight;
        yoffset=0;
        ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
        sprintf(pulldownmenutitles[0][1],"Height:  %5i ",screenheight);
        menuplacebar();
        if(showbar) { initstatusbar(); }
        cputcxy(screen_col,screen_row,plotscreencode+128);
    }
}

void versioninfo()
{
    windownew(2,5,15,38,1);
    cputsxy(4,6,"Version information and credits");
    cputsxy(4,8,"ORIC Screen Editor");
    cputsxy(4,9,"Written in 2022 by Xander Mol");
    sprintf(buffer,"version: %s",version);
    cputsxy(4,11,buffer);
    cputsxy(4,13,"Source, docs and credits at:");
    cputsxy(4,14,"github.com/xahmol/OricScreenEditor");
    cputsxy(4,16,"(C) 2022, IDreamtIn8bits.com");
    cputsxy(4,18,"Press a key to continue.");
    cgetc();
    windowrestore(0);
}

int chooseidandfilename(char* headertext, unsigned char maxlen, unsigned char validation)
{
    // Function to present dialogue to enter device id and filename
    // Input: Headertext to print, maximum length of filename input string

    unsigned char valid = 0;

    windownew(2,5,12,38,0);
    cputsxy(4,6,headertext);

    cputsxy(4,8,"Choose filename:            ");
    valid = textInput(4,9,filename,maxlen,validation);
    return valid;
}

void loadscreenmap()
{
    // Function to load screenmap

    unsigned int newwidth, newheight;
    unsigned int maxsize = MEMORYLIMIT - SCREENMAPBASE;
    char* ptrend;
    int escapeflag;
    int rc;
    int len = 0;
  
    escapeflag = chooseidandfilename("Load screen",9,7);

    if(escapeflag==-1) { windowrestore(0); return; }

    cputsxy(4,10,"Enter screen width:");
    sprintf(buffer,"%i",screenwidth);
    textInput(4,11,buffer,4,1);
    newwidth = (unsigned int)strtol(buffer,&ptrend,10);

    cputsxy(4,12,"Enter screen height:");
    sprintf(buffer,"%i",screenheight);
    textInput(4,13,buffer,4,1);
    newheight = (unsigned int)strtol(buffer,&ptrend,10);

    if((newwidth*newheight)  > maxsize || newwidth<40 || newheight<27)
    {
        cputsxy(4,14,"New size unsupported. Press key.");
        cgetc();
        windowrestore(0);
    }
    else
    {
        windowrestore(0);

        sprintf(buffer,"%s.scr",filename);
        rc=loadfile(buffer,(void*)SCREENMAPBASE,&len);

        if(len)
        {
            windowrestore(0);
            screenwidth = newwidth;
            screenheight = newheight;
            ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
            windowsave(0,1,0);
            menuplacebar();
            if(showbar) { initstatusbar(); }
        }
    }
}

void savescreenmap()
{
    // Function to save screenmap
    int escapeflag;
    int rc;
    int len = 0;
  
    escapeflag = chooseidandfilename("Save screen",9,3);

    windowrestore(0);

    if(escapeflag==-1) { return; }

    sprintf(buffer,"%s.scr",filename);
    rc = savefile(buffer,(void*)SCREENMAPBASE,screenwidth*screenheight);
    
    if(rc) { fileerrormessage(rc,0); }
}

void saveproject()
{
    // Function to save project (screen, charsets and metadata)
    int rc;
    int len = 0;
    char projbuffer[19];
    int escapeflag;
  
    escapeflag = chooseidandfilename("Save project",9,3);

    windowrestore(0);

    if(escapeflag==-1) { return; }

    // Store project data to buffer variable
    projbuffer[ 0] = charsetchanged[0];
    projbuffer[ 1] = charsetchanged[1];
    projbuffer[ 2] = screen_col;
    projbuffer[ 3] = screen_row;
    projbuffer[ 4] = (screenwidth>>8) & 0xff;
    projbuffer[ 5] = screenwidth & 0xff;
    projbuffer[ 6] = (screenheight>>8) & 0xff;
    projbuffer[ 7] = screenheight & 0xff;
    projbuffer[ 8] = (screentotal>>8) & 0xff;
    projbuffer[ 9] = screentotal & 0xff;
    projbuffer[10] = plotink;
    projbuffer[11] = plotpaper;
    projbuffer[12] = plotblink;
    projbuffer[13] = plotdouble;
    projbuffer[14] = plotaltchar;
    projbuffer[15] = plotscreencode;
    projbuffer[17] = xoffset;
    projbuffer[18] = yoffset;

    sprintf(buffer,"%s.prj",filename);

    rc = savefile(buffer,(void*)projbuffer,19);

    if(rc) { fileerrormessage(rc,0); }

    // Store screen data
    sprintf(buffer,"%s.scr",filename);
    rc = savefile(buffer,(void*)SCREENMAPBASE,screenwidth*screenheight);
    if(rc) { fileerrormessage(rc,0); }

    // Store standard charset
    if(charsetchanged[0]==1)
    {
        sprintf(buffer,"%s.chs",filename);
        rc = savefile(buffer,(void*)CHARSET_SWAP,768);
        if(rc) { fileerrormessage(rc,0); }
    }

    // Store alternate charset
    if(charsetchanged[1]==1)
    {
        sprintf(buffer,"%s.cha",filename);
        rc = savefile(buffer,(void*)CHARSET_ALT,640);
        if(rc) { fileerrormessage(rc,0); }
    }  
}

void loadproject()
{
    // Function to load project (screen, charsets and metadata)
    int rc;
    int len = 0;
    unsigned char projbuffer[19];
    int escapeflag;
  
    escapeflag = chooseidandfilename("Load project",9,7);

    windowrestore(0);

    if(escapeflag==-1) { return; }

    // Load project variables
    sprintf(buffer,"%s.prj",filename);
    rc = loadfile(buffer,(void*)projbuffer,&len);

    if(!len) { return; }

    charsetchanged[0]       = projbuffer[ 0];
    charsetchanged[1]       = projbuffer[ 1];
    screen_col              = projbuffer[ 2];
    screen_row              = projbuffer[ 3];
    screenwidth             = projbuffer[ 4]*256+projbuffer[ 5];
    sprintf(pulldownmenutitles[0][0],"Width:   %5i ",screenwidth);
    screenheight            = projbuffer[ 6]*256+projbuffer [7];
    sprintf(pulldownmenutitles[0][1],"Height:  %5i ",screenheight);
    screentotal             = projbuffer[ 8]*256+projbuffer[ 9];
    plotink                 = projbuffer[10];
    plotpaper               = projbuffer[11];
    plotblink               = projbuffer[12];
    plotdouble              = projbuffer[13];
    plotaltchar             = projbuffer[14];
    plotscreencode          = projbuffer[15];
    xoffset                 = projbuffer[17];
    yoffset                 = projbuffer[18];

    // Load screen
    sprintf(buffer,"%s.scr",filename);
    rc=loadfile(buffer,(void*)SCREENMAPBASE,&len);
    if(len)
    {
        windowrestore(0);
        ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
        windowsave(0,1,0);
        menuplacebar();
        if(showbar) { initstatusbar(); }
    }

    // Load standard charset
    if(charsetchanged[0]==1)
    {
        sprintf(buffer,"%s.chs",filename);
        rc=loadfile(buffer,(void*)CHARSET_SWAP,&len);
        if(!len) { charsetchanged[0]=0; }
    }

    // Load alternate charset
    if(charsetchanged[0]==1)
    {
        sprintf(buffer,"%s.cha",filename);
        rc=loadfile(buffer,(void*)CHARSET_ALT,&len);
        if(!len) { charsetchanged[1]=0; }
    }
}

void loadcharset(unsigned char stdoralt)
{
    // Function to load charset
    // Input: stdoralt: standard charset (0) or alternate charset (1)

    unsigned int charsetaddress;
    int escapeflag;
    int rc;
    int len = 0;
  
    escapeflag = chooseidandfilename("Load character set",9,7);

    windowrestore(0);

    if(escapeflag==-1) { return; }

    charsetaddress = (stdoralt==0)? CHARSET_SWAP : CHARSET_ALT;

    sprintf(buffer,"%s.%s",filename,(stdoralt==0)? "chs" : "cha");
    rc = loadfile(buffer,(void*)charsetaddress,&len);

    if(len)
    {
        charsetchanged[stdoralt]=1;
    }
}

void savecharset(unsigned char stdoralt)
{
    // Function to save charset
    // Input: stdoralt: standard charset (0) or alternate charset (1)

    unsigned int charsetaddress;
    int escapeflag;
    int rc;
    int len = 0;
  
    escapeflag = chooseidandfilename("Save character set",9,3);

    windowrestore(0);

    if(escapeflag==-1) { return; }

    charsetaddress = (stdoralt==0)? CHARSET_SWAP : CHARSET_ALT;

    sprintf(buffer,"%s.%s",filename,(stdoralt==0)? "chs" : "cha");
    rc = savefile(buffer,(void*)charsetaddress,(stdoralt==0)? 768 : 640);
    if(rc) { fileerrormessage(rc,0); }
}

void mainmenuloop()
{
    // Function for main menu selection loop

    unsigned char menuchoice;
    
    windowsave(0,1,1);

    do
    {
        menuchoice = menumain();
      
        switch (menuchoice)
        {
        case 11:
            resizewidth();
            break;

        case 12:
            resizeheight();
            break;

        case 13:
        case 14:
            ORIC_ScreenmapFill(SCREENMAPBASE,screenwidth,screenheight,plotink,plotpaper,(menuchoice==13)?CH_SPACE:plotscreencode);
            windowrestore(0);
            ORIC_CopyViewPort(SCREENMAPBASE,screenwidth,xoffset,yoffset,0,0,40,27);
            windowsave(0,1,0);
            menuplacebar();
            if(showbar) { initstatusbar(); }
            break;

        case 21:
            savescreenmap();
            break;

        case 22:
            loadscreenmap();
            break;
        
        case 23:
            saveproject();
            break;
        
        case 24:
            loadproject();
            break;
        
        case 31:
            loadcharset(0);
            break;
        
        case 32:
            loadcharset(1);
            break;
        
        case 33:
            savecharset(0);
            break;

        case 34:
            savecharset(1);
            break;

        case 41:
            versioninfo();
            break;

        case 42:
            appexit = 1;
            menuchoice = 99;
            break;

        default:
            break;
        }
    } while (menuchoice < 99);
    
    windowrestore(1);
}

// Main routine
void main()
{
    // Main application initialization, loop and exit
    
    unsigned char key;
    int rc;
    int len = 0;

    // Reset startvalues global variables
    charsetchanged[0] = 0;
    charsetchanged[1] = 0;
    appexit = 0;
    screen_col = 0;
    screen_row = 0;
    xoffset = 0;
    yoffset = 0;
    screenwidth = 40;
    screenheight = 27;
    screentotal = screenwidth*screenheight;
    plotscreencode = 33;
    plotink = A_FWWHITE;
    plotpaper = A_FWBLACK;
    plotblink = 0;
    plotdouble = 0;
    plotaltchar = 0;

    sprintf(pulldownmenutitles[0][0],"Width:   %5i ",screenwidth);
    sprintf(pulldownmenutitles[0][1],"Height:  %5i ",screenheight);

    // Set version number in string variable
    sprintf(version,
            "v%2i.%2i - %c%c%c%c%c%c%c%c-%c%c%c%c",
            VERSION_MAJOR, VERSION_MINOR,
            BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3, BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1,BUILD_HOUR_CH0, BUILD_HOUR_CH1, BUILD_MIN_CH0, BUILD_MIN_CH1);

    // Initialise screen and set standard charsets
    ORIC_Init();
    ORIC_RestoreStandardCharset();
    ORIC_RestoreAlternateCharset();

    // Load and show title screen
    //printcentered("Load title screen",10,26,20);
    //rc = loadfile("ose.tscr",(void*)SCREENMEMORY,&len);

    // Init overlays
    //initoverlay();

    // Load visual PETSCII map mapping data
    //printcentered("Load palette map",10,26,20);
    //rc = loadfile("ose.petv",(void*)VISUALCHAR,&len);

    // Clear screen map in bank 1 with spaces in text color white
    ORIC_ScreenmapFill(SCREENMAPBASE,screenwidth,screenheight,A_FWWHITE,A_FWBLACK,CH_SPACE);

    // Initialise swap charset with system charset
    memcpy((void*)CHARSET_SWAP,(void*)CHARSET_STD,768);
 
    // Wait for key press to start application
    setflags(SCREEN);
    printcentered("Press key.",10,26,20);
    
    cgetc();

    // Clear viewport of titlescreen
    clrscr();

    // Main program loop
    cputcxy(screen_col,screen_row,plotscreencode+128);
    strcpy(programmode,"Main");
    showbar = 1;

    initstatusbar();

    do
    {
        if(showbar) { printstatusbar(); }
        key = cgetc();

        switch (key)
        {
        // Cursor move
        case CH_CURS_LEFT:
        case CH_CURS_RIGHT:
        case CH_CURS_UP:
        case CH_CURS_DOWN:
            plotmove(key);
            break;
        
        // Increase screencode
        case '=':
            if(plotscreencode==127) { plotscreencode=32; } else { plotscreencode++; }
            cputcxy(screen_col,screen_row,plotscreencode+128);
            break;

        // Decrease screencode
        case '-':
            if(plotscreencode==32) { plotscreencode=127; } else { plotscreencode--; }
            cputcxy(screen_col,screen_row,plotscreencode+128);
            break;
        
        // Decrease ink
        case ',':
            if(plotink==0) { plotink = 7; } else { plotink--; }
            break;

        // Increase ink
        case '.':
            if(plotink==7) { plotink = 0; } else { plotink++; }
            break;

        // Decrease paper
        case ';':
            if(plotpaper==0) { plotpaper = 7; } else { plotpaper--; }
            break;

        // Increase paper
        case 39:
            if(plotpaper==7) { plotpaper = 0; } else { plotpaper++; }
            break;

         // Toggle blink
        case 'b':
            plotblink = (plotblink==0)? 1:0;
            break;
        
        // Toggle alternate character set
        case 'a':
            plotaltchar = (plotaltchar==0)? 1:0;
            break;

        // Toggle double
        case 'd':
            plotdouble = (plotdouble==0)? 1:0;
            break;

        // Character edit mode
        case 'e':
            chareditor();
            break;

        // Palette for character selection
        case 'p':
            palette();
            break;

        // Grab underlying character and attributes
        case 'g':
            plotscreencode = PEEK(screenmap_screenaddr(screen_row+yoffset,screen_col+xoffset,screenwidth));
            cputcxy(screen_col,screen_row,plotscreencode+128);
            break;

        // Write mode: type in screencodes
        case 'w':
            writemode();
            break;

        // Line and box mode
        case 'l':
            lineandbox(1);
            break;

        // Move mode
        case 'm':
            movemode();
            break;

        // Select mode
        case 's':
            selectmode();
            break;

        // Try
        case 't':
            plot_try();
            break;

        // Increase/decrease plot screencode by 128 (toggle 'RVS ON' and 'RVS OFF')
        case 'r':
            plotscreencode += 128;      // Will increase 128 if <128 and decrease by 128 if >128 by overflow
            cputcxy(screen_col,screen_row,plotscreencode+128);
            break;        

        // Plot present screencode
        case CH_SPACE:
            screenmapplot(screen_row+yoffset,screen_col+xoffset,plotscreencode);
            break;

        // Plot present ink
        case 'i':
            screenmapplot(screen_row+yoffset,screen_col+xoffset,plotink);
            plotmove(CH_CURS_DOWN);
            break;

        // Plot present paper
        case 'o':
            screenmapplot(screen_row+yoffset,screen_col+xoffset,16+plotpaper);
            plotmove(CH_CURS_DOWN);
            break;

        // Plot present character modifier
        case 'u':
            screenmapplot(screen_row+yoffset,screen_col+xoffset,ORIC_CharAttribute(plotaltchar,plotdouble,plotblink));
            plotmove(CH_CURS_DOWN);
            break;

        // Delete present screencode and attributes
        case CH_DEL:
            screenmapplot(screen_row+yoffset,screen_col+xoffset,CH_SPACE);
            break;

        // Store to favourites with SHIFT+0-9
        case 33:
            favourites[1] = plotscreencode;
            break;

        case 64:
            favourites[2] = plotscreencode;
            break;

        case 35:
            favourites[3] = plotscreencode;
            break;

        case 36:
            favourites[4] = plotscreencode;
            break;

        case 37:
            favourites[5] = plotscreencode;
            break;

        case 94:
            favourites[6] = plotscreencode;
            break;

        case 38:
            favourites[7] = plotscreencode;
            break;

        case 42:
            favourites[8] = plotscreencode;
            break;

        case 40:
            favourites[9] = plotscreencode;
            break;

        case 41:
            favourites[0] = plotscreencode;
            break;

        // Go to menu
        case CH_F1:
            mainmenuloop();
            cputcxy(screen_col,screen_row,plotscreencode);
            break;

        // Toggle statusbar
        case CH_F6:
            togglestatusbar();
            break;

        // Help screen
        case CH_F8:
            helpscreen_load(1);
            break;
        
        default:
            // 0-9: Favourites select
            if(key>47 && key<58)
            {
                plotscreencode = favourites[key-48];
                cputcxy(screen_col,screen_row,plotscreencode+128);
            }
            break;
        }
    } while (appexit==0);

    textcolor(COLOR_YELLOW);
    ORIC_Exit();
}
