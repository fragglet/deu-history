/*
   Doom Editor for Total Headcases, by Simon Oke and Antony Burden.
   
   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.
   
   This program comes with absolutely no warranty.
   
   DEU.H - Main game defines.
   */

/* the includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/* SO 24/4/95 */
#include <unistd.h>


#if defined(__TURBOC__)

#include <graphics.h>
#include <alloc.h>
#include <dos.h>
#include <bios.h>
#define DEU_VERSION	"5.2"	/* the version number */
typedef int            BCINT;
typedef unsigned int   UBCINT;

#elif defined(__GNUC__)

#include "deu-go32.h"
#define DEU_VERSION     "5.2 - DJGPP/GO32 version"
typedef short int            BCINT;
typedef unsigned short int   UBCINT;

#define DETH_VERSION	"2.3"

#endif

/* define some new colors */
#define DARKBLUE	16
#define DARKGREEN	17
#define DARKRED		18
#define DARKMAGENTA	19
#define GRAY		20
#define DARKERGRAY	21
#define	ORANGE		22

typedef unsigned char BYTE;

/* now safe to include this, as it uses BCINTs */
#include "wstructs.h"


/*
   the directory structure is the structure used by DOOM to order the
   data in it's WAD files
   */

typedef struct Directory huge *DirPtr;
struct Directory
{
    long start;			/* offset to start of data */
    long size;			/* byte size of data */
    char name[ 8];		/* name of data block */
};



/*
   The wad file pointer structure is used for holding the information
   on the wad files in a linked list.
   
   The first wad file is the main wad file. The rest are patches.
   */

typedef struct WadFileInfo huge *WadPtr;
struct WadFileInfo
{
    WadPtr next;		/* next file in linked list */
    char *filename;		/* name of the wad file */
    FILE *fileinfo;		/* C file stream information */
    char type[ 4];		/* type of wad file (IWAD or PWAD) */
    long dirsize;		/* directory size of WAD */
    long dirstart;		/* offset to start of directory */
    DirPtr directory;		/* array of directory information */
};



/*
   the master directory structure is used to build a complete directory
   of all the data blocks from all the various wad files
   */

typedef struct MasterDirectory huge *MDirPtr;
struct MasterDirectory
{
    MDirPtr next;		/* next in list */
    WadPtr wadfile;		/* file of origin */
    struct Directory dir;	/* directory data */
};



/*
   the selection list is used when more than one object is selected
   */

typedef struct SelectionList *SelPtr;
struct SelectionList
{
    SelPtr next;		/* next in list */
    BCINT objnum;		/* object number */
};


/*
   syntactic sugar
   */
typedef BCINT Bool;             /* Boolean data: true or false */


/*
   description of the command line arguments and config file keywords
   */

typedef struct
{
    char *short_name;		/* abbreviated command line argument */
    char *long_name;		/* command line arg. or keyword */
    enum                        /* type of this option */
	    {
			OPT_BOOLEAN,                     /* boolean (toggle) */
			OPT_INTEGER,                     /* integer number */
			OPT_STRING,                      /* character string */
			OPT_STRINGACC,                   /* character string, but store in a list */
			OPT_STRINGLIST,                  /* list of character strings */
			OPT_END                          /* end of the options description */
	    } opt_type;                    
    char *msg_if_true;		/* message printed if option is true */
    char *msg_if_false;		/* message printed if option is false */
    void *data_ptr;             /* pointer to the data */
} OptDesc;


/* generic string list type  -- used for holding level names,
   ftexture sections, texture sections */
typedef struct _SList {
	char *string;
	struct _SList *next;
} *SList;

/* sector type type (!) (actually a list type) */
typedef struct _sector_type {
	char *shortname, *longname;
	BCINT type;
	struct _sector_type *next;
} sector_type;

typedef struct _sector_class {
	char *name;
	sector_type *types;
	struct _sector_class *next;
} sector_class;

