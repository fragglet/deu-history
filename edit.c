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

/* the constants */
#define OBJSIZE   10          /* half the size of an object in map coords */

/* the global data */
MDirPtr Level = NULL;         /* master dictionary entry for the level */
int CurObject = -1;           /* the current object (thing, vertex,...) */
int LastObject = -1;          /* previous value of CurObject */
int NumThings = 0;            /* number of things */
TPtr Things;                  /* things data */
int NumLineDefs = 0;          /* number of line defs */
LDPtr LineDefs;               /* line defs data */
int NumSideDefs = 0;          /* number of side defs */
SDPtr SideDefs;               /* side defs data */
int NumVertexes = 0;          /* number of vertexes */
VPtr Vertexes;                /* vertex data */
int NumSegs = 0;	      /* number of segs */
SEPtr Segs;                   /* segs data */
int NumSSectors = 0;	      /* number of subsectors */
SSPtr SSectors;               /* subsectors data */
int NumSectors = 0;	      /* number of sectors */
SPtr Sectors;                 /* sectors data */
int MaxX = 0;                 /* maximum X value of map */
int MaxY = 0;                 /* maximum Y value of map */
int MinX = 0;                 /* minimum X value of map */
int MinY = 0;                 /* minimum Y value of map */
int MoveSpeed = 20;           /* movement speed */
int SaveChanges = FALSE;      /* save changes */
int MadeChanges = FALSE;      /* made changes changes */
int RedrawMap = FALSE;        /* do we need to redraw the map? */
int EditMode = EDIT_THINGS;   /* what are we editing? */
int DragObject = FALSE;       /* are we dragging an object? */

int DefType = THING_TROOPER;  /* the default thing type */
int DefAngle = 270;           /* the default thing angle */
int DefWhen = 0x07;           /* the default thing appears when */



/*
   the driving program
*/

