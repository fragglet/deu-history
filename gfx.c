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

#define BCabs(x)		((x < 0) ? -(x) : (x))

BCINT Palette[32];

/* if your graphics driver doesn't like circles, draw squares instead */

void circle_or_square( int x, int y, int r)
{
    if(square_circles) {
		line( x - r, y - r, x - r, y + r);
		line( x - r, y + r, x + r, y + r);
		line( x + r, y + r, x + r, y - r);
		line( x + r, y - r, x - r, y - r);
    }
    else
		circle(x, y, r);
}


#define BGI_PATH "."

/* the global variables */
BCINT GfxMode = 0;		/* graphics mode number, or 0 for text */
/* 1 = 320x200, 2 = 640x480, 3 = 800x600, 4 = 1024x768 */
/* positive = 16 colors, negative = 256 colors */
BCINT OrigX;				/* the X origin */
BCINT OrigY;				/* the Y origin */
float Scale;				/* the scale value */
BCINT PointerX;				/* X position of pointer */
BCINT PointerY;				/* Y position of pointer */
BCINT ScrMaxX;				/* maximum X screen coord */
BCINT ScrMaxY;				/* maximum Y screen coord */
BCINT ScrCenterX;		/* X coord of screen center */
BCINT ScrCenterY;		/* Y coord of screen center */


#if defined(__GNUC__)
static int res[5][3] = {{640,480,16},{320,200,256},{640,480,256},
							{800,600,256},{1024,768,256}};
#endif


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
    
#if defined(__TURBOC__)
    
    if (firsttime) {
		if (VideoMode > 0) {
			gdriver = installuserdriver( BGIDriver, NULL);
			gmode = VideoMode;
			initgraph( &gdriver, &gmode, BGI_PATH);
			errorcode = graphresult();
		}
		if (errorcode != grOk) {
			gdriver = VGA;
			gmode = VGAHI;
		}
    }
    if (gdriver == VGA || !firsttime) {
		initgraph( &gdriver, &gmode, BGI_PATH);
		errorcode = graphresult();
		if (errorcode != grOk)
			ProgError( "graphics error: %s", grapherrormsg( errorcode));
    }
    if (gdriver == VGA)
		GfxMode = 2; /* 640x480x16 */
    else {
		GfxMode = -gmode; /* 640x480x256, 800x600x256, or 1024x768x256 */
		SetDoomPalette( 0);
    }
    
#elif defined(__GNUC__)
    
    FakeCursor = FALSE;
    
    if (VideoMode < 0 || VideoMode > 4)
		VideoMode = 0;
    set_BGI_mode_whc( &gdriver, &gmode, res[VideoMode][0], res[VideoMode][1], res[VideoMode][2]);
    initgraph( &gdriver, &gmode, BGI_PATH);
    errorcode = graphresult();
    if(errorcode != grOk)
		ProgError( "graphics error: %s", grapherrormsg( errorcode));
    
    if (VideoMode == 0)
		GfxMode = 2; /* 640x480x16 */
    else {
		GfxMode = -VideoMode; /* 640x480x256, 800x600x256, or 1024x768x256 */
		SetDoomPalette( 0);
    }
