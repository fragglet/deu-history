/*
   DETH Doom Editor for Total Headcases, by Simon Oke and Antnoy Burden.
   
   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.
   
   This program comes with absolutely no warranty.
   
   DEU.C - Main program execution routines.
   
   Compile with command
   tcc -Z -mh -r -G -O -f -edeu *.c \tc\lib\graphics.lib
   
   Be sure to use the Huge memory model when you compile...

   Forget the above, use GCC - you don't have to worry about memory models!

   If you have GNU Make just type: make
   
   */


/* the includes */
#include "deu.h"
#include <time.h>

/* global variables */
FILE  *logfile =		NULL;		/* filepointer to the error log */
Bool  Registered = 		FALSE;		/* registered or shareware game? */
Bool  Doom2 = 			FALSE;		/* Is this Doom 2?? */
Bool  Debug = 			FALSE;		/* are we debugging? */
Bool  SwapButtons = 		FALSE;		/* swap right and middle mouse buttons */
Bool  Quiet = 			FALSE;		/* don't play a sound when an object is selected */
Bool  Quieter =			FALSE;		/* don't play any sound, even when an error occurs */
Bool  Expert = 			FALSE;		/* don't ask for confirmation for some operations */
char  *CfgFile = 		DEU_CONFIG_FILE;	/* name of the configuration file */
BCINT InitialScale = 		8;		/* initial zoom factor for map */
BCINT VideoMode = 		2;		/* default video mode for VESA/SuperVGA */
char  *BGIDriver = 		"VESA";	     	/* default extended BGI driver */
Bool  FakeCursor = 		FALSE;		/* use a "fake" mouse cursor */
Bool  CirrusCursor = 		FALSE;		/* use hardware cursor on Cirrus Logic VGA cards */
Bool  Colour2 = 		FALSE;		/* use the alternate set for things colors */
Bool  InfoShown = 		TRUE;		/* should we display the info bar? */
Bool  AdditiveSelBox = 		FALSE;		/* additive selection box or select in box only? */
Bool  square_circles = 		FALSE;   	/* draw ruler square instead of with circles */
Bool  ThingAngle = 		FALSE;   	/* draw things with directional arrow */
BCINT SplitFactor =		8;		/* factor used by the Nodes builder */
char  *DefaultWallTexture =	"GSTONE1\0";	/* default normal wall texture */
char  *DefaultUpperTexture = 	"GSTONE1\0";	/* default upper wall texture */
char  *DefaultLowerTexture = 	"GSTONE1\0";    /* default lower wall texture */
char  *DefaultFloorTexture = 	"DEM1_6\0\0";	/* default floor texture */
char  *DefaultCeilingTexture = 	"DEM1_5\0\0";	/* default ceiling texture */
char  *DefaultDoorTrack = 	"DOORTRAK";	/* default door track texture */
char  *DefaultDoorTexture = 	"MARBFAC3";	/* default door texture */
BCINT DefaultFloorHeight = 	0;		/* default floor height */
BCINT DefaultCeilingHeight = 	128;		/* default ceiling height */
BCINT DefaultLargeScroll = 	10;		/* default large scroll factor */
Bool  Select0 = 		TRUE;		/* select object 0 by default when switching modes */
Bool  Reminder = 		TRUE;		/* display a funny message when DETH starts */
char  *MainWad = 		"DOOM.WAD"; 	/* name of the main wad file */
char  **PatchWads = 		NULL;		/* list of patch wad files */
Bool  QisQuit = 		FALSE;		
char *Game = 			"doom\0\0\0\0";
char *RegTest = 		NULL;
OptDesc options[] =				/* description of the command line options */
{
/*  short & long names     type            message if true/changed        message if false              where to store the value */
    { "d",  "debug",       OPT_BOOLEAN,    "Debug mode ON",               "Debug mode OFF",             &Debug },
    { "q",  "quiet",       OPT_BOOLEAN,    "Quiet mode ON",               "Quiet mode OFF",             &Quiet },
    { "qq", "quieter",     OPT_BOOLEAN,    "Quieter mode ON",             "Quieter mode OFF",           &Quieter },
    { "e",  "expert",      OPT_BOOLEAN,    "Expert mode ON",              "Expert mode OFF",            &Expert },
    { "qi",  "qisquit",    OPT_BOOLEAN,    "Quit on Q",                   "Save on Q",                  &QisQuit },
    { "sb", "swapbuttons", OPT_BOOLEAN,    "Mouse buttons swapped",       "Mouse buttons restored",     &SwapButtons },
    { "w",  "main",        OPT_STRING,     "Main WAD file",               NULL,                         &MainWad },
    { "",   "file",        OPT_STRINGLIST, "Patch WAD file",              NULL,                         &PatchWads },
    { "pw", "pwad",        OPT_STRINGACC,  "Patch WAD file",              NULL,                         &PatchWads },
    { "",   "config",      OPT_STRING,     "Config file",                 NULL,                         &CfgFile },
    { "z",  "zoom",        OPT_INTEGER,    "Initial zoom factor",         NULL,                         &InitialScale },
    { "v",  "video",       OPT_INTEGER,    "Default video mode",          NULL,                         &VideoMode },
    { "",   "bgi",         OPT_STRING,     "Default video driver",        NULL,                         &BGIDriver },
    { "fc", "fakecursor",  OPT_BOOLEAN,    "Fake cursor ON",              "Fake cursor OFF",            &FakeCursor },
    { "cc", "cirruscursor",OPT_BOOLEAN,    "Cirrus hardware cursor ON",   "Cirrus hardware cursor OFF", &CirrusCursor },
    { "c",  "color2",      OPT_BOOLEAN,    "Alternate Things color set",  "Normal Things color set",    &Colour2 },
    { "i",  "infobar",     OPT_BOOLEAN,    "Info bar shown",              "Info bar hidden",            &InfoShown },
    { "a",  "addselbox",   OPT_BOOLEAN,    "Additive selection box",      "Select objects in box only", &AdditiveSelBox },
    { "sf", "splitfactor", OPT_INTEGER,    "Split factor",                NULL,                         &SplitFactor },
    { "sq", "squarecircles",OPT_BOOLEAN,   "Drawing ruler with squares",  NULL, 			&square_circles },
    { "",   "walltexture", OPT_STRING,     "Default wall texture",        NULL,                         &DefaultWallTexture },
    { "",   "lowertexture",OPT_STRING,     "Default lower wall texture",  NULL,                         &DefaultLowerTexture },
    { "",   "uppertexture",OPT_STRING,     "Default upper wall texture",  NULL,                         &DefaultUpperTexture },
    { "",   "floortexture",OPT_STRING,     "Default floor texture",       NULL,                         &DefaultFloorTexture },
    { "",   "ceiltexture", OPT_STRING,     "Default ceiling texture",     NULL,                         &DefaultCeilingTexture },
    { "",   "doortrack",   OPT_STRING,     "Default door track texture",  NULL,                         &DefaultDoorTrack },
    { "",   "doortexture", OPT_STRING,     "Default door texture",        NULL,                         &DefaultDoorTexture },
    { "",   "floorheight", OPT_INTEGER,    "Default floor height",        NULL,                         &DefaultFloorHeight },
    { "",   "ceilheight",  OPT_INTEGER,    "Default ceiling height",      NULL,                         &DefaultCeilingHeight },
    { "",   "largescroll", OPT_INTEGER,    "Default large scroll factor", NULL, 			&DefaultLargeScroll },
    { "s0", "select0",     OPT_BOOLEAN,    "Select 0 by default",         "No default selection",       &Select0 },
    { "",   "reminder1",   OPT_BOOLEAN,    "",                            "",                           &Reminder },
    { "ta", "thingangle",  OPT_BOOLEAN,    "Show Thing angle",            "",                           &ThingAngle },
    { "",   "gamedef",	   OPT_STRING,     "Game to edit",                NULL,                         &Game },
    { NULL, NULL,          OPT_END,        NULL,			  NULL,                         NULL }
    
};



