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
Bool UseMouse;			/* is there a mouse driver? */


/*
   initialize the mouse driver
*/

void CheckMouseDriver()
{
   union  REGS  regs;
   struct SREGS sregs;

   regs.x.ax = 0x0000;
   int86(MOUSE, &regs, &regs);
   if (regs.x.ax == 0xffff)
   {
      UseMouse = TRUE; /* mouse */
#ifdef CIRRUS_PATCH
      /*
         note from RQ:
            This test is temporary and should be removed in DEU 5.3
            We should create a better "fake cursor" by using the
            mouse callback function.  Remember to remove the callback
            when DEU exits...
      */
      if (CirrusCursor == TRUE)
      {
         regs.x.ax = 0x000C;
         regs.x.cx = 0x0001;
         regs.x.dx = FP_OFF( MouseCallBackFunction);
         sregs.es  = FP_SEG( MouseCallBackFunction);
         int86x( MOUSE, &regs, &regs, &sregs);
      }
#endif /* CIRRUS_PATCH */
   }
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
   regs.x.dx = (unsigned) ScrMaxX;
   int86(MOUSE, &regs, &regs);
   regs.x.ax = 0x0008;
   regs.x.cx = (unsigned) 0;
   regs.x.dx = (unsigned) ScrMaxY;
   int86(MOUSE, &regs, &regs);
}


/*
   mouse callback function
*/

void MouseCallBackFunction()
{
#ifdef CIRRUS_PATCH
   if (CirrusCursor == TRUE)
      SetHWCursorPos(_CX, _DX);
#endif /* CIRRUS_PATCH */
}

/* end of file */
