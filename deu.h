/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   DEU.H - Main doom defines.
*/

/* the includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <graphics.h>
#include <alloc.h>
#include <dos.h>
#include <bios.h>


/*
   the version information
*/

#define DEU_VERSION	"5.21"	/* the version number */



/*
   the directory structure is the structre used by DOOM to order the
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
   WadPtr next;			/* next file in linked list */
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
   SelPtr next;			/* next in list */
   int objnum;			/* object number */
};


/*
   syntactic sugar
*/
typedef int Bool;               /* Boolean data: true or false */


/*
   description of the command line arguments and config file keywords
*/

typedef struct
{
   char *short_name;		/* abbreviated command line argument */
   char *long_name;		/* command line arg. or keyword */
   enum				/* type of this option */
   {
      OPT_BOOLEAN,			/* boolean (toggle) */
      OPT_INTEGER,			/* integer number */
      OPT_STRING,			/* character string */
      OPT_STRINGACC,			/* character string, but store in a list */
      OPT_STRINGLIST,			/* list of character strings */
      OPT_END				/* end of the options description */
   } opt_type;
   char *msg_if_true;		/* message printed if option is true */
   char *msg_if_false;		/* message printed if option is false */
   void *data_ptr;              /* pointer to the data */
} OptDesc;


/*
   the macros and constants
*/

/* name of the configuration file */
#define DEU_CONFIG_FILE		"DEU.INI"

/* name of the log file (debug mode) */
#define DEU_LOG_FILE		"DEU.LOG"

/* convert screen coordinates to map coordinates */
#define MAPX(x)			(OrigX + (int) (((x) - ScrCenterX) / Scale))
#define MAPY(y)			(OrigY + (int) ((ScrCenterY - (y)) / Scale))

/* convert map coordinates to screen coordinates */
#define SCREENX(x)		(ScrCenterX + (int) (((x) - OrigX) * Scale))
#define SCREENY(y)		(ScrCenterY + (int) ((OrigY - (y)) * Scale))

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
#define TRUE			1
#define FALSE			0

/* half the size of an object (Thing or Vertex) in map coords */
#define OBJSIZE			7


/*
   the interfile global variables
*/

/* from deu.c */
extern Bool  Registered;	/* registered or shareware WAD file? */
extern Bool  Debug;		/* are we debugging? */
extern Bool  SwapButtons;	/* swap right and middle mouse buttons */
extern Bool  Quiet;		/* don't play a sound when an object is selected */
extern Bool  Quieter;		/* don't play any sound, even when an error occurs */
extern Bool  Expert;		/* don't ask for confirmation for some operations */
extern int   InitialScale;	/* initial zoom factor for map */
extern int   VideoMode;		/* default video mode for VESA cards */
extern char *BGIDriver;		/* default extended BGI driver */
extern Bool  FakeCursor;	/* use a "fake" mouse cursor */
extern Bool  CirrusCursor;	/* use hardware cursor on Cirrus Logic VGA cards */
extern Bool  Colour2;		/* use the alternate set for things colors */
extern Bool  AdditiveSelBox;	/* additive selection box or select in box only? */
extern int   SplitFactor;	/* factor used by the Nodes builder */
extern Bool  Select0;		/* select object 0 by default when switching modes */
extern char *MainWad;		/* name of the main wad file */
extern FILE *logfile;		/* filepointer to the error log */

/* from wads.c */
extern WadPtr  WadFileList;	/* list of wad files */
extern MDirPtr MasterDir;	/* the master directory */

/* from edit.c */
extern Bool InfoShown;          /* is the bottom line displayed? */

/* from gfx.c */
extern int   GfxMode;		/* current graphics mode, or 0 for text */
extern float Scale;		/* scale to draw map 20 to 1 */
extern int   OrigX;		/* the X origin */
extern int   OrigY;		/* the Y origin */
extern int   PointerX;		/* X position of pointer */
extern int   PointerY;		/* Y position of pointer */
extern int   ScrMaxX;		/* maximum X screen coord */
extern int   ScrMaxY;		/* maximum Y screen coord */
extern int   ScrCenterX;	/* X coord of screen center */
extern int   ScrCenterY;	/* Y coord of screen center */

/* from mouse.c */
extern Bool UseMouse;		/* is there a mouse driver? */



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
void PlaySound( int, int);
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
int Exists( char *);
void DumpDirectoryEntry( FILE *, char *);
void SaveDirectoryEntry( FILE *, char *);
void SaveEntryToRawFile( FILE *, char *);
void SaveEntryFromRawFile( FILE *, FILE *, char *);

/* from levels.c */
void ReadLevelData( int, int); /* SWAP! */
void ForgetLevelData( void); /* SWAP! */
void SaveLevelData( char *); /* SWAP! */
void ReadWTextureNames( void);
void ForgetFTextureNames( void);
void ReadFTextureNames( void);
void ForgetWTextureNames( void);

/* from edit.c */
void EditLevel( int, int, Bool);
void SelectLevel( int *, int *);
void EditorLoop( int, int); /* SWAP! */
void DrawMap( int, int, Bool); /* SWAP! */
void CenterMapAroundCoords( int, int);
void GoToObject( int, int); /* SWAP! */