/*
   the main program
   */

int main( int argc, char *argv[])
{
    BCINT i;
    
    Credits( stdout);
    argv++;
    argc--;
    /* InitSwap must be called before any call to GetMemory(), etc. */
    InitSwap();
    /* quick and dirty check for a "-config" option */
    for (i = 0; i < argc - 1; i++)
	if (!strcmp( argv[ i], "-config")) {
	    CfgFile = argv[ i + 1];
	    break;
	}
    /* read config file and command line options */
    ParseConfigFileOptions( CfgFile);
    ParseCommandLineOptions( argc, argv);
	readcfg(Game);
	LogMessage(": Welcome to DETH!\n");
    if (Quieter == TRUE)
	Quiet = TRUE;
    /* load the wad files */     
    if (Reminder == TRUE)
	FunnyMessage( stdout);
    OpenMainWad( MainWad);
    if (PatchWads)
	while (PatchWads[ 0]) {
	    OpenPatchWad( strupr( PatchWads[ 0]));
	    PatchWads++;
	}
    /* sanity check */
    CloseUnusedWadFiles();
    
    /* SO: here seems like a good place to make the ld_types index */
    /* not so good now that the data structures have changed! */
    index_ld_types();
    
    /* all systems go! */
    MainLoop();
    /* that's all, folks! */
    CloseWadFiles();
    LogMessage( ": The end!\n\n\n");
    if (logfile != NULL)
	fclose( logfile);
    return 0;
}