void EditLevel( int game, int level, char *dest)
{
   char *outfile = GetMemory( strlen( dest) + 2);

   strcpy( outfile, dest);
   ReadLevelData( game, level);
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
   TPtr next, cur;

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
   what are we editing?
*/
char *GetEditObjectType()
{
   switch (EditMode)
   {
   case EDIT_THINGS:
      return "Things";
   case EDIT_LINEDEFS:
      return "LineDefs";
   case EDIT_SIDEDEFS:
      return "SideDefs";
   case EDIT_VERTEXES:
      return "Vertexes";
   case EDIT_SEGS:
      return "Segments";
   case EDIT_SSECTORS:
      return "Sub-sectors";
   case EDIT_NODES:
      return "Nodes";
   case EDIT_SECTORS:
      return "Sectors";
   case EDIT_REJECT:
      return "Reject data";
   case EDIT_BLOCKMAP:
      return "Blockmap";
   }
   return "< Bug! >";
}


/*
   display the help screen
 */

void DisplayHelp()
{
   if (UseMouse)
      HideMousePointer();
   /* put in the instructions */
   DrawScreenBox3D( 140, 100, 500, 380);
   setcolor( LIGHTCYAN);
   DrawScreenText( 240, 122, "Doom Editor Utility");
   DrawScreenText( 272 - strlen(GetEditObjectType()) * 4, 135, "- %s Editor -", GetEditObjectType());
   setcolor( BLACK);
   DrawScreenText( 150, 165, "Use the mouse or the cursor keys to move");
   DrawScreenText( 150, 175, "around.  The map scrolls when the pointer");
   DrawScreenText( 150, 185, "reaches the edge of the screen.");
   DrawScreenText( 150, 205, "Other useful keys are:");
   DrawScreenText( 150, 220, "Space - Change the move/scroll speed.");
   DrawScreenText( 150, 230, "Esc   - Exit without saving changes.");
   if (Registered)
      DrawScreenText( 150, 240, "Q     - Quit, saving changes.");
   else
   {
      setcolor( DARKGRAY);
      DrawScreenText( 150, 240, "Q     - Quit without saving changes.");
      setcolor( BLACK);
   }
   DrawScreenText( 150, 250, "+/-   - Change the map scale.");
   if (EditMode == EDIT_THINGS)
   {
      if (CurObject >= 0)
      {
	 DrawScreenText( 150, 260, "Ins   - Drag the current thing");
	 DrawScreenText( 150, 270, "        (%s).", GetThingName( Things[ CurObject].type));
      }
      else
      {
	 DrawScreenText( 150, 260, "Ins   - Insert a default thing");
	 DrawScreenText( 150, 270, "        (%s).", GetThingName( DefType));
      }
   }
   else
   {
      DrawScreenText( 150, 260, "Ins   - Insert a new object or drag");
      DrawScreenText( 150, 270, "        the current one.");
   }
   if ((EditMode == EDIT_THINGS) && (CurObject >= 0))
   {
      DrawScreenText( 150, 280, "Del   - Delete the selected object");
      DrawScreenText( 150, 290, "        (%s).", GetThingName( Things[ CurObject].type));
   }
   else
   {
      setcolor( DARKGRAY);
      DrawScreenText( 150, 280, "Del   - Delete the selected object");
      DrawScreenText( 150, 290, "        (none for now).");
      setcolor( BLACK);
   }
   DrawScreenText( 150, 300, "Enter - Edit the default/selected object.");
   DrawScreenText( 150, 320, "You may also use the left mouse button to");
   DrawScreenText( 150, 330, "insert/drag an object and the right mouse");
   DrawScreenText( 150, 340, "button to delete any selected object.");
   setcolor( YELLOW);
   DrawScreenText( 150, 365, "Press any key to return to the editor...");
   bioskey( 0);
   if (UseMouse)
      ShowMousePointer();
   RedrawMap = TRUE;
}



/*
   the editor main loop
*/

void EditorLoop()
{
   int  key, buttons, oldbuttons;

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
   EditMode = EDIT_THINGS;
   DragObject = FALSE;
   RedrawMap = TRUE;

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

      /* if we are dragging an object, move it */
      if (DragObject)
      {
	 switch (EditMode)
	 {
	    case EDIT_THINGS:
	       Things[ CurObject].xpos  = MAPX(PointerX);
	       Things[ CurObject].ypos  = MAPY(PointerY);
	       break;
	    case EDIT_VERTEXES:
	       Vertexes[ CurObject].x = MAPX( PointerX);
	       Vertexes[ CurObject].y = MAPY( PointerY);
	       break;
	 }
	 MadeChanges = TRUE;
	 RedrawMap = TRUE;
      }
      else
	 /* check if there is something near the pointer */
	 GetCurObject();

      /* draw the map */
      if (RedrawMap)
	 DrawMap();

      /* (* debug *)
      if (EditMode == EDIT_VERTEXES)
      {
	 setcolor( BLUE);
	 DrawScreenBox( 0, 15, 130, 25);
	 setcolor( WHITE);
	 DrawScreenText( 0, 15, "(%d, %d)", MAPX( PointerX), MAPY( PointerY));
	 delay( 10);
      }
      */

      /* get user input */
      if (bioskey( 1) || key)
      {
	 if (! key)
	    key = bioskey( 0);

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
	    DisplayHelp();

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
	    PointerY -= MoveSpeed;
	 else if ((key & 0xFF00) == 0x5000 && (PointerY + MoveSpeed) <= 479)
	    PointerY += MoveSpeed;
	 else if ((key & 0xFF00) == 0x4B00 && (PointerX - MoveSpeed) >= 0)
	    PointerX -= MoveSpeed;
	 else if ((key & 0xFF00) == 0x4D00 && (PointerX + MoveSpeed) <= 639)
	    PointerX += MoveSpeed;

	 /* user wants to change the movement speed */
	 else if ((key & 0x00FF) == ' ')
	    MoveSpeed = MoveSpeed == 1 ? 20 : 1;

	 /* user wants to change the edit mode */
	 else if ((key & 0x00FF) == 0x0009)
	 {
	    EditMode++;
	    if (EditMode > EDIT_SECTORS)
	      EditMode = EDIT_THINGS;
	    LastObject = -1;
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x0F00)
	 {
	    EditMode--;
	    if (EditMode < EDIT_THINGS)
	      EditMode = EDIT_SECTORS;
	    LastObject = -1;
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to disable the mouse (*debug*) */
	 else if ((key & 0x00FF) == 'K')
	 {
	    if (UseMouse)
	      HideMousePointer();
	    UseMouse = FALSE;
	 }

	 /* user wants to edit the current or default object */
	 else if ((key & 0x00FF) == 0x000D)
	 {
	    switch (EditMode)
	    {
	    case EDIT_THINGS:
	       if (CurObject >= 0)
	       {
		  int type  = Things[ CurObject].type;
		  int angle = Things[ CurObject].angle;
		  int when  = Things[ CurObject].when;
		  EditThingInfo( &type, &angle, &when);
		  if (type != Things[ CurObject].type)
		  {
		     Things[ CurObject].type = type;
		     MadeChanges = TRUE;
		  }
		  if (when != Things[ CurObject].when)
		  {
		     Things[ CurObject].when = when;
		     MadeChanges = TRUE;
		  }
		  if (angle != Things[ CurObject].angle)
		  {
		     Things[ CurObject].angle = angle;
		     MadeChanges = TRUE;
		  }
	       }
	       else
		  EditThingInfo( &DefType, &DefAngle, &DefWhen);
	       break;
	    case EDIT_LINEDEFS:
	       EditLineDefInfo();
	       MadeChanges = TRUE; /* (*temporary*) */
	       break;
	    case EDIT_SIDEDEFS:
	       EditSideDefInfo();
	       MadeChanges = TRUE; /* (*temporary*) */
	       break;
	    case EDIT_SECTORS:
	       EditSectorInfo();
	       MadeChanges = TRUE; /* (*temporary*) */
	       break;
	    default:
	       Beep();
	    }
	    RedrawMap = TRUE;
	 }

	 /* user wants to delete the current object */
	 else if ((key & 0xFF00) == 0x5300 && (CurObject >= 0))
	 {
	    MadeChanges = TRUE;
	    DeleteObject( EditMode, CurObject);
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to insert a default object or drag the current one */
	 else if ((key & 0xFF00) == 0x5200)
	 {
	    /* (* only useful for THINGS, VERTEXES and maybe NODES *) */
	    if (CurObject >= 0)
	    {
	       int n;

	       /* Adjust angles in Segs objects if a Vertex has moved */
	       if (EditMode == EDIT_VERTEXES && DragObject)
		 for (n = 0; n < NumSegs; n++)
		   if (Segs[ n].start == CurObject || Segs[ n].end == CurObject)
		     Segs[ n].angle = ComputeAngle(Vertexes[ Segs[ n].end].x - Vertexes[ Segs[ n].start].x,
						   Vertexes[ Segs[ n].end].y - Vertexes[ Segs[ n].start].y);
	       DragObject = !DragObject;
	    }
	    else
	    {
	       MadeChanges = TRUE;
	       InsertObject( EditMode);
	       DragObject = FALSE;
	       RedrawMap = TRUE;
	    }
	 }

	 /* user likes music */
	 else
	    Beep();

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

void DrawMap()
{
   int  n, m;
   char texname[9];

   if (UseMouse)
      HideMousePointer();

   /* clear the screen */
   ClearScreen();

   /* draw the linedefs to form the map */
   if (EditMode == EDIT_LINEDEFS)
   {
      setcolor( WHITE);
      for (n = 0; n < NumLineDefs; n++)
	 DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
   }
   else
      if (EditMode != EDIT_SEGS)
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
   if (EditMode == EDIT_SEGS)
   {
      for (n = 0; n < NumSegs; n++)
      {
	 setcolor( LIGHTGREEN + (Segs[ n].flip & 1));
	 DrawMapVector( Vertexes[ Segs[ n].start].x, Vertexes[ Segs[ n].start].y,
			Vertexes[ Segs[ n].end].x, Vertexes[ Segs[ n].end].y);
	 /* (*debug*)
	 setcolor( LIGHTRED + (Segs[ n].flip & 1));
	 DrawMapArrow( Vertexes[ Segs[ n].start].x, Vertexes[ Segs[ n].start].y,
		       Segs[ n].angle);
	 */
      }
   }

   /* draw in the vertexes */
   if (EditMode == EDIT_VERTEXES)
   {
      setcolor( LIGHTGREEN);
      for (n = 0; n < NumVertexes; n++)
      {
	 DrawMapLine( Vertexes[ n].x - OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x + OBJSIZE, Vertexes[ n].y + OBJSIZE);
	 DrawMapLine( Vertexes[ n].x + OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x - OBJSIZE, Vertexes[ n].y + OBJSIZE);
      }
   }

   /* draw in the things */
   if (EditMode == EDIT_THINGS)
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
   DrawScreenText( 5, 3, "Editing %s on %s", GetEditObjectType(), Level->dir.name);
   DrawScreenText( 499, 3, "Press F1 for Help");

   /* highlight the selected object and display the information box */
   switch (EditMode)
   {
   case EDIT_THINGS:
      DrawScreenBox3D( 447, 434, 638, 478);
      if (CurObject >= 0)
      {
	 setcolor( YELLOW);
	 DrawMapLine( Things[ CurObject].xpos - OBJSIZE * 2, Things[ CurObject].ypos - OBJSIZE * 2, Things[ CurObject].xpos - OBJSIZE * 2, Things[ CurObject].ypos + OBJSIZE * 2);
	 DrawMapLine( Things[ CurObject].xpos - OBJSIZE * 2, Things[ CurObject].ypos + OBJSIZE * 2, Things[ CurObject].xpos + OBJSIZE * 2, Things[ CurObject].ypos + OBJSIZE * 2);
	 DrawMapLine( Things[ CurObject].xpos + OBJSIZE * 2, Things[ CurObject].ypos + OBJSIZE * 2, Things[ CurObject].xpos + OBJSIZE * 2, Things[ CurObject].ypos - OBJSIZE * 2);
	 DrawMapLine( Things[ CurObject].xpos + OBJSIZE * 2, Things[ CurObject].ypos - OBJSIZE * 2, Things[ CurObject].xpos - OBJSIZE * 2, Things[ CurObject].ypos - OBJSIZE * 2);
	 /* draw a box around the info box */
	 setcolor( GetThingColour( Things[ CurObject].type));
	 DrawScreenLine( 446, 433, 639, 433);
	 DrawScreenLine( 639, 433, 639, 479);
	 DrawScreenLine( 639, 479, 446, 479);
	 DrawScreenLine( 446, 479, 446, 433);
	 /* display the thing information */
	 setcolor( BLACK);
	 DrawScreenText( 451, 438, "THING at (%d, %d)", Things[ CurObject].xpos, Things[ CurObject].ypos);
	 DrawScreenText( 451, 448, "Type:  %s", GetThingName( Things[ CurObject].type));
	 DrawScreenText( 451, 458, "Angle: %s", GetAngleName( Things[ CurObject].angle));
	 DrawScreenText( 451, 468, "When:  %s", GetWhenName( Things[ CurObject].when));
      }
      else
      {
	 DrawScreenText( 451, 438, "DEFAULT THING Info");
	 DrawScreenText( 451, 448, "Type:  %s", GetThingName( DefType));
	 DrawScreenText( 451, 458, "Angle: %s", GetAngleName( DefAngle));
	 DrawScreenText( 451, 468, "When:  %s", GetWhenName( DefWhen));
      }
      break;
   case EDIT_LINEDEFS:
      if (LastObject >= 0)
      {
	 DrawScreenBox3D( 0, 435, 176, 480);
	 if (CurObject >= 0)
	 {
	    setcolor( YELLOW);
	    DrawMapVector( Vertexes[ LineDefs[ CurObject].start].x, Vertexes[ LineDefs[ CurObject].start].y,
			   Vertexes[ LineDefs[ CurObject].end].x, Vertexes[ LineDefs[ CurObject].end].y);
	    setcolor( BLACK);
	    DrawScreenText( 5, 440, "SELECTED LINEDEF Info");
	 }
	 else
	    DrawScreenText( 5, 440, "LAST LINEDEF Info");
	 /* (*"flags1" and "flags2" should be replaced by a descriptive text string*) */
	 DrawScreenText( 5, 450, "Flags 1:    %d", LineDefs[ LastObject].flags1);
	 DrawScreenText( 5, 460, "Flags 2:    %d", LineDefs[ LastObject].flags2);
	 DrawScreenText( 5, 470, "Sector tag: %d", LineDefs[ LastObject].tag);
      }
      break;
   case EDIT_SIDEDEFS:
      if (LastObject >= 0)
      {
	 DrawScreenBox3D( 0, 415, 200, 480);
	 if (CurObject >= 0)
	 {
	    setcolor( YELLOW);
	    for (n = 0; n < NumLineDefs; n++)
	       if (LineDefs[ n].sidedef1 == CurObject || LineDefs[ n].sidedef2 == CurObject)
		  DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
				 Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
	    setcolor( BLACK);
	    DrawScreenText( 5, 420, "SELECTED SIDEDEF Info");
	 }
	 else
	    DrawScreenText( 5, 420, "LAST SIDEDEF Info");
	 DrawScreenText( 5, 430, "X offset:       %d", SideDefs[ LastObject].xoff);
	 DrawScreenText( 5, 440, "Y offset:       %d", SideDefs[ LastObject].yoff);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ LastObject].tex3, 8);
	 DrawScreenText( 5, 450, "Normal texture: %s", texname);
	 strncpy( texname, SideDefs[ LastObject].tex1, 8);
	 DrawScreenText( 5, 460, "Texture above:  %s", texname);
	 strncpy( texname, SideDefs[ LastObject].tex2, 8);
	 DrawScreenText( 5, 470, "Texture below:  %s", texname);
      }
      break;
   case EDIT_VERTEXES:
      if (CurObject >= 0)
      {
	 setcolor( YELLOW);
	 DrawMapLine( Vertexes[ CurObject].x - OBJSIZE * 2, Vertexes[ CurObject].y - OBJSIZE * 2, Vertexes[ CurObject].x - OBJSIZE * 2, Vertexes[ CurObject].y + OBJSIZE * 2);
	 DrawMapLine( Vertexes[ CurObject].x - OBJSIZE * 2, Vertexes[ CurObject].y + OBJSIZE * 2, Vertexes[ CurObject].x + OBJSIZE * 2, Vertexes[ CurObject].y + OBJSIZE * 2);
	 DrawMapLine( Vertexes[ CurObject].x + OBJSIZE * 2, Vertexes[ CurObject].y + OBJSIZE * 2, Vertexes[ CurObject].x + OBJSIZE * 2, Vertexes[ CurObject].y - OBJSIZE * 2);
	 DrawMapLine( Vertexes[ CurObject].x + OBJSIZE * 2, Vertexes[ CurObject].y - OBJSIZE * 2, Vertexes[ CurObject].x - OBJSIZE * 2, Vertexes[ CurObject].y - OBJSIZE * 2);
      }
      break;
   case EDIT_SEGS:
      if (CurObject >= 0)
      {
	 setcolor( YELLOW);
	 DrawMapVector( Vertexes[ Segs[ CurObject].start].x, Vertexes[ Segs[ CurObject].start].y,
			Vertexes[ Segs[ CurObject].end].x, Vertexes[ Segs[ CurObject].end].y);
      }
      break;
   case EDIT_SECTORS:
      if (LastObject >= 0)
      {
	 DrawScreenBox3D( 0, 395, 200, 480);
	 if (CurObject >= 0)
	 {
	    setcolor( YELLOW);
	    for (n = 0; n < NumLineDefs; n++)
	       if (SideDefs[ LineDefs[ n].sidedef1].sector == LastObject || SideDefs[ LineDefs[ n].sidedef2].sector == LastObject)
		  DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
				 Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
	    setcolor( BLACK);
	    DrawScreenText( 5, 400, "SELECTED SECTOR Info");
	 }
	 else
	    DrawScreenText( 5, 400, "LAST SECTOR Info");
	 DrawScreenText( 5, 410, "Floor height:  %d", Sectors[ LastObject].floorh);
	 DrawScreenText( 5, 420, "Ceil. height:  %d", Sectors[ LastObject].ceilh);
	 texname[ 8] = '\0';
	 strncpy( texname, Sectors[ LastObject].floort, 8);
	 DrawScreenText( 5, 430, "Floor texture: %s", texname);
	 strncpy( texname, Sectors[ LastObject].ceilt, 8);
	 DrawScreenText( 5, 440, "Ceil. texture: %s", texname);
	 DrawScreenText( 5, 450, "Light level:   %d", Sectors[ LastObject].light);
	 /* (*"special" should be replaced by a text string*) */
	 DrawScreenText( 5, 460, "Special flags: %d", Sectors[ LastObject].special);
	 DrawScreenText( 5, 470, "LineDef tag:   %d", Sectors[ LastObject].tag);
      }
      break;
   }

   /* the map is now up to date */
   RedrawMap = FALSE;

   if (UseMouse)
      ShowMousePointer();
   else
      DrawPointer();
}



/*
   edit a thing info
*/

void EditThingInfo( int *type, int *angle, int *when)
{
   char line1[80], line2[80], line3[80];
   int  key, n, val;

   sprintf( line1, "Change Type          (Current: %s)", GetThingName( *type));
   sprintf( line2, "Change Angle         (Current: %s)", GetAngleName( *angle));
   sprintf( line3, "Change When Appears  (Current: %s)", GetWhenName( *when));
   if (CurObject >= 0)
      val = DisplayMenu( 0, 30, "Edit Selected Thing", line1, line2, line3, NULL);
   else
      val = DisplayMenu( 0, 30, "Edit Default Thing", line1, line2, line3, NULL);
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
	 if (val < 0)
	 {
	    Beep();
	    return;
	 }
	 *type = val;
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
	 if (val < 0)
	 {
	    Beep();
	    return;
	 }
	 *type = val;
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
	 if (val < 0)
	 {
	    Beep();
	    return;
	 }
	 *type = val;
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
	 if (val < 0)
	 {
	    Beep();
	    return;
	 }
	 *type = val;
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
	 if (val < 0)
	 {
	    Beep();
	    return;
	 }
	 *type = val;
	 break;

      case 6:
	 val = InputIntegerValue( 84, 146, 0, 9999, *type);
	 if (val >= 0)
	    *type = val;
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
	 *angle = 90;
	 break;
      case 2:
	 *angle = 135;
	 break;
      case 3:
	 *angle = 180;
	 break;
      case 4:
	 *angle = 225;
	 break;
      case 5:
	 *angle = 270;
	 break;
      case 6:
	 *angle = 315;
	 break;
      case 7:
	 *angle = 0;
	 break;
      case 8:
	 *angle = 45;
	 break;
      default:
	 Beep();
	 return;
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
			 NULL);
      if (val < 1 || val > 7)
      {
	 Beep();
	 return;
      }
      *when = val;
      break;

   default:
      Beep();
      return;
   }
   return;
}



/*
   edit a line def info
*/
void EditLineDefInfo()
{
   char *menustr[ 30];
   int   n, val;

   for (n = 0; n < 7; n++)
      menustr[ n] = GetMemory( 60);
   sprintf( menustr[ 0], "Change Flags 1            (Current: %d)", LineDefs[ LastObject].flags1);
   sprintf( menustr[ 1], "Change Flags 2            (Current: %d)", LineDefs[ LastObject].flags2);
   sprintf( menustr[ 2], "Change Sector tag         (Current: %d)", LineDefs[ LastObject].tag);
   sprintf( menustr[ 3], "Change Starting Vertex    (Current: %d)", LineDefs[ LastObject].start);
   sprintf( menustr[ 4], "Change Ending Vertex      (Current: %d)", LineDefs[ LastObject].end);
   sprintf( menustr[ 5], "Change 1st SideDef number (Current: %d)", LineDefs[ LastObject].sidedef1);
   sprintf( menustr[ 6], "Change 2nd SideDef number (Current: %d)", LineDefs[ LastObject].sidedef2);
   if (CurObject >= 0)
      val = DisplayMenuArray( 0, 30, "Edit Selected LineDef", 7, menustr);
   else
      val = DisplayMenuArray( 0, 30, "Edit Last LineDef", 7, menustr);
   for (n = 0; n < 7; n++)
      free( menustr[ n]);
   switch (val)
   {
   case 1:
      val = InputIntegerValue( 42, 64, 0, 255, LineDefs[ LastObject].flags1);
      if (val >= 0)
	LineDefs[ LastObject].flags1 = val;
      break;
   case 2:
      val = InputIntegerValue( 42, 74, 0, 255, LineDefs[ LastObject].flags2);
      if (val >= 0)
	LineDefs[ LastObject].flags2 = val;
      break;
   case 3:
      val = InputIntegerValue( 42, 84, 0, 255, LineDefs[ LastObject].tag);
      if (val >= 0)
	LineDefs[ LastObject].tag = val;
      break;
   default:
      NotImplemented();
   }
}



/*
   edit a side def info
*/
void EditSideDefInfo()
{
   char *menustr[ 30];
   char  texname[ 9];
   int   n, val;

   for (n = 0; n < 6; n++)
      menustr[ n] = GetMemory( 60);
   sprintf( menustr[ 0], "Change X offset (?)   (Current: %d)", SideDefs[ LastObject].xoff);
   sprintf( menustr[ 1], "Change Y offset (?)   (Current: %d)", SideDefs[ LastObject].yoff);
   texname[ 8] = '\0';
   strncpy( texname, SideDefs[ LastObject].tex3, 8);
   sprintf( menustr[ 2], "Change Normal texture (Current: %s)", texname);
   strncpy( texname, SideDefs[ LastObject].tex1, 8);
   sprintf( menustr[ 3], "Change Texture above  (Current: %s)", texname);
   strncpy( texname, SideDefs[ LastObject].tex2, 8);
   sprintf( menustr[ 4], "Change Texture below  (Current: %s)", texname);
   sprintf( menustr[ 5], "Change Sector number  (Current: %d)", SideDefs[ LastObject].sector);
   if (CurObject >= 0)
      val = DisplayMenuArray( 0, 30, "Edit Selected SideDef", 6, menustr);
   else
      val = DisplayMenuArray( 0, 30, "Edit Last SideDef", 6, menustr);
   for (n = 0; n < 6; n++)
      free( menustr[ n]);
   switch (val)
   {
   case 1:
      val = InputIntegerValue( 42, 64, -9999, 9999, SideDefs[ LastObject].xoff);
      if (val >= -9999)
	SideDefs[ LastObject].xoff = val;
      break;
   case 2:
      val = InputIntegerValue( 42, 74, -9999, 9999, SideDefs[ LastObject].yoff);
      if (val >= -9999)
	SideDefs[ LastObject].yoff = val;
      break;
   default:
      NotImplemented();
   }
}



/*
   edit a sector info
*/
void EditSectorInfo()
{
   char *menustr[ 30];
   char  texname[ 9];
   int   n, val;

   for (n = 0; n < 7; n++)
      menustr[ n] = GetMemory( 60);
   sprintf( menustr[ 0], "Change Floor height  (Current: %d)", Sectors[ LastObject].floorh);
   sprintf( menustr[ 1], "Change Ceil. height  (Current: %d)", Sectors[ LastObject].ceilh);
   texname[ 8] = '\0';
   strncpy( texname, Sectors[ LastObject].floort, 8);
   sprintf( menustr[ 2], "Change Floor texture (Current: %s)", texname);
   strncpy( texname, Sectors[ LastObject].ceilt, 8);
   sprintf( menustr[ 3], "Change Ceil. texture (Current: %s)", texname);
   sprintf( menustr[ 4], "Change Light level   (Current: %d)", Sectors[ LastObject].light);
   sprintf( menustr[ 5], "Change Special flags (Current: %d)", Sectors[ LastObject].special);
   sprintf( menustr[ 6], "Change LineDef tag   (Current: %d)", Sectors[ LastObject].tag);
   if (CurObject >= 0)
      val = DisplayMenuArray( 0, 30, "Edit Selected Sector", 7, menustr);
   else
      val = DisplayMenuArray( 0, 30, "Edit Last Sector", 7, menustr);
   for (n = 0; n < 7; n++)
      free( menustr[ n]);
   switch (val)
   {
   case 1:
      val = InputIntegerValue( 42, 64, -264, 264, Sectors[ LastObject].floorh);
      if (val >= -264)
	Sectors[ LastObject].floorh = val;
      break;
   case 2:
      val = InputIntegerValue( 42, 74, -264, 264, Sectors[ LastObject].ceilh);
      if (val >= -264)
	Sectors[ LastObject].ceilh = val;
      break;
   case 5:
      val = InputIntegerValue( 42, 104, 0, 255, Sectors[ LastObject].light);
      if (val >= 0)
	Sectors[ LastObject].light = val;
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
	   Sectors[ LastObject].special = 0;
	   break;
	case 2:
	   Sectors[ LastObject].special = 1;
	   break;
	case 3:
	   Sectors[ LastObject].special = 2;
	   break;
	case 4:
	   Sectors[ LastObject].special = 3;
	   break;
	case 5:
	   Sectors[ LastObject].special = 4;
	   break;
	case 6:
	   Sectors[ LastObject].special = 5;
	   break;
	case 7:
	   Sectors[ LastObject].special = 7;
	   break;
	case 8:
	   Sectors[ LastObject].special = 8;
	   break;
	case 9:
	   Sectors[ LastObject].special = 9;
	   break;
	case 10:
	   val = InputIntegerValue( 84, 238, 0, 255, Sectors[ LastObject].special);
	   if (val >= 0)
	     Sectors[ LastObject].special = val;
	   break;
      }
      break;
   case 7:
      val = InputIntegerValue( 42, 124, 0, 255, Sectors[ LastObject].tag);
      if (val >= 0)
	Sectors[ LastObject].tag = val;
      break;
   default:
      NotImplemented();
   }
}



