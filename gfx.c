/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   GFX.C - Graphics routines.
*/

/* the includes */
#include "deu.h"
#include <math.h>

/* the global variables */
int GfxMode = 0;        /* in graphics mode */
int OrigX;              /* the X origin */
int OrigY;              /* the Y origin */
int Scale;              /* the scale value */
int PointerX;           /* X position of pointer */
int PointerY;           /* Y position of pointer */



/*
   initialise the graphics display
*/

void InitGfx()
{
   int gdriver = VGA, gmode = VGAHI, errorcode;
   initgraph( &gdriver, &gmode, NULL);
   errorcode = graphresult();
   if (errorcode != grOk)
      ProgError( "graphics error: %s", grapherrormsg( errorcode));
   setlinestyle( 0, 0, 1);
   setbkcolor( BLACK);
   settextstyle( 0, 0, 1);
   GfxMode = 1;
}



/*
   terminate the graphics display
*/

void TermGfx()
{
   if (GfxMode)
   {
      closegraph();
      GfxMode = 0;
   }
}



/*
   clear the screen
*/

void ClearScreen()
{
   cleardevice();
}



/*
   draw a line on the screen from map coords
*/

void DrawMapLine( int mapXstart, int mapYstart, int mapXend, int mapYend)
{
   int scrXstart = (mapXstart - OrigX) / Scale + 319;
   int scrYstart = (OrigY - mapYstart) / Scale + 239;
   int scrXend   = (mapXend - OrigX)   / Scale + 319;
   int scrYend   = (OrigY - mapYend)   / Scale + 239;
   line( scrXstart, scrYstart, scrXend, scrYend);
}



/*
   draw an arrow on the screen from map coords
*/

void DrawMapVector( int mapXstart, int mapYstart, int mapXend, int mapYend)
{
   int scrXstart = (mapXstart - OrigX) / Scale + 319;
   int scrYstart = (OrigY - mapYstart) / Scale + 239;
   int scrXend   = (mapXend - OrigX)   / Scale + 319;
   int scrYend   = (OrigY - mapYend)   / Scale + 239;
   int r         = abs(scrXstart - scrXend) + abs(scrYstart - scrYend); /* no sqrt! */
   int scrXoff   = r ? (scrXstart - scrXend) * 16 / r / Scale : 0;
   int scrYoff   = r ? (scrYstart - scrYend) * 16 / r / Scale : 0;

   line( scrXstart, scrYstart, scrXend, scrYend);
   scrXstart = scrXend + 2 * scrXoff;
   scrYstart = scrYend + 2 * scrYoff;
   line( scrXstart - scrYoff, scrYstart + scrXoff, scrXend, scrYend);
   line( scrXstart + scrYoff, scrYstart - scrXoff, scrXend, scrYend);
   line( scrXstart - scrYoff, scrYstart + scrXoff, scrXend + scrXoff, scrYend + scrYoff);
   line( scrXstart + scrYoff, scrYstart - scrXoff, scrXend + scrXoff, scrYend + scrYoff);
}



/*
   draw an arrow on the screen from map coords
*/

void DrawMapArrow( int mapXstart, int mapYstart, int angle)
{
   int mapXend = mapXstart + 50 * cos(angle / 10430.37835);
   int mapYend = mapYstart + 50 * sin(angle / 10430.37835);
   int scrXstart = (mapXstart - OrigX) / Scale + 319;
   int scrYstart = (OrigY - mapYstart) / Scale + 239;
   int scrXend   = (mapXend - OrigX)   / Scale + 319;
   int scrYend   = (OrigY - mapYend)   / Scale + 239;
   int r         = abs(scrXstart - scrXend) + abs(scrYstart - scrYend); /* no sqrt! */
   int scrXoff   = r ? (scrXstart - scrXend) * 16 / r / Scale : 0;
   int scrYoff   = r ? (scrYstart - scrYend) * 16 / r / Scale : 0;

   line( scrXstart, scrYstart, scrXend, scrYend);
   scrXstart = scrXend + 2 * scrXoff;
   scrYstart = scrYend + 2 * scrYoff;
   line( scrXstart - scrYoff, scrYstart + scrXoff, scrXend, scrYend);
   line( scrXstart + scrYoff, scrYstart - scrXoff, scrXend, scrYend);
}



/*
   draw a line on the screen from screen coords
*/

void DrawScreenLine( int Xstart, int Ystart, int Xend, int Yend)
{
   line( Xstart, Ystart, Xend, Yend);
}



/*
   draw a filled in box on the screen from screen coords
*/

void DrawScreenBox( int Xstart, int Ystart, int Xend, int Yend)
{
   setfillstyle( 1, getcolor());
   bar( Xstart, Ystart, Xend, Yend);
}



/*
   draw a filled-in 3D-box on the screen from screen coords
*/

void DrawScreenBox3D( int Xstart, int Ystart, int Xend, int Yend)
{
   setfillstyle( 1, LIGHTGRAY);
   bar( Xstart + 1, Ystart + 1, Xend - 1, Yend - 1);
   setcolor( DARKGRAY);
   line( Xstart, Yend, Xend, Yend);
   line( Xend, Ystart, Xend, Yend);
   if (Xend - Xstart > 20 && Yend - Ystart > 20)
   {
      line( Xstart + 1, Yend - 1, Xend - 1, Yend - 1);
      line( Xend - 1, Ystart + 1, Xend - 1, Yend - 1);
      setcolor( WHITE);
      line( Xstart + 1, Ystart + 1, Xstart + 1, Yend - 1);
      line( Xstart + 1, Ystart + 1, Xend - 1, Ystart + 1);
   }
   setcolor( WHITE);
   line( Xstart, Ystart, Xend, Ystart);
   line( Xstart, Ystart, Xstart, Yend);
   setcolor( BLACK);
}



/*
   write text to the screen
*/

void DrawScreenText( int Xstart, int Ystart, char *msg, ...)
{
   char temp[ 120];
   va_list args;
   va_start( args, msg);
   vsprintf( temp, msg, args);
   va_end( args);
   outtextxy( Xstart, Ystart, temp);
}



/*
   draw (or erase) the pointer if we aren't using the mouse
*/

void DrawPointer()
{
   /* use XOR mode : drawing the pointer twice erases it */
   setwritemode( XOR_PUT);
   /* draw the pointer */
   setcolor( YELLOW);
   DrawScreenLine( PointerX - 15, PointerY - 15, PointerX + 15, PointerY + 15);
   DrawScreenLine( PointerX - 15, PointerY + 15, PointerX + 15, PointerY - 15);
   /* restore normal write mode */
   setwritemode( COPY_PUT);
}



/*
   translate (dx, dy) into an integer angle value (0-65535)
*/
int ComputeAngle( int dx, int dy)
{
   return (int) (atan2( (double) dy, (double) dx) * 10430.37835);
   /* Yes, I know this function could be in another file, but */
   /* this is the only source file that includes <math.h>...  */
}


/* end of file */
