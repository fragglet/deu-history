/*
   Texture display by Rapha‰l Quinet <quinet@montefiore.ulg.ac.be>
		  and Trevor Phillips <rphillip@cc.curtin.edu.au>

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   TEXTURES.C - Textures in 256 colors.
*/

/* the includes */
#include "deu.h"


/*
   display a floor or ceiling texture at coords x0, y0
*/

void DisplayFloorTexture( int x0, int y0, char *texname)
{
   MDirPtr             dir;
   int                 n;
   unsigned char huge *pixels;

   dir = FindMasterDir( MasterDir, texname);
   if (dir == NULL)
   {
      SetColor( DARKGRAY);
      DrawScreenLine( x0, y0, x0 + 63, y0 + 63);
      DrawScreenLine( x0, y0 + 63, x0 + 63, y0);
      return;
   }
   BasicWadSeek( dir->wadfile, dir->dir.start);
   pixels = GetFarMemory( 4100 * sizeof( char));
   BasicWadRead( dir->wadfile, &(pixels[ 4]), 4096L);
   if (GfxMode < -1)
   {
      /* Probably a bug in the VESA driver...    */
      /* It requires "size-1" instead of "size"! */
      ((unsigned int huge *)pixels)[ 0] = 63;
      ((unsigned int huge *)pixels)[ 1] = 63;
   }
   else
   {
      ((unsigned int huge *)pixels)[ 0] = 64;
      ((unsigned int huge *)pixels)[ 1] = 64;
   }
   putimage( x0, y0, pixels, COPY_PUT);
   farfree( pixels);
}



/*
   display a picture "picname" at coords x0, y0
*/

void DisplayPic( int x0, int y0, char *picname)
{
   MDirPtr             dir;
   int                 xsize, ysize, xofs, yofs;
   int                 x, y;
   long                offset;
   unsigned char       srow, rowlen, pixel;
#ifndef OLD_PIX
   unsigned char huge *pixels;
   long                l;
   long huge          *offsets;
#endif

   dir = FindMasterDir( MasterDir, picname);
   if (dir == NULL)
   {
      SetColor( DARKGRAY);
      DrawScreenLine( x0, y0, x0 + 63, y0 + 63);
      DrawScreenLine( x0, y0 + 63, x0 + 63, y0);
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
#ifdef OLD_PIX
   for (x = 0; x < xsize && !bioskey( 1); x++)
   {
      /* Seek to point of next pointer. */
      BasicWadSeek( dir->wadfile, dir->dir.start + 8L + x * 4L);
      BasicWadRead( dir->wadfile, &offset, 4L);
      /* Seek to start of column data. */
      BasicWadSeek( dir->wadfile, dir->dir.start + offset);
      /* Read starting row. */
      BasicWadRead( dir->wadfile, &srow, 1L);
      while (srow != 255)
      {
	 /* Read length of row. */
	 BasicWadRead( dir->wadfile, &rowlen, 1L);
	 /* Read excess 0. */
	 BasicWadRead( dir->wadfile, &pixel, 1L);
	 for (y = 0; y < rowlen; y++)
	 {
	    /* Read next pixel colour. */
	    BasicWadRead( dir->wadfile, &pixel, 1L);
	    putpixel( x0 + xofs + x, y0 + yofs + srow + y, pixel);
	 }
	 /* Read excess 0. */
	 BasicWadRead( dir->wadfile, &pixel, 1L);
	 /* Read next starting row. */
	 BasicWadRead( dir->wadfile, &srow, 1L);
      }
   }
#else
   pixels = GetFarMemory( (xsize * ysize + 4) * sizeof( char));
   for (l = 0; l < xsize * ysize; l++)
      pixels[ 4 + l] = 0;
   offsets = GetFarMemory( xsize * sizeof( long));
   BasicWadRead( dir->wadfile, offsets, (long) xsize * 4L);
   for (x = 0; x < xsize && !bioskey( 1); x++)
   {
      /* Seek to start of column data. */
      BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ x]);
      /* Read starting row. */
      BasicWadRead( dir->wadfile, &srow, 1L);
      while (srow != 255)
      {
	 /* Read length of row. */
	 BasicWadRead( dir->wadfile, &rowlen, 1L);
	 /* Read excess 0. */
	 BasicWadRead( dir->wadfile, &pixel, 1L);
	 for (y = 0; y < rowlen; y++)
	 {
	    /* Read next pixel colour. */
	    BasicWadRead( dir->wadfile, &pixel, 1L);
	    l = (long) xofs + (long) x + (long) (yofs + srow + y) * (long) xsize;
	    if (l < (long) xsize * (long) ysize)
	       pixels[ 4L + l] = pixel;
	 }
	 /* Read excess 0. */
	 BasicWadRead( dir->wadfile, &pixel, 1L);
	 /* Read next starting row. */
	 BasicWadRead( dir->wadfile, &srow, 1L);
      }
   }
   farfree( offsets);
   if (GfxMode < -1)
   {
      /* Probably a bug in the VESA driver...    */
      /* It requires "size-1" instead of "size"! */
      ((unsigned int huge *)pixels)[ 0] = xsize - 1;
      ((unsigned int huge *)pixels)[ 1] = ysize - 1;
   }
   else
   {
      ((unsigned int huge *)pixels)[ 0] = xsize;
      ((unsigned int huge *)pixels)[ 1] = ysize;
   }
   putimage( x0, y0, pixels, COPY_PUT);
   farfree( pixels);
