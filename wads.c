/*
   Doom Editor Utility, by Brendon Wyber and Rapha�l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   WAD.C - Wad files routines.
*/

/* the includes */
#include "deu.h"

/* global variables */
WadPtr WadFileList = NULL;       /* linked list of wad files */
MDirPtr MasterDir = NULL;        /* the master directory */


/*
   open the main wad file, read in its directory and create the
   master directory
*/

void OpenMainWad( char *filename)
{
   MDirPtr lastp, newp;
   long n;
   WadPtr wad;

   /* open the wad file */
   printf( "\nLoading main WAD file: %s...\n", filename);
   wad = BasicWadOpen( filename);
   if (strncmp( wad->type, "IWAD", 4))
      ProgError( "\"%s\" is not the main WAD file", filename);

   /* create the master directory */
   lastp = NULL;
   for (n = 0; n < wad->dirsize; n++)
   {
      newp = GetMemory( sizeof( struct MasterDirectory));
      newp->next = NULL;
      newp->wadfile = wad;
      memcpy( &(newp->dir), &(wad->directory[ n]), sizeof( struct Directory));
      if (MasterDir)
	 lastp->next = newp;
      else
	 MasterDir = newp;
      lastp = newp;
   }

   /* check if registered version */
   if (FindMasterDir( MasterDir, "E2M1") == NULL)
   {
      printf( "   *-------------------------------------------------*\n");
      printf( "   | Warning: this is the shareware version of DOOM. |\n");
      printf( "   |   You won't be allowed to save your changes.    |\n");
      printf( "   |     PLEASE REGISTER YOUR COPY OF THE GAME.      |\n");
      printf( "   *-------------------------------------------------*\n");
      Registered = FALSE; /* If you remove this, bad things will happen to you... */
   }
   else
      Registered = TRUE;
}



/*
   open a patch wad file, read in its directory and alter the master
   directory
*/

void OpenPatchWad( char *filename)
{
   WadPtr wad;
   MDirPtr mdir;
   int n, l;
   char entryname[9];

   /* ignore the file if it doesn't exist */
   if (! Exists( filename))
   {
      printf( "Warning: patch WAD file \"%s\" doesn't exist.  Ignored.\n", filename);
      return;
   }

   /* open the wad file */
   printf( "Loading patch WAD file: %s...\n", filename);
   wad = BasicWadOpen( filename);
   if (strncmp( wad->type, "PWAD", 4))
      ProgError( "\"%s\" is not a patch WAD file", filename);

   /* alter the master directory */
   l = 0;
   for (n = 0; n < wad->dirsize; n++)
   {
      strncpy( entryname, wad->directory[ n].name, 8);
      entryname[8] = '\0';
      if (l > 0)
      {
	 mdir = mdir->next;
	 /* the level data should replace an existing level */
	 if (mdir == NULL || strncmp(mdir->dir.name, wad->directory[ n].name, 8))
	    ProgError( "\%s\" is not an understandable PWAD file (error with %s)", filename, entryname);
	 l--;
      }
      else
      {
	 mdir = FindMasterDir( MasterDir, wad->directory[ n].name);
	 /* if this entry is not in the master directory, then add it */
	 if (mdir == NULL)
	 {
	    printf( "   [Adding new entry %s]\n", entryname);
	    mdir = MasterDir;
	    while (mdir->next)
	       mdir = mdir->next;
	    mdir->next = GetMemory( sizeof( struct MasterDirectory));
	    mdir = mdir->next;
	    mdir->next = NULL;
	 }
	 /* if this is a level, then copy this entry and the next 10 */
	 else if (wad->directory[ n].name[ 0] == 'E' && wad->directory[ n].name[ 2] == 'M' && wad->directory[ n].name[ 4] == '\0')
	 {
	    printf( "   [Updating level %s]\n", entryname);
	    l = 10;
	 }
	 else
	    printf( "   [Updating entry %s]\n", entryname);
      }
      mdir->wadfile = wad;
      memcpy( &(mdir->dir), &(wad->directory[ n]), sizeof( struct Directory));
   }
}



