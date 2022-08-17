// ====================================================================================
// oric_core.c
// Core assembly routines for oric_core.c
//
// Credits for code and inspiration:
//
// Sedoric disk operations routines:
//-  lib-sedoric from oricOpenLibrary
//    https://github.com/iss000/oricOpenLibrary/blob/main/lib-sedoric/libsedoric.s
//   )              _
//   )  ___ ___ _ _|_|___ ___
//   ) |  _| .'|_'_| |_ -|_ -|
//   ) |_| |__,|_,_|_|___|___|
//   )         raxiss (c) 2021
//   )
//   ) GNU General Public License v3.0
//   ) See https://github.com/iss000/oricOpenLibrary/blob/main/LICENSE
//   )
//
// - 6502.org: Practical Memory Move Routines
//   http://6502.org/source/general/memory_move.html
//
// =====================================================================================

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <peekpoke.h>
#include <atmos.h>
#include "defines.h"
#include "osdklib.h"
#include "defines.h"
#include "oric_core.h"

// Generic screen and scroll routines

void ORIC_HChar(unsigned char row, unsigned char col, unsigned char character, unsigned char length)
{
	// Function to draw horizontal line with given character (draws from left to right)
	// Input: row and column of start position (left end of line), screencode of character to draw line with,
	//		  length in number of character positions, attribute color value

	unsigned int startaddress = ORIC_RowColToAddress(row,col);
	ORIC_addrh = (startaddress>>8) & 0xff;	// Obtain high byte of start address
	ORIC_addrl = startaddress & 0xff;		// Obtain low byte of start address
	ORIC_tmp1 = character;					// Obtain character value
	ORIC_tmp2 = length;					    // Obtain length value

	ORIC_HChar_core();
}

void ORIC_VChar(unsigned char row, unsigned char col, unsigned char character, unsigned char length)
{
	// Function to draw vertical line with given character (draws from top to bottom)
	// Input: row and column of start position (top end of line), screencode of character to draw line with,
	//		  length in number of character positions, attribute color value

	unsigned int startaddress = ORIC_RowColToAddress(row,col);
	ORIC_addrh = (startaddress>>8) & 0xff;	// Obtain high byte of start address
	ORIC_addrl = startaddress & 0xff;		// Obtain low byte of start address
	ORIC_tmp1 = character;					// Obtain character value
	ORIC_tmp2 = length;						// Obtain length value

	ORIC_VChar_core();
}

void ORIC_FillArea(unsigned char row, unsigned char col, unsigned char character, unsigned char length, unsigned char height)
{
	// Function to draw area with given character (draws from topleft to bottomright)
	// Input: row and column of start position (topleft), screencode of character to draw line with,
	//		  length and height in number of character positions, attribute color value

	unsigned int startaddress = ORIC_RowColToAddress(row,col);
	ORIC_addrh = (startaddress>>8) & 0xff;	// Obtain high byte of start address
	ORIC_addrl = startaddress & 0xff;		// Obtain low byte of start address
	ORIC_tmp1 = character;					// Obtain character value
	ORIC_tmp2 = length;						// Obtain length value
	ORIC_tmp4 = height;						// Obtain number of lines

	ORIC_FillArea_core();
}

void ORIC_Init(void)
{
	// Init screen
	setflags(SCREEN+NOKEYCLICK);
    bgcolor(COLOR_BLACK);
    textcolor(COLOR_WHITE);
    clrscr();
}

void ORIC_Exit(void)
{
	setflags(SCREEN);
	clrscr();
}

unsigned int ORIC_RowColToAddress(unsigned char row, unsigned char col)
{
	/* Function returns a memory address for a given row and column */

	unsigned int addr;
	addr = row * 40 + col;

	if (addr < 1080)
	{
		addr += SCREENMEMORY;
		return addr;
	}
	else
	{
		return -1;
	}
}

unsigned char ORIC_CharAttribute(unsigned char charset, unsigned char doublesize, unsigned char blink)
{
	// Function to return serial attribute code for the charset modifiers
    // Input: Charset (0=standard, 1=alternate), doublesize (0=off, 1=on), blink (0=off, 1=on)

    return 8+charset+(2*doublesize)+(4*blink);
}