/*
   check if there is something of interest near the pointer
   (*bug: you some lines cannot be selected with this method*)
*/
void GetCurObject()
{
   int  x0 = MAPX(PointerX) - OBJSIZE;
   int  x1 = MAPX(PointerX) + OBJSIZE;
   int  y0 = MAPY(PointerY) - OBJSIZE;
   int  y1 = MAPY(PointerY) + OBJSIZE;
   int  n, m, old;

   old = CurObject;
   CurObject = -1;

   switch (EditMode)
   {
   case EDIT_THINGS:
      for (n = 0; n < NumThings; n++)
	 if (Things[ n].xpos >= x0 && Things[ n].xpos <= x1 && Things[ n].ypos >= y0 && Things[ n].ypos <= y1)
	 {
	    CurObject = n;
	    break;
	 }
      break;
   case EDIT_LINEDEFS:
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    CurObject = n;
	    break;
	 }
      }
      break;
   case EDIT_SIDEDEFS:
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    CurObject = LineDefs[ n].sidedef1;
	    break;
	 }
      }
      break;
   case EDIT_VERTEXES:
      for (n = 0; n < NumVertexes; n++)
	 if (Vertexes[ n].x >= x0 && Vertexes[ n].x <= x1 && Vertexes[ n].y >= y0 && Vertexes[ n].y <= y1)
	 {
	    CurObject = n;
	    break;
	 }
      break;
   case EDIT_SEGS:
      for (n = 0; n < NumSegs; n++)
      {
	 m = Segs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    CurObject = n;
	    break;
	 }
      }
      break;
   case EDIT_SECTORS:
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    CurObject = SideDefs[ LineDefs[ n].sidedef1].sector;
	    break;
	 }
      }
      break;
   }

   if (CurObject >= 0)
     LastObject = CurObject;
   if (CurObject != old)
     RedrawMap = TRUE;
}



