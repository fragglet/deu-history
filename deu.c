/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   DEU.C - Main program execution routines.

   compile with command
      tcc -Z -mc -r -G -O -f -enewdeu *.c \tc\lib\graphics.lib

*/

/* the includes */
#include "deu.h"



/*
   the main program
*/

int main( int argc, char *argv[])
{
   Credits( stdout);
   ParseArgs( argc, argv);
   OpenWadFiles( argc - 1, argv + 1);
   MainLoop();
   CloseWadFiles();
   return 0;
}



/*
   output the credits of the program to the specified file
*/

void Credits( FILE *where)
{
   fprintf( where, "New DEU: Doom Editor Utility, ver %s.\n", DEU_VERSION);
   fprintf( where, "By Rapha‰l Quinet (quinet@montefiore.ulg.ac.be).\n");
   fprintf( where, "Based on DEU by Brendon J Wyber (b.wyber@csc.canterbury.ac.nz),\n\n");
}



/*
   parse the command arguements
*/

void ParseArgs( int argc, char *argv[])
{
   if (argc < 2)
   {
      printf( "Usage:   %s  <wadfile> [<patchwadfile1> <patchwadfile2> ...]\n", argv[ 0]);
      exit( 1);
   }
}



/*
   play a fascinating tune
*/
void Beep()
{
   sound( 640);
   delay( 100);
   nosound();
}



/*
   terminate the program reporting an error
*/

void ProgError( char *errstr, ...)
{
   va_list args;

   TermGfx();
   va_start( args, errstr);
   printf( "\nProgram Error: *** ");
   vprintf( errstr, args);
   printf( " ***\n");
   va_end( args);
   Beep();
   ForgetLevelData();
   CloseWadFiles();
   exit( 5);
}



/*
   allocate memory with error checking
*/

void *GetMemory( size_t size)
{
   void *ret = malloc( size);
   if (!ret)
      ProgError( "out of memory (cannot allocate %u bytes)", size);
   return ret;
}



/*
   reallocate memory with error checking
*/

void *ResizeMemory( void *old, size_t size)
{
   void *ret = realloc( old, size);
   if (!ret)
      ProgError( "out of memory (cannot reallocate %u bytes)", size);
   return ret;
}



/*
   the main program menu loop
*/

