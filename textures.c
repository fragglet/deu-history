/*
   Texture display by Rapha‰l Quinet <quinet@montefiore.ulg.ac.be>,
		      Trevor Phillips <rphillip@cc.curtin.edu.au>,
		  and Christian Johannes Schladetsch <s924706@yallara.cs.rmit.OZ.AU>

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   TEXTURES.C - Textures in 256 colors.

   Note from CJS:
      DisplayPic() could be further speeded up by avoiding reading
      the same column numerous times. However, this approach involves
      exhorbitant memory usage for certain pictures.
*/

/* the includes */
#include "deu.h"


/*
   display a floor or ceiling texture at coords x0, y0 and not beyond x1, y1
*/

void DisplayFloorTexture( BCINT x0, BCINT y0, BCINT x1, BCINT y1, char *texname)
{
   MDirPtr             dir;
   unsigned char huge *pixels;

   dir = FindMasterDir( MasterDir, texname);
   if (dir == NULL)
   {
      SetColor( DARKGRAY);
      DrawScreenLine( x0, y0, x1, y1);
      DrawScreenLine( x0, y1, x1, y0);
      return;
   }
   BasicWadSeek( dir->wadfile, dir->dir.start);

#if defined(__TURBOC__)

   pixels = (unsigned char huge*) GetFarMemory( 4100 * sizeof( char));
   BasicWadRead( dir->wadfile, &(pixels[ 4]), 4096L);
   if (GfxMode < -1)
   {
      /* Probably a bug in the VESA driver...    */
      /* It requires "size-1" instead of "size"! */
      ((UBCINT huge *)pixels)[ 0] = 63;
      ((UBCINT huge *)pixels)[ 1] = 63;
   }
   else
   {
      ((UBCINT huge *)pixels)[ 0] = 64;
      ((UBCINT huge *)pixels)[ 1] = 64;
   }

#elif defined(__GNUC__)

   /* bcc2grx's getimage-bitmap has a lot more info in it's header
      than borland's */

   pixels = (unsigned char*) GetFarMemory( 4096+sizeof(GrContext));
   getimage( x0, y0, x0+63, y0+63, pixels);   /* Hack! */
   BasicWadRead( dir->wadfile, pixels+sizeof(GrContext), 4096L);

#endif

   putimage( x0, y0, pixels, COPY_PUT);
   FreeFarMemory( pixels);
}



/*
   display a picture "picname" at coords x0, y0 and not beyond x1, y1
*/

