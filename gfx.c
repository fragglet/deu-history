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

/* if your graphics driver doesn't like circles, draw squares instead */
#ifdef NO_CIRCLES
#define circle( x, y, r)	line( x - r, y - r, x - r, y + r); \
				line( x - r, y + r, x + r, y + r); \
				line( x + r, y + r, x + r, y - r); \
				line( x + r, y - r, x - r, y - r)
#endif /* NO_CIRCLES */

/* the global variables */
int GfxMode = 0;	/* graphics mode number, or 0 for text */
			/* 1 = 320x200, 2 = 640x480, 3 = 800x600, 4 = 1024x768 */
			/* positive = 16 colors, negative = 256 colors */
int OrigX;		/* the X origin */
int OrigY;		/* the Y origin */
float Scale;		/* the scale value */
int PointerX;		/* X position of pointer */
int PointerY;		/* Y position of pointer */
int ScrMaxX;		/* maximum X screen coord */
int ScrMaxY;		/* maximum Y screen coord */
int ScrCenterX;		/* X coord of screen center */
int ScrCenterY;		/* Y coord of screen center */

#ifdef CIRRUS_PATCH
char mp[ 256];

char HWCursor[] =
{
   0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
   0x30, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00,
   0x1F, 0x00, 0x00, 0x00, 0x1F, 0xC0, 0x00, 0x00,
   0x0F, 0xF0, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00,
   0x07, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00,
   0x03, 0x70, 0x00, 0x00, 0x02, 0x38, 0x00, 0x00,
   0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x3F, 0xFF, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF,
   0x83, 0xFF, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF,
   0xC0, 0x3F, 0xFF, 0xFF, 0xC0, 0x0F, 0xFF, 0xFF,
   0xE0, 0x07, 0xFF, 0xFF, 0xE0, 0x0F, 0xFF, 0xFF,
   0xF0, 0x1F, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF,
   0xF8, 0x07, 0xFF, 0xFF, 0xF8, 0x83, 0xFF, 0xFF,
   0xFD, 0xC7, 0xFF, 0xFF, 0xFF, 0xEF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
#endif /* CIRRUS_PATCH */


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
#ifdef CIRRUS_PATCH
   if (CirrusCursor == TRUE)
      SetHWCursorMap( HWCursor);
#endif /* CIRRUS_PATCH */
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
   line( SCREENX( mapXstart), SCREENY( mapYstart), SCREENX( mapXend), SCREENY( mapYend));
}



/*
   draw a circle on the screen from map coords
*/

void DrawMapCircle( int mapXcenter, int mapYcenter, int mapRadius)
{
   circle( SCREENX( mapXcenter), SCREENY( mapYcenter), (int) (mapRadius * Scale));
}



/*
   draw an arrow on the screen from map coords
*/