/*
   Append a string to a null-terminated string list
   */

void AppendItemToList( char ***list, char *item)
{
    BCINT i;
    
    i = 0;
    if (*list != NULL) {
	/* count the number of elements in the list (last = null) */
	while ((*list)[ i] != NULL)
	    i++;
	/* expand the list */
	*list = (char**)ResizeMemory( *list, (i + 2) * sizeof( char **));
    }
    else {
	/* create a new list */
	*list = (char**)GetMemory( 2 * sizeof( char **));
    }
    /* append the new element */
    (*list)[ i] = item;
    (*list)[ i + 1] = NULL;
}



/* Handle command line options */

void ParseCommandLineOptions( int argc, char *argv[])
{
    BCINT optnum;
    
    while (argc > 0) {
	if (argv[ 0][ 0] != '-' && argv[ 0][ 0] != '+')
	    ProgError( "options must start with '-' or '+'");
	if (!strcmp( argv[ 0], "-?") || !stricmp( argv[ 0], "-h") || !stricmp( argv[ 0], "-help")) {
	    Usage( stdout);
	    exit( 0);
	}
	for (optnum = 0; options[ optnum].opt_type != OPT_END; optnum++) {
	    if (!stricmp( &(argv[ 0][ 1]), options[ optnum].short_name) || !stricmp( &(argv[ 0][ 1]), options[ optnum].long_name)) {
		switch (options[ optnum].opt_type) {
		case OPT_BOOLEAN:
		    if (argv[ 0][ 0] == '-') {
			*((Bool *) (options[ optnum].data_ptr)) = TRUE;
			if (options[ optnum].msg_if_true)
			    printf("%s\n", options[ optnum].msg_if_true);
		    }
		    else {
			*((Bool *) (options[ optnum].data_ptr)) = FALSE;
			if (options[ optnum].msg_if_false)
			    printf("%s\n", options[ optnum].msg_if_false);
		    }
		    break;
		case OPT_INTEGER:
		    if (argc <= 1)
			ProgError( "missing argument after \"%s\"", argv[ 0]);
		    argv++;
		    argc--;
		    *((BCINT *) (options[ optnum].data_ptr)) = atoi( argv[ 0]);
		    if (options[ optnum].msg_if_true)
			printf("%s: %d\n", options[ optnum].msg_if_true, atoi( argv[ 0]));
		    break;
		case OPT_STRING:
		    if (argc <= 1)
			ProgError( "missing argument after \"%s\"", argv[ 0]);
		    argv++;
		    argc--;
		    *((char **) (options[ optnum].data_ptr)) = argv[ 0];
		    if (options[ optnum].msg_if_true)
			printf("%s: %s\n", options[ optnum].msg_if_true, argv[ 0]);
		    break;
		case OPT_STRINGACC:
		    if (argc <= 1)
			ProgError( "missing argument after \"%s\"", argv[ 0]);
		    argv++;
		    argc--;
		    AppendItemToList( (char ***) options[ optnum].data_ptr, argv[ 0]);
		    if (options[ optnum].msg_if_true)
			printf("%s: %s\n", options[ optnum].msg_if_true, argv[ 0]);
		    break;
		case OPT_STRINGLIST:
		    if (argc <= 1)
			ProgError( "missing argument after \"%s\"", argv[ 0]);
		    while (argc > 1 && argv[ 1][ 0] != '-' && argv[ 1][ 0] != '+') {
			argv++;
			argc--;
			AppendItemToList( (char ***) options[ optnum].data_ptr, argv[ 0]);
			if (options[ optnum].msg_if_true)
			    printf("%s: %s\n", options[ optnum].msg_if_true, argv[ 0]);
		    }
		    break;
		default:
		    ProgError( "unknown option type (BUG!)");
		}
		break;
	    }
	}
	if (options[ optnum].opt_type == OPT_END)
	    ProgError( "invalid argument: \"%s\"", argv[ 0]);
	argv++;
	argc--;
    }
}



