/*
   Doom Editor Utility, by Brendon Wyber. Use and Abuse!

   DOOM.H - Main doom defines.
*/

/* the includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <graphics.h>
#include "things.h"



/*
   the version information
*/

#define DEU_VERSION    "3.00"       /* the version number */



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
   this data structure contains the information about the things
*/

typedef struct Thing *TPtr;
struct Thing
{
   int xpos;      /* x position */
   int ypos;      /* y position */
   int angle;     /* facing angle */
   int type;      /* thing type */
   int when;      /* appears when?? */
   TPtr next;     /* next thing in list */
};



/*
   the interfile global variables
*/

/* from wads.c */
extern WadPtr  WadFileList;   /* list of wad files */
extern MDirPtr MasterDir;     /* the master directory */

/* from gfx.c */
extern int Scale;             /* scale to draw map 20 to 1 */
extern int OrigX;             /* the X origen */
extern int OrigY;             /* the Y origen */



/*
   the function prototypes
*/

/* from deu.c */
int main( int, char *[]);
void Credits( FILE *);
void ParseArgs( int, char *[]);
void ProgError( char *, ...);
void *GetMemory( size_t);
void MainLoop( void);

/* from wads.c */
void OpenWadFiles( int, char *[]);
void CloseWadFiles( void);
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
void EditLevel( int, int, char *);
void ReadLevelData( int, int);
void ForgetLevelData( void);
void SaveLevelData( char *);
void EditorLoop( void);
void DrawMap( void);
void EditThingInfo( int *, int *, int *);

/* from gfx.c */
void InitGfx( void);
void TermGfx( void);
void ClearScreen( void);
void DrawMapLine( int, int, int, int);
void DrawScreenLine( int, int, int, int);
void DrawScreenBox( int, int, int, int);
void DrawScreenText( int, int, char *, ...);

/* from thing.c */
int GetThingColour( int);
char *GetThingName( int);
char *GetAngleName( int);
char *GetWhenName( int);

/* end of file */