/* type for a list of linedef types */
typedef struct _ld_type {
	char *shortname, *longname;
	BCINT type;
	struct _ld_type *next;
} ld_type;

/* type for the list of linedef type classes (phew!) */
typedef struct _ld_class {
	char *name;
	ld_type *types;
	struct _ld_class *next;
} ld_class;

/* as above, but for a thing */
typedef struct _thing_type {
    BCINT	type, col2, col1, radius;
    char 	*name;
    struct _thing_type *next;
} thing_type;

/* and a list of thing classes */
typedef struct _thing_class {
	char *name;
	thing_type *types;
	struct _thing_class *next;
} thing_class;


/*
   the macros and constants
   */

/* name of the configuration file */
#define DEU_CONFIG_FILE		"DETH.INI"

/* name of the log file (debug mode) */
#define DEU_LOG_FILE		"DETH.LOG"

/* convert screen coordinates to map coordinates */
#define MAPX(x)			(OrigX + (BCINT) (((x) - ScrCenterX) / Scale))
#define MAPY(y)			(OrigY + (BCINT) ((ScrCenterY - (y)) / Scale))

/* convert map coordinates to screen coordinates */
#define SCREENX(x)		(ScrCenterX + (BCINT) (((x) - OrigX) * Scale))
#define SCREENY(y)		(ScrCenterY + (BCINT) ((OrigY - (y)) * Scale))

/* object types */
#define OBJ_THINGS		1
#define OBJ_LINEDEFS		2
#define OBJ_SIDEDEFS		3
#define OBJ_VERTEXES		4
#define OBJ_SEGS		5
#define OBJ_SSECTORS		6
#define OBJ_NODES		7
#define OBJ_SECTORS		8
#define OBJ_REJECT		9
#define OBJ_BLOCKMAP		10

/* boolean constants */
#ifndef TRUE
#define TRUE			1
#define FALSE			0
#endif

/* half the size of an object (Thing or Vertex) in map coords */
#define OBJSIZE			7


/* the interfile global variables */

/* from deu.c */
extern Bool CirrusCursor;      	/* use hardware cursor on Cirrus Logic VGA cards */
extern Bool Select0;           	/* select object 0 by default when switching modes */
extern Bool Debug;		/* are we debugging? */
extern Bool SwapButtons;	/* swap right and middle mouse buttons */
extern Bool Quiet;		/* don't play a sound when an object is selected */
extern Bool Quieter;		/* don't play any sound, even when an error occurs */
extern Bool Expert;		/* don't ask for confirmation for some operations */
extern Bool VertConf;		/* don't ask for confirmation on Vertices merging even if in expert mode */
extern Bool QisQuit;		
extern BCINT InitialScale;	/* initial zoom factor for map */
extern BCINT VideoMode;	  	/* default video mode for VESA cards */
extern char *BGIDriver;		/* default extended BGI driver */
extern Bool FakeCursor;	/* use a "fake" mouse cursor */
extern Bool Colour2;		/* use the alternate set for things colors */
extern Bool AdditiveSelBox;	/* additive selection box or select in box only? */
extern BCINT SplitFactor;       /* factor used by the nodes builder */
extern char *MainWad;		/* name of the main wad file */
extern FILE *logfile;		/* filepointer to the error log */
extern Bool square_circles;
extern Bool ThingAngle;		/* draw things with arrow */
extern char LevelName[];		/* what level we are editing */
extern Bool UseOwnBSP;      /* whether to use the built-in node builder */
extern SList LevelNameFormat;
extern char *RegTest;
extern Bool Registered;		/* registered or shareware WAD file? */

/* from wads.c */
extern WadPtr  WadFileList;	/* list of wad files */
extern MDirPtr MasterDir;	/* the master directory */
extern SList LevelNames;
int isalev(char *);

/* from edit.c */
extern Bool InfoShown;          /* is the bottom line displayed? */