void DisplayPic( BCINT x0, BCINT y0, BCINT x1, BCINT y1, char *picname)
{
   MDirPtr            dir;
   BCINT              xsize, ysize, xofs, yofs;
   BCINT              x, y;

   unsigned char huge *lpColumnData;
   unsigned char huge *lpColumn;
   long	   	 huge  *lpNeededOffsets;
   BCINT		          nColumns, nCurrentColumn;
   long               lCurrentOffset;
   BCINT		          fColumnInMemory;
   BCINT		          i, n;
   unsigned char      bRowStart, bColored;

   if (bioskey( 1) != 0)
      return; /* speedup */
   dir = FindMasterDir( MasterDir, picname);
   if (dir == NULL)
   {
      SetColor( DARKGRAY);
      DrawScreenLine( x0, y0, x1, y1);
      DrawScreenLine( x0, y1, x1, y0);
      return;
   }
   BasicWadSeek( dir->wadfile, dir->dir.start);
   BasicWadRead( dir->wadfile, &xsize, 2L);
   BasicWadRead( dir->wadfile, &ysize, 2L);
   BasicWadRead( dir->wadfile, &xofs, 2L);
   BasicWadRead( dir->wadfile, &yofs, 2L);
   /* ignore the picture offsets */
   xofs = 0;
   yofs = 0;

#define TEX_COLUMNBUFFERSIZE	(60L * 1024L)
#define TEX_COLUMNSIZE	  	512L

   nColumns = xsize;

   lpColumnData    = (unsigned char huge*) GetMemory( TEX_COLUMNBUFFERSIZE);
   lpNeededOffsets = (long huge*) GetMemory( nColumns * 4L);

   BasicWadRead( dir->wadfile, lpNeededOffsets, nColumns * 4L);

   /* read first column data, and subsequent column data */
   BasicWadSeek( dir->wadfile, dir->dir.start + lpNeededOffsets[ 0]);
   BasicWadRead( dir->wadfile, lpColumnData, TEX_COLUMNBUFFERSIZE);

   for (nCurrentColumn = 0; nCurrentColumn < nColumns; nCurrentColumn++)
   {
     lCurrentOffset = lpNeededOffsets[ nCurrentColumn];
     fColumnInMemory = lCurrentOffset >= lpNeededOffsets[ 0] && lCurrentOffset < (long)(lpNeededOffsets[ 0] + TEX_COLUMNBUFFERSIZE - TEX_COLUMNSIZE);
     if (fColumnInMemory)
     {
   	lpColumn = &lpColumnData[ lCurrentOffset - lpNeededOffsets[ 0]];
     }
     else
     {
   	lpColumn = (unsigned char huge*) GetFarMemory( TEX_COLUMNSIZE);
   	BasicWadSeek( dir->wadfile, dir->dir.start + lCurrentOffset);
   	BasicWadRead( dir->wadfile, lpColumn, TEX_COLUMNSIZE);
     }

     /* we now have the needed column data, one way or another, so write it */
     n = 1;
     bRowStart = lpColumn[ 0];
     while (bRowStart != 255 && n < TEX_COLUMNSIZE)
     {
   	bColored = lpColumn[ n];
   	n += 2;	      	     		/* skip over 'null' pixel in data */
   	for (i = 0; i < bColored; i++)
   	{
   	   x = x0 + xofs + nCurrentColumn;
   	   y = y0 + yofs + bRowStart + i;
   	   if (x >= x0 && y >= y0 && x <= x1 && y <= y1)
   	      putpixel(x, y, lpColumn[ i + n]);
   	}
   	n += bColored + 1;	/* skip over written pixels, and the 'null' one */
   	bRowStart = lpColumn[ n++];
     }
     if (bRowStart != 255)
   	ProgError( "BUG: bRowStart != 255.");

     if (!fColumnInMemory)
   	FreeFarMemory( lpColumn);
   }
   FreeMemory( lpColumnData);
   FreeMemory( lpNeededOffsets);
}



/*
   display a wall texture ("texture1" or "texture2" object) at coords x0, y0
*/

