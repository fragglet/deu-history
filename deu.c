/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   DEU.C - Main program execution routines.

   Compile with command
      tcc -Z -ml -r -G -O -f -edeu *.c \tc\lib\graphics.lib

   Be sure to use the Large memory model when you compile...

*/

/* the includes */
#include "deu.h"

/* global variables */
Bool Registered = FALSE;	/* registered or shareware game? */
Bool Debug = FALSE;		/* are we debugging? */
Bool SwapButtons = FALSE;	/* swap right and middle mouse buttons */
Bool Quiet = FALSE;		/* don't play a sound when an object is selected */
Bool Expert = FALSE;		/* don't ask for confirmation for some operations */
char *MainWad = "DOOM.WAD";	/* name of the main wad file */
char **PatchWads = NULL;	/* list of patch wad files */
OptDesc options[] =		/* description of the command line options */
{
   { "d", "debug", OPT_BOOLEAN, "Debug mode ON.  Useless now..", "Debug mode OFF", &Debug },
   { "q", "quiet", OPT_BOOLEAN, "Quiet mode ON", "Quiet mode OFF", &Quiet },
   { "e", "expert", OPT_BOOLEAN, "Expert mode ON", "Expert mode OFF", &Expert },
   { "sb", "swapbuttons", OPT_BOOLEAN, "Mouse buttons swapped", "Mouse buttons restored", &SwapButtons },
   { "w", "main", OPT_STRING, "Main WAD file", NULL, &MainWad },
   { NULL, "file", OPT_STRINGLIST, "Patch WAD file", NULL, &PatchWads },
   { "pw", "pwad", OPT_STRINGACC, "Patch WAD file", NULL, &PatchWads },
   { NULL, NULL, OPT_NONE, NULL, NULL, NULL }
};


/*
   the main program
*/

int main( int argc, char *argv[])
{
   Credits( stdout);
   argv++;
   argc--;
   /* read config file and command line options */
   ParseConfigFileOptions( DEU_CONFIG_FILE);
   ParseCommandLineOptions( argc, argv);
   /* load the wad files */
   OpenMainWad( MainWad);
   if (PatchWads)
      while (PatchWads[ 0])
      {
	 OpenPatchWad( strupr( PatchWads[ 0]));
	 PatchWads++;
      }
   /* sanity check */
   CloseUnusedWadFiles();
   /* all systems go! */
   MainLoop();
   /* that's all, folks! */
   CloseWadFiles();
   return 0;
}



/*
   Append a string to a null-terminated string list
*/

void AppendItemToList( char ***list, char *item)
{
   int i;

   i = 0;
   if (*list != NULL)
   {
      /* count the number of elements in the list (last = null) */
      while ((*list)[ i] != NULL)
	 i++;
      /* expand the list */
      *list = ResizeMemory( *list, (i + 2) * sizeof( char **));
   }
   else
   {
      /* create a new list */
      *list = GetMemory( 2 * sizeof( char **));
   }
   /* append the new element */
   (*list)[ i] = item;
   (*list)[ i + 1] = NULL;
}



/*
   Handle command line options
*/