/* from gfx.c */
extern BCINT GfxMode;		/* current graphics mode, or 0 for text */
extern float Scale;		/* scale to draw map 20 to 1 */
extern BCINT OrigX;		/* the X origin */
extern BCINT OrigY;		/* the Y origin */
extern BCINT PointerX;		/* X position of pointer */
extern BCINT PointerY;		/* Y position of pointer */
extern BCINT ScrMaxX;		/* maximum X screen coord */
extern BCINT ScrMaxY;		/* maximum Y screen coord */
extern BCINT ScrCenterX;	/* X coord of screen center */
extern BCINT ScrCenterY;	/* Y coord of screen center */

/* from mouse.c */
extern Bool UseMouse;		/* is there a mouse driver? */

/* from things2.c */
extern thing_class *Thing_classes;

/* from names.c */
extern ld_class *Linedef_classes;
extern sector_class *Sector_classes;

/* from levels.c */
extern SList Ftexture_sections;
extern SList Texture_sections;

/*
   the function prototypes
   */

/* from deu.c */
int main( int, char *[]);
void ParseCommandLineOptions( int, char *[]);
void ParseConfigFileOptions( char *);
void Usage( FILE *);
void Credits( FILE *);
void FunnyMessage( FILE *);
void Beep( void);
void PlaySound( BCINT , BCINT );
void ProgError( char *, ...);
void LogMessage( char *, ...);
void MainLoop( void);

/* from memory.c */
void *GetMemory( size_t);
void *ResizeMemory( void *, size_t);
void FreeMemory( void *);
void huge *GetFarMemory( unsigned long size);
void huge *ResizeFarMemory( void huge *old, unsigned long size);
void FreeFarMemory( void huge *);

/* from wads.c */
int isalev(char *);
void OpenMainWad( char *);
void OpenPatchWad( char *);
void CloseWadFiles( void);
void CloseUnusedWadFiles( void);
WadPtr BasicWadOpen( char *);
void BasicWadRead( WadPtr, void huge *, long);
void BasicWadSeek( WadPtr, long);
MDirPtr FindMasterDir( MDirPtr, char *);
void ListMasterDirectory( FILE *);
void ListFileDirectory( FILE *, WadPtr);
void BuildNewMainWad( char *, Bool);
void WriteBytes( FILE *, void huge *, long);
void CopyBytes( FILE *, FILE *, long);
BCINT  Exists( char *);
void DumpDirectoryEntry( FILE *, char *);
void SaveDirectoryEntry( FILE *, char *);
void SaveEntryToRawFile( FILE *, char *);
void SaveEntryFromRawFile( FILE *, FILE *, char *);

/* from levels.c */
void ReadLevelData();
void ForgetLevelData( void);
void SaveLevelData( char *); 
void ReadWTextureNames( void);
void ForgetFTextureNames( void);
void ReadFTextureNames( void);
void ReadFTextureNamesIn( char *);
void ForgetWTextureNames( void);

/* from edit.c */
void MakeLevelName(SList);
void EditLevel(Bool);
void SelectLevel();
void EditorLoop();
void DrawMap( BCINT , BCINT, Bool ); /* SWAP! */
void CenterMapAroundCoords( BCINT , BCINT );
void GoToObject( BCINT , BCINT ); /* SWAP! */
void FindThing(int);
void SaveAs(int);
void CopyFile(const char *, const char *);