void DisplayWallTexture( BCINT x0, BCINT y0, BCINT x1, BCINT y1, char *texname)
{
   MDirPtr  dir, pdir;
   long    *offsets;
   BCINT    n, xsize, ysize, xofs, yofs, fields, pnameind, junk;
   long     numtex, texofs;
   char     tname[9], picname[9];

   if (bioskey( 1) != 0)
      return; /* speedup */

   /* offset for texture we want. */
   texofs = 0;
   /* search for texname in texture1 names */
   dir = FindMasterDir( MasterDir, "TEXTURE1");
   BasicWadSeek( dir->wadfile, dir->dir.start);
   BasicWadRead( dir->wadfile, &numtex, 4);
   /* read in the offsets for texture1 names and info. */
   offsets = (long*) GetMemory( numtex * sizeof( long));
   for (n = 0; n < numtex; n++)
      BasicWadRead( dir->wadfile, &(offsets[ n]), 4L);
   for (n = 0; n < numtex && !texofs; n++)
   {
      BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ n]);
      BasicWadRead( dir->wadfile, &tname, 8);
      if (!strnicmp( tname, texname, 8))
	 texofs = dir->dir.start + offsets[ n];
   }
   FreeMemory( offsets);
   if (Registered && texofs == 0)
   {
      /* search for texname in texture2 names */
      dir = FindMasterDir( MasterDir, "TEXTURE2");
      BasicWadSeek( dir->wadfile, dir->dir.start);
      BasicWadRead( dir->wadfile, &numtex, 4);
      /* read in the offsets for texture2 names */
      offsets = (long*) GetMemory( numtex * sizeof( long));
      for (n = 0; n < numtex; n++)
	 BasicWadRead( dir->wadfile, &(offsets[ n]), 4L);
      for (n = 0; n < numtex && !texofs; n++)
      {
	 BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ n]);
	 BasicWadRead( dir->wadfile, &tname, 8);
	 if (!strnicmp( tname, texname, 8))
	    texofs = dir->dir.start + offsets[ n];
      }
      FreeMemory( offsets);
   }

   /* clear the box where the texture size will be drawn - see below */
   SetColor( LIGHTGRAY);
   DrawScreenBox( x0 - 171, y0 + 40, x0 - 110, y0 + 50);

   /* texture name not found */
   if (texofs == 0)
      return;

   /* read the info for this texture */
   BasicWadSeek( dir->wadfile, texofs + 12L);
   BasicWadRead( dir->wadfile, &xsize, 2L);
   BasicWadRead( dir->wadfile, &ysize, 2L);
   BasicWadSeek( dir->wadfile, texofs + 20L);
   BasicWadRead( dir->wadfile, &fields, 2L);

   /* display the texture size - yes, you can laugh at the way I did it... */
   SetColor( BLACK);
   DrawScreenText( x0 - 171, y0 + 40, "%dx%d", xsize, ysize);

   if (bioskey( 1) != 0)
      return; /* speedup */

   if (x1 - x0 > xsize)
      x1 = x0 + xsize;
   if (y1 - y0 > ysize)
      y1 = y0 + ysize;
   /* not really necessary, except when xofs or yofs < 0 */
   setviewport( x0, y0, x1, y1, TRUE);
   /* display the texture */
   for (n = 0; n < fields; n++)
   {
      BasicWadSeek( dir->wadfile, texofs + 22L + n * 10L);
      BasicWadRead( dir->wadfile, &xofs, 2L);
      BasicWadRead( dir->wadfile, &yofs, 2L);
      BasicWadRead( dir->wadfile, &pnameind, 2L);
      BasicWadRead( dir->wadfile, &junk, 2L);  /* Junk should = 1. */
      BasicWadRead( dir->wadfile, &junk, 2L);  /* Junk should = 0. */
      /* OK, now look up the pic's name in the PNAMES entry. */
      pdir = FindMasterDir( MasterDir, "PNAMES");
      BasicWadSeek( pdir->wadfile, pdir->dir.start + 4L + pnameind * 8L);
      BasicWadRead( pdir->wadfile, &picname, 8L);
      picname[ 8] = '\0';
      /* coords changed because of the "setviewport" */
      DisplayPic( xofs, yofs, x1 - x0, y1 - y0, strupr( picname));
   }
   /* restore the normal viewport */
   setviewport( 0, 0, ScrMaxX, ScrMaxY, TRUE);
}



/*
    Function to get the size of a wall texture
*/

void GetWallTextureSize( BCINT *xsize_r, BCINT *ysize_r, char *texname)
{
    MDirPtr  dir;      /* pointer in main directory to texname */
    long    *offsets;  /* array of offsets to texture names */
    BCINT    n;        /* general counter */
    long     numtex;   /* number of texture names in TEXTURE* list */
    long     texofs;   /* offset in doom.wad for the texture data */
    char     tname[9]; /* texture name */

    /* offset for texture we want. */
    texofs = 0;
    /* search for texname in texture1 names */
    dir = FindMasterDir( MasterDir, "TEXTURE1");
    BasicWadSeek( dir->wadfile, dir->dir.start);
    BasicWadRead( dir->wadfile, &numtex, 4);
    /* read in the offsets for texture1 names and info. */
    offsets = GetMemory( numtex * sizeof( long));
    for (n = 0; n < numtex; n++)
       BasicWadRead( dir->wadfile, &(offsets[ n]), 4L);
    for (n = 0; n < numtex && !texofs; n++)
    {
       BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ n]);
       BasicWadRead( dir->wadfile, &tname, 8);
       if (!strnicmp(tname, texname, 8))
        texofs = dir->dir.start + offsets[ n];
    }
    FreeMemory( offsets);
    if (Registered && texofs == 0)
    {
       /* search for texname in texture2 names */
       dir = FindMasterDir( MasterDir, "TEXTURE2");
       BasicWadSeek( dir->wadfile, dir->dir.start);
       BasicWadRead( dir->wadfile, &numtex, 4);
       /* read in the offsets for texture2 names */
       offsets = GetMemory( numtex * sizeof( long));
       for (n = 0; n < numtex; n++)
         BasicWadRead( dir->wadfile, &(offsets[ n]), 4L);
       for (n = 0; n < numtex && !texofs; n++)
       {
         BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ n]);
         BasicWadRead( dir->wadfile, &tname, 8);
         if (!strnicmp( tname, texname, 8))
           texofs = dir->dir.start + offsets[ n];
       }
       FreeMemory( offsets);
    }

    if (texofs != 0)
    {
       /* read the info for this texture */
       BasicWadSeek( dir->wadfile, texofs + 12L);
       BasicWadRead( dir->wadfile, xsize_r, 2L);
       BasicWadRead( dir->wadfile, ysize_r, 2L);
    }
    else
    {
       /* texture data not found */
       *xsize_r = -1;
       *ysize_r = -1;
    }
}