/*
   close all the wad files, deallocating the WAD file structures
*/

void CloseWadFiles()
{
   WadPtr curw, nextw;
   MDirPtr curd, nextd;

   /* close the wad files */
   curw = WadFileList;
   WadFileList = NULL;
   while (curw)
   {
      nextw = curw->next;
      fclose( curw->fileinfo);
      free( curw->directory);
      free( curw);
      curw = nextw;
   }

   /* delete the master directory */
   curd = MasterDir;
   MasterDir = NULL;
   while (curd)
   {
      nextd = curd->next;
      free( curd);
      curd = nextd;
   }
}



/*
   forget unused patch wad files
*/

void CloseUnusedWadFiles()
{
   WadPtr curw, prevw;
   MDirPtr mdir;

   prevw = NULL;
   curw = WadFileList;
   while (curw)
   {
      /* check if the wad file is used by a directory entry */
      mdir = MasterDir;
      while (mdir && mdir->wadfile != curw)
	 mdir = mdir->next;
      if (mdir)
	 prevw = curw;
      else
      {
	 /* if this wad file is never used, close it */
	 if (prevw)
	    prevw->next = curw->next;
	 else
	    WadFileList = curw->next;
	 fclose( curw->fileinfo);
	 free( curw->directory);
	 free( curw);
      }
      curw = prevw->next;
   }
}



/*
   basic opening of WAD file and creation of node in Wad linked list
*/

WadPtr BasicWadOpen( char *filename)
{
   WadPtr curw, prevw;

   /* find the wad file in the wad file list */
   prevw = WadFileList;
   if (prevw)
   {
      curw = prevw->next;
      while (curw && strcmp( filename, curw->filename))
      {
	 prevw = curw;
	 curw = prevw->next;
      }
   }
   else
      curw = NULL;

   /* if this entry doesn't exist, add it to the WadFileList */
   if (curw == NULL)
   {
      curw = GetMemory( sizeof( struct WadFileInfo));
      if (prevw == NULL)
	 WadFileList = curw;
      else
	 prevw->next = curw;
      curw->next = NULL;
      curw->filename = filename;
   }

   /* open the file */
   if ((curw->fileinfo = fopen( filename, "rb")) == NULL)
      ProgError( "error opening \"%s\"", filename);

   /* read in the WAD directory info */
   BasicWadRead( curw, curw->type, 4);
   if (strncmp( curw->type, "IWAD", 4) && strncmp( curw->type, "PWAD", 4))
      ProgError( "\"%s\" is not a valid WAD file", filename);
   BasicWadRead( curw, &curw->dirsize, sizeof( curw->dirsize));
   BasicWadRead( curw, &curw->dirstart, sizeof( curw->dirstart));

   /* read in the WAD directory itself */
   curw->directory = GetMemory( sizeof( struct Directory) * curw->dirsize);
   BasicWadSeek( curw, curw->dirstart);
   BasicWadRead( curw, curw->directory, sizeof( struct Directory) * curw->dirsize);

   /* all done */
   return curw;
}



/*
   read bytes from a file and store it into an address with error checking
*/

void BasicWadRead( WadPtr wadfile, void huge *addr, long size)
{
   if (fread( addr, 1, size, wadfile->fileinfo) != size)
      ProgError( "error reading from \"%s\"", wadfile->filename);
}



/*
   go to offset of wad file with error checking
*/

void BasicWadSeek( WadPtr wadfile, long offset)
{
   if (fseek( wadfile->fileinfo, offset, 0))
      ProgError( "error reading from \"%s\"", wadfile->filename);
}



/*
   find an entry in the master directory
*/

MDirPtr FindMasterDir( MDirPtr from, char *name)
{
   while (from)
   {
      if (!strncmp( from->dir.name, name, 8))
	 break;
      from = from->next;
   }
   return from;
}



/*
   list the master directory
*/