/* from gfx.c */
void InitGfx( void);
Bool SwitchToVGA256( void);
Bool SwitchToVGA16( void);
void TermGfx( void);
void ClearScreen( void);
void ClearMapScreen(BCINT);
void SetColor( BCINT );
void DrawMapLine( BCINT , BCINT , BCINT , BCINT );
void DrawMapCircle( BCINT , BCINT , BCINT );
void DrawMapVector( BCINT , BCINT , BCINT , BCINT );
void DrawMapArrow( BCINT , BCINT , UBCINT);
void DrawScreenLine( BCINT , BCINT , BCINT , BCINT );
void DrawScreenBox( BCINT , BCINT , BCINT , BCINT );
void DrawScreenBox3D( BCINT , BCINT , BCINT , BCINT );
void DrawScreenBoxHollow( BCINT , BCINT , BCINT , BCINT );
void DrawScreenMeter( BCINT , BCINT , BCINT , BCINT , float);
void DrawScreenText( BCINT , BCINT , char *, ...);
void DrawPointer( Bool);
void SetDoomPalette( BCINT );
BCINT TranslateToGameColor( BCINT );
UBCINT ComputeAngle( BCINT , BCINT );
UBCINT ComputeDist( BCINT , BCINT );
void InsertPolygonVertices( BCINT , BCINT , BCINT , BCINT );
void RotateAndScaleCoords( BCINT  *, BCINT  *, double, double);

/* from things2.c */
int BCINT_lessp(const void *, const void *);
void index_things(void);
BCINT  GetThingColour( BCINT );
char *GetThingName( BCINT );
BCINT  GetThingRadius( BCINT );
char *GetAngleName( BCINT );
char *GetWhenName( BCINT );

/* from names.c */
char *GetObjectTypeName( BCINT );
char *GetEditModeName( BCINT );
char *GetLineDefTypeName( BCINT );
char *GetLineDefTypeLongName( BCINT );
char *GetLineDefFlagsName( BCINT );
char *GetLineDefFlagsLongName( BCINT );
char *GetSectorTypeName( BCINT );
char *GetSectorTypeLongName( BCINT );
void index_ld_types();

/* from mouse.c */
void CheckMouseDriver( void);
void ShowMousePointer( void);
void HideMousePointer( void);
void GetMouseCoords( BCINT  *, BCINT  *, BCINT  *);
void SetMouseCoords( BCINT , BCINT );
void SetMouseLimits( BCINT , BCINT , BCINT , BCINT );
void ResetMouseLimits( void);

/* from menus.c */
BCINT DisplayMenuArray( BCINT , BCINT , char *, BCINT , BCINT  *, char *[ 30], BCINT  [30]);
BCINT DisplayMenu( BCINT , BCINT , char *, ...);
BCINT PullDownMenu( BCINT , BCINT , ...);
BCINT InputInteger( BCINT , BCINT , BCINT  *, BCINT , BCINT );
BCINT InputIntegerValue( BCINT , BCINT , BCINT , BCINT , BCINT );
void InputNameFromListWithFunc( BCINT , BCINT , char *, BCINT , char **, BCINT , char *, BCINT , BCINT , void (*hookfunc)(BCINT , BCINT , BCINT , BCINT , char *));
	 
void InputNameFromList( BCINT , BCINT , char *, BCINT , char **, char *);
void InputFileName( BCINT , BCINT , char *, BCINT , char *);
Bool Confirm( BCINT , BCINT , char *, char *);
void Notify( BCINT , BCINT , char *, char *);
void DisplayMessage( BCINT , BCINT , char *, ...);
void NotImplemented( void);

