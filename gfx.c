/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   GFX.C - Graphics routines.
*/

/* the includes */
#include "deu.h"
#include <math.h>
#include <dos.h>

/* the global variables */
int GfxMode = 0;	/* graphics mode number, or 0 for text */
			/* 1 = 320x200, 2 = 640x480, 3 = 800x600, 4 = 1024x768 */
			/* positive = 16 colors, negative = 256 colors */
int OrigX;		/* the X origin */
int OrigY;		/* the Y origin */
int Scale;		/* the scale value */
int PointerX;		/* X position of pointer */
int PointerY;		/* Y position of pointer */
int ScrMaxX;		/* maximum X screen coord */
int ScrMaxY;		/* maximum Y screen coord */
int ScrCenterX;		/* X coord of screen center */
int ScrCenterY;		/* Y coord of screen center */


/*
   initialise the graphics display
*/

void InitGfx()
{
   static Bool firsttime = TRUE;
   static int  gdriver;
   static int  gmode;
   int         errorcode = grNoInitGraph;

   printf( "Switching to graphics mode...\n");
   if (firsttime)
   {
      if (VideoMode > 0)
      {
	 gdriver = installuserdriver( BGIDriver, NULL);
	 gmode = VideoMode;
	 initgraph( &gdriver, &gmode, NULL);
	 errorcode = graphresult();
      }
      if (errorcode != grOk)
      {
	 gdriver = VGA;
	 gmode = VGAHI;
      }
   }
   if (gdriver == VGA || !firsttime)
   {
      initgraph( &gdriver, &gmode, NULL);
      errorcode = graphresult();
      if (errorcode != grOk)
	 ProgError( "graphics error: %s", grapherrormsg( errorcode));
   }
   if (gdriver == VGA)
      GfxMode = 2; /* 640x480x16 */
   else
   {
      GfxMode = -gmode; /* 640x480x256, 800x600x256, or 1024x768x256 */
      SetDoomPalette( 0);
   }
   setlinestyle( 0, 0, 1);
   setbkcolor( TranslateToDoomColor( BLACK));
   settextstyle( 0, 0, 1);
   firsttime = FALSE;
   ScrMaxX = getmaxx();
   ScrMaxY = getmaxy();
   ScrCenterX = ScrMaxX / 2;
   ScrCenterY = ScrMaxY / 2;
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
   switch from VGA 16 colours to VGA 256 colours
*/

Bool SwitchToVGA256()
{
   static int gdriver = -1;
   int gmode, errorcode;

   if (GfxMode > 0 && gdriver != VGA) /* if 16 colors and not failed before */
   {
      if (gdriver == -1)
      {
	 gdriver = installuserdriver( "VGA256", NULL);
	 errorcode = graphresult();
      }
      if (UseMouse)
	 HideMousePointer();
      closegraph();
      gmode = 0;
      initgraph( &gdriver, &gmode, NULL);
      errorcode = graphresult();
      if (errorcode != grOk)
      {
	 /* failed for 256 colors - back to 16 colors */
	 gdriver = VGA;
	 gmode = VGAHI;
	 initgraph( &gdriver, &gmode, NULL);
	 errorcode = graphresult();
      }
      if (errorcode != grOk) /* shouldn't happen */
	 ProgError( "graphics error: %s", grapherrormsg( errorcode));
      if (UseMouse)
	 ShowMousePointer();
      GfxMode = -1 /* 320x200x256 */;
      SetDoomPalette( 0);
      ScrMaxX = getmaxx();
      ScrMaxY = getmaxy();
      ScrCenterX = ScrMaxX / 2;
      ScrCenterY = ScrMaxY / 2;
      return TRUE;
   }
   return FALSE;
}



/*
   switch from VGA 256 colours to VGA 16 colours
*/

Bool SwitchToVGA16()
{
   int gdriver, gmode, errorcode;

   if (GfxMode == -1) /* switch only if we are in 320x200x256 colors */
   {
      if (UseMouse)
	 HideMousePointer();
      closegraph();
      gdriver = VGA;
      gmode = VGAHI;
      initgraph( &gdriver, &gmode, NULL);
      errorcode = graphresult();
      if (errorcode != grOk) /* shouldn't happen */
	 ProgError( "graphics error: %s", grapherrormsg( errorcode));
      if (UseMouse)
	 ShowMousePointer();
      GfxMode = 2; /* 640x480x16 */
      ScrMaxX = getmaxx();
      ScrMaxY = getmaxy();
      ScrCenterX = ScrMaxX / 2;
      ScrCenterY = ScrMaxY / 2;
      return TRUE;
   }
   return FALSE;
}



/*
   clear the screen
*/

void ClearScreen()
{
   cleardevice();
}



/*
   set the current drawing color
*/

void SetColor( int color)
{
   if (GfxMode < 0)
      setcolor( TranslateToDoomColor(color));
   else
      setcolor( color);
}



/*
   draw a line on the screen from map coords
*/

void DrawMapLine( int mapXstart, int mapYstart, int mapXend, int mapYend)
{
   line( (mapXstart - OrigX) / Scale + ScrCenterX,
	 (OrigY - mapYstart) / Scale + ScrCenterY,
	 (mapXend - OrigX)   / Scale + ScrCenterX,
	 (OrigY - mapYend)   / Scale + ScrCenterY);
}



/*
   draw a circle on the screen from map coords
*/

void DrawMapCircle( int mapXcenter, int mapYcenter, int mapRadius)
{
   circle( (mapXcenter - OrigX) / Scale + ScrCenterX,
	   (OrigY - mapYcenter) / Scale + ScrCenterY,
	   mapRadius / Scale);
}



/*
   draw an arrow on the screen from map coords
*/

void DrawMapVector( int mapXstart, int mapYstart, int mapXend, int mapYend)
{
   int    scrXstart = (mapXstart - OrigX) / Scale + ScrCenterX;
   int    scrYstart = (OrigY - mapYstart) / Scale + ScrCenterY;
   int    scrXend   = (mapXend - OrigX)   / Scale + ScrCenterX;
   int    scrYend   = (OrigY - mapYend)   / Scale + ScrCenterY;
   double r         = hypot( scrXstart - scrXend, scrYstart - scrYend);
   int    scrXoff   = (r >= 1.0) ? (scrXstart - scrXend) * 16.0 / r / Scale : 0;
   int    scrYoff   = (r >= 1.0) ? (scrYstart - scrYend) * 16.0 / r / Scale : 0;

   line( scrXstart, scrYstart, scrXend, scrYend);
   scrXstart = scrXend + 2 * scrXoff;
   scrYstart = scrYend + 2 * scrYoff;
   line( scrXstart - scrYoff, scrYstart + scrXoff, scrXend, scrYend);
   line( scrXstart + scrYoff, scrYstart - scrXoff, scrXend, scrYend);
/*
   line( scrXstart - scrYoff, scrYstart + scrXoff, scrXend + scrXoff, scrYend + scrYoff);
   line( scrXstart + scrYoff, scrYstart - scrXoff, scrXend + scrXoff, scrYend + scrYoff);
*/
}



/*
   draw an arrow on the screen from map coords and angle (0 - 65535)
*/

void DrawMapArrow( int mapXstart, int mapYstart, unsigned angle)
{
   int    mapXend   = mapXstart + 50 * cos(angle / 10430.37835);
   int    mapYend   = mapYstart + 50 * sin(angle / 10430.37835);
   int    scrXstart = (mapXstart - OrigX) / Scale + ScrCenterX;
   int    scrYstart = (OrigY - mapYstart) / Scale + ScrCenterY;
   int    scrXend   = (mapXend - OrigX)   / Scale + ScrCenterX;
   int    scrYend   = (OrigY - mapYend)   / Scale + ScrCenterY;
   double r         = hypot( scrXstart - scrXend, scrYstart - scrYend);
   int    scrXoff   = (r >= 1.0) ? (scrXstart - scrXend) * 16.0 / r / Scale : 0;
   int    scrYoff   = (r >= 1.0) ? (scrYstart - scrYend) * 16.0 / r / Scale : 0;

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
   setfillstyle( 1, TranslateToDoomColor( LIGHTGRAY));
   bar( Xstart + 1, Ystart + 1, Xend - 1, Yend - 1);
   SetColor( DARKGRAY);
   line( Xstart, Yend, Xend, Yend);
   line( Xend, Ystart, Xend, Yend);
   if (Xend - Xstart > 20 && Yend - Ystart > 20)
   {
      line( Xstart + 1, Yend - 1, Xend - 1, Yend - 1);
      line( Xend - 1, Ystart + 1, Xend - 1, Yend - 1);
      SetColor( WHITE);
      line( Xstart + 1, Ystart + 1, Xstart + 1, Yend - 1);
      line( Xstart + 1, Ystart + 1, Xend - 1, Ystart + 1);
   }
   SetColor( WHITE);
   line( Xstart, Ystart, Xend, Ystart);
   line( Xstart, Ystart, Xstart, Yend);
   SetColor( BLACK);
}



/*
   draw a hollow 3D-box on the screen from screen coords
*/

void DrawScreenBoxHollow( int Xstart, int Ystart, int Xend, int Yend)
{
   setfillstyle( 1, TranslateToDoomColor( BLACK));
   bar( Xstart + 1, Ystart + 1, Xend - 1, Yend - 1);
   SetColor( WHITE);
   line( Xstart, Yend, Xend, Yend);
   line( Xend, Ystart, Xend, Yend);
   if (Xend - Xstart > 20 && Yend - Ystart > 20)
   {
      line( Xstart + 1, Yend - 1, Xend - 1, Yend - 1);
      line( Xend - 1, Ystart + 1, Xend - 1, Yend - 1);
      SetColor( DARKGRAY);
      line( Xstart + 1, Ystart + 1, Xstart + 1, Yend - 1);
      line( Xstart + 1, Ystart + 1, Xend - 1, Ystart + 1);
   }
   SetColor( DARKGRAY);
   line( Xstart, Ystart, Xend, Ystart);
   line( Xstart, Ystart, Xstart, Yend);
   SetColor( WHITE);
}



/*
   draw a meter bar on the screen from screen coords (in a hollow box); max. value = 1.0
*/

void DrawScreenMeter( int Xstart, int Ystart, int Xend, int Yend, float value)
{
   if (value < 0.0)
      value = 0.0;
   if (value > 1.0)
      value = 1.0;
   setfillstyle( 1, TranslateToDoomColor( BLACK));
   bar( Xstart + 1 + (int) ((Xend - Xstart - 2) * value), Ystart + 1, Xend - 1, Yend - 1);
   setfillstyle( 1, TranslateToDoomColor( LIGHTGREEN));
   bar( Xstart + 1, Ystart + 1, Xstart + 1 + (int) ((Xend - Xstart - 2) * value), Yend - 1);
}



/*
   write text to the screen
*/

void DrawScreenText( int Xstart, int Ystart, char *msg, ...)
{
   static int lastX;
   static int lastY;
   char temp[ 120];
   va_list args;

   va_start( args, msg);
   vsprintf( temp, msg, args);
   va_end( args);
   if (Xstart < 0)
      Xstart = lastX;
   if (Ystart < 0)
      Ystart = lastY;
   outtextxy( Xstart, Ystart, temp);
   lastX = Xstart;
   lastY = Ystart + 10;  /* or textheight("W") ? */
}



/*
   draw (or erase) the pointer if we aren't using the mouse
*/

void DrawPointer()
{
   /* use XOR mode : drawing the pointer twice erases it */
   setwritemode( XOR_PUT);
   /* draw the pointer */
   SetColor( YELLOW);
   DrawScreenLine( PointerX - 15, PointerY - 13, PointerX + 15, PointerY + 13);
   DrawScreenLine( PointerX - 15, PointerY + 13, PointerX + 15, PointerY - 13);
   /* restore normal write mode */
   setwritemode( COPY_PUT);
}



/*
   load one "playpal" palette and change all palette colours
*/

void SetDoomPalette( int playpalnum)
{
   MDirPtr             dir;
   unsigned char huge *dpal;
   int                 n;

   if (playpalnum < 0 && playpalnum > 13)
      return;
   dir = FindMasterDir( MasterDir, "PLAYPAL");
   if (dir)
   {
      dpal = GetFarMemory( 768 * sizeof( char));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n <= playpalnum; n++)
	 BasicWadRead( dir->wadfile, dpal, 768L);
      for (n = 0; n < 768; n++)
	 dpal[n] /= 4;
      _AX = 0x1012;
      _BX = 0;
      _CX = 256;
      _ES = FP_SEG(dpal);
      _DX = FP_OFF(dpal);
      __int__(0x10);
      farfree( dpal);
    }
}



