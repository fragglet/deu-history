/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   EDIT.C - Editor routines.
*/

/* the includes */
#include "deu.h"
#include "wstructs.h"
#include "things.h"

/* the global data */
MDirPtr Level = NULL;         /* master dictionary entry for the level */
int NumThings = 0;            /* number of things */
TPtr Things;                  /* things data */
int NumLineDefs = 0;          /* number of line defs */
LDPtr LineDefs;               /* line defs data */
int NumSideDefs = 0;          /* number of side defs */
SDPtr SideDefs;               /* side defs data */
int NumVertexes = 0;          /* number of vertexes */
VPtr Vertexes;                /* vertex data */
int NumSegs = 0;	      /* number of segments */
SEPtr Segs;                   /* segments data */
int NumSSectors = 0;	      /* number of subsectors */
SSPtr SSectors;               /* subsectors data */
int NumSectors = 0;	      /* number of sectors */
SPtr Sectors;                 /* sectors data */
int NumTexture1 = 0;	      /* number of wall textures */
char **Texture1;              /* array of "texture1" names (walls) */
int NumFTexture = 0;	      /* number of floor/ceiling textures */
char **FTexture;              /* array of texture names */
int MaxX = 0;                 /* maximum X value of map */
int MaxY = 0;                 /* maximum Y value of map */
int MinX = 0;                 /* minimum X value of map */
int MinY = 0;                 /* minimum Y value of map */
int MoveSpeed = 20;           /* movement speed */
int SaveChanges = FALSE;      /* save changes */
int MadeChanges = FALSE;      /* made changes changes */


/*
   the driving program
*/

void EditLevel( int game, int level, char *dest)
{
   char *outfile = GetMemory( strlen( dest) + 2);

   strcpy( outfile, dest);
   ReadLevelData( game, level);
   ReadTexture1Names();
   ReadFTextureNames();
   InitGfx();
   CheckMouseDriver();
   EditorLoop();
   TermGfx();
   if (! Registered)
      printf( "Read-only mode: file \"%s\" not saved.\n", outfile);
   else if (SaveChanges == FALSE)
      printf( "Editing aborted...\n");
   else if (MadeChanges == FALSE)
      printf( "No changes made, file \"%s\" not saved.\n", outfile);
   else
      SaveLevelData( outfile);
   ForgetLevelData();
   ForgetTexture1Names();
   ForgetFTextureNames();
   if (SaveChanges && MadeChanges)
      OpenPatchWad( outfile);
}



/*
   read in the level data
*/

void ReadLevelData( int game, int level)
{
   MDirPtr ThingsMDP, LineDefsMDP, SideDefsMDP, VertexesMDP;
   MDirPtr SegsMDP, SSectorsMDP, SectorsMDP;
   char name[ 5];
   int n;
   int val;

   /* find the various level information from the master dictionary */
   name[ 0] = 'E';
   name[ 1] = '0' + game;
   name[ 2] = 'M';
   name[ 3] = '0' + level;
   name[ 4] = '\0';
   printf( "Reading data for level %s.\n", name);
   Level = FindMasterDir( MasterDir, name);
   if (!Level)
      ProgError( "level data not found");
   ThingsMDP = FindMasterDir( Level, "THINGS");
   LineDefsMDP = FindMasterDir( Level, "LINEDEFS");
   SideDefsMDP = FindMasterDir( Level, "SIDEDEFS");
   VertexesMDP = FindMasterDir( Level, "VERTEXES");
   SegsMDP = FindMasterDir( Level, "SEGS");
   SSectorsMDP = FindMasterDir( Level, "SSECTORS");
   SectorsMDP = FindMasterDir( Level, "SECTORS");

   /* read in the things data */
   NumThings = ThingsMDP->dir.size / 10;
   Things = GetMemory( NumThings * sizeof( struct Thing));
   BasicWadSeek( ThingsMDP->wadfile, ThingsMDP->dir.start);
   for (n = 0; n < NumThings; n++)
   {
      BasicWadRead( ThingsMDP->wadfile, &(Things[ n].xpos), 2);
      BasicWadRead( ThingsMDP->wadfile, &(Things[ n].ypos), 2);
      BasicWadRead( ThingsMDP->wadfile, &(Things[ n].angle), 2);
      BasicWadRead( ThingsMDP->wadfile, &(Things[ n].type), 2);
      BasicWadRead( ThingsMDP->wadfile, &(Things[ n].when), 2);
   }

   /* read in the line def information */
   NumLineDefs = LineDefsMDP->dir.size / 14;
   LineDefs = GetMemory( NumLineDefs * sizeof( struct LineDef));
   BasicWadSeek( LineDefsMDP->wadfile, LineDefsMDP->dir.start);
   for (n = 0; n < NumLineDefs; n++)
   {
      BasicWadRead( LineDefsMDP->wadfile, &(LineDefs[ n].start), 2);
      BasicWadRead( LineDefsMDP->wadfile, &(LineDefs[ n].end), 2);
      BasicWadRead( LineDefsMDP->wadfile, &(LineDefs[ n].flags1), 2);
      BasicWadRead( LineDefsMDP->wadfile, &(LineDefs[ n].flags2), 2);
      BasicWadRead( LineDefsMDP->wadfile, &(LineDefs[ n].tag), 2);
      BasicWadRead( LineDefsMDP->wadfile, &(LineDefs[ n].sidedef1), 2);
      BasicWadRead( LineDefsMDP->wadfile, &(LineDefs[ n].sidedef2), 2);
   }

   /* read in the side def information */
   NumSideDefs = SideDefsMDP->dir.size / 30;
   SideDefs = GetMemory( NumSideDefs * sizeof( struct SideDef));
   BasicWadSeek( SideDefsMDP->wadfile, SideDefsMDP->dir.start);
   for (n = 0; n < NumSideDefs; n++)
   {
      BasicWadRead( SideDefsMDP->wadfile, &(SideDefs[ n].xoff), 2);
      BasicWadRead( SideDefsMDP->wadfile, &(SideDefs[ n].yoff), 2);
      BasicWadRead( SideDefsMDP->wadfile, &(SideDefs[ n].tex1), 8);
      BasicWadRead( SideDefsMDP->wadfile, &(SideDefs[ n].tex2), 8);
      BasicWadRead( SideDefsMDP->wadfile, &(SideDefs[ n].tex3), 8);
      BasicWadRead( SideDefsMDP->wadfile, &(SideDefs[ n].sector), 2);
   }

   /* read in the vertexes which are all the corners of the level */
   NumVertexes = VertexesMDP->dir.size / 4;
   Vertexes = GetMemory( NumVertexes * sizeof( struct Vertex));
   BasicWadSeek( VertexesMDP->wadfile, VertexesMDP->dir.start);
   for (n = 0; n < NumVertexes; n++)
   {
      BasicWadRead( VertexesMDP->wadfile, &val, 2);
      if (val < MinX)
	 MinX = val;
      if (val > MaxX)
	 MaxX = val;
      Vertexes[ n].x = val;
      BasicWadRead( VertexesMDP->wadfile, &val, 2);
      if (val < MinY)
	 MinY = val;
      if (val > MaxY)
	 MaxY = val;
      Vertexes[ n].y = val;
   }

   /* read in the segments information */
   NumSegs = SegsMDP->dir.size / 12;
   Segs = GetMemory( NumSegs * sizeof( struct Seg));
   BasicWadSeek( SegsMDP->wadfile, SegsMDP->dir.start);
   for (n = 0; n < NumSegs; n++)
   {
      BasicWadRead( SegsMDP->wadfile, &(Segs[ n].start), 2);
      BasicWadRead( SegsMDP->wadfile, &(Segs[ n].end), 2);
      BasicWadRead( SegsMDP->wadfile, &(Segs[ n].angle), 2);
      BasicWadRead( SegsMDP->wadfile, &(Segs[ n].linedef), 2);
      BasicWadRead( SegsMDP->wadfile, &(Segs[ n].flip), 2);
      BasicWadRead( SegsMDP->wadfile, &(Segs[ n].flags), 2);
   }

   /* read in the subsectors information */
   NumSSectors = SSectorsMDP->dir.size / 4;
   SSectors = GetMemory( NumSSectors * sizeof( struct SSector));
   BasicWadSeek( SSectorsMDP->wadfile, SSectorsMDP->dir.start);
   for (n = 0; n < NumSSectors; n++)
   {
      BasicWadRead( SSectorsMDP->wadfile, &(SSectors[ n].num), 2);
      BasicWadRead( SSectorsMDP->wadfile, &(SSectors[ n].first), 2);
   }

   /* ignore the nodes */

   /* read in the sectors information */
   NumSectors = SectorsMDP->dir.size / 26;
   Sectors = GetMemory( NumSectors * sizeof( struct Sector));
   BasicWadSeek( SectorsMDP->wadfile, SectorsMDP->dir.start);
   for (n = 0; n < NumSectors; n++)
   {
      BasicWadRead( SectorsMDP->wadfile, &(Sectors[ n].floorh), 2);
      BasicWadRead( SectorsMDP->wadfile, &(Sectors[ n].ceilh), 2);
      BasicWadRead( SectorsMDP->wadfile, &(Sectors[ n].floort), 8);
      BasicWadRead( SectorsMDP->wadfile, &(Sectors[ n].ceilt), 8);
      BasicWadRead( SectorsMDP->wadfile, &(Sectors[ n].light), 2);
      BasicWadRead( SectorsMDP->wadfile, &(Sectors[ n].special), 2);
      BasicWadRead( SectorsMDP->wadfile, &(Sectors[ n].tag), 2);
   }

   /* ignore the last entries (reject & blockmap) */
}