/* from objects.c */
BCINT GetMaxObjectNum(BCINT );
void HighlightSelection( BCINT , SelPtr); /* SWAP! */
Bool IsSelected( SelPtr, BCINT );
void SelectObject( SelPtr *, BCINT );
void UnSelectObject( SelPtr *, BCINT );
void ForgetSelection( SelPtr *);
BCINT GetMaxObjectNum( BCINT);
BCINT GetCurObject( BCINT , BCINT , BCINT , BCINT , BCINT ); /* SWAP! */
SelPtr SelectObjectsInBox( BCINT , BCINT , BCINT , BCINT , BCINT ); /* SWAP! */
void HighlightObject( BCINT , BCINT , BCINT ); /* SWAP! */
void DeleteObject( BCINT , BCINT ); /* SWAP! */
void DeleteObjects( BCINT , SelPtr *); /* SWAP! */
void InsertObject( BCINT , BCINT , BCINT , BCINT ); /* SWAP! */
BCINT GetOppositeSector( BCINT, Bool); /* SWAP ! */
Bool IsLineDefInside( BCINT , BCINT , BCINT , BCINT , BCINT ); /* SWAP - needs Vertexes & LineDefs */
void CopyObjects( BCINT , SelPtr); /* SWAP! */
Bool MoveObjectsToCoords( BCINT , SelPtr, BCINT , BCINT , BCINT ); /* SWAP! */
void GetObjectCoords( BCINT , BCINT , BCINT  *, BCINT  *); /* SWAP! */
void RotateAndScaleObjects( BCINT , SelPtr, double, double); /* SWAP! */
BCINT FindFreeTag(void); /* SWAP! */
void FlipLineDefs( SelPtr, Bool); /* SWAP! */
void DeleteVerticesJoinLineDefs( SelPtr ); /* SWAP! */
void MergeVertices( SelPtr *); /* SWAP! */
Bool AutoMergeVertices( SelPtr *); /* SWAP! */
void SplitLineDefs( SelPtr); /* SWAP! */
void SplitSector( BCINT , BCINT ); /* SWAP! */
void SplitLineDefsAndSector( BCINT , BCINT ); /* SWAP! */
void MergeSectors( SelPtr *); /* SWAP! */
void DeleteLineDefsJoinSectors( SelPtr *); /* SWAP! */
void MakeDoorFromSector( BCINT ); /* SWAP! */
void MakeLiftFromSector( BCINT ); /* SWAP! */
void AlignTexturesY( SelPtr *, Bool, Bool); /* SWAP! */
void AlignTexturesX( SelPtr *, Bool, Bool); /* SWAP! */
void DistributeSectorFloors( SelPtr); /* SWAP! */
BCINT CommonVertex(BCINT, BCINT);
SelPtr rev_list(SelPtr);	/* return a new list, which is the reverse
							   of the original */
void delete_list(SelPtr);	/* use this to delete it afterwards */
void DistributeSectorCeilings( SelPtr); /* SWAP! */
void DistributeLightLevels( SelPtr); /* SWAP! */


/* from editobj.c */
void DisplayObjectInfo( BCINT , BCINT ); /* SWAP! */
BCINT InputObjectNumber( BCINT , BCINT , BCINT , BCINT );
BCINT InputObjectXRef( BCINT , BCINT , BCINT , Bool, BCINT );
Bool Input2VertexNumbers( BCINT, BCINT, char *, BCINT *, BCINT *);
void EditObjectsInfo( BCINT , BCINT , BCINT , SelPtr);
void CheckLevel( BCINT , BCINT ); /* SWAP! */
Bool CheckStartingPos( void); /* SWAP! */
void InsertStandardObject( BCINT , BCINT , BCINT , BCINT ); /* SWAP! */
void MiscOperations( BCINT , BCINT , BCINT , SelPtr *); /* SWAP! */
void Preferences( BCINT , BCINT );
BCINT SelectThingType(void);

/* from nodes.c */
void ShowProgress( BCINT );

/* from textures.c */
void ChooseFloorTexture( BCINT , BCINT , char *, BCINT , char **, char *);
void ChooseWallTexture( BCINT , BCINT , char *, BCINT , char **, char *);
void ChooseSprite( BCINT , BCINT , char *, char *);
void GetWallTextureSize( BCINT *, BCINT *, char *);
void *GetResource(char *);
void ForgetResource(char *);
Texture *FindTexture(char *);
Texture *FindTextureIn(char *, void *);
void ForgetAllResources(void);

/* from readcfg.c */
void readcfg(char *);
SList SList_append(SList, char *);

/* from swapmem.c */

#if defined(__TURBOC__)
void InitSwap( void);
void FreeSomeMemory( void);
void ObjectsNeeded( BCINT , ...);
#endif

/* end of file */