/*
   delete an object (*should be recursive if a vertex is deleted*)
*/
void DeleteObject( int objtype, int objnum)
{
   int n;

   switch (objtype)
   {
   case EDIT_THINGS:
      NumThings--;
      for (n = objnum; n < NumThings; n++)
	Things[ n] = Things[ n + 1];
      Things = ResizeMemory( Things, NumThings * sizeof( struct Thing));
      break;
/*
   case EDIT_VERTEXES:
      NumVertexes--;
      for (n = objnum; n < NumVertexes; n++)
	Vertexes[ n] = Vertexes[ n + 1];
      Vertexes = ResizeMemory( Vertexes, NumVertexes * sizeof( struct Vertex));
      (* decrease linedefs & segs refs ! *)
      break;
*/
   default:
      Beep();
   }
}



/*
   insert a new object
*/
void InsertObject(int objtype)
{
   int last;

   switch (objtype)
   {
   case EDIT_THINGS:
      last = NumThings++;
      Things = ResizeMemory( Things, NumThings * sizeof( struct Thing));
      Things[ last].xpos  = MAPX(PointerX);
      Things[ last].ypos  = MAPY(PointerY);
      Things[ last].type  = DefType;
      Things[ last].angle = DefAngle;
      Things[ last].when  = DefWhen;
      break;
/*
   case EDIT_VERTEXES:
      last = NumVertexes++;
      Vertexes = ResizeMemory( Vertexes, NumVertexes * sizeof( struct Vertex));
      Vertexes[ last].x  = MAPX(PointerX);
      Vertexes[ last].y  = MAPY(PointerY);
      break;
*/
   default:
      Beep();
   }
}



/* end of file */
