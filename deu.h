/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   DEU.H - Main doom defines.
*/

/* the includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <graphics.h>



/*
   the version information
*/

#define DEU_VERSION    "4.20"       /* the version number */



/*
   the directory structure if the structre used by DOOM to order the
   data in it's WAD files
*/

typedef struct Directory *DirPtr;
struct Directory
{
   long start;          /* offset to start of data */
   long size;           /* byte size of data */
   char name[ 8];       /* name of data block */
};



/*
   The wad file pointer structure is used for holding the information
   on the wad files in a linked list.

   The first wad file in the main wad file. The rest a patches.
*/

typedef struct WadFileInfo *WadPtr;
struct WadFileInfo
{
   WadPtr next;               /* next file in linked list */
   char *filename;            /* name of the wad file */
   FILE *fileinfo;            /* c file stream information */
   char type[ 4];             /* type of wad file (IWAD or PWAD) */
   long dirsize;              /* directory size of WAD */
   long dirstart;             /* offset to start of directory */
   DirPtr directory;          /* array of directory information */
};



/*
   the master directory structure is used to build a complete directory
   of all the data blocks from all the various wad files
*/

typedef struct MasterDirectory *MDirPtr;
struct MasterDirectory
{
   MDirPtr next;              /* next in list */
   WadPtr wadfile;            /* file of origin */
   struct Directory dir;      /* directory data */
};



/*
   the macros and constants
*/

/* convert pointer coordinates to map coordinates */
#define MAPX(x)               (OrigX + (x - 319) * Scale)
#define MAPY(y)               (OrigY + (239 - y) * Scale)

/* object types */
#define OBJ_THINGS           1
#define OBJ_LINEDEFS         2
#define OBJ_SIDEDEFS         3
#define OBJ_VERTEXES         4
#define OBJ_SEGS             5
#define OBJ_SSECTORS         -1 /* Not implemented */
#define OBJ_NODES            -2 /* Not implemented */
#define OBJ_SECTORS          6
#define OBJ_REJECT           -3 /* Not implemented */
#define OBJ_BLOCKMAP         -4 /* Not implemented */

/* boolean constants */
#define TRUE                  1
#define FALSE                 0

/* half the size of an object (Thing or Vertex) in map coords */
#define OBJSIZE   10

/*
   the interfile global variables
*/

/* from deu.c */
extern int Registered;        /* registered or shareware WAD file? */

/* from wads.c */
extern WadPtr  WadFileList;   /* list of wad files */
extern MDirPtr MasterDir;     /* the master directory */

/* from edit.c */
extern int NumWTexture;       /* number of wall textures */
extern char **WTexture;       /* wall texture names */
extern int NumFTexture;       /* number of floor/ceiling textures */
extern char **FTexture;       /* floor/ceiling texture names */

/* from gfx.c */
extern int Scale;             /* scale to draw map 20 to 1 */
extern int OrigX;             /* the X origin */
extern int OrigY;             /* the Y origin */
extern int PointerX;          /* X position of pointer */
extern int PointerY;          /* Y position of pointer */

/* from mouse.c */
extern int UseMouse;          /* is there a mouse driver? */



/*
   the function prototypes
*/

/* from deu.c */
int main( int, char *[]);
void Credits( FILE *);
void Beep( void);
void ProgError( char *, ...);
void *GetMemory( size_t);
void *ResizeMemory( void *, size_t);
void MainLoop( void);

/* from wads.c */
void OpenWadFiles( int, char *[]);
void CloseWadFiles( void);
void CloseUnusedWadFiles( void);
void OpenMainWad( char *);
void OpenPatchWad( char *);
WadPtr BasicWadOpen( char *);
void BasicWadRead( WadPtr, void *, long);
void BasicWadSeek( WadPtr, long);
MDirPtr FindMasterDir( MDirPtr, char *);
void ListMasterDirectory( FILE *);
void ListFileDirectory( FILE *, WadPtr);
void BuildNewMainWad( char *);
void WriteBytes( FILE *, void *, long);

/* from edit.c */
void EditLevel( int, int);
void SelectLevel( int *, int *);
void ReadLevelData( int, int);
void ForgetLevelData( void);
void SaveLevelData( char *);
void ReadWTextureNames( void);
void ForgetFTextureNames( void);
void ReadFTextureNames( void);
void ForgetWTextureNames( void);
void EditorLoop( void);
void DrawMap( int);

/* from gfx.c */
void InitGfx( void);
void TermGfx( void);
void ClearScreen( void);
void DrawMapLine( int, int, int, int);
void DrawMapVector( int, int, int, int);
void DrawMapArrow( int, int, int);
void DrawScreenLine( int, int, int, int);
void DrawScreenBox( int, int, int, int);
void DrawScreenBox3D( int, int, int, int);
void DrawScreenText( int, int, char *, ...);
void DrawPointer( void);
int ComputeAngle( int, int);

/* from things.c */
int GetThingColour( int);
char *GetThingName( int);
char *GetAngleName( int);
char *GetWhenName( int);

/* from mouse.c */
void CheckMouseDriver( void);
void ShowMousePointer( void);
void HideMousePointer( void);
void GetMouseCoords( int *, int *, int *);
void SetMouseCoords( int, int);
void SetMouseLimits( int, int, int, int);
void ResetMouseLimits( void);

/* from menus.c */
int DisplayMenuArray( int, int, char *, int, char *[ 30]);
int DisplayMenu( int, int, char *, ...);
int InputInteger( int, int, int *, int, int);
int InputIntegerValue( int, int, int, int, int);
void InputNameFromList( int, int, char *, int, char **, char *);
void InputFileName( int, int, char *, int, char *);
int Confirm( int, int, char *, char *);
void DisplayMessage( int, int, char *, ...);
void NotImplemented( void);

/* from objects.c */
char *GetObjectTypeName( int);
char *GetEditModeName( int);
int GetCurObject( int);
void HighlightObject( int, int, int);
void DisplayObjectInfo( int, int);
void DeleteObject( int, int);
void InsertObject( int, int);
int DisplayThingsMenu( int, int, char *, ...);
int InputObjectNumber( int, int, int, int);
int InputObjectXRef( int, int, int, int, int);
void EditObjectInfo( int, int);
int IsLineDefInside( int, double, double, double, double);

/* from names.c */
char *GetLineDefTypeName( int);
char *GetLineDefFlagsName( int);
char *GetSectorTypeName( int);

/* end of file */
