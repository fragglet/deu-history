/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   WAD.C - Wad files routines.
*/

/* the includes */
#include "deu.h"

/* global variables */
WadPtr WadFileList = NULL;       /* linked list of wad files */
MDirPtr MasterDir = NULL;        /* the master directory */
int Registered = FALSE;          /* registered or shareware game? */


/*
   open the wad file

   the first file in the list is the main wad file, the rest will be
   patch wad files.
*/

void OpenWadFiles( int number, char *list[])
{
   int n;
   OpenMainWad( list[ 0]);
   for (n = 1; n < number; n++)
      OpenPatchWad( list[ n]);
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
      free (curw->directory);
      free( curw);
      curw = nextw;
   }

   /* delete the master dictionary */
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
   open the main wad file, read in its directory and create the
   master directory
*/

void OpenMainWad( char *filename)
{
   MDirPtr last, new;
   long n;
   WadPtr wad;

   /* open the wad file */
   printf( "Loading main WAD file: %s...\n", filename);
   wad = BasicWadOpen( filename);
   if (strncmp( wad->type, "IWAD", 4))
      ProgError( "\"%s\" is not the main WAD file");

   /* create the master directory */
   last = NULL;
   for (n = 0; n < wad->dirsize; n++)
   {
      new = GetMemory( sizeof( struct MasterDirectory));
      new->next = NULL;
      new->wadfile = wad;
      memcpy( &(new->dir), &(wad->directory[ n]), sizeof( struct Directory));
      if (MasterDir)
	 last->next = new;
      else
	 MasterDir = new;
      last = new;
   }

   /* check if registered version */
   if (FindMasterDir( MasterDir, "E2M2") == NULL)
   {
      printf( "  *-------------------------------------------------*\n");
      printf( "  | Warning: this is the shareware version of DOOM. |\n");
      printf( "  |   You won't be allowed to save your changes.    |\n");
      printf( "  |     PLEASE REGISTER YOUR COPY OF THE GAME.      |\n");
      printf( "  *-------------------------------------------------*\n");
      Registered = FALSE; /* If you change this, bad things will happen to you... */
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
   int n;

   /* open the wad file */
   printf( "Loading patch WAD file: %s...\n", filename);
   wad = BasicWadOpen( filename);
   if (strncmp( wad->type, "PWAD", 4))
      ProgError( "\"%s\" is not a patch WAD file");

   /* check PWAD file just contains a level */
   if (wad->dirsize != 11 || wad->directory[ 0].name[ 0] != 'E' || wad->directory[ 0].name[ 2] != 'M' || wad->directory[ 0].name[ 4] != '\0')
      ProgError( "\%s\" is not a understandable PWAD file");

   /* alter the master directory */
   mdir = FindMasterDir( MasterDir, wad->directory[ 0].name);
   for (n = 0; n < 11; n++)
   {
      mdir->wadfile = wad;
      memcpy( &(mdir->dir), &(wad->directory[ n]), sizeof( struct Directory));
      mdir = mdir->next;
   }
}



/*
   basic opening of WAD file and creation of node in Wad linked list
*/

WadPtr BasicWadOpen( char *filename)
{
   WadPtr *list;

   /* find last link in wad file list */
   for (list = &WadFileList; *list; list = &(*list)->next)
      ;

   /* create the new WadFileList structure and fill in the values */
   *list = GetMemory( sizeof( struct WadFileInfo));
   (*list)->next = NULL;
   (*list)->filename = filename;

   /* open the file */
   if (((*list)->fileinfo = fopen( filename, "rb")) == NULL)
      ProgError( "error opening \"%s\"", filename);

   /* read in the WAD directory info */
   BasicWadRead( *list, (*list)->type, 4);
   if (strncmp( (*list)->type, "IWAD", 4) && strncmp( (*list)->type, "PWAD", 4))
      ProgError( "\"%s\" is not a valid WAD file", filename);
   BasicWadRead( *list, &(*list)->dirsize, sizeof( (*list)->dirsize));
   BasicWadRead( *list, &(*list)->dirstart, sizeof( (*list)->dirstart));

   /* read in the WAD directory itself */
   (*list)->directory = GetMemory( sizeof( struct Directory) * (*list)->dirsize);
   BasicWadSeek( *list, (*list)->dirstart);
   BasicWadRead( *list, (*list)->directory, sizeof( struct Directory) * (*list)->dirsize);

   /* all done */
   return *list;
}



/*
   read bytes from a file and store it into an address with error checking
*/

void BasicWadRead( WadPtr wadfile, void *addr, long size)
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

void BuildNewMainWad( char *filename)
{
   FILE *file;
   long counter = 12;
   MDirPtr cur;
   void *data;
   long size;
   long dirstart;
   long dirnum;

   /* open the file and store signatures */
   printf( "Building a new Main Wad file \"%s\" (size approx 10000K)\n", filename);
   if (FindMasterDir( MasterDir, "E2M4") == NULL)
      ProgError( "You were warned: you are not allowed to do this.");
   if ((file = fopen( filename, "wb")) == NULL)
      ProgError( "unable to open file \"%s\"", filename);
   WriteBytes( file, "IWAD", 4);
   WriteBytes( file, &(WadFileList->dirsize), 4L);
   WriteBytes( file, &counter, 4L);      /* put true value in later */

   /* output the directory data chuncks */
   data = GetMemory( 0x8000 + 2);
   for (cur = MasterDir; cur; cur = cur->next)
   {
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

   /* output the directory */
   dirstart = counter;
   counter = 12;
   for (cur = MasterDir, dirnum = 0; cur; cur = cur->next, dirnum++)
   {
      if (dirnum % 100 == 0)
	 printf( "Outputting directory %04d...\r", dirnum);
      if (cur->dir.start)
	 WriteBytes( file, &counter, 4L);
      else
         WriteBytes( file, &(cur->dir.start), 4L);
      WriteBytes( file, &(cur->dir.size), 4L);
      WriteBytes( file, &(cur->dir.name), 8L);
      counter += cur->dir.size;
   }

   /* fix up the directory start information */
   if (fseek( file, 8L, 0))
      ProgError( "error writing to file");
   WriteBytes( file, &dirstart, 4L);

   /* close the file */
   printf( "                            \r");
   fclose( file);
}



/*
   output bytes to a binary file with error checking
*/

void WriteBytes( FILE *file, void *addr, long size)
{
   if (! Registered)
      return;
   if (fwrite( addr, 1, size, file) != size)
      ProgError( "error writing to file");
}



/*
   dump a directory entry (* debug *)
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



/* end of file */