/*
   forget the level data
*/

void ForgetLevelData()
{
   /* forget the level pointer */
   Level = NULL;

   /* forget the things */
   NumThings = 0;
   free( Things);

   /* forget the line defs */
   NumLineDefs = 0;
   free( LineDefs);

   /* forget the side defs */
   NumSideDefs = 0;
   free( SideDefs);

   /* forget the vertexes */
   NumVertexes = 0;
   free( Vertexes);
   MaxX = 0;
   MaxY = 0;
   MinX = 0;
   MinY = 0;

   /* forget the segments */
   NumSegs = 0;
   free( Segs);

   /* forget the subsectors */
   NumSSectors = 0;
   free( SSectors);

   /* forget the sectors */
   NumSectors = 0;
   free( Sectors);
}



/*
   save the level data to a PWAD file
*/

void SaveLevelData( char *outfile)
{
   FILE *file;
   MDirPtr dir;
   long counter = 11;
   int n;
   void *data;
   long size;
   long dirstart;
   TPtr thing;

   /* open the file */
   printf( "Saving data to \"%s\"...\n", outfile);
   if ((file = fopen( outfile, "wb")) == NULL)
      ProgError( "Unable to open file \"%s\"", outfile);
   WriteBytes( file, "PWAD", 4L);     /* PWAD file */
   WriteBytes( file, &counter, 4L);   /* 11 entries */
   WriteBytes( file, &counter, 4L);   /* fix this up later */
   counter = 12;
   dir = Level->next;

   /* output the things data */
   for (n = 0; n < NumThings; n++)
   {
      WriteBytes( file, &(Things[ n].xpos), 2L);
      WriteBytes( file, &(Things[ n].ypos), 2L);
      WriteBytes( file, &(Things[ n].angle), 2L);
      WriteBytes( file, &(Things[ n].type), 2L);
      WriteBytes( file, &(Things[ n].when), 2L);
      counter += 10;
   }
   dir = dir->next;

   /* output the linedefs */
   for (n = 0; n < NumLineDefs; n++)
   {
      WriteBytes( file, &(LineDefs[ n].start), 2L);
      WriteBytes( file, &(LineDefs[ n].end), 2L);
      WriteBytes( file, &(LineDefs[ n].flags1), 2L);
      WriteBytes( file, &(LineDefs[ n].flags2), 2L);
      WriteBytes( file, &(LineDefs[ n].tag), 2L);
      WriteBytes( file, &(LineDefs[ n].sidedef1), 2L);
      WriteBytes( file, &(LineDefs[ n].sidedef2), 2L);
      counter += 14;
   }
   dir = dir->next;

   /* output the sidedefs */
   for (n = 0; n < NumSideDefs; n++)
   {
      WriteBytes( file, &(SideDefs[ n].xoff), 2L);
      WriteBytes( file, &(SideDefs[ n].yoff), 2L);
      WriteBytes( file, &(SideDefs[ n].tex1), 8L);
      WriteBytes( file, &(SideDefs[ n].tex2), 8L);
      WriteBytes( file, &(SideDefs[ n].tex3), 8L);
      WriteBytes( file, &(SideDefs[ n].sector), 2L);
      counter += 30;
   }
   dir = dir->next;

   /* output the vertexes */
   for (n = 0; n < NumVertexes; n++)
   {
      WriteBytes( file, &(Vertexes[ n].x), 2L);
      WriteBytes( file, &(Vertexes[ n].y), 2L);
      counter += 4;
   }
   dir = dir->next;

   /* output the segs */
   for (n = 0; n < NumSegs; n++)
   {
      WriteBytes( file, &(Segs[ n].start), 2L);
      WriteBytes( file, &(Segs[ n].end), 2L);
      WriteBytes( file, &(Segs[ n].angle), 2L);
      WriteBytes( file, &(Segs[ n].linedef), 2L);
      WriteBytes( file, &(Segs[ n].flip), 2L);
      WriteBytes( file, &(Segs[ n].flags), 2L);
      counter += 12;
   }
   dir = dir->next;

   /* output the ssectors */
   for (n = 0; n < NumSSectors; n++)
   {
      WriteBytes( file, &(SSectors[ n].num), 2L);
      WriteBytes( file, &(SSectors[ n].first), 2L);
      counter += 4;
   }
   dir = dir->next;

   /* copy the nodes */
   data = GetMemory( 0x8000 + 2);
   size = dir->dir.size;
   counter += size;
   BasicWadSeek( dir->wadfile, dir->dir.start);
   while (size > 0x8000)
   {
      BasicWadRead( dir->wadfile, data, 0x8000);
      WriteBytes( file, data, 0x8000);
      size -= 0x8000;
   }
   BasicWadRead( dir->wadfile, data, size);
   WriteBytes( file, data, size);
   dir = dir->next;
   free( data);

   /* output the sectors */
   for (n = 0; n < NumSectors; n++)
   {
      WriteBytes( file, &(Sectors[ n].floorh), 2L);
      WriteBytes( file, &(Sectors[ n].ceilh), 2L);
      WriteBytes( file, &(Sectors[ n].floort), 8L);
      WriteBytes( file, &(Sectors[ n].ceilt), 8L);
      WriteBytes( file, &(Sectors[ n].light), 2L);
      WriteBytes( file, &(Sectors[ n].special), 2L);
      WriteBytes( file, &(Sectors[ n].tag), 2L);
      counter += 26;
   }
   dir = dir->next;

   /* copy the other 2 directory entries with data (reject & blockmap) */
   data = GetMemory( 0x8000 + 2);
   for (n = 0; n < 2; n++)
   {
      size = dir->dir.size;
      counter += size;
      BasicWadSeek( dir->wadfile, dir->dir.start);
      while (size > 0x8000)
      {
	 BasicWadRead( dir->wadfile, data, 0x8000);
	 WriteBytes( file, data, 0x8000);
	 size -= 0x8000;
      }
      BasicWadRead( dir->wadfile, data, size);
      WriteBytes( file, data, size);
      dir = dir->next;
   }
   free( data);

   /* nasty trick... */
   if (FindMasterDir( MasterDir, "E3M1") == NULL)
      counter++;

   /* output the actual directory */
   dirstart = counter;
   counter = 12;
   size = 0;
   dir = Level;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, &(dir->dir.name), 8L);
   dir = dir->next;

   size = NumThings * 10;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "THINGS\0\0", 8L);
   counter += size;
   dir = dir->next;

   size = NumLineDefs * 14;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "LINEDEFS", 8L);
   counter += size;
   dir = dir->next;

   size = NumSideDefs * 30;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SIDEDEFS", 8L);
   counter += size;
   dir = dir->next;

   size = NumVertexes * 4;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "VERTEXES", 8L);
   counter += size;
   dir = dir->next;

   size = NumSegs * 12;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SEGS\0\0\0\0", 8L);
   counter += size;
   dir = dir->next;

   size = NumSSectors * 4;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SSECTORS", 8L);
   counter += size;
   dir = dir->next;

   size = dir->dir.size;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, &(dir->dir.name), 8L); /* NODES */
   counter += size;
   dir = dir->next;

   size = NumSectors * 26;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SECTORS\0", 8L);
   counter += size;
   dir = dir->next;

   for (n = 0; n < 2; n++)
   {
      size = dir->dir.size;
      WriteBytes( file, &counter, 4L);
      WriteBytes( file, &size, 4L);
      WriteBytes( file, &(dir->dir.name), 8L);
      counter += size;
      dir = dir->next;
   }

   /* fix up the directory start information */
   if (fseek( file, 8L, 0))
      ProgError( "error writing to file");
   WriteBytes( file, &dirstart, 4L);

   /* close the file */
   fclose( file);
}



