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
unsigned char plotblink;
unsigned char plotdouble;
unsigned char plotaltchar;
unsigned int select_startx, select_starty, select_endx, select_endy, select_width, select_height, select_accept;
unsigned char rowsel = 0;
unsigned char colsel = 0;
unsigned char palettechar;
unsigned char visualmap = 0;
unsigned char favourites[10] = {31,31,31,31,31,31,31,31,31,31};

char buffer[81];
char version[22];

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

unsigned int screenmap_screenaddr(unsigned char row, unsigned char col, unsigned int width, unsigned int height)
{
    // Function to calculate screenmap address for the character space
    // Input: row, col, width and height for screenmap

    return SCREENMAPBASE+(row*width)+col;
}


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
            gotoxy(xpos+1,ypos+1);
            cspaces(size+1);
            cputsxy(xpos+1, ypos+1, str);
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
                gotoxy(xpos+idx+1, ypos+1);
                cputc(str[idx] ? 128+str[idx] : CH_INVSPACE);
                cputc(str[idx+1] ? str[idx+1] : CH_SPACE);
                gotoxy(xpos+idx+1, ypos+1);
            }
            break;

        case CH_CURS_LEFT:
            if (idx)
            {
                --idx;
                gotoxy(xpos+idx+1, ypos+1);
                cputc(str[idx] ? 128+str[idx] : CH_INVSPACE);
                cputc(str[idx+1] ? str[idx+1] : CH_SPACE);
                gotoxy(xpos+idx+1, ypos+1);

            }
            break;

        case CH_CURS_RIGHT:
            if (idx < strlen(str) && idx < size)
            {
                ++idx;
                gotoxy(xpos+idx, ypos+1);
                cputc(str[idx-1]);
                cputc(str[idx] ? 128+str[idx] : CH_INVSPACE);
                gotoxy(xpos + idx + 1, ypos+1);
            }
            break;

        default:
            if (isprint(c) && idx < size)
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

/* Overlay functions */

void initoverlay()
{
    // Load all overlays into memory if possible

    unsigned char x;
    unsigned int address=OVERLAYBASE;
    int rc;
    int len = 0;
    unsigned char inmemoryflag = 1;

    for(x=0;x<OVERLAYNUMBER;x++)
    {
        // Update load status message
        sprintf(buffer,"Memory overlay %u",x+1);
        printcentered(buffer,10,26,20);
        
        // Compose filename
        sprintf(buffer,"ose.ovl%u",x+1);

        // Load overlay file, exit if not found
        rc = loadfile(buffer,(void*)OVERLAYLOAD,&len);
        if (!len)
        {
            printf("\nLoading overlay file failed\n");
            exit(1);
        }

        // Copy to overlay storage memory location
        if(inmemoryflag)
        {
            OverlayMemCopy(OVERLAYLOAD,address,OVERLAYSIZE);
            overlaydata[x]=address;
            address+=OVERLAYSIZE;
            if(address+OVERLAYSIZE<OVERLAYBASE)
            {
                inmemoryflag=0;
            }
        }
        else
        {
            overlaydata[x]=0;
        }
    }
}

