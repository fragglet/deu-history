/*
   Mouse interface by Rapha�l Quinet <quinet@montefiore.ulg.ac.be>

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...
   Put a credit notice somewhere with my name on it.  Thanks!  ;-)

   MOUSE.C - Mouse driver routines.
*/

/* the includes */
#include "deu.h"
#include <dos.h>

/* mouse interrupt number */
#define MOUSE 0x33

/* the global data */
Bool UseMouse;			/* is there a mouse driver? */


/*
   initialize the mouse driver
*/

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

void GetMouseCoords(int *x, int *y, int *buttons)
{
   union REGS regs;

   regs.x.ax = 0x0003;
   int86(MOUSE, &regs, &regs);
   if (x)
      *x = regs.x.cx;
   if (y)
      *y = regs.x.dx;
   if (buttons)
      *buttons = regs.x.bx;
}



/*
   change pointer coordinates
*/

void SetMouseCoords( int x, int y)
{
   union REGS regs;

   regs.x.ax = 0x0004;
   regs.x.cx = (unsigned) x;
   regs.x.dx = (unsigned) y;
   int86(MOUSE, &regs, &regs);
}



/*
   set horizontal and vertical limits (constrain pointer in a box)
*/

void SetMouseLimits( int x0, int y0, int x1, int y1)
{
   union REGS regs;

   regs.x.ax = 0x0007;
   regs.x.cx = (unsigned) x0;
   regs.x.dx = (unsigned) x1;
   int86(MOUSE, &regs, &regs);
   regs.x.ax = 0x0008;
   regs.x.cx = (unsigned) y0;
   regs.x.dx = (unsigned) y1;
   int86(MOUSE, &regs, &regs);
}



/*
   reset horizontal and vertical limits
*/

void ResetMouseLimits()
{
   union REGS regs;

   regs.x.ax = 0x0007;
   regs.x.cx = (unsigned) 0;
   regs.x.dx = (unsigned) 639;
   int86(MOUSE, &regs, &regs);
   regs.x.ax = 0x0008;
   regs.x.cx = (unsigned) 0;
   regs.x.dx = (unsigned) 479;
   int86(MOUSE, &regs, &regs);
}



/* end of file */