/* read the config file */

void ParseConfigFileOptions( char *filename)
{
    FILE *cfgfile;
    char  line[ 1024];
    char *value;
    char *option;
    char *p;
    BCINT   optnum;
    
    if ((cfgfile = fopen (filename, "r")) == NULL) {
	printf( "Warning: Configuration file not found (%s)\n", filename);
	return;
    }
    while (fgets (line, 1024, cfgfile) != NULL) {
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
	if (value[ 0] == '=') {
	    /* mark the end of the option name */
	    value[ 0] = '\0';
	}
	else {
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
	for (optnum = 0; options[ optnum].opt_type != OPT_END; optnum++) {
	    if (!stricmp( option, options[ optnum].long_name)) {
		switch (options[ optnum].opt_type) {
		case OPT_BOOLEAN:
		    if (!stricmp(value, "yes") || !stricmp(value, "true") || !stricmp(value, "on") || !stricmp(value, "1")) {
			*((Bool *) (options[ optnum].data_ptr)) = TRUE;
			if (options[ optnum].msg_if_true)
			    printf("%s\n", options[ optnum].msg_if_true);
		    }
		    else if (!stricmp(value, "no") || !stricmp(value, "false") || !stricmp(value, "off") || !stricmp(value, "0")) {
			*((Bool *) (options[ optnum].data_ptr)) = FALSE;
			if (options[ optnum].msg_if_false)
			    printf("%s\n", options[ optnum].msg_if_false);
		    }
		    else
			ProgError( "invalid value for option %s: \"%s\"", option, value);
		    break;
		case OPT_INTEGER:
		    *((BCINT *) (options[ optnum].data_ptr)) = atoi( value);
		    if (options[ optnum].msg_if_true)
			printf("%s: %d\n", options[ optnum].msg_if_true, atoi( value));
		    break;
		case OPT_STRING:
		    p = (char*)GetMemory( (strlen( value) + 1) * sizeof( char));
		    strcpy( p, value);
		    *((char **) (options[ optnum].data_ptr)) = p;
		    if (options[ optnum].msg_if_true)
			printf("%s: %s\n", options[ optnum].msg_if_true, value);
		    break;
		case OPT_STRINGACC:
		    p = (char*)GetMemory( (strlen( value) + 1) * sizeof( char));
		    strcpy( p, value);
		    AppendItemToList( (char ***) options[ optnum].data_ptr, p);
		    if (options[ optnum].msg_if_true)
			printf("%s: %s\n", options[ optnum].msg_if_true, value);
		    break;
		case OPT_STRINGLIST:
		    while (value[ 0]) {
			option = value;
			while (option[ 0] && !isspace( option[ 0]))
			    option++;
			option[ 0] = '\0';
			option++;
			while (isspace( option[ 0]))
			    option++;
			p = (char*)GetMemory( (strlen( value) + 1) * sizeof( char));
			strcpy( p, value);
			AppendItemToList( (char ***) options[ optnum].data_ptr, p);
			if (options[ optnum].msg_if_true)
			    printf("%s: %s\n", options[ optnum].msg_if_true, value);
			value = option;
		    }
		    break;
		default:
		    ProgError( "unknown option type (BUG!)");
		}
		break;
	    }
	}
	if (options[ optnum].opt_type == OPT_END)
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
    fprintf( where, "DETH [-w <main_wad_file>] [-d] [-sb] [-c] [-q] [-qq] [-sq] [-e] [-a] [-i] [-z <zoom>] [-bgi <driver>] [-v <mode>] [-fc] [-config <ini_file>] [-pw <pwad_file>] [-file <pwad_files>...]\n");
    fprintf( where, "   -w    Gives the name of the main wad file (also -main).  Default is DOOM.WAD\n");
    fprintf( where, "   -d    Enter debug mode (also -debug).\n");
    fprintf( where, "   -c    Use the alternate Things color set (also -color2).\n");
    fprintf( where, "   -q    Suppresses some sounds (also -quiet).\n");
    fprintf( where, "   -qq   Suppresses all sounds (also -quieter).\n");
    fprintf( where, "   -e    Stops prompts for confirmation (also -expert).\n");
    fprintf( where, "   -a    To have an additive selection box (also -addselbox).\n");
    fprintf( where, "   -i    Show the info bar in the editors (also -infobar).\n");
    fprintf( where, "   -z    Set the initial zoom factor for the editors (also -zoom).\n");
    fprintf( where, "   -v    Set the default video mode number (also -video).\n");
    fprintf( where, "   -bgi  Set the default video driver (*.BGI file).\n");
    fprintf( where, "   -fc   Use a \"fake\" mouse cursor (also -fakecursor).\n");
    fprintf( where, "   -sq   To draw ruler wuth squares instead of circles (also -squarecircles).\n");
    fprintf( where, "   -sb   Swaps the mouse buttons (also -swapbuttons).\n");
    fprintf( where, "   -pw   To add one patch wad file to be loaded; may be repeated (also -pwad).\n");
    fprintf( where, "   -file To add a list of patch wad files to be loaded.\n");
    fprintf( where, "   -config Gives the name of the config file.\n");
    fprintf( where, "Put a '+' instead of a '-' before boolean options to reverse their effect.\n");
    
}


/* output the credits of the program to the specified file */

void Credits( FILE *where) {
    fprintf( where, "\nDETH: Doom Editor for Total Headcases, ver %s.\n", DETH_VERSION);
    fprintf( where,   " By Simon Oke and Antony Burden. (cserve 100141,277)\n\n");
    fprintf( where,   " Thanks to John Anderson and Jim F Flynn for support and bug hunting.\n\n");
    fprintf( where,   "Derived from DEU, ver %s.\n", DEU_VERSION);
    fprintf( where,   " By Rapha‰l Quinet and Brendon J Wyber, Per Allansson and Per Kofad.\n\n");
}


/* display a funny message on the screen */

void FunnyMessage( FILE *where)
{
    fprintf( where, "\n");
    fprintf( where, "*----------------------------------------------------------------------------*\n");
    fprintf( where, "| Welcome to DETH!  This is a powerful utility and, come withs no manual or  |\n");
    fprintf( where, "| warranty what so ever. Hint: the DEU manual might provide you with a few   |\n");
    fprintf( where, "| clues!                                                                     |\n");
    fprintf( where, "|                                                                            |\n");
    fprintf( where, "| If you are an experienced DEU user and want to know what has changed then  |\n");
    fprintf( where, "| you should read the file CHANGES.TXT                                       |\n");
    fprintf( where, "|                                                                            |\n");
    fprintf( where, "| And if you have lots of suggestions for improvements, bug reports, or even |\n");
    fprintf( where, "| complaints about this program, be sure to read README.1ST first.           |\n");
    fprintf( where, "| Hint: you can easily disable this message.  Read the .ini carefully...     |\n");
    fprintf( where, "*----------------------------------------------------------------------------*\n");
}


/* play a fascinating tune */

void Beep()
{
    if (Quieter == FALSE) {
	sound( 640);
	delay( 100);
	nosound();
    }
}



/* play a sound */

void PlaySound( BCINT freq, BCINT msec)
{
    if (Quiet == FALSE) {
	sound( freq);
	delay( msec);
	nosound();
    }
}



/* terminate the program reporting an error */

void ProgError( char *errstr, ...)
{
    va_list args;
    
    Beep();
    Beep();
    if (GfxMode) {
	sleep( 1);
	TermGfx();
    }
    va_start( args, errstr);
    printf( "\nProgram Error: *** ");
    vprintf( errstr, args);
    printf( " ***\n");
    if (Debug == TRUE && logfile != NULL) {
	fprintf( logfile, "\nProgram Error: *** ");
	vfprintf( logfile, errstr, args);
	fprintf( logfile, " ***\n");
    }
    va_end( args);
    /* clean up things and free swap space */
    ForgetLevelData();
    ForgetWTextureNames();
    ForgetFTextureNames();
    CloseWadFiles();
    exit( 5);
}

/* write a message in the log file */

/* SO: changed this 13/3/95.
   we now open the logfile before, and close it after, writing each
   line. This way we guarantee that all messages get into the file
   (even if the program crashes). */

void LogMessage( char *logstr, ...)
{
    va_list  args;
    time_t   tval;
    char    *tstr;
    
    if (Debug == TRUE) {
		logfile = fopen(DEU_LOG_FILE, "a");
		va_start( args, logstr);
		/* if the messsage begins with ":",
		   output the current date & time first */
		if (logstr[ 0] == ':') {
		    time( &tval);
		    tstr = ctime( &tval);
		    tstr[ strlen( tstr) - 1] = '\0';
		    fprintf(logfile, "%s", tstr);
		}
		vfprintf( logfile, logstr, args);
		va_end( args);
		fclose(logfile);
    }
}



/* the main program menu loop */

void MainLoop()
{
    char input[ 120];
    char *com, *out;
    FILE *file, *raw;
    WadPtr wad;
    BCINT episode, level;
    
    for (;;) {
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
	else if (!strcmp( com, "?")) {
	    printf( "?                                 -- to display this text\n");
	    printf( "B[uild] <WadFile>                 -- to build a new main WAD file\n");
	    printf( "C[reate] [episode [level]]        -- to create and edit a new (empty) level\n");
	    printf( "D[ump] <DirEntry> [outfile]       -- to dump a directory entry in hex\n");
	    printf( "E[dit] [episode [level]]          -- to edit a game level saving results to\n");
	    printf( "                                          a patch wad file\n");
	    printf( "G[roup] <WadFile>                 -- to group all patch wads in a file\n");
	    printf( "I[nsert] <RawFile> <DirEntry>     -- to insert a raw file in a patch wad file\n");
	    printf( "L[ist] <WadFile> [outfile]        -- to list the directory of a wadfile\n");
	    printf( "M[aster] [outfile]                -- to list the master directory\n");
	    printf( "Q[uit]                            -- to quit\n");
	    printf( "R[ead] <WadFile>                  -- to read a new wad patch file\n");
	    printf( "S[ave] <DirEntry> <WadFile>       -- to save one object to a separate file\n");
	    printf( "V[iew] [SpriteName]               -- to display the sprites\n");
	    printf( "W[ads]                            -- to display the open wads\n");
	    printf( "X[tract] <DirEntry> <RawFile>     -- to save (extract) one object to a raw file\n");
	}
	
	/* user asked for list of open WAD files */
	else if (!strcmp( com, "WADS") || !strcmp( com, "W")) {
	    printf( "%-20s  IWAD  (Main wad file)\n", WadFileList->filename);
	    for (wad = WadFileList->next; wad; wad = wad->next) {
		if (wad->directory[ 0].name[ 0] == 'E' && wad->directory[ 0].name[ 2] == 'M')
		    printf( "%-20s  PWAD  (Patch wad file for episode %c level %c)\n", wad->filename, wad->directory[ 0].name[ 1], wad->directory[ 0].name[ 3]);
		else {
		    /* kluge */
		    strncpy( input, wad->directory[ 0].name, 8);
		    input[ 8] = '\0';
		    printf( "%-20s  PWAD  (Patch wad file for %s)\n", wad->filename, input);
		}
	    }
	}
	
	/* user asked to quit */
	else if (!strcmp( com, "QUIT") || !strcmp( com, "Q")) {
	    if (!(Registered || Doom2))
		printf("Remember to register your copy of DOOM!\n");
	    printf( "Goodbye...\n");
	    break;
	}
	
	/* user asked to edit a level */
	else if (!strcmp( com, "EDIT") || !strcmp( com, "E") || !strcmp( com, "CREATE") || !strcmp( com, "C")) {
	    episode = 0;
	    level = 0;
	    com = strtok( NULL, " ");
	    if (com != NULL) {
		episode = atoi( com);
		com = strtok( NULL, " ");
		if (com != NULL) {
		    level = atoi( com);
		    if (level < 1 || level> 9) {
			printf( "[Invalid game level number (%s).]\n", com);
			continue;
		    }
		}
		else
		    level = -1;
	    }
	    com = strtok( input, " ");
	    EditLevel( episode, level, !strcmp( com, "CREATE") || !strcmp( com, "C"));
	}
	
	/* user asked to build a new main WAD file */
	else if (!strcmp( com, "BUILD") || !strcmp( com, "B")) {
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Wad file name argument missing.]\n");
		continue;
	    }
	    for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( com, wad->filename))
		    break;
	    if (wad) {
		printf( "[File \"%s\" is opened and cannot be overwritten.]\n", com);
		continue;
	    }
	    BuildNewMainWad( com, FALSE);
	}
	
	/* user asked to build a compound patch WAD file */
	else if (!strcmp( com, "GROUP") || !strcmp( com, "G")) {
	    if (WadFileList->next == NULL || WadFileList->next->next == NULL) {
		printf( "[You need at least two open wad files if you want to group them.]\n");
		continue;
	    }
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Wad file name argument missing.]\n");
		continue;
	    }
	    for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( com, wad->filename))
		    break;
	    if (wad) {
		printf( "[File \"%s\" is opened and cannot be overwritten.]\n", com);
		continue;
	    }
	    BuildNewMainWad( com, TRUE);
	}
	
	/* user ask for a listing of a WAD file */
	else if (!strcmp( com, "LIST") || !strcmp( com, "L")) {
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Wad file name argument missing.]\n");
		continue;
	    }
	    for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( com, wad->filename))
		    break;
	    if (wad == NULL) {
		printf( "[Wad file \"%s\" is not open.]\n", com);
		continue;
	    }
	    out = strtok( NULL, " ");
	    if (out) {
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
	else if (!strcmp( com, "MASTER") || !strcmp( com, "M")) {
	    out = strtok( NULL, " ");
	    if (out) {
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
	else if (!strcmp( com, "READ") || !strcmp( com, "R")) {
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Wad file name argument missing.]\n");
		continue;
	    }
	    out = strtok( NULL, " ");
	    if (out)
		*out = '\0';
	    out = (char*)GetMemory( (strlen( com) + 1) * sizeof( char));
	    strcpy( out, com);
	    OpenPatchWad( out);
	    CloseUnusedWadFiles();
	}
	
	/* user asked to dump the contents of a WAD file */
	else if (!strcmp( com, "DUMP") || !strcmp( com, "D")) {
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Object name argument missing.]\n");
		continue;
	    }
	    out = strtok( NULL, " ");
	    if (out) {
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
	
	/* user asked to view the sprites */
	else if (!strcmp( com, "VIEW") || !strcmp( com, "V")) {
	    InitGfx();
	    com = strtok( NULL, " ");
	    ChooseSprite( -1, -1, "Sprite viewer", com);
	    TermGfx();
	}
	
	/* user asked to save an object to a separate PWAD file */
	else if (!strcmp( com, "SAVE") || !strcmp( com, "S")) {
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Object name argument missing.]\n");
		continue;
	    }
	    if (strlen( com) > 8 || strchr( com, '.') != NULL) {
		printf( "[Invalid object name.]\n");
		continue;
	    }
	    out = strtok( NULL, " ");
	    if (out == NULL) {
		printf( "[Wad file name argument missing.]\n");
		continue;
	    }
	    for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( out, wad->filename))
		    break;
	    if (wad) {
		printf( "[This Wad file is already in use.  You may not overwrite it.]\n");
		continue;
	    }
	    printf( "Saving directory entry data to \"%s\".\n", out);
	    if ((file = fopen( out, "wb")) == NULL)
		ProgError( "error opening output file \"%s\"", out);
	    SaveDirectoryEntry( file, com);
	    fclose( file);
	}
	
	/* user asked to encapsulate a raw file in a PWAD file */
	else if (!strcmp( com, "INSERT") || !strcmp( com, "I")) {
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Raw file name argument missing.]\n");
		continue;
	    }
	    out = strtok( NULL, " ");
	    if (out == NULL) {
		printf( "[Object name argument missing.]\n");
		continue;
	    }
	    if (strlen( out) > 8 || strchr( out, '.') != NULL) {
		printf( "[Invalid object name.]\n");
		continue;
	    }
	    if ((raw = fopen( com, "rb")) == NULL)
		ProgError( "error opening input file \"%s\"", com);
	    /* kluge */
	    strcpy( input, out);
	    strcat( input, ".WAD");
	    for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( input, wad->filename))
		    break;
	    if (wad) {
		printf( "[This Wad file is already in use (%s).  You may not overwrite it.]\n", input);
		continue;
	    }
	    printf( "Including new object %s in \"%s\".\n", out, input);
	    if ((file = fopen( input, "wb")) == NULL)
		ProgError( "error opening output file \"%s\"", input);
	    SaveEntryFromRawFile( file, raw, out);
	    fclose( raw);
	    fclose( file);
	}
	
	/* user asked to extract an object to a raw binary file */
	else if (!strcmp( com, "XTRACT") || !strcmp( com, "EXTRACT") || !strcmp( com, "X")) {
	    com = strtok( NULL, " ");
	    if (com == NULL) {
		printf( "[Object name argument missing.]\n");
		continue;
	    }
	    if (strlen( com) > 8 || strchr( com, '.') != NULL) {
		printf( "[Invalid object name.]\n");
		continue;
	    }
	    out = strtok( NULL, " ");
	    if (out == NULL) {
		printf( "[Raw file name argument missing.]\n");
		continue;
	    }
	    for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( out, wad->filename))
		    break;
	    if (wad) {
		printf( "[You may not overwrite an opened Wad file with raw data.]\n");
		continue;
	    }
	    printf( "Saving directory entry data to \"%s\".\n", out);
	    if ((file = fopen( out, "wb")) == NULL)
		ProgError( "error opening output file \"%s\"", out);
	    SaveEntryToRawFile( file, com);
	    fclose( file);
	}
	
	/* unknown command */
	else
	    printf( "[Unknown command \"%s\"!]\n", com);
    }
}

/* end of file */