/*
   read in the "texture1" names (walls)
*/

void ReadTexture1Names()
{
   MDirPtr dir;
   long *offsets;
   int n;
   long val;

   printf("Reading wall texture names\n");
   dir = FindMasterDir( MasterDir, "TEXTURE1");
   BasicWadSeek( dir->wadfile, dir->dir.start);
   BasicWadRead( dir->wadfile, &val, 4);
   NumTexture1 = val + 1;
   /* read in the offsets for texture1 names */
   offsets = GetMemory( NumTexture1 * sizeof( long));
   for (n = 1; n < NumTexture1; n++)
      BasicWadRead( dir->wadfile, &(offsets[ n]), 4);
   /* read in the actual names */
   Texture1 = GetMemory( NumTexture1 * sizeof( char *));
   Texture1[ 0] = GetMemory( 9 * sizeof( char));
   strcpy(Texture1[ 0], "-");
   for (n = 1; n < NumTexture1; n++)
   {
      Texture1[ n] = GetMemory( 9 * sizeof( char));
      BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ n]);
      BasicWadRead( dir->wadfile, Texture1[ n], 8);
      Texture1[ n][ 8] = '\0';
   }
   free( offsets);
}



/*
   forget the "texture1" names
*/

void ForgetTexture1Names()
{
   int n;

   /* forget all names */
   for (n = 0; n < NumTexture1; n++)
      free( Texture1[ n]);

   /* forget the array */
   NumTexture1 = 0;
   free( Texture1);
}



/*
   fuction used by qsort to sort the texture names
*/
int SortTextures( const void *a, const void *b)
{
   return strcmp( *((char **)a), *((char **)b));
}


/*
   read in the floor/ceiling texture names
*/

void ReadFTextureNames()
{
   MDirPtr dir;
   int n;

   printf("Reading floor/ceiling texture names\n");
   /* count the names */
   dir = FindMasterDir( MasterDir, "F1_START");
   dir = dir->next;
   for (n = 0; dir && strcmp(dir->dir.name, "F1_END"); n++)
      dir = dir->next;
   NumFTexture = n;
   /* get the actual names from master dir. */
   dir = FindMasterDir( MasterDir, "F1_START");
   dir = dir->next;
   FTexture = GetMemory( NumFTexture * sizeof( char *));
   for (n = 0; n < NumFTexture; n++)
   {
      FTexture[ n] = GetMemory( 9 * sizeof( char));
      strncpy( FTexture[ n], dir->dir.name, 8);
      FTexture[ n][ 8] = '\0';
      dir = dir->next;
   }
   /* sort the names */
   qsort( FTexture, NumFTexture, sizeof( char *), SortTextures);
}



/*
   forget the floor/ceiling texture names
*/

void ForgetFTextureNames()
{
   int n;

   /* forget all names */
   for (n = 0; n < NumFTexture; n++)
      free( FTexture[ n]);

   /* forget the array */
   NumFTexture = 0;
   free( FTexture);
}



/*
   display the help screen
 */