/*
   choose a floor or ceiling texture
*/

void ChooseFloorTexture( BCINT x0, BCINT y0, char *prompt, BCINT listsize, char **list, char *name)
{
   if (UseMouse)
      HideMousePointer(); 
   SwitchToVGA256();
   /* if we only have a 320x200x256 VGA driver, we must change x0 and y0.  Yuck! */
   if (GfxMode > -2)
   {
      x0 = -1;
      y0 = -1;
   }
   InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 5, name, 64, 64, DisplayFloorTexture);
   SwitchToVGA16();
   if (UseMouse)
      ShowMousePointer();
}



/*
   choose a wall texture
*/

void ChooseWallTexture( BCINT x0, BCINT y0, char *prompt, BCINT listsize, char **list, char *name)
{
   if (UseMouse)
      HideMousePointer();
   SwitchToVGA256();
   /* if we only have a 320x200x256 VGA driver, we must change x0 and y0.  Yuck! */
   if (GfxMode > -2)
   {
      x0 = 0;
      y0 = -1;
   }
   InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 11, name, 256, 128, DisplayWallTexture);
   SwitchToVGA16();
   if (UseMouse)
      ShowMousePointer();
}



/*
   function used by qsort to sort the sprite names
*/
int SortSprites( const void *a, const void *b)
{
   return strcmp( *((char **)a), *((char **)b));
}



/*
   choose a "sprite"
*/

void ChooseSprite( BCINT x0, BCINT y0, char *prompt, char *sname)
{
   MDirPtr dir;
   BCINT n, listsize;
   char **list;
   char name[ 9];

   /* count the names */
   dir = FindMasterDir( MasterDir, "S_START");
   dir = dir->next;
   for (n = 0; dir && strcmp(dir->dir.name, "S_END"); n++)
      dir = dir->next;
   listsize = n;
   /* get the actual names from master dir. */
   dir = FindMasterDir( MasterDir, "S_START");
   dir = dir->next;
   list = (char**) GetMemory( listsize * sizeof( char *));
   for (n = 0; n < listsize; n++)
   {
      list[ n] = (char*) GetMemory( 9 * sizeof( char));
      strncpy( list[ n], dir->dir.name, 8);
      list[ n][ 8] = '\0';
      dir = dir->next;
   }
   qsort( list, listsize, sizeof( char *), SortSprites);
   if (sname != NULL)
      strncpy( name, sname, 8);
   else
      strcpy( name, list[ 0]);
/*   if (UseMouse)
      HideMousePointer(); */
   SwitchToVGA256();
   /* if we only have a 320x200x256 VGA driver, we must change x0 and y0.  Yuck! */
   if (GfxMode > -2)
   {
      x0 = 0;
      y0 = -1;
   }
   InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 11, name, 256, 128, DisplayPic);
/*   SwitchToVGA16();
   if (UseMouse)
      ShowMousePointer(); */
   for (n = 0; n < listsize; n++)
      FreeMemory( list[ n]);
   FreeMemory( list);
}


      