void loadoverlay(unsigned char overlay_select)
{
    // Load memory overlay with given number

    int rc;
    int len = 0;

    // Returns if overlay allready active
    if(overlay_select != overlay_active)
    {
        overlay_active = overlay_select;
        if(overlaydata[overlay_select-1])
        {
            OverlayMemCopy(overlaydata[overlay_select-1],OVERLAYLOAD,OVERLAYSIZE);
        }
        else
        {
            // Compose filename
            sprintf(buffer,"ose.ovl%u",overlay_select);

            // Load overlay file, exit if not found
            rc = loadfile(buffer,(void*)OVERLAYLOAD,&len);
            if (!len)
            {
                printf("\nLoading overlay file failed\n");
                exit(1);
            }
        }
    }   
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
    sprintf(buffer,"%2x",PEEK(screenmap_screenaddr(screen_row+yoffset,screen_col+xoffset,screenwidth,screenheight)));
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

// Generic screen map routines
void screenmapplot(unsigned char row, unsigned char col, unsigned char screencode)
{
    // Function to plot a screencode
	// Input: row and column, screencode to plot

    POKE(screenmap_screenaddr(row,col,screenwidth,screenheight),screencode);
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
    sprintf(buffer,"ose.hsc%u",screennumber);
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

    cputcxy(screen_col,screen_row,PEEK(screenmap_screenaddr(yoffset+screen_row,xoffset+screen_col,screenwidth,screenheight)));

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

    // Initialise VDC screen and VDC assembly routines
    ORIC_Init();

    // Load and show title screen
    //printcentered("Load title screen",10,26,20);
    //rc = loadfile("ose.tscr",(void*)SCREENMEMORY,&len);

    // Init overlays
    //initoverlay();

    // Load visual PETSCII map mapping data
    //printcentered("Load palette map",10,26,20);
    //rc = loadfile("ose.petv",(void*)VISUALCHAR,&len);

    // Clear screen map in bank 1 with spaces in text color white
    ORIC_ScreenmapFill(SCREENMAPBASE,screenwidth,screenheight,A_FWWHITE,A_FWBLACK,A_STD);
 
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
            if(plotscreencode==126) { plotscreencode=32; } else { plotscreencode++; }
            cputcxy(screen_col,screen_row,plotscreencode+128);
            break;

        // Decrease screencode
        case '-':
            if(plotscreencode==32) { plotscreencode=126; } else { plotscreencode--; }
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

        // Character eddit mode
        //case 'e':
        //    chareditor();
        //    break;

        // Palette for character selection
        //case 'p':
        //    palette();
        //    break;

        // Grab underlying character and attributes
        case 'g':
            plotscreencode = PEEK(screenmap_screenaddr(screen_row+yoffset,screen_col+xoffset,screenwidth,screenheight));
            cputcxy(screen_col,screen_row,plotscreencode+128);
            break;

        // Write mode: type in screencodes
        //case 'w':
        //    writemode();
        //    break;
        
        // Color mode: type colors
        //case 'c':
        //    colorwrite();
        //    break;

        // Line and box mode
        //case 'l':
        //    lineandbox(1);
        //    break;

        // Move mode
        //case 'm':
        //    movemode();
        //    break;

        // Select mode
        //case 's':
        //    selectmode();
        //    break;

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
            plotmove(CH_CURS_RIGHT);
            break;

        // Plot present paper
        case 'o':
            screenmapplot(screen_row+yoffset,screen_col+xoffset,16+plotpaper);
            plotmove(CH_CURS_RIGHT);
            break;

        // Plot present character modifier
        case 'm':
            screenmapplot(screen_row+yoffset,screen_col+xoffset,ORIC_CharAttribute(plotaltchar,plotdouble,plotblink));
            plotmove(CH_CURS_RIGHT);
            break;

        // Delete present screencode and attributes
        case CH_DEL:
            screenmapplot(screen_row+yoffset,screen_col+xoffset,CH_SPACE);
            break;

        // Go to menu
        //case CH_F1:
        //    mainmenuloop();
        //    cputcxy(screen_col,screen_row,plotscreencode);
        //    break;

        // Toggle statusbar
        case CH_F6:
            togglestatusbar();
            break;

        // Help screen
        //case CH_F8:
        //    helpscreen_load(1);
        //    break;
        
        default:
            // 0-9: Favourites select
            if(key>47 && key<58)
            {
                plotscreencode = favourites[key-48];
                cputcxy(screen_col,screen_row,plotscreencode+128);
            }
            // Shift 1-9 or *: Store present character in favourites slot
            if(key>32 && key<43)
            {
                favourites[key-33] = plotscreencode;
            }
            break;
        }
    } while (appexit==0);

    textcolor(COLOR_YELLOW);
    ORIC_Exit();
}