void DrawMapVector( int mapXstart, int mapYstart, int mapXend, int mapYend)
{
   int    scrXstart = SCREENX( mapXstart);
   int    scrYstart = SCREENY( mapYstart);
   int    scrXend   = SCREENX( mapXend);
   int    scrYend   = SCREENY( mapYend);
   double r         = hypot( (double) (scrXstart - scrXend), (double) (scrYstart - scrYend));
   int    scrXoff   = (r >= 1.0) ? (int) ((scrXstart - scrXend) * 8.0 / r * Scale) : 0;
   int    scrYoff   = (r >= 1.0) ? (int) ((scrYstart - scrYend) * 8.0 / r * Scale) : 0;

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
   int    mapXend   = mapXstart + (int) (50 * cos(angle / 10430.37835));
   int    mapYend   = mapYstart + (int) (50 * sin(angle / 10430.37835));
   int    scrXstart = SCREENX( mapXstart);
   int    scrYstart = SCREENY( mapYstart);
   int    scrXend   = SCREENX( mapXend);
   int    scrYend   = SCREENY( mapYend);
   double r         = hypot( scrXstart - scrXend, scrYstart - scrYend);
   int    scrXoff   = (r >= 1.0) ? (int) ((scrXstart - scrXend) * 8.0 / r * Scale) : 0;
   int    scrYoff   = (r >= 1.0) ? (int) ((scrYstart - scrYend) * 8.0 / r * Scale) : 0;

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

void DrawPointer( Bool rulers)
{
   int r;

   /* use XOR mode : drawing the pointer twice erases it */
   setwritemode( XOR_PUT);
   /* draw the pointer */
   if ( rulers)
   {
      SetColor( MAGENTA);
      r = (int) (512 * Scale);
      circle( PointerX, PointerY, r);
      r >>= 1;
      circle( PointerX, PointerY, r);
      r >>= 1;
      circle( PointerX, PointerY, r);
      r >>= 1;
      circle( PointerX, PointerY, r);
      r = (int) (1024 * Scale);
      line( PointerX - r, PointerY, PointerX + r, PointerY);
      line( PointerX, PointerY - r, PointerX, PointerY + r);
   }
   else
   {
      SetColor( YELLOW);
      line( PointerX - 15, PointerY - 13, PointerX + 15, PointerY + 13);
      line( PointerX - 15, PointerY + 13, PointerX + 15, PointerY - 13);
   }
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
	 dpal[ n] /= 4;
      _AX = 0x1012;
      _BX = 0;
      _CX = 256;
      _ES = FP_SEG( dpal);
      _DX = FP_OFF( dpal);
      __int__( 0x10);
      FreeFarMemory( dpal);
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
   return (unsigned) (hypot( (double) dx, (double) dy) + 0.5);
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



/*
   move (x, y) to a new position: rotate and scale around (0, 0)
*/

void RotateAndScaleCoords( int *x, int *y, double angle, double scale)
{
   double r, theta;

   r = hypot( (double) *x, (double) *y);
   theta = atan2( (double) *y, (double) *x);
   *x = (int) (r * scale * cos( theta + angle) + 0.5);
   *y = (int) (r * scale * sin( theta + angle) + 0.5);
   /* Yes, I know... etc. */
}



#ifdef CIRRUS_PATCH
/*
   Cirrus Logic Hardware Mouse Cursor Stuff
*/

#define CRTC 0x3D4
#define ATTR 0x3C0
#define SEQ  0x3C4
#define GRC  0x3CE
#define LOBYTE(w)           ((unsigned char)(w))
#define HIBYTE(w)           ((unsigned char)((unsigned int)(w) >> 8))

unsigned rdinx( unsigned pt, unsigned inx)
{
   if (pt == ATTR)
      inportb( CRTC + 6);
   outportb( pt, inx);
   return inportb( pt + 1);
}

void wrinx( int pt, unsigned inx, unsigned val)
{
   if (pt == ATTR)
   {
      inportb( CRTC + 6);
      outportb( pt, inx);
      outportb( pt, val);
   }
   else
   {
      outportb( pt, inx);
      outportb( pt + 1, val);
   }
}

void modinx( unsigned pt, unsigned inx, unsigned mask, unsigned nwv)
{
   unsigned temp;

   temp = (rdinx( pt, inx) & ~mask) + (nwv & mask);
   wrinx( pt, inx, temp);
}

void clrinx( unsigned pt, unsigned inx, unsigned val)
{
   unsigned x;

   x = rdinx( pt, inx);
   wrinx( pt, inx, x & ~val);
}

void SetHWCursorPos( unsigned x, unsigned y)
{
   outport( SEQ, (x << 5) | 0x10);
   outport( SEQ, (y << 5) | 0x11);
}

void SetHWCursorCol( long fgcol, long bgcol)
{
   modinx( SEQ, 0x12, 3, 2);
   outportb( 0x3C8, 0xFF);
   outportb( 0x3C9, LOBYTE( fgcol) >> 2);
   outportb( 0x3C9, HIBYTE( bgcol) >> 2);
   outportb( 0x3C8, 0);
   outportb( 0x3C9, LOBYTE( bgcol) >> 2);
   outportb( 0x3C9, HIBYTE( bgcol) >> 2);
   outportb( 0x3C9, bgcol >> 18);
   modinx( SEQ, 0x12, 3, 1);
}

void CopyHWCursorMap( unsigned bytes)
{
   char    *curmapptr = 0xA000FF00L;
   unsigned lbank = (1024 / 64) - 1;

   if ((rdinx( GRC, 0x0B) & 32)==0)
      lbank = lbank << 2;
   wrinx( GRC, 9, lbank << 2);
   memmove( curmapptr, &mp, bytes);
}

void SetHWCursorMap( char *map)
{
   memmove( &mp, map, 128);
   memmove( &mp + 128, &mp, 128);
   CopyHWCursorMap( 256);
   SetHWCursorCol( 0xFF00000L, 0xFF);
   wrinx( SEQ, 0x13, 0x3F);
}

#endif /* CIRRUS_PATCH */



/* end of file */