void ListMasterDirectory( FILE *file)
{
   char dataname[ 9];
   MDirPtr dir;
   char key;
   int lines = 3;

   dataname[ 8] = '\0';
   fprintf( file, "The Master Directory\n");
   fprintf( file, "====================\n\n");
   fprintf( file, "NAME____  FILE________________  SIZE__  START____\n");
   for (dir = MasterDir; dir; dir = dir->next)
   {
      strncpy( dataname, dir->dir.name, 8);
      fprintf( file, "%-8s  %-20s  %6ld  x%08lx\n", dataname, dir->wadfile->filename, dir->dir.size, dir->dir.start);
      if (file == stdout && lines++ > 21)
      {
	 lines = 0;
         printf( "[Q to abort, any other key to continue]");
         key = getch();
	 printf( "\r                                       \r");
         if (key == 'Q' || key == 'q')
            break;
      }
   }
}



/*
   list the directory of a file
*/

void ListFileDirectory( FILE *file, WadPtr wad)
{
   char dataname[ 9];
   char key;
   int lines = 5;
   long n;

   dataname[ 8] = '\0';
   fprintf( file, "WAD File Directory\n");
   fprintf( file, "==================\n\n");
   fprintf( file, "Wad File: %s\n\n", wad->filename);
   fprintf( file, "NAME____  SIZE__  START____  END______\n");
   for (n = 0; n < wad->dirsize; n++)
   {
      strncpy( dataname, wad->directory[n].name, 8);
      fprintf( file, "%-8s  %6ld  x%08lx  x%08lx\n", dataname, wad->directory[n].size, wad->directory[n].start, wad->directory[n].size + wad->directory[n].start - 1);
      if (file == stdout && lines++ > 21)
      {
	 lines = 0;
	 printf( "[Q to abort, any other key to continue]");
	 key = getch();
	 printf( "\r                                       \r");
	 if (key == 'Q' || key == 'q')
	    break;
      }
   }
}



/*
   build a new wad file from master dictionary
*/

void BuildNewMainWad( char *filename, Bool patchonly)
{
   FILE *file;
   long counter = 12;
   MDirPtr cur;
   void huge *data;
   long size;
   long dirstart;
   long dirnum;

   /* open the file and store signatures */
   if (patchonly)
      printf( "Building a compound Patch Wad file \"%s\".\n", filename);
   else
      printf( "Building a new Main Wad file \"%s\" (size approx 10000K)\n", filename);
   if (FindMasterDir( MasterDir, "E2M4") == NULL)
      ProgError( "You were warned: you are not allowed to do this.");
   if ((file = fopen( filename, "wb")) == NULL)
      ProgError( "unable to open file \"%s\"", filename);
   if (patchonly)
      WriteBytes( file, "PWAD", 4);
   else
      WriteBytes( file, "IWAD", 4);
   WriteBytes( file, &counter, 4L);      /* put true value in later */
   WriteBytes( file, &counter, 4L);      /* put true value in later */

   /* output the directory data chuncks */
   data = GetFarMemory( 0x8000 + 2);
   for (cur = MasterDir; cur; cur = cur->next)
   {
      if (patchonly && cur->wadfile == WadFileList)
	 continue;
      size = cur->dir.size;
      counter += size;
      BasicWadSeek( cur->wadfile, cur->dir.start);
      while (size > 0x8000)
      {
	 BasicWadRead( cur->wadfile, data, 0x8000);
	 WriteBytes( file, data, 0x8000);
	 size -= 0x8000;
      }
      if (size)
      {
	 BasicWadRead( cur->wadfile, data, size);
	 WriteBytes( file, data, size);
      }
      printf( "Size: %dK\r", counter / 1024);
   }
   farfree( data);

   /* output the directory */
   dirstart = counter;
   counter = 12;
   dirnum = 0;
   for (cur = MasterDir; cur; cur = cur->next)
   {
      if (patchonly && cur->wadfile == WadFileList)
	 continue;
      if (dirnum % 100 == 0)
	 printf( "Outputting directory %04d...\r", dirnum);
      if (cur->dir.start)
	 WriteBytes( file, &counter, 4L);
      else
	 WriteBytes( file, &(cur->dir.start), 4L);
      WriteBytes( file, &(cur->dir.size), 4L);
      WriteBytes( file, &(cur->dir.name), 8L);
      counter += cur->dir.size;
      dirnum++;
   }

   /* fix up the number of entries and directory start information */
   if (fseek( file, 4L, 0))
      ProgError( "error writing to file");
   WriteBytes( file, &dirnum, 4L);
   WriteBytes( file, &dirstart, 4L);

   /* close the file */
   printf( "                            \r");
   fclose( file);
}