void ParseCommandLineOptions( int argc, char *argv[])
{
   int optnum;

   while (argc > 0)
   {
      if (argv[ 0][ 0] != '-' && argv[ 0][ 0] != '+')
	 ProgError( "options must start with '-' or '+'");
      if (!strcmp( argv[ 0], "-?") || !strcmp( argv[ 0], "-h") || !strcmp( argv[ 0], "-help"))
      {
	 Usage( stdout);
	 exit( 0);
      }
      for (optnum = 0; options[ optnum].opt_type != OPT_NONE; optnum++)
      {
	 if (!strcmp( &(argv[ 0][ 1]), options[ optnum].short_name) || !strcmp( &(argv[ 0][ 1]), options[ optnum].long_name))
	 {
	    switch (options[ optnum].opt_type)
	    {
	    case OPT_BOOLEAN:
	       if (argv[ 0][ 0] == '-')
	       {
		  *((Bool *) (options[ optnum].data_ptr)) = TRUE;
		  if (options[ optnum].msg_if_true)
		     printf("%s.\n", options[ optnum].msg_if_true);
	       }
	       else
	       {
		  *((Bool *) (options[ optnum].data_ptr)) = FALSE;
		  if (options[ optnum].msg_if_false)
		     printf("%s.\n", options[ optnum].msg_if_false);
	       }
	       break;
	    case OPT_INTEGER:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       argv++;
	       argc--;
	       *((int *) (options[ optnum].data_ptr)) = atoi( argv[ 0]);
	       if (options[ optnum].msg_if_true)
		  printf("%s: %d.\n", options[ optnum].msg_if_true, atoi( argv[ 0]));
	       break;
	    case OPT_STRING:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       argv++;
	       argc--;
	       *((char **) (options[ optnum].data_ptr)) = argv[ 0];
	       if (options[ optnum].msg_if_true)
		  printf("%s: %s.\n", options[ optnum].msg_if_true, argv[ 0]);
	       break;
	    case OPT_STRINGACC:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       argv++;
	       argc--;
	       AppendItemToList( (char ***) options[ optnum].data_ptr, argv[ 0]);
	       if (options[ optnum].msg_if_true)
		  printf("%s: %s.\n", options[ optnum].msg_if_true, argv[ 0]);
	       break;
	    case OPT_STRINGLIST:
	       if (argc <= 1)
		  ProgError( "missing argument after \"%s\"", argv[ 0]);
	       while (argc > 1 && argv[ 1][ 0] != '-' && argv[ 1][ 0] != '+')
	       {
		  argv++;
		  argc--;
		  AppendItemToList( (char ***) options[ optnum].data_ptr, argv[ 0]);
		  if (options[ optnum].msg_if_true)
		     printf("%s: %s.\n", options[ optnum].msg_if_true, argv[ 0]);
	       }
	       break;
	    default:
	       ProgError( "unknown option type (BUG!)");
	    }
	    break;
	 }
      }
      if (options[ optnum].opt_type == OPT_NONE)
	 ProgError( "invalid argument: \"%s\"", argv[ 0]);
      argv++;
      argc--;
   }
}



/*
   read the config file
*/