void DisplayHelp( int objtype, int objnum)
{
   if (UseMouse)
      HideMousePointer();
   /* put in the instructions */
   DrawScreenBox3D( 140, 70, 500, 395);
   setcolor( LIGHTCYAN);
   DrawScreenText( 240, 92, "Doom Editor Utility");
   DrawScreenText( 272 - strlen(GetObjectType( objtype)) * 4, 105, "- %s Editor -", GetObjectType( objtype));
   setcolor( BLACK);
   DrawScreenText( 150, 135, "Use the mouse or the cursor keys to move");
   DrawScreenText( 150, 145, "around.  The map scrolls when the pointer");
   DrawScreenText( 150, 155, "reaches the edge of the screen.");
   DrawScreenText( 150, 175, "Other useful keys are:");
   DrawScreenText( 150, 190, "Tab   - Switch to the next editing mode.");
   DrawScreenText( 150, 200, "Space - Change the move/scroll speed.");
   DrawScreenText( 150, 210, "Esc   - Exit without saving changes.");
   if (Registered)
      DrawScreenText( 150, 220, "Q     - Quit, saving changes.");
   else
   {
      setcolor( DARKGRAY);
      DrawScreenText( 150, 220, "Q     - Quit without saving changes.");
      setcolor( BLACK);
   }
   DrawScreenText( 150, 230, "+/-   - Change the map scale (current: %d).", Scale);
   if (GetCurObject( objtype) >= 0)
      setcolor( DARKGRAY);
   DrawScreenText( 150, 240, "N, >  - Jump to the next object.");
   DrawScreenText( 150, 250, "P, <  - Jump to the previous object.");
   setcolor( BLACK);
   if (objtype == OBJ_THINGS)
   {
      if (GetCurObject( objtype) >= 0)
	 DrawScreenText( 150, 260, "Ins   - Drag the current thing");
      else
	 DrawScreenText( 150, 260, "Ins   - Insert a default thing");
      DrawScreenText( 150, 270, "        (%s).", GetThingName( Things[ objnum].type));
   }
   else
   {
      DrawScreenText( 150, 260, "Ins   - Insert a new object or drag");
      DrawScreenText( 150, 270, "        the current one.");
   }
   if ((objtype == OBJ_THINGS) && (objnum >= 0))
   {
      DrawScreenText( 150, 280, "Del   - Delete the selected object");
      DrawScreenText( 150, 290, "        (%s).", GetThingName( Things[ objnum].type));
   }
   else
   {
      setcolor( DARKGRAY);
      DrawScreenText( 150, 280, "Del   - Delete the selected object");
      DrawScreenText( 150, 290, "        (none for now).");
      setcolor( BLACK);
   }
   DrawScreenText( 150, 300, "Enter - Edit the default/selected object.");
   DrawScreenText( 150, 320, "Mouse buttons:");
   DrawScreenText( 150, 335, "Left  - Insert or drag an object.");
   DrawScreenText( 150, 345, "Middle- Edit the default/selected object.");
   if ((objtype != OBJ_THINGS) || (objnum < 0))
      setcolor( DARKGRAY);
   DrawScreenText( 150, 355, "Right - Delete the selected object.");
   setcolor( YELLOW);
   DrawScreenText( 150, 380, "Press any key to return to the editor...");
   bioskey( 0);
   if (UseMouse)
      ShowMousePointer();
}



/*
   the editor main loop
*/