/*
   output bytes to a binary file with error checking
*/

void WriteBytes( FILE *file, void huge *addr, long size)
{
   if (! Registered)
      return;
   if (fwrite( addr, 1, size, file) != size)
      ProgError( "error writing to file");
}



/*
   check if a file exists and is readable
*/

Bool Exists( char *filename)
{
   FILE *test;

   if ((test = fopen( filename, "rb")) == NULL)
      return FALSE;
   fclose( test);
   return TRUE;
}



/*
   dump a directory entry in hex
*/

void DumpDirectoryEntry( FILE *file, char *entryname)
{
   MDirPtr entry;
   char dataname[ 9];
   char key;
   int lines = 5;
   long n, c, i;
   unsigned char buf[16];


   c = 0;
   entry = MasterDir;
   while (entry)
   {
      if (!strnicmp( entry->dir.name, entryname, 8))
      {
	 strncpy( dataname, entry->dir.name, 8);
	 dataname[ 8] = '\0';
	 fprintf( file, "Contents of entry %s (size = %ld bytes):\n", dataname, entry->dir.size);
	 BasicWadSeek( entry->wadfile, entry->dir.start);
	 n = 0;
	 for (c = 0; c < entry->dir.size; c += i)
	 {
	    fprintf( file, "%04X: ", n);
	    for (i = 0; i < 16; i++)
	    {
	       BasicWadRead( entry->wadfile, &(buf[ i]), 1);
	       fprintf( file, " %02X", buf[ i]);
	       n++;
	    }
	    fprintf( file, "   ");
	    for (i = 0; i < 16; i++)
	    {
	       if (buf[ i] >= 32)
		  fprintf( file, "%c", buf[ i]);
	       else
		  fprintf( file, " ");
	    }
	    fprintf( file, "\n");
	    if (file == stdout && lines++ > 21)
	    {
	       lines = 0;
	       printf( "[%d%% - Q to abort, S to skip this entry, any other key to continue]", n * 100 / entry->dir.size);
	       key = getch();
	       printf( "\r                                                                    \r");
	       if (key == 'S' || key == 's')
		  break;
	       if (key == 'Q' || key == 'q')
		  return;
	    }
	 }
      }
      entry = entry->next;
   }
   if (! c)
   {
      printf( "[Entry not in master directory]\n");
      return;
   }
}



/*
   save a directory entry to disk
*/

void SaveDirectoryEntry( FILE *file, char *entryname)
{
   MDirPtr entry;
   long    counter;
   long    size;
   void   *data;

   for (entry = MasterDir; entry; entry = entry->next)
      if (!strnicmp( entry->dir.name, entryname, 8))
	 break;
   if (entry)
   {
      WriteBytes( file, "PWAD", 4L);     /* PWAD file */
      counter = 1L;
      WriteBytes( file, &counter, 4L);   /* 1 entry */
      counter = 12L;
      WriteBytes( file, &counter, 4L);
      counter = 28L;
      WriteBytes( file, &counter, 4L);
      size = entry->dir.size;
      WriteBytes( file, &size, 4L);
      WriteBytes( file, &(entry->dir.name), 8L);
      data = GetMemory( 0x8000 + 2);
      BasicWadSeek( entry->wadfile, entry->dir.start);
      while (size > 0x8000)
      {
	 BasicWadRead( entry->wadfile, data, 0x8000);
	 WriteBytes( file, data, 0x8000);
	 size -= 0x8000;
      }
      BasicWadRead( entry->wadfile, data, size);
      WriteBytes( file, data, size);
   }
   else
   {
      printf( "[Entry not in master directory]\n");
      return;
   }
}



/* end of file */