void ORIC_CopyViewPort(unsigned int sourcebase, unsigned int sourcewidth, unsigned int sourcexoffset, unsigned int sourceyoffset, unsigned char xcoord, unsigned char ycoord, unsigned char viewwidth, unsigned char viewheight )
{
	// Function to copy a viewport on the source screen map to the visible screen
	// Input:
	// - Source:	sourcebase			= source base address in memory
	//				sourcewidth			= number of characters per line in source screen map
	//				sourceheight		= number of lines in source screen map
	//				sourcexoffset		= horizontal offset on source screen map to start upper left corner of viewpoint
	//				sourceyoffset		= vertical offset on source screen map to start upper left corner of viewpoint
	// - Viewport:	xcoord				= x coordinate of viewport upper left corner
	//				ycoord				= y coordinate of viewport upper left corner
	//				viewwidth			= width of viewport in number of characters
	//				viewheight			= height of viewport in number of lines

	unsigned int stride = sourcewidth;
	unsigned int Screenbase = ORIC_RowColToAddress(ycoord,xcoord);

	sourcebase += (sourceyoffset * sourcewidth ) + sourcexoffset;

	ORIC_addrh = (sourcebase>>8) & 0xff;				// Obtain high byte of source address
	ORIC_addrl = sourcebase & 0xff;						// Obtain low byte of source address
	ORIC_desth = (Screenbase>>8) & 0xff;				// Obtain high byte of destination address
	ORIC_destl = Screenbase & 0xff;						// Obtain low byte of destination address
	ORIC_strideh = (stride>>8) & 0xff;					// Obtain high byte of stride
	ORIC_stridel = stride & 0xff;						// Obtain low byte of stride
	ORIC_tmp1 = viewheight;								// Obtain number of lines to copy
	ORIC_tmp2 = viewwidth;								// Obtain length of lines to copy

	ORIC_CopyViewPort_core();
}

void ORIC_ScrollCopy(unsigned int sourcebase, unsigned int sourcewidth, unsigned int sourcexoffset, unsigned int sourceyoffset, unsigned char xcoord, unsigned char ycoord, unsigned char viewwidth, unsigned char viewheight, unsigned char direction)
{
	// Function to scroll a viewport on the source screen map on the in the given direction
	// Input:
	// - Source:	sourcebase			= source base address in memory
	//				sourcewidth			= number of characters per line in source screen map
	//				sourceheight		= number of lines in source screen map
	//				sourcexoffset		= horizontal offset on source screen map to start upper left corner of viewpoint
	//				sourceyoffset		= vertical offset on source screen map to start upper left corner of viewpoint
	// - Viewport:	xcoord				= x coordinate of viewport upper left corner
	//				ycoord				= y coordinate of viewport upper left corner
	//				viewwidth			= width of viewport in number of characters
	//				viewheight			= height of viewport in number of lines
	// - Direction:	direction			= Bit pattern for direction of scroll:
	//									  bit 7 set ($01): Left
	//									  bit 6 set ($02): right
	//									  bit 5 set ($04): down
	//									  bit 4 set ($08): up


	// First perform scroll
	ORIC_ScrollMove(xcoord,ycoord,viewwidth,viewheight,direction,0);

	// Finally add the new line or column
	switch (direction)
	{
	case SCROLL_LEFT:
		sourcexoffset += viewwidth;
		xcoord += --viewwidth;
		viewwidth = 1;
		break;

	case SCROLL_RIGHT:
		sourcexoffset--;
		viewwidth = 1;
		break;

	case SCROLL_DOWN:
		sourceyoffset--;
		viewheight = 1;
		break;

	case SCROLL_UP:
		sourceyoffset += viewheight;
		ycoord += --viewheight;
		viewheight = 1;
		break;
	
	default:
		break;
	}
	ORIC_CopyViewPort(sourcebase,sourcewidth,sourcexoffset,sourceyoffset,xcoord,ycoord,viewwidth,viewheight);
}