/* from gfx.c */
void InitGfx( void);
Bool SwitchToVGA256( void);
Bool SwitchToVGA16( void);
void TermGfx( void);
void ClearScreen( void);
void SetColor( int);
void DrawMapLine( int, int, int, int);
void DrawMapCircle( int, int, int);
void DrawMapVector( int, int, int, int);
void DrawMapArrow( int, int, unsigned);
void DrawScreenLine( int, int, int, int);
void DrawScreenBox( int, int, int, int);
void DrawScreenBox3D( int, int, int, int);
void DrawScreenBoxHollow( int, int, int, int);
void DrawScreenMeter( int, int, int, int, float);
void DrawScreenText( int, int, char *, ...);
void DrawPointer( Bool);
void SetDoomPalette( int);
int TranslateToDoomColor( int);
unsigned ComputeAngle( int, int);
unsigned ComputeDist( int, int);
void InsertPolygonVertices( int, int, int, int);
void RotateAndScaleCoords( int *, int *, double, double);
#ifdef CIRRUS_PATCH
void SetHWCursorPos( unsigned, unsigned);
void SetHWCursorCol( long, long);
void SetHWCursorMap( char *);
#endif /* CIRRUS_PATCH */

/* from things.c */
int GetThingColour( int);
char *GetThingName( int);
int GetThingRadius( int);
char *GetAngleName( int);
char *GetWhenName( int);

/* from names.c */
char *GetObjectTypeName( int);
char *GetEditModeName( int);
char *GetLineDefTypeName( int);
char *GetLineDefTypeLongName( int);
char *GetLineDefFlagsName( int);
char *GetLineDefFlagsLongName( int);
char *GetSectorTypeName( int);
char *GetSectorTypeLongName( int);

/* from mouse.c */
void CheckMouseDriver( void);
void ShowMousePointer( void);
void HideMousePointer( void);
void GetMouseCoords( int *, int *, int *);
void SetMouseCoords( int, int);
void SetMouseLimits( int, int, int, int);
void ResetMouseLimits( void);
void MouseCallBackFunction( void);

/* from menus.c */
int DisplayMenuArray( int, int, char *, int, int *, char *[ 30], int [30]);
int DisplayMenu( int, int, char *, ...);
int PullDownMenu( int, int, ...);
int InputInteger( int, int, int *, int, int);
int InputIntegerValue( int, int, int, int, int);
void InputNameFromListWithFunc( int, int, char *, int, char **, int, char *, int, int, void (*hookfunc)(int, int, int, int, char *));
void InputNameFromList( int, int, char *, int, char **, char *);
void InputFileName( int, int, char *, int, char *);
Bool Confirm( int, int, char *, char *);
void Notify( int, int, char *, char *);
void DisplayMessage( int, int, char *, ...);
void NotImplemented( void);

/* from objects.c */
void HighlightSelection( int, SelPtr); /* SWAP! */
Bool IsSelected( SelPtr, int);
void SelectObject( SelPtr *, int);
void UnSelectObject( SelPtr *, int);
void ForgetSelection( SelPtr *);
int GetMaxObjectNum( int);
int GetCurObject( int, int, int, int, int); /* SWAP! */
SelPtr SelectObjectsInBox( int, int, int, int, int); /* SWAP! */
void HighlightObject( int, int, int); /* SWAP! */
void DeleteObject( int, int); /* SWAP! */
void DeleteObjects( int, SelPtr *); /* SWAP! */
void InsertObject( int, int, int, int); /* SWAP! */
Bool IsLineDefInside( int, int, int, int, int); /* SWAP - needs Vertexes & LineDefs */
int GetOppositeSector( int, Bool); /* SWAP! */
void CopyObjects( int, SelPtr); /* SWAP! */
Bool MoveObjectsToCoords( int, SelPtr, int, int, int); /* SWAP! */
void GetObjectCoords( int, int, int *, int *); /* SWAP! */
void RotateAndScaleObjects( int, SelPtr, double, double); /* SWAP! */
int FindFreeTag( void); /* SWAP! */
void FlipLineDefs( SelPtr, Bool); /* SWAP! */
void DeleteVerticesJoinLineDefs( SelPtr ); /* SWAP! */
void MergeVertices( SelPtr *); /* SWAP! */
Bool AutoMergeVertices( SelPtr *); /* SWAP! */
void SplitLineDefs( SelPtr); /* SWAP! */
void SplitSector( int, int); /* SWAP! */
void SplitLineDefsAndSector( int, int); /* SWAP! */
void MergeSectors( SelPtr *); /* SWAP! */
void DeleteLineDefsJoinSectors( SelPtr *); /* SWAP! */
void MakeDoorFromSector( int); /* SWAP! */
void MakeLiftFromSector( int); /* SWAP! */
void AlignTexturesY( SelPtr *); /* SWAP! */
void AlignTexturesX( SelPtr *); /* SWAP! */
void DistributeSectorFloors( SelPtr); /* SWAP! */
void DistributeSectorCeilings( SelPtr); /* SWAP! */

/* from editobj.c */
void DisplayObjectInfo( int, int); /* SWAP! */
int DisplayThingsMenu( int, int, char *, ...);
int DisplayLineDefTypeMenu( int, int, char *, ...);
int InputObjectNumber( int, int, int, int);
int InputObjectXRef( int, int, int, Bool, int);
Bool Input2VertexNumbers( int, int, char *, int *, int *);
void EditObjectsInfo( int, int, int, SelPtr);
void CheckLevel( int, int); /* SWAP! */
Bool CheckStartingPos( void); /* SWAP! */
void InsertStandardObject( int, int, int, int); /* SWAP! */
void MiscOperations( int, int, int, SelPtr *); /* SWAP! */
void Preferences( int, int);

/* from nodes.c */
void ShowProgress( int);

/* from textures.c */
void ChooseFloorTexture( int, int, char *, int, char **, char *);
void ChooseWallTexture( int, int, char *, int, char **, char *);
void ChooseSprite( int, int, char *, char *);
void GetWallTextureSize( int *, int *, char *);

/* from swapmem.c */
void InitSwap( void);
void FreeSomeMemory( void);
void ObjectsNeeded( int, ...);


/* end of file */