#endif
    
    setlinestyle( 0, 0, 1);
    setbkcolor( TranslateToGameColor( BLACK));
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
    if (GfxMode) {
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
    
#if defined(__TURBOC__)
    
    if (GfxMode > 0 && gdriver != VGA) /* if 16 colors and not failed before */ {
		if (gdriver == -1) {
			gdriver = installuserdriver( "VGA256", NULL);
			errorcode = graphresult();
		}
		closegraph();
		gmode = 0;
		initgraph( &gdriver, &gmode, BGI_PATH);
		errorcode = graphresult();
		if (errorcode != grOk) {
			/* failed for 256 colors - back to 16 colors */
			gdriver = VGA;
			gmode = VGAHI;
			initgraph( &gdriver, &gmode, BGI_PATH);
			errorcode = graphresult();
		}
	    if (errorcode != grOk) /* shouldn't happen */
			ProgError( "graphics error: %s", grapherrormsg( errorcode));
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

#elif defined(__GNUC__)

    if (GfxMode > 0) {
		closegraph();
		set_BGI_mode_whc( &gdriver, &gmode, res[1][0], res[1][1], res[1][2]);
		initgraph( &gdriver, &gmode, BGI_PATH);
		errorcode = graphresult();
		if(errorcode != grOk) {
			/* failed for 256 colors - back to 16 colors */
			set_BGI_mode_whc( &gdriver, &gmode, res[0][0], res[0][1], res[0][2]);
			initgraph( &gdriver, &gmode, BGI_PATH);
			errorcode = graphresult();
		}
		
		
		if (errorcode != grOk) /* shouldn't happen */
			ProgError( "graphics error: %s", grapherrormsg( errorcode));
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
#endif


/*
   switch from VGA 256 colours to VGA 16 colours
   */

Bool SwitchToVGA16()
{
    if (GfxMode == -1) { /* switch only if we are in 320x200x256 colors */ 
#if defined(__TURBOC__)
		int gdriver, gmode, errorcode;
		
		closegraph();
		gdriver = VGA;
		gmode = VGAHI;
		initgraph( &gdriver, &gmode, BGI_PATH);
		errorcode = graphresult();
		if (errorcode != grOk) /* shouldn't happen */
			ProgError( "graphics error: %s", grapherrormsg( errorcode));
		GfxMode = 2; /* 640x480x16 */
		ScrMaxX = getmaxx();
		ScrMaxY = getmaxy();
		ScrCenterX = ScrMaxX / 2;
		ScrCenterY = ScrMaxY / 2;
		
#elif defined(__GNUC__)
		TermGfx();    /* This is a hack, I just didn't have the time to */
		InitGfx();    /* find out why "the other way" didn't work ... */
		CheckMouseDriver();
		if (UseMouse)
			ShowMousePointer();
#endif
		
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
   clear the map screen
   */




/*AJB*/
void ClearMapScreen( BCINT maxy)
{
	setviewport(0, 17, ScrMaxX, ScrMaxY - maxy, FALSE);
	clearviewport();
	setviewport(0, 0, ScrMaxX, ScrMaxY, FALSE);
}



/*
   set the current drawing color
   */

void SetColor( BCINT color)
{
    if (GfxMode < 0)
		setcolor( TranslateToGameColor(color));
    else
		setcolor( color);
}



/*
   draw a line on the screen from map coords
   */

void DrawMapLine( BCINT mapXstart, BCINT mapYstart, BCINT mapXend, BCINT mapYend)
{
    line( SCREENX( mapXstart), SCREENY( mapYstart), SCREENX( mapXend), SCREENY( mapYend));
}



/*
   draw a circle on the screen from map coords
   */

void DrawMapCircle( BCINT mapXcenter, BCINT mapYcenter, BCINT mapRadius)
{
    circle( SCREENX( mapXcenter), SCREENY( mapYcenter), (int) (mapRadius * Scale));
}



/*
   draw an arrow on the screen from map coords
   */

void DrawMapVector( BCINT mapXstart, BCINT mapYstart, BCINT mapXend, BCINT mapYend)
{
	BCINT  scrXstart = SCREENX( mapXstart);
	BCINT  scrYstart = SCREENY( mapYstart);
	BCINT  scrXend   = SCREENX( mapXend);
	BCINT  scrYend   = SCREENY( mapYend);
	double r         = hypot( (double) (scrXstart - scrXend), (double) (scrYstart - scrYend));
	BCINT  scrXoff   = (r >= 1.0) ? (BCINT) ((scrXstart - scrXend) * 4.0 / r * Scale) : 0;
	BCINT  scrYoff   = (r >= 1.0) ? (BCINT) ((scrYstart - scrYend) * 4.0 / r * Scale) : 0;
	
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

void DrawMapArrow( BCINT mapXstart, BCINT mapYstart, UBCINT angle)
{
	BCINT  mapXend   = mapXstart + (BCINT) (50 * cos(angle / 10430.37835));
	BCINT  mapYend   = mapYstart + (BCINT) (50 * sin(angle / 10430.37835));
	BCINT  scrXstart = SCREENX( mapXstart);
	BCINT  scrYstart = SCREENY( mapYstart);
	BCINT  scrXend   = SCREENX( mapXend);
	BCINT  scrYend   = SCREENY( mapYend);
	double r         = hypot( scrXstart - scrXend, scrYstart - scrYend);
	BCINT  scrXoff   = (r >= 1.0) ? (BCINT) ((scrXstart - scrXend) * 4.0 / r * Scale) : 0;
	BCINT  scrYoff   = (r >= 1.0) ? (BCINT) ((scrYstart - scrYend) * 4.0 / r * Scale) : 0;
	
	line( scrXstart, scrYstart, scrXend, scrYend);
	scrXstart = scrXend + 2 * scrXoff;
	scrYstart = scrYend + 2 * scrYoff;
	line( scrXstart - scrYoff, scrYstart + scrXoff, scrXend, scrYend);
	line( scrXstart + scrYoff, scrYstart - scrXoff, scrXend, scrYend);
}


/*
   draw a line on the screen from screen coords
   */

void DrawScreenLine( BCINT Xstart, BCINT Ystart, BCINT Xend, BCINT Yend)
{
    line( Xstart, Ystart, Xend, Yend);
}



/*
   draw a filled in box on the screen from screen coords
   */

void DrawScreenBox( BCINT Xstart, BCINT Ystart, BCINT Xend, BCINT Yend)
{
    setfillstyle( 1, getcolor());
    bar( Xstart, Ystart, Xend, Yend);
}



/*
   draw a filled-in 3D-box on the screen from screen coords
   */

void DrawScreenBox3D( BCINT Xstart, BCINT Ystart, BCINT Xend, BCINT Yend)
{
    setfillstyle( 1, TranslateToGameColor(DARKGRAY));
    bar( Xstart + 1, Ystart + 1, Xend - 1, Yend - 1);
    SetColor(DARKERGRAY);
    line( Xstart, Yend, Xend, Yend);
    line( Xend, Ystart, Xend, Yend);
    /*if (Xend - Xstart > 20 && Yend - Ystart > 20) {
	  line( Xstart + 1, Yend - 1, Xend - 1, Yend - 1);
	  line( Xend - 1, Ystart + 1, Xend - 1, Yend - 1);
	  SetColor(LIGHTGRAY);
	  line( Xstart + 1, Ystart + 1, Xstart + 1, Yend - 1);
	  line( Xstart + 1, Ystart + 1, Xend - 1, Ystart + 1);
	  }*/
    SetColor(LIGHTGRAY);
    line( Xstart, Ystart, Xend, Ystart);
    line( Xstart, Ystart, Xstart, Yend);
    SetColor( BLACK);
}



/*
   draw a hollow 3D-box on the screen from screen coords
   */

void DrawScreenBoxHollow( BCINT Xstart, BCINT Ystart, BCINT Xend, BCINT Yend)
{
    setfillstyle( 1, TranslateToGameColor( BLACK));
    bar( Xstart + 1, Ystart + 1, Xend - 1, Yend - 1);
    SetColor(LIGHTGRAY);
    line( Xstart, Yend, Xend, Yend);
    line( Xend, Ystart, Xend, Yend);
    if (Xend - Xstart > 20 && Yend - Ystart > 20) {
		line( Xstart + 1, Yend - 1, Xend - 1, Yend - 1);
		line( Xend - 1, Ystart + 1, Xend - 1, Yend - 1);
		SetColor(DARKERGRAY);
		line( Xstart + 1, Ystart + 1, Xstart + 1, Yend - 1);
		line( Xstart + 1, Ystart + 1, Xend - 1, Ystart + 1);
    }
    SetColor(DARKERGRAY);
    line( Xstart, Ystart, Xend, Ystart);
    line( Xstart, Ystart, Xstart, Yend);
    SetColor(LIGHTGRAY);
}



/*
   draw a meter bar on the screen from screen coords (in a hollow box); max. value = 1.0
   */

void DrawScreenMeter( BCINT Xstart, BCINT Ystart, BCINT Xend, BCINT Yend, float value)
{
    if (value < 0.0)
		value = 0.0;
    if (value > 1.0)
		value = 1.0;
    setfillstyle( 1, TranslateToGameColor( BLACK));
    bar( Xstart + 1 + (BCINT) ((Xend - Xstart - 2) * value), Ystart + 1, Xend - 1, Yend - 1);
    setfillstyle( 1, TranslateToGameColor( LIGHTGREEN));
    bar( Xstart + 1, Ystart + 1, Xstart + 1 + (BCINT) ((Xend - Xstart - 2) * value), Yend - 1);
}



/*
   write text to the screen
   */

void DrawScreenText( BCINT Xstart, BCINT Ystart, char *msg, ...)
{
    static BCINT lastX;
    static BCINT lastY;
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
    BCINT r;
    
    /* use XOR mode : drawing the pointer twice erases it */
    setwritemode( XOR_PUT);
    /* draw the pointer */
    if ( rulers) {
		SetColor( MAGENTA);
		r = (BCINT) (512 * Scale);
		circle_or_square( PointerX, PointerY, r);
		r >>= 1;
		circle_or_square( PointerX, PointerY, r);
		r >>= 1;
		circle_or_square( PointerX, PointerY, r);
		r >>= 1;
		circle_or_square( PointerX, PointerY, r);
		r = (BCINT) (1024 * Scale);
		line( PointerX - r, PointerY, PointerX + r, PointerY);
		line( PointerX, PointerY - r, PointerX, PointerY + r);
    }
    else {
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

void SetDoomPalette( BCINT playpalnum)
{
    MDirPtr             dir;
    unsigned char huge *dpal;
    BCINT                 n;
    
    if (playpalnum < 0 && playpalnum > 13)
		return;
    dir = FindMasterDir( MasterDir, "PLAYPAL");
    if (dir) {
		dpal = (unsigned char*)GetFarMemory( 768 * sizeof( char));
		BasicWadSeek( dir->wadfile, dir->dir.start);
		for (n = 0; n <= playpalnum; n++)
			BasicWadRead( dir->wadfile, dpal, 768L);
		
#if defined(__GNUC__)
		
		GrResetColors();
		for(n=0;n<254;n++)
			GrAllocCell();
		
		for(n=0;n<256;n++)
			GrSetColor(n,dpal[3*n],dpal[3*n+1],dpal[3*n+2]);
		
#elif defined(__TURBOC__)
		
		for (n = 0; n < 768; n++)
			dpal[ n] /= 4;
		
		_AX = 0x1012;
		_BX = 0;
		_CX = 256;
		_ES = FP_SEG( dpal);
		_DX = FP_OFF( dpal);
		__int__( 0x10);
		
#endif
		
		FreeFarMemory( dpal );
    }
}



/*
   translate a standard color to Doom palette 0 (approx.)
   */

BCINT TranslateToGameColor( BCINT color)
{
	return Palette[color];
}



/*
   translate (dx, dy) into an integer angle value (0-65535)
   */

UBCINT ComputeAngle( BCINT dx, BCINT dy)
{
    return (UBCINT) (atan2( (double) dy, (double) dx) * 10430.37835 + 0.5);
    /* Yes, I know this function could be in another file, but */
    /* this is the only source file that includes <math.h>...  */
}



/*
   compute the distance from (0, 0) to (dx, dy)
   */

UBCINT ComputeDist( BCINT dx, BCINT dy)
{
    return (UBCINT) (hypot( (double) dx, (double) dy) + 0.5);
    /* Yes, I know this function could be in another file, but */
    /* this is the only source file that includes <math.h>...  */
}



/*
   insert the vertices of a new polygon
   */

void InsertPolygonVertices( BCINT centerx, BCINT centery, BCINT sides, BCINT radius)
{
    BCINT n;
    
    for (n = 0; n < sides; n++)
		InsertObject( OBJ_VERTEXES, -1, centerx + (BCINT) ((double) radius * cos( 6.28 * (double) n / (double) sides)), centery + (BCINT) ((double) radius * sin( 6.2832 * (double) n / (double) sides)));
    /* Yes, I know... etc. */
}



/*
   move (x, y) to a new position: rotate and scale around (0, 0)
   */

void RotateAndScaleCoords( BCINT *x, BCINT *y, double angle, double scale)
{
    double r, theta;
    
    r = hypot( (double) *x, (double) *y);
    theta = atan2( (double) *y, (double) *x);
    *x = (BCINT) (r * scale * cos( theta + angle) + 0.5);
    *y = (BCINT) (r * scale * sin( theta + angle) + 0.5);
    /* Yes, I know... etc. */
}

/* end of file */