/*
   translate a standard color to Doom palette 0 (approx.)
*/

int TranslateToDoomColor( int color)
{
   if (GfxMode < 0)
      switch (color)
      {
      case BLACK:
	 return 0;
      case BLUE:
	 return 202;
      case GREEN:
	 return 118;
      case CYAN:
	 return 194;
      case RED:
	 return 183;
      case MAGENTA:
	 return 253;
      case BROWN:
	 return 144;
      case LIGHTGRAY:
	 return 88;
      case DARKGRAY:
	 return 96;
      case LIGHTBLUE:
	 return 197;
      case LIGHTGREEN:
	 return 112;
      case LIGHTCYAN:
	 return 193;
      case LIGHTRED:
	 return 176;
      case LIGHTMAGENTA:
	 return 250;
      case YELLOW:
	 return 231;
      case WHITE:
	 return 4;
      }
   return color;
}



/*
   translate (dx, dy) into an integer angle value (0-65535)
*/

unsigned ComputeAngle( int dx, int dy)
{
   return (unsigned) (atan2( (double) dy, (double) dx) * 10430.37835 + 0.5);
   /* Yes, I know this function could be in another file, but */
   /* this is the only source file that includes <math.h>...  */
}



/*
   compute the distance from (0, 0) to (dx, dy)
*/

unsigned ComputeDist( int dx, int dy)
{
   return (unsigned) (hypot( (double) dy, (double) dx) + 0.5);
   /* Yes, I know this function could be in another file, but */
   /* this is the only source file that includes <math.h>...  */
}



/*
   insert the vertices of a new polygon
*/

void InsertPolygonVertices( int centerx, int centery, int sides, int radius)
{
   int n;

   for (n = 0; n < sides; n++)
      InsertObject( OBJ_VERTEXES, -1, centerx + (int) ((double) radius * cos( 6.28 * (double) n / (double) sides)), centery + (int) ((double) radius * sin( 6.2832 * (double) n / (double) sides)));
   /* Yes, I know... etc. */
}

/* end of file */