void ParseConfigFileOptions( char *filename)
{
   FILE *cfgfile;
   char  line[ 1024];
   char *value;
   char *option;
   char *p;
   int   optnum;

   if ((cfgfile = fopen (filename, "r")) == NULL)
   {
      printf( "Warning: Configuration file not found (%s)\n", filename);
      return;
   }
   while (fgets (line, 1024, cfgfile) != NULL)
   {
      if (line[0] == '#' || strlen( line) < 2)
	 continue;
      if (line[ strlen( line) - 1] == '\n')
	 line[ strlen( line) - 1] = '\0';
      /* skip blanks before the option name */
      option = line;
      while (isspace( option[ 0]))
	 option++;
      /* skip the option name */
      value = option;
      while (value[ 0] && value[ 0] != '=' && !isspace( value[ 0]))
	 value++;
      if (!value[ 0])
	 ProgError( "invalid line in %s (ends prematurely)", filename);
      if (value[ 0] == '=')
      {
	 /* mark the end of the option name */
	 value[ 0] = '\0';
      }
      else
      {
	 /* mark the end of the option name */
	 value[ 0] = '\0';
	 value++;
	 /* skip blanks after the option name */
	 while (isspace( value[ 0]))
	    value++;
	 if (value[ 0] != '=')
	    ProgError( "invalid line in %s (no '=')", filename);
      }
      value++;
      /* skip blanks after the equal sign */
      while (isspace( value[ 0]))
	 value++;
      for (optnum = 0; options[ optnum].opt_type != OPT_NONE; optnum++)
      {
	 if (!strcmp( option, options[ optnum].long_name))
	 {
	    switch (options[ optnum].opt_type)
	    {
	    case OPT_BOOLEAN:
	       if (!stricmp(value, "yes") || !stricmp(value, "true") || !stricmp(value, "on") || !stricmp(value, "1"))
	       {
		  *((Bool *) (options[ optnum].data_ptr)) = TRUE;
		  if (options[ optnum].msg_if_true)
		     printf("%s.\n", options[ optnum].msg_if_true);
	       }
	       else if (!stricmp(value, "no") || !stricmp(value, "false") || !stricmp(value, "off") || !stricmp(value, "0"))
	       {
		  *((Bool *) (options[ optnum].data_ptr)) = FALSE;
		  if (options[ optnum].msg_if_false)
		     printf("%s.\n", options[ optnum].msg_if_false);
	       }
	       else
		  ProgError( "invalid value for option %s: \"%s\"", option, value);
	       break;
	    case OPT_INTEGER:
	       *((int *) (options[ optnum].data_ptr)) = atoi( value);
	       if (options[ optnum].msg_if_true)
		  printf("%s: %d.\n", options[ optnum].msg_if_true, atoi( value));
	       break;
	    case OPT_STRING:
	       p = GetMemory( (strlen( value) + 1) * sizeof( char));
	       strcpy( p, value);
	       *((char **) (options[ optnum].data_ptr)) = p;
	       if (options[ optnum].msg_if_true)
		  printf("%s: %s.\n", options[ optnum].msg_if_true, value);
	       break;
	    case OPT_STRINGACC:
	       p = GetMemory( (strlen( value) + 1) * sizeof( char));
	       strcpy( p, value);
	       AppendItemToList( (char ***) options[ optnum].data_ptr, p);
	       if (options[ optnum].msg_if_true)
		  printf("%s: %s.\n", options[ optnum].msg_if_true, value);
	       break;
	    case OPT_STRINGLIST:
	       while (value[ 0])
	       {
		  option = value;
		  while (option[ 0] && !isspace( option[ 0]))
		     option++;
		  option[ 0] = '\0';
		  option++;
		  while (isspace( option[ 0]))
		     option++;
		  p = GetMemory( (strlen( value) + 1) * sizeof( char));
		  strcpy( p, value);
		  AppendItemToList( (char ***) options[ optnum].data_ptr, p);
		  if (options[ optnum].msg_if_true)
		     printf("%s: %s.\n", options[ optnum].msg_if_true, value);
		  value = option;
	       }
	       break;
	    default:
	       ProgError( "unknown option type (BUG!)");
	    }
	    break;
	 }
      }
      if (options[ optnum].opt_type == OPT_NONE)
	 ProgError( "Invalid option in %s: \"%s\"", filename, option);
   }
   fclose( cfgfile);
}


/*
   output the program usage to the specified file
*/

void Usage( FILE *where)
{
   fprintf( where, "Usage:\n");
   fprintf( where, "DEU [-w <main_wad_file>] [-d] [-sb] [-q] [-e] [-file <pwad_files>...]\n");
   fprintf( where, "   -w    Gives the name of the main wad file. (also -main)  Default is DOOM.WAD\n");
   fprintf( where, "   -d    Enter debug mode. (also -debug)\n");
   fprintf( where, "   -q    Suppresses sounds. (also -quiet)\n");
   fprintf( where, "   -e    Stops prompts for confirmation. (also -expert)\n");
   fprintf( where, "   -sb   Swaps the mouse buttons. (also -swapbuttons)\n");
   fprintf( where, "   -pw   To add one patch wad file to be loaded (may be repeated). (also -pwad)\n");
   fprintf( where, "   -file To add a list of patch wad files to be loaded.\n");
}



/*
   output the credits of the program to the specified file
*/

