/*
   Doom Editor Utility, by Brendon Wyber. Use and Abuse!

   GFX.C - Graphics routines.
*/

/* the includes */
#include "deu.h"

/* the global variables */
int GfxMode = 0;        /* in graphics mode */
int OrigX;              /* the X origin */
int OrigY;              /* the y origin */
int Scale;              /* the scale value */



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
   int scrXstart = (mapXstart - OrigX) / Scale + 320;
   int scrYstart = (OrigY - mapYstart) / Scale + 240;
   int scrXend   = (mapXend - OrigX)   / Scale + 320;
   int scrYend   = (OrigY - mapYend)   / Scale + 240;
   line( scrXstart, scrYstart, scrXend, scrYend);
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

/* end of file */
