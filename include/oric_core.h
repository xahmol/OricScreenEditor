// ====================================================================================
// oric_core.h
//
// Core Oric system routines
//
// =====================================================================================

#ifndef _ORIC_CORE_H__
#define _ORIC_CORE_H__

/* ================================================================== *
 * libsedoric code                                                    *
 * ================================================================== */
/*               _
 **  ___ ___ _ _|_|___ ___
 ** |  _| .'|_'_| |_ -|_ -|
 ** |_| |__,|_,_|_|___|___|
 **         raxiss (c) 2021
 **
 ** GNU General Public License v3.0
 ** See https://github.com/iss000/oricOpenLibrary/blob/main/LICENSE
 **
 */

extern const char* sed_fname;
extern void* sed_begin;
extern void* sed_end;
extern unsigned int sed_size;
extern int sed_err;

extern void sed_savefile(void);
extern void sed_loadfile(void);

int savefile(const char* fname, void* buf, int len);
int loadfile(const char* fname, void* buf, int* len);

// Own routines

// Defines for scroll directions
#define SCROLL_LEFT             0x01
#define SCROLL_RIGHT            0x02
#define SCROLL_DOWN             0x04
#define SCROLL_UP               0x08

// Variables in core Functions
extern unsigned char ORIC_addrh;
extern unsigned char ORIC_addrl;
extern unsigned char ORIC_desth;
extern unsigned char ORIC_destl;
extern unsigned char ORIC_strideh;
extern unsigned char ORIC_stridel;
extern unsigned char ORIC_value;
extern unsigned char ORIC_tmp1;
extern unsigned char ORIC_tmp2;
extern unsigned char ORIC_tmp3;
extern unsigned char ORIC_tmp4;

// Import assembly core Functions
void ORIC_HChar_core();
void ORIC_VChar_core();
void ORIC_FillArea_core();
void ORIC_CopyViewPort_core();
void ORIC_ScrollCopy_core();
void ORIC_Scroll_right_core();
void ORIC_Scroll_left_core();
void ORIC_Scroll_down_core();
void ORIC_Scroll_up_core();
void ORIC_RestoreStandardCharset();
void ORIC_RestoreAlternateCharset();

// Function Prototypes
void ORIC_HChar(unsigned char row, unsigned char col, unsigned char character, unsigned char length);
void ORIC_VChar(unsigned char row, unsigned char col, unsigned char character, unsigned char length);
void ORIC_FillArea(unsigned char row, unsigned char col, unsigned char character, unsigned char length, unsigned char height);
void ORIC_Init(void);
void ORIC_Exit(void);
unsigned int ORIC_RowColToAddress(unsigned char row, unsigned char col);
unsigned char ORIC_CharAttribute(unsigned char charset, unsigned char doublesize, unsigned char blink);
void ORIC_CopyViewPort(unsigned int sourcebase, unsigned int sourcewidth, unsigned int sourcexoffset, unsigned int sourceyoffset, unsigned char xcoord, unsigned char ycoord, unsigned char viewwidth, unsigned char viewheight );
void ORIC_ScrollCopy(unsigned int sourcebase, unsigned int sourcewidth, unsigned int sourcexoffset, unsigned int sourceyoffset, unsigned char xcoord, unsigned char ycoord, unsigned char viewwidth, unsigned char viewheight, unsigned char direction);
void ORIC_ScrollMove(unsigned char xcoord, unsigned char ycoord, unsigned char viewwidth, unsigned char viewheight, unsigned char direction, unsigned char clear);
void ORIC_ScreenmapFill(unsigned int sourcebase, unsigned int sourcewidth, unsigned int sourceheight, unsigned char ink, unsigned char paper, unsigned char character);


#endif /* __ORIC_CORE_H__ */