void MainLoop()
{
   char input[ 120];
   char *com, *out;
   FILE *file;
   WadPtr wad;
   int game, level;

   for (;;)
   {
      /* get the input */
      printf( "\n[? for help]> ");
      gets( input);
      printf( "\n");
      strupr( input);

      /* eat the white space and get the first command word */
      com = strtok( input, " ");

      /* user just hit return */
      if (com == NULL)
         printf( "[Please enter a command.]\n");

      /* user inputting for help */
      else if (!strcmp( com, "?"))
      {
	 printf( "?                                 -- to display this text\n");
	 printf( "B[uild] <WadFile>                 -- to build a new main WAD file\n");
	 printf( "D[ump] <DirEntry> [outfile]       -- to dump a directory entry in hex\n");
	 printf( "E[dit] <game> <level> <WadFile>   -- to edit a game level saving results to\n");
	 printf( "                                          a patch wad file\n");
	 printf( "L[ist] <WadFile> [outfile]        -- to list the directory of a wadfile\n");
	 printf( "M[aster] [outfile]                -- to list the master directory\n");
	 printf( "Q[uit]                            -- to quit\n");
	 printf( "W[ads]                            -- to display the open wads\n");
      }

      /* user asked for list of open WAD files */
      else if (!strcmp( com, "WADS") || !strcmp( com, "W"))
      {
	 printf( "%-20s  IWAD  (Main wad file)\n", WadFileList->filename);
	 for (wad = WadFileList->next; wad; wad = wad->next)
	    printf( "%-20s  PWAD  (Patch wad file for game %c level %c)\n", wad->filename, wad->directory[ 0].name[ 1], wad->directory[ 0].name[ 3]);
      }

      /* user asked to quit */
      else if (!strcmp( com, "QUIT") || !strcmp( com, "Q"))
      {
	 if (! Registered)
	    printf("Remember to register your copy of DOOM!\n");
	 printf( "Goodbye...\n");
	 break;
      }

      /* user asked to edit a level */
      else if (!strcmp( com, "EDIT") || !strcmp( com, "E"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
         {
	    printf( "[Game episode number missing.]\n");
	    continue;
         }
         game = atoi( com);
         com = strtok( NULL, " ");
	 if (com == NULL)
	 {
            printf( "[Game level number missing.]\n");
            continue;
         }
	 if (game < 1 || game > 3)
	 {
	    printf( "[Invalid game episode number (%s).]\n", com);
	    continue;
	 }
	 level = atoi( com);
	 if (level < 1 || level> 9)
	 {
	    printf( "[Invalid game level number (%s).]\n", com);
	    continue;
	 }
	 out = strtok( NULL, " ");
	 if (out == NULL)
	 {
	    printf( "[Resulting Patch Wadfile name missing.]\n");
	    continue;
	 }
	 for (wad = WadFileList; wad; wad = wad->next)
	    if (!stricmp( out, wad->filename))
	       break;
	 if (wad)
	 {
	    printf( "[File \"%s\" is opened and cannot be overwritten.]\n", out);
	    continue;
	 }
	 EditLevel( game, level, out);
      }

      /* user asked to build a new main WAD file */
      else if (!strcmp( com, "BUILD") || !strcmp( com, "B"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Wad file name argument missing.]\n");
	    continue;
	 }
	 for (wad = WadFileList; wad; wad = wad->next)
	    if (!stricmp( com, wad->filename))
	       break;
	 if (wad)
	 {
	    printf( "[File \"%s\" is opened and cannot be overwritten.]\n", com);
	    continue;
	 }
	 BuildNewMainWad( com);
      }

      /* user ask for a listing of a WAD file */
      else if (!strcmp( com, "LIST") || !strcmp( com, "L"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Wad file name argument missing.]\n");
	    continue;
	 }
	 for (wad = WadFileList; wad; wad = wad->next)
	    if (!stricmp( com, wad->filename))
	       break;
	 if (wad == NULL)
	 {
	    printf( "[Wad file \"%s\" is not open.]\n", com);
	    continue;
	 }
	 out = strtok( NULL, " ");
	 if (out)
	 {
	    printf( "Outputting directory of \"%s\" to \"%s\".\n", wad->filename, out);
	    if ((file = fopen( out, "wt")) == NULL)
	       ProgError( "error opening output file \"%s\"", com);
	    Credits( file);
	    ListFileDirectory( file, wad);
	    fprintf( file, "\nEnd of file.\n");
	    fclose( file);
	 }
	 else
	    ListFileDirectory( stdout, wad);
      }

      /* user asked for the list of the master directory */
      else if (!strcmp( com, "MASTER") || !strcmp( com, "M"))
      {
	 out = strtok( NULL, " ");
	 if (out)
	 {
	    printf( "Outputting master directory to \"%s\".\n", out);
	    if ((file = fopen( out, "wt")) == NULL)
	       ProgError( "error opening output file \"%s\"", com);
	    Credits( file);
	    ListMasterDirectory( file);
	    fprintf( file, "\nEnd of file.\n");
	    fclose( file);
	 }
	 else
	    ListMasterDirectory( stdout);
      }

      else if (!strcmp( com, "DUMP") || !strcmp( com, "D"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Object name argument missing.]\n");
	    continue;
	 }
	 out = strtok( NULL, " ");
	 if (out)
	 {
	    printf( "Outputting directory entry data to \"%s\".\n", out);
	    if ((file = fopen( out, "wt")) == NULL)
	       ProgError( "error opening output file \"%s\"", com);
	    Credits( file);
	    DumpDirectoryEntry( file, com);
	    fprintf( file, "\nEnd of file.\n");
	    fclose( file);
	 }
	 else
	    DumpDirectoryEntry( stdout, com);
      }

      /* unknown command */
      else
	 printf( "[Unknown command \"%s\"!]\n", com);
   }
}

/* end of file */