#endif
}



/*
   display a wall texture ("texture1" or "texture2" object) at coords x0, y0
*/

void DisplayWallTexture( int x0, int y0, char *texname)
{
   MDirPtr  dir, pdir;
   long    *offsets;
   int      n, xsize, ysize, xofs, yofs, fields, pnameind, junk;
   long     numtex, texofs;
   char     tname[9], picname[9];

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
      if (!strncmp(tname, texname, 8))
	 texofs = dir->dir.start + offsets[ n];
   }
   free( offsets);
   if (Registered && !texofs)
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
	 if (!strncmp( tname, texname, 8))
	    texofs = dir->dir.start + offsets[ n];
      }
      free( offsets);
   }

   /* texture name not found */
   if (!texofs)
      return;

   /* read the info for this texture */
   BasicWadSeek(dir->wadfile, texofs + 12L);
   BasicWadRead(dir->wadfile, &xsize, 2L);
   BasicWadRead(dir->wadfile, &ysize, 2L);
   BasicWadSeek(dir->wadfile, texofs + 20L);
   BasicWadRead(dir->wadfile, &fields, 2L);

   for (n = 0; n < fields; n++)
   {
      BasicWadSeek(dir->wadfile, texofs + 22L + n * 10L);
      BasicWadRead(dir->wadfile, &xofs, 2L);
      BasicWadRead(dir->wadfile, &yofs, 2L);
      BasicWadRead(dir->wadfile, &pnameind, 2L);
      BasicWadRead(dir->wadfile, &junk, 2L);  /* Junk should = 1. */
      BasicWadRead(dir->wadfile, &junk, 2L);  /* Junk should = 0. */
      /* OK, now look up the pic's name in the PNAMES entry. */
      pdir = FindMasterDir( MasterDir, "PNAMES");
      BasicWadSeek(pdir->wadfile, pdir->dir.start + 4L + pnameind * 8L);
      BasicWadRead(pdir->wadfile, &picname, 8L);
      picname[ 8] = '\0';
      DisplayPic(x0 + xofs, y0 + yofs, picname);
   }
}



/*
   choose a floor or ceiling texture
*/

void ChooseFloorTexture( int x0, int y0, char *prompt, int listsize, char **list, char *name)
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

void ChooseWallTexture( int x0, int y0, char *prompt, int listsize, char **list, char *name)
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

void ChooseSprite( int x0, int y0, char *prompt, char *sname)
{
   MDirPtr dir;
   int n, listsize;
   char **list;
   char *name;

   /* count the names */
   dir = FindMasterDir( MasterDir, "S_START");
   dir = dir->next;
   for (n = 0; dir && strcmp(dir->dir.name, "S_END"); n++)
      dir = dir->next;
   listsize = n;
   /* get the actual names from master dir. */
   dir = FindMasterDir( MasterDir, "S_START");
   dir = dir->next;
   list = GetMemory( listsize * sizeof( char *));
   for (n = 0; n < listsize; n++)
   {
      list[ n] = GetMemory( 9 * sizeof( char));
      strncpy( list[ n], dir->dir.name, 8);
      list[ n][ 8] = '\0';
      dir = dir->next;
   }
   qsort( list, listsize, sizeof( char *), SortSprites);
   name = GetMemory( 9 * sizeof( char));
   if (sname != NULL)
      strncpy( name, sname, 8);
   else
      strcpy( name, list[ 0]);
   if (UseMouse)
      HideMousePointer();
   SwitchToVGA256();
   /* if we only have a 320x200x256 VGA driver, we must change x0 and y0.  Yuck! */
   if (GfxMode > -2)
   {
      x0 = 0;
      y0 = -1;
   }
   InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 11, name, 256, 128, DisplayPic);
   SwitchToVGA16();
   if (UseMouse)
      ShowMousePointer();
   free( name);
   for (n = 0; n < listsize; n++)
      free( list[ n]);
   free( list);
}


