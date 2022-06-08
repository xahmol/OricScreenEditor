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
struct OverlayStruct overlaydata[4];
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
char filename[21];
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
unsigned char plotreverse;
unsigned char plotblink;
unsigned char plotdouble;
unsigned char plotaltchar;
unsigned int select_startx, select_starty, select_endx, select_endy, select_width, select_height, select_accept;
unsigned char rowsel = 0;
unsigned char colsel = 0;
unsigned char palettechar;
unsigned char visualmap = 0;
unsigned char favourites[10][2];

char buffer[81];
char version[22];

// Generic routines
int textInput(unsigned char xpos, unsigned char ypos, char* str, unsigned char size)
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

    register unsigned char c;
    register unsigned char idx = strlen(str);

    cursor(1);
    cputsxy(xpos,ypos,str);
    
    while(1)
    {
        c = cgetc();
        switch (c)
        {
        case CH_ESC:
            cursor(0);
            return -1;

        case CH_ENTER:
            idx = strlen(str);
            str[idx] = 0;
            cursor(0);
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
                gotoxy(xpos+idx,ypos);
            }
            break;

        case CH_CURS_LEFT:
            if (idx)
            {
                --idx;
                gotoxy(xpos+idx,ypos);
            }
            break;

        case CH_CURS_RIGHT:
            if (idx < strlen(str) && idx < size)
            {
                ++idx;
                gotoxy(xpos+idx,ypos);
            }
            break;

        default:
            if (isprint(c) && idx < size)
            {
                unsigned char flag = (str[idx] == 0);
                str[idx] = c;
                cputcxy(xpos+idx++,ypos,c);
                if (flag) { str[idx+1] = 0; }
                break;
            }
            break;
        }
    }
    return 0;
}

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

// Functions for windowing and menu system

void windowsave(unsigned char ypos, unsigned char height, unsigned char loadsyscharset)
{
    /* Function to save a window
       Input:
       - ypos: startline of window
       - height: height of window    
       - loadsyscharset: load syscharset if userdefined charset is loaded enabled (1) or not (0) */

    unsigned int baseaddress = SCREENMEMORY + (ypos*40);
    unsigned int length =height*40;

    Window[windownumber].address = windowaddress;
    Window[windownumber].ypos = ypos;
    Window[windownumber].height = height;

    // Copy screen
    memcpy((int*)windowaddress,(int*)baseaddress,length);
    windowaddress += length;

    windownumber++;

    // Load system charset if needed
    if(loadsyscharset == 1 && charsetchanged[1] == 1)
    {
        charset_swap(0);
    }
}

void windowrestore(unsigned char restorealtcharset)
{
    /* Function to restore a window
       Input: restorealtcharset: request to restore user defined charset if needed enabled (1) or not (0) */

    unsigned int baseaddress = SCREENMEMORY + (Window[--windownumber].ypos*40);
    unsigned int length = Window[windownumber].height*40;

    windowaddress = Window[windownumber].address;

    // Restore screem
    memcpy((int*)baseaddress,(int*)windowaddress,length); 

    // Restore custom charset if needed
    if(restorealtcharset == 1 && charsetchanged[1] == 1)
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
        gotoxy(xpos,ypos+x);
        cputc(A_BGCYAN);
        cputc(A_FWBLACK);
        cprintf(" %s",pulldownmenutitles[menunumber-1][x]);
        cputc(endcolor);
    }
  
    do
    {
        gotoxy(xpos,ypos+menuchoice-1);
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
            gotoxy(xpos,ypos+menuchoice-1);
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

// Main routine
void main()
{
    unsigned char x,y,character,key,xoffset,yoffset;
    unsigned int address;

    ORIC_Init();

    for(x=0;x<10;x++)
    {
        address = 0xC000+x;
        gotoxy(1,x);
        POKEO(address,x);
        cprintf("Nr: %2u %4X ROM: %2X RAM: %2X",x,address,PEEK(address),PEEKO(address));
        address = 0xF000+x;
        gotoxy(1,10+x);
        POKEO(address,x);
        cprintf("Nr: %2u %4X ROM: %2X RAM: %2X",x,address,PEEK(address),PEEKO(address));
    }

    cgetc();
    clrscr();
    OverlayMemSet(0xC000,0x20,10);
    OverlayMemCopy(0xC000,0xF000,10);
    for(x=0;x<10;x++)
    {
        address = 0xC000+x;
        gotoxy(1,x);
        cprintf("Nr: %2u %4X ROM: %2X RAM: %2X",x,address,PEEK(address),PEEKO(address));
        address = 0xF000+x;
        gotoxy(1,10+x);
        cprintf("Nr: %2u %4X ROM: %2X RAM: %2X",x,address,PEEK(address),PEEKO(address));
    }

    cgetc();
    clrscr();

    for(x=0;x<40;x++)
    {
        cputcxy(x,0,48+x%10);
        if(x<27) { cputcxy(0,x,48+x%10); }
    }

    ORIC_HChar(5,5,65,10);
    ORIC_VChar(5,16,A_FWRED,10);
    ORIC_VChar(5,17,66,10);
    ORIC_VChar(5,18,A_FWGREEN,10);
    ORIC_FillArea(5,20,67,10,10);

    cgetc();

    ORIC_ScreenmapFill(SCREENMAPBASE,80,50,3,0,A_STD);

    character=32;

    for(y=0;y<50;y++)
    {
        for(x=2;x<80;x++)
        {
            POKE(SCREENMAPBASE+x+(y*80),character++);
            if(character>126 && character<160) { character=160; }
            if(character>254) { character=32; }
        }
    }

    xoffset=5;
    yoffset=5;

    ORIC_CopyViewPort(SCREENMAPBASE,80,5,5,5,5,30,18);

    do
    {
        key = cgetc();
        switch (key)
        {
        case CH_CURS_LEFT:
            if(xoffset>0)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,80,xoffset--,yoffset,5,5,30,18,2);
            }
            break;

        case CH_CURS_RIGHT:
            if(xoffset<50)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,80,xoffset++,yoffset,5,5,30,18,1);
            }
            break;

        case CH_CURS_UP:
            if(yoffset>0)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,80,xoffset,yoffset--,5,5,30,18,4);
            }
            break;

        case CH_CURS_DOWN:
            if(yoffset<8)
            {
                ORIC_ScrollCopy(SCREENMAPBASE,80,xoffset,yoffset++,5,5,30,18,8);
            }
            break;
        
        default:
            break;
        }
    } while (key != CH_ESC);

    menumain();

    messagepopup("Dit is een test",0); 

    memcpy((void*)CHARSET_SWAP,(void*)CHARSET_STD,768);
    for(x=0;x<96;x++)
    {
        POKE(CHARSET_SWAP+(x*8)+7,0xff);
    }
    charset_swap(1);
    cgetc();

    charset_swap(0);
    cgetc();

    charset_swap(1);
    cgetc();

    charset_swap(0);
    cgetc();

    ORIC_Exit();

}