void EditorLoop()
{
   int EditMode = OBJ_THINGS;
   int CurObject = -1;
   int OldObject = -1;
   int RedrawMap = TRUE;
   int RedrawObj = FALSE;
   int DragObject = FALSE;
   int *NumObjPtr = &NumThings;
   int key, buttons, oldbuttons;

   OrigX = (MinX + MaxX) / 2;
   OrigY = (MinY + MaxY) / 2;
   Scale = 10;
   SaveChanges = MadeChanges = 0;
   PointerX = 319;
   PointerY = 239;
   if (UseMouse)
   {
      SetMouseCoords(PointerX, PointerY);
      ShowMousePointer();
      oldbuttons = 0;
   }

   for (;;)
   {
      key = 0;

      /* get mouse position and button status */
      if (UseMouse)
      {
	 GetMouseCoords( &PointerX, &PointerY, &buttons);
	 if (buttons != oldbuttons)
	    switch (buttons)
	    {
	    case 1:
	       if (! DragObject)
		  key = 0x5200; /* Press left button = Ins */
	       break;
	    case 2:
	       key = 0x5300; /* Press right button = Del */
	       break;
	    case 3:
	    case 4:
	       key = 0x000D; /* Press middle button = Enter */
	       break;
	    default:
	       if (DragObject)
		  key = 0x5200; /* Release left button = Ins */
	       break;
	    }
	 oldbuttons = buttons;
      }

      if (DragObject)
      {
	 /* if we are dragging an object, move it */
	 switch (EditMode)
	 {
	    case OBJ_THINGS:
	       Things[ CurObject].xpos  = MAPX(PointerX);
	       Things[ CurObject].ypos  = MAPY(PointerY);
	       break;
	    case OBJ_VERTEXES:
	       Vertexes[ CurObject].x = MAPX( PointerX);
	       Vertexes[ CurObject].y = MAPY( PointerY);
	       break;
	 }
	 RedrawMap = TRUE;
      }
      else if (!RedrawObj)
      {
	 /* check if there is something near the pointer */
	 OldObject = CurObject;
	 CurObject = GetCurObject( EditMode);
	 if (CurObject < 0)
	    CurObject = OldObject;
      }

      /* draw the map */
      if (RedrawMap)
      {
	 if (UseMouse)
	    HideMousePointer();
	 DrawMap( EditMode);
	 if (UseMouse)
	    ShowMousePointer();
      }

      /* highlight the selected object and display the information box */
      if (RedrawMap || CurObject != OldObject || RedrawObj)
      {
	 RedrawObj = FALSE;
	 if (UseMouse)
	    HideMousePointer();
	 if (!RedrawMap && OldObject >= 0)
	    HighlightObject( EditMode, OldObject);
	 if (CurObject != OldObject)
	 {
	    sound( 50);
	    delay( 10);
	    nosound();
	    OldObject = CurObject;
	 }
	 if (bioskey( 1)) /* speedup */
	    RedrawObj = TRUE;
	 else
	    DisplayObjectInfo( EditMode, CurObject);
	 if (CurObject >= 0)
	    HighlightObject( EditMode, CurObject);
	 if (UseMouse)
	    ShowMousePointer();
      }
      if (RedrawMap && !UseMouse)
	 DrawPointer();

      /* the map is up to date */
      RedrawMap = FALSE;

      /* get user input */
      if (bioskey( 1) || key)
      {
	 if (! key)
	    key = bioskey( 0);

	 /* erase the (keyboard) pointer */
	 if (! UseMouse)
	    DrawPointer();

	 /* user wants to exit */
	 if ((key & 0x00FF) == 'Q' || (key & 0x00FF) == 'q')
	 {
	    SaveChanges = Registered;
	    break;
	 }
	 else if ((key & 0x00FF) == 0x001B)
	 {
	    if (! MadeChanges)
	       break;
	    if (UseMouse)
	       HideMousePointer();
	    DrawScreenBox3D( 80, 220, 560, 250);
	    setcolor( YELLOW);
	    DrawScreenText( 91, 232, "Really abandon edit? (Y to confirm, any other key aborts)");
	    key = bioskey( 0);
	    if ((key & 0x00FF) == 'Y' || (key & 0x00FF) == 'y')
	       break;
	    if (UseMouse)
	       ShowMousePointer();
	    RedrawMap = TRUE;
	 }

	 /* user is lost */
	 else if ((key & 0xFF00) == 0x3B00)
	 {
	    DisplayHelp( EditMode, CurObject);
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the scale */
	 else if (((key & 0x00FF) == '+' || (key & 0x00FF) == '=') && Scale > 1)
	 {
	    Scale--;
	    RedrawMap = TRUE;
	 }
	 else if (((key & 0x00FF) == '-' || (key & 0x00FF) == '_') && Scale < 20)
	 {
	    Scale++;
	    RedrawMap = TRUE;
	 }

	 /* user wants to move */
	 else if ((key & 0xFF00) == 0x4800 && (PointerY - MoveSpeed) >= 0)
	 {
	    PointerY -= MoveSpeed;
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY);
	 }
	 else if ((key & 0xFF00) == 0x5000 && (PointerY + MoveSpeed) <= 479)
	 {
	    PointerY += MoveSpeed;
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY);
	 }
	 else if ((key & 0xFF00) == 0x4B00 && (PointerX - MoveSpeed) >= 0)
	 {
	    PointerX -= MoveSpeed;
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY);
	 }
	 else if ((key & 0xFF00) == 0x4D00 && (PointerX + MoveSpeed) <= 639)
	 {
	    PointerX += MoveSpeed;
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY);
	 }

	 /* user wants to change the movement speed */
	 else if ((key & 0x00FF) == ' ')
	    MoveSpeed = MoveSpeed == 1 ? 20 : 1;

	 /* user wants to change the edit mode */
	 else if ((key & 0x00FF) == 0x0009)
	 {
	    switch (EditMode)
	    {
	    case OBJ_THINGS:
	       EditMode = OBJ_LINEDEFS;
	       NumObjPtr = &NumLineDefs;
	       break;
	    case OBJ_LINEDEFS:
	       EditMode = OBJ_VERTEXES;
	       NumObjPtr = &NumVertexes;
	       break;
	    case OBJ_VERTEXES:
	       EditMode = OBJ_SEGS;
	       NumObjPtr = &NumSegs;
	       break;
	    case OBJ_SEGS:
	       EditMode = OBJ_SECTORS;
	       NumObjPtr = &NumSectors;
	       break;
	    case OBJ_SECTORS:
	       EditMode = OBJ_THINGS;
	       NumObjPtr = &NumThings;
	       break;
	    }
	    if (*NumObjPtr > 0)
	       CurObject = 0;
	    else
	       CurObject = -1;
	    OldObject = CurObject;
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x0F00)
	 {
	    CurObject = -1;
	    switch (EditMode)
	    {
	    case OBJ_THINGS:
	       EditMode = OBJ_SECTORS;
	       NumObjPtr = &NumSectors;
	       break;
	    case OBJ_LINEDEFS:
	       EditMode = OBJ_THINGS;
	       NumObjPtr = &NumThings;
	       break;
	    case OBJ_VERTEXES:
	       EditMode = OBJ_LINEDEFS;
	       NumObjPtr = &NumLineDefs;
	       break;
	    case OBJ_SEGS:
	       EditMode = OBJ_VERTEXES;
	       NumObjPtr = &NumVertexes;
	       break;
	    case OBJ_SECTORS:
	       EditMode = OBJ_SEGS;
	       NumObjPtr = &NumSegs;
	       break;
	    }
	    if (*NumObjPtr > 0)
	       CurObject = 0;
	    else
	       CurObject = -1;
	    OldObject = -1;
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to select the next or previous object */
	 else if (((key & 0x00FF) == 'N' || (key & 0x00FF) == 'n' || (key & 0x00FF) == '>') && GetCurObject( EditMode) < 0)
	 {
	    if (CurObject < *NumObjPtr - 1)
	       CurObject++;
	    else
	       CurObject = 0;
	    RedrawObj = TRUE;
	 }
	 else if (((key & 0x00FF) == 'P' || (key & 0x00FF) == 'p' || (key & 0x00FF) == '<') && GetCurObject( EditMode) < 0)
	 {
	    if (CurObject > 0)
	       CurObject--;
	    else
	       CurObject = *NumObjPtr - 1;
	    RedrawObj = TRUE;
	 }

	 /* user wants to disable the mouse (*debug*) */
	 else if ((key & 0x00FF) == 'K')
	 {
	    if (UseMouse)
	      HideMousePointer();
	    UseMouse = FALSE;
	 }

	 /* user wants to edit the current object */
	 else if ((key & 0x00FF) == 0x000D && CurObject >= 0)
	 {
	    EditObjectInfo( EditMode, CurObject);
	    RedrawMap = TRUE;
	 }

	 /* user wants to delete the current object */
	 else if ((key & 0xFF00) == 0x5300 && CurObject >= 0)
	 {
	    MadeChanges = TRUE;
	    DeleteObject( EditMode, CurObject);
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to insert a new object or drag the current one */
	 else if ((key & 0xFF00) == 0x5200)
	 {
	    MadeChanges = TRUE;
	    if ((EditMode == OBJ_THINGS || EditMode == OBJ_VERTEXES) && GetCurObject( EditMode) >= 0)
	    {
	       int n;

	       /* Adjust angles in Segs objects if a Vertex has moved */
	       if (EditMode == OBJ_VERTEXES && DragObject)
		 for (n = 0; n < NumSegs; n++)
		   if (Segs[ n].start == CurObject || Segs[ n].end == CurObject)
		     Segs[ n].angle = ComputeAngle(Vertexes[ Segs[ n].end].x - Vertexes[ Segs[ n].start].x,
						   Vertexes[ Segs[ n].end].y - Vertexes[ Segs[ n].start].y);
	       DragObject = !DragObject;
	    }
	    else
	    {
	       InsertObject( EditMode, CurObject);
	       DragObject = FALSE;
	       RedrawMap = TRUE;
	    }
	 }

	 /* user likes music */
	 else
	    Beep();

	 /* draw the (keyboard) pointer */
	 if (! UseMouse)
	    DrawPointer();
      }

      /* move the map if the pointer is near the edge of the screen */
      if (PointerY < 30)
      {
	 if (! UseMouse)
	    PointerY += MoveSpeed;
	 if (MAPY(239) < MaxY)
	 {
	    OrigY += MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
      }
      if (PointerY > 449)
      {
	 if (! UseMouse)
	    PointerY -= MoveSpeed;
	 if (MAPY(239) > MinY)
	 {
	    OrigY -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
      }
      if (PointerX < 30)
      {
	 if (! UseMouse)
	    PointerX += MoveSpeed;
	 if (MAPX(319) > MinX)
	 {
	    OrigX -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
      }
      if (PointerX > 609)
      {
	 if (! UseMouse)
	    PointerX -= MoveSpeed;
	 if (MAPX(319) < MaxX)
	 {
	    OrigX += MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
      }
   }
}



/*
   draw the actual game map
*/

void DrawMap( int editmode)
{
   int  n, m;
   char texname[9];

   /* clear the screen */
   ClearScreen();

   /* draw the linedefs to form the map */
   if (editmode == OBJ_VERTEXES)
   {
      setcolor( LIGHTGRAY);
      for (n = 0; n < NumLineDefs; n++)
	 DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
   }
   else
      if (editmode != OBJ_SEGS)
	 for (n = 0; n < NumLineDefs; n++)
	 {
	    if (LineDefs[ n].flags1 & 1)
	       setcolor( WHITE);
	    else
	       setcolor( LIGHTGRAY);
	    DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			 Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
	 }

   /* draw the segs */
   if (editmode == OBJ_SEGS)
   {
      for (n = 0; n < NumSegs; n++)
      {
	 setcolor( LIGHTGREEN + (Segs[ n].flip & 1));
	 DrawMapLine( Vertexes[ Segs[ n].start].x, Vertexes[ Segs[ n].start].y,
		      Vertexes[ Segs[ n].end].x, Vertexes[ Segs[ n].end].y);
	 /* (*debug*)
	 setcolor( LIGHTRED + (Segs[ n].flip & 1));
	 DrawMapArrow( Vertexes[ Segs[ n].start].x, Vertexes[ Segs[ n].start].y,
		       Segs[ n].angle);
	 */
      }
   }

   /* draw in the vertexes */
   if (editmode == OBJ_VERTEXES || editmode == OBJ_SEGS)
   {
      setcolor( LIGHTGREEN);
      for (n = 0; n < NumVertexes; n++)
      {
	 DrawMapLine( Vertexes[ n].x - OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x + OBJSIZE, Vertexes[ n].y + OBJSIZE);
	 DrawMapLine( Vertexes[ n].x + OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x - OBJSIZE, Vertexes[ n].y + OBJSIZE);
      }
   }
   if (editmode == OBJ_LINEDEFS || editmode == OBJ_SECTORS)
   {
      setcolor( LIGHTGREEN);
      for (n = 0; n < NumLineDefs; n++)
      {
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x - OBJSIZE, Vertexes[ LineDefs[ n].start].y - OBJSIZE, Vertexes[ LineDefs[ n].start].x + OBJSIZE, Vertexes[ LineDefs[ n].start].y + OBJSIZE);
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x + OBJSIZE, Vertexes[ LineDefs[ n].start].y - OBJSIZE, Vertexes[ LineDefs[ n].start].x - OBJSIZE, Vertexes[ LineDefs[ n].start].y + OBJSIZE);
      }
   }

   /* draw in the things */
   if (editmode == OBJ_THINGS)
   {
      for (n = 0; n < NumThings; n++)
      {
	 setcolor( GetThingColour( Things[ n].type));
	 DrawMapLine( Things[ n].xpos - OBJSIZE, Things[ n].ypos, Things[ n].xpos + OBJSIZE, Things[ n].ypos);
	 DrawMapLine( Things[ n].xpos, Things[ n].ypos - OBJSIZE, Things[ n].xpos, Things[ n].ypos + OBJSIZE);
      }
   }
   else
   {
      setcolor( LIGHTGRAY);
      for (n = 0; n < NumThings; n++)
      {
	 DrawMapLine( Things[ n].xpos - OBJSIZE, Things[ n].ypos, Things[ n].xpos + OBJSIZE, Things[ n].ypos);
	 DrawMapLine( Things[ n].xpos, Things[ n].ypos - OBJSIZE, Things[ n].xpos, Things[ n].ypos + OBJSIZE);
      }
   }

   /* draw in the title bar */
   DrawScreenBox3D( 0, 0, 639, 12);
   setcolor( WHITE);
   DrawScreenText( 5, 3, "Editing %s on %s", GetObjectType( editmode), Level->dir.name);
   DrawScreenText( 499, 3, "Press F1 for Help");
}



/*
   edit an object
*/

void EditObjectInfo( int objtype, int objnum)
{
   char *menustr[ 30];
   char  texname[ 9];
   int   n, val;

   switch (objtype)
   {
   case OBJ_THINGS:
      for (n = 0; n < 6; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 5], "Edit Thing #%d", objnum);
      sprintf( menustr[ 0], "Change Type          (Current: %s)", GetThingName( Things[ objnum].type));
      sprintf( menustr[ 1], "Change Angle         (Current: %s)", GetAngleName( Things[ objnum].angle));
      sprintf( menustr[ 2], "Change When Appears  (Current: %s)", GetWhenName( Things[ objnum].when));
      sprintf( menustr[ 3], "Change X position    (Current: %d)", Things[ objnum].xpos);
      sprintf( menustr[ 4], "Change Y position    (Current: %d)", Things[ objnum].ypos);
      val = DisplayMenuArray( 0, 30, menustr[ 5], 5, menustr);
      for (n = 0; n < 6; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 switch (DisplayMenu( 42, 64, "Select Class",
			      "Player",
			      "Enemy",
			      "Weapon",
			      "Bonus",
			      "Decoration",
			      "Enter a decimal value",
			      NULL))
	 {
	 case 1:
	    val = DisplayThingsMenu( 84, 98, "Select Start Position Type",
				     THING_PLAYER1,
				     THING_PLAYER2,
				     THING_PLAYER3,
				     THING_PLAYER4,
				     THING_DEATHMATCH,
				     0);
	    if (val >= 0)
	    {
	       Things[ objnum].type = val;
	       MadeChanges = TRUE;
	    }
	    break;

	 case 2:
	    val = DisplayThingsMenu( 84, 108, "Select Enemy",
				     THING_TROOPER,
				     THING_SARGEANT,
				     THING_IMP,
				     THING_DEMON,
				     THING_SPECTOR,
				     THING_BARON,
				     THING_LOSTSOUL,
				     THING_COCADEMON,
				     THING_SPIDERBOSS,
				     THING_CYBERDEMON,
				     0);
	    if (val >= 0)
	    {
	       Things[ objnum].type = val;
	       MadeChanges = TRUE;
	    }
	    break;

	 case 3:
	    val = DisplayThingsMenu( 84, 118, "Select Weapon",
				     THING_SHOTGUN,
				     THING_CHAINGUN,
				     THING_LAUNCHER,
				     THING_PLASMAGUN,
				     THING_CHAINSAW,
				     THING_SHELLS,
				     THING_AMMOCLIP,
				     THING_ROCKET,
				     THING_ENERGYCELL,
				     THING_BFG9000,
				     THING_SHELLBOX,
				     THING_AMMOBOX,
				     THING_ROCKETBOX,
				     THING_ENERGYPACK,
				     THING_BACKPACK,
				     0);
	    if (val >= 0)
	    {
	       Things[ objnum].type = val;
	       MadeChanges = TRUE;
	    }
	    break;

	 case 4:
	    val = DisplayThingsMenu( 84, 128, "Select Bonus",
				     THING_REDCARD,
				     THING_YELLOWCARD,
				     THING_BLUECARD,
				     THING_ARMBONUS1,
				     THING_GREENARMOR,
				     THING_BLUEARMOR,
				     THING_HLTBONUS1,
				     THING_STIMPACK,
				     THING_MEDKIT,
				     THING_SOULSPHERE,
				     THING_BLURSPHERE,
				     THING_MAP,
				     THING_RADSUIT,
				     THING_BESERK,
				     THING_INVULN,
				     THING_LITEAMP,
				     0);
	    if (val >= 0)
	    {
	       Things[ objnum].type = val;
	       MadeChanges = TRUE;
	    }
	    break;

	 case 5:
	    val = DisplayThingsMenu( 84, 138, "Select Decoration",
				     THING_BARREL,
				     THING_BONES,
				     THING_BONES2,
				     THING_DEADBUDDY,
				     THING_POOLOFBLOOD,
				     THING_CANDLE,
				     THING_LAMP,
				     THING_CANDLESTICK,
				     THING_TORCH,
				     THING_TECHCOLUMN,
				     0);
	    if (val >= 0)
	    {
	       Things[ objnum].type = val;
	       MadeChanges = TRUE;
	    }
	    break;

	 case 6:
	    val = InputIntegerValue( 84, 146, 0, 9999, Things[ objnum].type);
	    if (val >= 0)
	    {
	       Things[ objnum].type = val;
	       MadeChanges = TRUE;
	    }
	    break;

	 default:
	    Beep();
	    return;
	 }
	 break;

      case 2:
	 switch (DisplayMenu( 42, 74, "Select Angle",
			      "North",
			      "NorthEast",
			      "East",
			      "SouthEast",
			      "South",
			      "SouthWest",
			      "West",
			      "NorthWest",
			      NULL))
	 {
	 case 1:
	    Things[ objnum].angle = 90;
	    MadeChanges = TRUE;
	    break;
	 case 2:
	    Things[ objnum].angle = 135;
	    MadeChanges = TRUE;
	    break;
	 case 3:
	    Things[ objnum].angle = 180;
	    MadeChanges = TRUE;
	    break;
	 case 4:
	    Things[ objnum].angle = 225;
	    MadeChanges = TRUE;
	    break;
	 case 5:
	    Things[ objnum].angle = 270;
	    MadeChanges = TRUE;
	    break;
	 case 6:
	    Things[ objnum].angle = 315;
	    MadeChanges = TRUE;
	    break;
	 case 7:
	    Things[ objnum].angle = 0;
	    MadeChanges = TRUE;
	    break;
	 case 8:
	    Things[ objnum].angle = 45;
	    MadeChanges = TRUE;
	    break;
	 }
	 break;

      case 3:
	 val = DisplayMenu( 42, 84, "Choose the difficulty level(s)",
			    "D12         (Easy only)",
			    "D3          (Medium only)",
			    "D12, D3     (Easy and Medium)",
			    "D4          (Hard only)",
			    "D12, D4     (Easy and Hard)",
			    "D3, D4      (Medium and Hard)",
			    "D12, D3, D4 (Easy, Medium, Hard)",
			    "Enter a decimal value",
			    NULL);
	 if (val == 8)
	    val = InputIntegerValue( 84,  188, 1, 31, Things[ objnum].when);
	 if (val > 0)
	 {
	    Things[ objnum].when = val;
	    MadeChanges = TRUE;
	 }
	 break;

      case 4:
	 val = InputIntegerValue( 42, 94, MinX, MaxX, Things[ objnum].xpos);
	 if (val >= MinX)
	 {
	    Things[ objnum].xpos = val;
	    MadeChanges = TRUE;
	 }
	 break;

      case 5:
	 val = InputIntegerValue( 42, 104, MinY, MaxY, Things[ objnum].ypos);
	 if (val >= MinY)
	 {
	    Things[ objnum].ypos = val;
	    MadeChanges = TRUE;
	 }
	 break;
      }
      break;

   case OBJ_LINEDEFS:
      switch (DisplayMenu( 0, 30, "Choose the object to edit:",
			   "Edit the LineDef",
			   "Edit the 1st SideDef",
			   "Edit the 2nd SideDef",
			   NULL))
      {
      case 1:
	 for (n = 0; n < 8; n++)
	    menustr[ n] = GetMemory( 60);
	 sprintf( menustr[ 7], "Edit LineDef #%d", objnum);
	 sprintf( menustr[ 0], "Change Flags 1          (Current: %d)", LineDefs[ objnum].flags1);
	 sprintf( menustr[ 1], "Change Flags 2          (Current: %d)", LineDefs[ objnum].flags2);
	 sprintf( menustr[ 2], "Change Sector tag       (Current: %d)", LineDefs[ objnum].tag);
	 sprintf( menustr[ 3], "Change Starting Vertex  (Current: #%d)", LineDefs[ objnum].start);
	 sprintf( menustr[ 4], "Change Ending Vertex    (Current: #%d)", LineDefs[ objnum].end);
	 sprintf( menustr[ 5], "Change 1st SideDef ref. (Current: #%d)", LineDefs[ objnum].sidedef1);
	 sprintf( menustr[ 6], "Change 2nd SideDef ref. (Current: #%d)", LineDefs[ objnum].sidedef2);
	 val = DisplayMenuArray( 42, 64, menustr[ 7], 7, menustr);
	 for (n = 0; n < 8; n++)
	    free( menustr[ n]);
	 switch (val)
	 {
	 case 1:
	    val = InputIntegerValue( 84, 98, 0, 255, LineDefs[ objnum].flags1);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].flags1 = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 2:
	    val = InputIntegerValue( 84, 108, 0, 255, LineDefs[ objnum].flags2);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].flags2 = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 3:
	    val = InputIntegerValue( 84, 118, 0, 255, LineDefs[ objnum].tag);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].tag = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 4:
	 case 5:
	 case 6:
	 case 7:
	    NotImplemented();
	    break;
	 }
	 break;

      case 2:
	 if (LineDefs[ objnum].sidedef1 >= 0)
	 {
	    objtype = OBJ_SIDEDEFS;
	    objnum = LineDefs[ objnum].sidedef1;
	 }
	 else
	 {
	    NotImplemented(); /* (*add a new sidedef*) */
	    break;
	 }
      case 3:
	 if (objtype != OBJ_SIDEDEFS)
	 {
	    if (LineDefs[ objnum].sidedef2 >= 0)
	    {
	       objtype = OBJ_SIDEDEFS;
	       objnum = LineDefs[ objnum].sidedef2;
	    }
	    else
	    {
	       NotImplemented(); /* (*add a new sidedef*) */
	       break;
	    }
	 }
	 for (n = 0; n < 7; n++)
	    menustr[ n] = GetMemory( 60);
	 sprintf( menustr[ 6], "Edit SideDef #%d", objnum);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ objnum].tex3, 8);
	 sprintf( menustr[ 0], "Change Normal Texture   (Current: %s)", texname);
	 strncpy( texname, SideDefs[ objnum].tex1, 8);
	 sprintf( menustr[ 1], "Change Texture above    (Current: %s)", texname);
	 strncpy( texname, SideDefs[ objnum].tex2, 8);
	 sprintf( menustr[ 2], "Change Texture below    (Current: %s)", texname);
	 sprintf( menustr[ 3], "Change Texture X offset (Current: %d)", SideDefs[ objnum].xoff);
	 sprintf( menustr[ 4], "Change Texture Y offset (Current: %d)", SideDefs[ objnum].yoff);
	 sprintf( menustr[ 5], "Change Sector ref.      (Current: #%d)", SideDefs[ objnum].sector);
	 val = DisplayMenuArray( 42, 84, menustr[ 6], 6, menustr);
	 for (n = 0; n < 7; n++)
	    free( menustr[ n]);
	 switch (val)
	 {
	 case 1:
	    strncpy( texname, SideDefs[ objnum].tex3, 8);
	    InputNameFromList( 84, 118, "Enter a wall texture name:", NumTexture1, Texture1, texname);
	    if (strlen(texname) > 0)
	    {
	       strncpy( SideDefs[ objnum].tex3, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 2:
	    strncpy( texname, SideDefs[ objnum].tex1, 8);
	    InputNameFromList( 84, 128, "Enter a wall texture name:", NumTexture1, Texture1, texname);
	    if (strlen(texname) > 0)
	    {
	       strncpy( SideDefs[ objnum].tex1, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 3:
	    strncpy( texname, SideDefs[ objnum].tex2, 8);
	    InputNameFromList( 84, 138, "Enter a wall texture name:", NumTexture1, Texture1, texname);
	    if (strlen(texname) > 0)
	    {
	       strncpy( SideDefs[ objnum].tex2, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 4:
	    val = InputIntegerValue( 84, 148, -100, 100, SideDefs[ objnum].xoff);
	    if (val >= -100)
	    {
	       SideDefs[ objnum].xoff = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 5:
	    val = InputIntegerValue( 84, 158, -100, 100, SideDefs[ objnum].yoff);
	    if (val >= -100)
	    {
	       SideDefs[ objnum].yoff = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 6:
	    NotImplemented();
	    break;
	 }
	 break;

      }
      break;

   case OBJ_VERTEXES:
      for (n = 0; n < 3; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 2], "Edit Vertex #%d", objnum);
      sprintf( menustr[ 0], "Change X position (Current: %d)", Vertexes[ objnum].x);
      sprintf( menustr[ 1], "Change Y position (Current: %d)", Vertexes[ objnum].y);
      val = DisplayMenuArray( 0, 30, menustr[ 2], 2, menustr);
      for (n = 0; n < 3; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( 42, 94, MinX, MaxX, Vertexes[ objnum].x);
	 if (val >= MinX)
	 {
	    Vertexes[ objnum].x = val;
	    MadeChanges = TRUE;
	 }
	 break;

      case 2:
	 val = InputIntegerValue( 42, 104, MinY, MaxY, Vertexes[ objnum].y);
	 if (val >= MinY)
	 {
	    Vertexes[ objnum].y = val;
	    MadeChanges = TRUE;
	 }
	 break;
      }
      break;


   case OBJ_SEGS:
      for (n = 0; n < 7; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 6], "Edit Seg #%d", objnum);
      sprintf( menustr[ 0], "Change Flags 1         (Current: %d)", Segs[ objnum].flip);
      sprintf( menustr[ 1], "Change Flags 2         (Current: %d)", Segs[ objnum].flags);
      sprintf( menustr[ 2], "Change Angle           (Current: %d)", Segs[ objnum].angle);
      sprintf( menustr[ 3], "Change Starting Vertex (Current: #%d)", Segs[ objnum].start);
      sprintf( menustr[ 4], "Change Ending Vertex   (Current: #%d)", Segs[ objnum].end);
      sprintf( menustr[ 5], "Change LineDef ref.    (Current: #%d)", Segs[ objnum].linedef);
      val = DisplayMenuArray( 0, 30, menustr[ 6], 6, menustr);
      for (n = 0; n < 7; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( 42, 64, 0, 255, Segs[ objnum].flip);
	 if (val >= 0)
	 {
	    Segs[ objnum].flip = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 2:
	 val = InputIntegerValue( 42, 64, 0, 255, Segs[ objnum].flags);
	 if (val >= 0)
	 {
	    Segs[ objnum].flags = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 3:
      case 4:
      case 5:
      case 6:
	 NotImplemented();
	 break;
      }
      break;

   case OBJ_SECTORS:
      for (n = 0; n < 8; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 7], "Edit Sector #%d", objnum);
      sprintf( menustr[ 0], "Change Floor height    (Current: %d)", Sectors[ objnum].floorh);
      sprintf( menustr[ 1], "Change Ceiling height  (Current: %d)", Sectors[ objnum].ceilh);
      texname[ 8] = '\0';
      strncpy( texname, Sectors[ objnum].floort, 8);
      sprintf( menustr[ 2], "Change Floor texture   (Current: %s)", texname);
      strncpy( texname, Sectors[ objnum].ceilt, 8);
      sprintf( menustr[ 3], "Change Ceiling texture (Current: %s)", texname);
      sprintf( menustr[ 4], "Change Light level     (Current: %d)", Sectors[ objnum].light);
      sprintf( menustr[ 5], "Change Special flags   (Current: %d)", Sectors[ objnum].special);
      sprintf( menustr[ 6], "Change LineDef tag     (Current: %d)", Sectors[ objnum].tag);
      val = DisplayMenuArray( 0, 30, menustr[ 7], 7, menustr);
      for (n = 0; n < 8; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( 42, 64, -264, 264, Sectors[ objnum].floorh);
	 if (val >= -264)
	 {
	    Sectors[ objnum].floorh = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 2:
	 val = InputIntegerValue( 42, 74, -264, 264, Sectors[ objnum].ceilh);
	 if (val >= -264)
	 {
	    Sectors[ objnum].ceilh = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 3:
	 strncpy( texname, Sectors[ objnum].floort, 8);
	 InputNameFromList( 42, 84, "Enter a floor texture name:", NumFTexture, FTexture, texname);
	 if (strlen(texname) > 0)
	 {
	    strncpy( Sectors[ objnum].floort, texname, 8);
	    MadeChanges = TRUE;
	 }
	 break;
      case 4:
	 strncpy( texname, Sectors[ objnum].ceilt, 8);
	 InputNameFromList( 42, 94, "Enter a ceiling texture name:", NumFTexture, FTexture, texname);
	 if (strlen(texname) > 0)
	 {
	    strncpy( Sectors[ objnum].ceilt, texname, 8);
	    MadeChanges = TRUE;
	 }
	 break;
      case 5:
	 val = InputIntegerValue( 42, 104, 0, 255, Sectors[ objnum].light);
	 if (val >= 0)
	 {
	    Sectors[ objnum].light = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 6:
	 switch (DisplayMenu( 42, 114, "Choose a special behaviour",
			      "Normal",
			      "Light blinks randomly",
			      "Light quickly pulsates",
			      "Light blinks",
			      "Light pulsates and -20% health",
			      "-10% health",
			      "-5% health",
			      "Light pulsates",
			      "Secret (credit if discovered)",
			      "Enter a decimal value",
			      NULL))
	 {
	 case 1:
	    Sectors[ objnum].special = 0;
	    MadeChanges = TRUE;
	    break;
	 case 2:
	    Sectors[ objnum].special = 1;
	    MadeChanges = TRUE;
	    break;
	 case 3:
	    Sectors[ objnum].special = 2;
	    MadeChanges = TRUE;
	    break;
	 case 4:
	    Sectors[ objnum].special = 3;
	    MadeChanges = TRUE;
	    break;
	 case 5:
	    Sectors[ objnum].special = 4;
	    MadeChanges = TRUE;
	    break;
	 case 6:
	    Sectors[ objnum].special = 5;
	    MadeChanges = TRUE;
	    break;
	 case 7:
	    Sectors[ objnum].special = 7;
	    MadeChanges = TRUE;
	    break;
	 case 8:
	    Sectors[ objnum].special = 8;
	    MadeChanges = TRUE;
	    break;
	 case 9:
	    Sectors[ objnum].special = 9;
	    MadeChanges = TRUE;
	    break;
	 case 10:
	    val = InputIntegerValue( 84, 238, 0, 255, Sectors[ objnum].special);
	    if (val >= 0)
	    {
	       Sectors[ objnum].special = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 }
	 break;
      case 7:
	 val = InputIntegerValue( 42, 124, 0, 255, Sectors[ objnum].tag);
	 if (val >= 0)
	 {
	    Sectors[ objnum].tag = val;
	    MadeChanges = TRUE;
	 }
	 break;
      }
      break;
   }
}



/* end of file */