void Credits( FILE *where)
{
   fprintf( where, "DEU: Doom Editor Utility, ver %s.\n", DEU_VERSION);
   fprintf( where, " By Rapha‰l Quinet (quinet@montefiore.ulg.ac.be),\n");
   fprintf( where, "and Brendon J Wyber (b.wyber@csc.canterbury.ac.nz).\n\n");

   fprintf( where, "*   This is a beta version: please test it and report any bugs to me:   *\n");
   fprintf( where, "*   quinet@montefiore.ulg.ac.be.  Take a look from time to time on my   *\n");
   fprintf( where, "*   FTP server (bear.montefiore.ulg.ac.be) for any newer versions.      *\n");
   fprintf( where, "*   Please do not distribute this temporary version.  Thanks.           *\n\n");
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

   Beep();
   if (GfxMode)
   {
      sleep( 1);
      TermGfx();
   }
   va_start( args, errstr);
   printf( "\nProgram Error: *** ");
   vprintf( errstr, args);
   printf( " ***\n");
   va_end( args);
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
   allocate memory from the far heap with error checking
*/

void huge *GetFarMemory( unsigned long size)
{
   void huge *ret = farmalloc( size);
   if (!ret)
      ProgError( "out of memory (cannot allocate %lu far bytes)", size);
   return ret;
}



/*
   reallocate memory from the far heap with error checking
*/

void huge *ResizeFarMemory( void huge *old, unsigned long size)
{
   void huge *ret = farrealloc( old, size);
   if (!ret)
      ProgError( "out of memory (cannot reallocate %lu far bytes)", size);
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
   int episode, level;

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
	 printf( "E[dit] [episode [level]]          -- to edit a game level saving results to\n");
	 printf( "                                          a patch wad file\n");
	 printf( "L[ist] <WadFile> [outfile]        -- to list the directory of a wadfile\n");
	 printf( "M[aster] [outfile]                -- to list the master directory\n");
	 printf( "Q[uit]                            -- to quit\n");
	 printf( "R[ead] <WadFile>                  -- to read a new wad patch file\n");
	 printf( "S[prites] [sname]                 -- to display the sprites\n");
	 printf( "W[ads]                            -- to display the open wads\n");
      }

      /* user asked for list of open WAD files */
      else if (!strcmp( com, "WADS") || !strcmp( com, "W"))
      {
	 printf( "%-20s  IWAD  (Main wad file)\n", WadFileList->filename);
	 for (wad = WadFileList->next; wad; wad = wad->next)
	 {
	    if (wad->directory[ 0].name[ 0] == 'E' && wad->directory[ 0].name[ 2] == 'M')
	       printf( "%-20s  PWAD  (Patch wad file for episode %c level %c)\n", wad->filename, wad->directory[ 0].name[ 1], wad->directory[ 0].name[ 3]);
	    else
	    {
	       /* kluge */
	       strncpy( input, wad->directory[ 0].name, 8);
	       input[ 8] = '\0';
	       printf( "%-20s  PWAD  (Patch wad file for %s)\n", wad->filename, input);
	    }
	 }
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
	 episode = 0;
	 level = 0;
	 com = strtok( NULL, " ");
	 if (com != NULL)
	 {
	    episode = atoi( com);
	    if (episode < 1 || episode > 3)
	    {
	       printf( "[Invalid game episode number (%s).]\n", com);
	       continue;
	    }
	    com = strtok( NULL, " ");
	    if (com != NULL)
	    {
	       level = atoi( com);
	       if (level < 1 || level> 9)
	       {
		  printf( "[Invalid game level number (%s).]\n", com);
		  continue;
	       }
	    }
	 }
	 EditLevel( episode, level);
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

      /* user asked to read a new patch WAD file */
      else if (!strcmp( com, "READ") || !strcmp( com, "R"))
      {
	 com = strtok( NULL, " ");
	 if (com == NULL)
	 {
	    printf( "[Wad file name argument missing.]\n");
	    continue;
	 }
	 out = strtok( NULL, " ");
	 if (out)
	   *out = '\0';
	 out = GetMemory( (strlen( com) + 1) * sizeof( char));
	 strcpy( out, com);
	 OpenPatchWad( out);
	 CloseUnusedWadFiles();
      }

      /* user asked to dump the contents of a WAD file */
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

      /* user asked to see the sprites */
      else if (!strcmp( com, "SPRITES") || !strcmp( com, "S"))
      {
	 InitGfx();
	 com = strtok( NULL, " ");
	 ChooseSprite( -1, -1, "Sprite viewer", com);
	 TermGfx();
      }

      /* unknown command */
      else
	 printf( "[Unknown command \"%s\"!]\n", com);
   }
}

/* end of file */