void ORIC_ScrollMove(unsigned char xcoord, unsigned char ycoord, unsigned char viewwidth, unsigned char viewheight, unsigned char direction, unsigned char clear)
{
	// Function to scroll a viewport without filling in the emptied row or column
	// Input:
	// - Viewport:	xcoord				= x coordinate of viewport upper left corner
	//				ycoord				= y coordinate of viewport upper left corner
	//				viewwidth			= width of viewport in number of characters
	//				viewheight			= height of viewport in number of lines
	// - Direction:	direction			= Bit pattern for direction of scroll:
	//									  bit 7 set ($01): Left
	//									  bit 6 set ($02): right
	//									  bit 5 set ($04): down
	//									  bit 4 set ($08): up
	// - Clear:							= 1 for clear scrolled out area

	unsigned int sourceaddr = ORIC_RowColToAddress(ycoord,xcoord);

	// Set input for core routines
	ORIC_tmp1 = viewheight;				// Obtain number of lines to copy
	ORIC_tmp2 = viewwidth;				// Obtain length of lines to copy

	// Scroll in desired direction
	switch (direction)
	{
	case SCROLL_LEFT:
		ORIC_addrh = (sourceaddr>>8) & 0xff;	// Obtain high byte of source address
		ORIC_addrl = sourceaddr & 0xff;			// Obtain low byte of source address
		ORIC_Scroll_left_core();
		if(clear) { ORIC_FillArea(ycoord,xcoord+viewwidth-1,CH_SPACE,1,viewheight); }
		break;

	case SCROLL_RIGHT:
		ORIC_addrh = (sourceaddr>>8) & 0xff;	// Obtain high byte of source address
		ORIC_addrl = sourceaddr & 0xff;			// Obtain low byte of source address
		ORIC_Scroll_right_core();
		if(clear) { ORIC_FillArea(ycoord,xcoord,CH_SPACE,1,viewheight); }
		break;

	case SCROLL_DOWN:
		sourceaddr += (viewheight-2)*40;
		ORIC_addrh = (sourceaddr>>8) & 0xff;	// Obtain high byte of source address
		ORIC_addrl = sourceaddr & 0xff;			// Obtain low byte of source address
		ORIC_Scroll_down_core();
		if(clear) { ORIC_FillArea(ycoord,xcoord,CH_SPACE,viewwidth,1); }
		break;

	case SCROLL_UP:
		sourceaddr += 40;
		ORIC_addrh = (sourceaddr>>8) & 0xff;	// Obtain high byte of source address
		ORIC_addrl = sourceaddr & 0xff;			// Obtain low byte of source address
		ORIC_Scroll_up_core();
		if(clear) { ORIC_FillArea(ycoord+viewheight-1,xcoord,CH_SPACE,viewwidth,1); }
		break;
	
	default:
		break;
	}
}

void ORIC_ScreenmapFill(unsigned int sourcebase, unsigned int sourcewidth, unsigned int sourceheight, unsigned char ink, unsigned char paper, unsigned char character)
{
    // Function to clear screenmap memory area, filling screenmap with default paper and ink in first to columns,
    // rest attribute for standard charset

    // First: Fill whole area with serial attribute for standard charset
    memset((void*)sourcebase,character,sourcewidth*sourceheight);

    // Second: Plot paper color in first column
	ORIC_addrh = (sourcebase>>8) & 0xff;	// Obtain high byte of start address
	ORIC_addrl = sourcebase & 0xff;		    // Obtain low byte of start address
	ORIC_tmp1 = 16+paper;					// Obtain paper attribute value
	ORIC_tmp2 = sourceheight;				// Obtain length value = source height
	ORIC_VChar_core();

    // Second: Plot ink color in second column
	ORIC_addrh = (++sourcebase>>8) & 0xff;	// Obtain high byte of start address + 1 for second column
	ORIC_addrl = sourcebase & 0xff;		    // Obtain low byte of start address
	ORIC_tmp1 = ink;					    // Obtain ink attribute value
	ORIC_tmp2 = sourceheight;				// Obtain length value = source height
	ORIC_VChar_core();
}
