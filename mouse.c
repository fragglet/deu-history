/*
   Mouse interface by Rapha‰l Quinet <quinet@montefiore.ulg.ac.be>
   
   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.
   
   This program comes with absolutely no warranty.
   
   MOUSE.C - Mouse driver routines.
*/

/* the includes */
#include "deu.h"
#include <dos.h>

/* mouse interrupt number */
#define MOUSE 0x33

/* the global data */
Bool UseMouse;						/* is there a mouse driver? */


/*
   initialize the mouse driver
   */

#if defined(__TURBOC__)

void CheckMouseDriver()
{
    union REGS regs;
    
    regs.x.ax = 0x0000;
    int86(MOUSE, &regs, &regs);
    if (regs.x.ax == 0xffff)
		UseMouse = TRUE; /* mouse */
    else
		UseMouse = FALSE; /* no mouse */
}



/*
   show the pointer
   */

void ShowMousePointer()
{
    union REGS regs;
    
    regs.x.ax = 0x0001;
    int86(MOUSE, &regs, &regs);
}



/*
   hide the pointer
   */

void HideMousePointer()
{
    union REGS regs;
    
    regs.x.ax = 0x0002;
    int86(MOUSE, &regs, &regs);
}


/*
   read pointer coordinates
   */

void GetMouseCoords(BCINT *x, BCINT *y, BCINT *buttons)
{
    union REGS regs;
    
    regs.x.ax = 0x0003;
    int86(MOUSE, &regs, &regs);
    if (x != NULL)
		*x = regs.x.cx;
    if (y != NULL)
		*y = regs.x.dx;
    if (buttons)
		*buttons = regs.x.bx;
}


/*
   change pointer coordinates
   */

void SetMouseCoords( BCINT x, BCINT y)
{
    union REGS regs;
    
    regs.x.ax = 0x0004;
    regs.x.cx = (UBCINT) x;
    regs.x.dx = (UBCINT) y;
    int86(MOUSE, &regs, &regs);
}



/*
   set horizontal and vertical limits (constrain pointer in a box)
   */

void SetMouseLimits( BCINT x0, BCINT y0, BCINT x1, BCINT y1)
{
    union REGS regs;
    
    regs.x.ax = 0x0007;
    regs.x.cx = (UBCINT) x0;
    regs.x.dx = (UBCINT) x1;
    int86(MOUSE, &regs, &regs);
    regs.x.ax = 0x0008;
    regs.x.cx = (UBCINT) y0;
    regs.x.dx = (UBCINT) y1;
    int86(MOUSE, &regs, &regs);
}



/*
   reset horizontal and vertical limits
   */

void ResetMouseLimits()
{
    union REGS regs;
    
    regs.x.ax = 0x0007;
    regs.x.cx = (UBCINT) 0;
    regs.x.dx = (UBCINT) ScrMaxX;
    int86(MOUSE, &regs, &regs);
    regs.x.ax = 0x0008;
    regs.x.cx = (UBCINT) 0;
    regs.x.dx = (UBCINT) ScrMaxY;
    int86(MOUSE, &regs, &regs);
}

#elif defined(__GNUC__)

#include <mousex.h>

void CheckMouseDriver()
{
    if (MouseDetect()) {
		UseMouse = TRUE;
		MouseEventMode(0);
		MouseInit();
		MouseSetColors(TranslateToGameColor(WHITE),GrNOCOLOR);
    }
    else {
		UseMouse = FALSE;
    }
}


void ShowMousePointer()
{
    MouseDisplayCursor();
    MouseEventEnable(0,1);
    
}



void HideMousePointer()
{
    MouseEraseCursor();
    MouseEventEnable(0,0);
}


void GetMouseCoords(BCINT *x, BCINT *y, BCINT *buttons)
{
    MouseEvent mevent;
    MouseGetEvent(M_POLL|M_BUTTON_DOWN|M_BUTTON_UP,&mevent);
    *x = mevent.x;
    *y = mevent.y;
    *buttons = mevent.buttons;
}



void SetMouseCoords( BCINT x, BCINT y)
{
    MouseWarp(x,y);
}



void SetMouseLimits( BCINT x0, BCINT y0, BCINT x1, BCINT y1)
{
    MouseSetLimits(x0,y0,x1,y1);
}



void ResetMouseLimits()
{
    MouseSetLimits(0,0,ScrMaxX,ScrMaxY);
}

#endif

/* end of file */
