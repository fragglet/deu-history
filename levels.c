/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   LEVELS.C - Level loading and saving routines.
*/

/* the includes */
#include "deu.h"
#include "wstructs.h"
#include "things.h"

/* external function from objects.c */
extern Bool CreateNodes( NPtr *, int *, SEPtr); /* SWAP - needs Vertexes */

/* the global data */
MDirPtr Level = NULL;		/* master dictionary entry for the level */
int NumThings = 0;		/* number of things */
TPtr Things;			/* things data */
int NumLineDefs = 0;		/* number of line defs */
LDPtr LineDefs;			/* line defs data */
int NumSideDefs = 0;		/* number of side defs */
SDPtr SideDefs;			/* side defs data */
int NumVertexes = 0;		/* number of vertexes */
VPtr Vertexes;			/* vertex data */
int NumSectors = 0;		/* number of sectors */
SPtr Sectors;			/* sectors data */
int NumSegs = 0;		/* number of segments */
SEPtr Segs = NULL;		/* list of segments */
SEPtr LastSeg = NULL;		/* last segment in the list */
int NumSSectors = 0;		/* number of subsectors */
SSPtr SSectors = NULL;		/* list of subsectors */
SSPtr LastSSector = NULL;	/* last subsector in the list */
int NumNodes = 0;		/* number of Nodes */
NPtr Nodes = NULL;		/* nodes tree */
int NumWTexture = 0;		/* number of wall textures */
char **WTexture;		/* array of wall texture names */
int NumFTexture = 0;		/* number of floor/ceiling textures */
char **FTexture;		/* array of texture names */
int MapMaxX = -32767;		/* maximum X value of map */
int MapMaxY = -32767;		/* maximum Y value of map */
int MapMinX = 32767;		/* minimum X value of map */
int MapMinY = 32767;		/* minimum Y value of map */
Bool MadeChanges = FALSE;	/* made changes? */
Bool MadeMapChanges = FALSE;	/* made changes that need rebuilding? */


/*
   read in the level data
*/

void ReadLevelData( int episode, int mission) /* SWAP! */
{
   MDirPtr dir;
   char name[ 7];
   int n, m;
   int val;
   int OldNumVertexes;
   int *VertexUsed;

   /* No objects are needed: they may be swapped after they have been read */
   ObjectsNeeded( 0);

   /* find the various level information from the master directory */
   sprintf( name, "E%dM%d", episode, mission);
   DisplayMessage( -1, -1, "Reading data for level %s...", name);
   Level = FindMasterDir( MasterDir, name);
   if (!Level)
      ProgError( "level data not found");

   /* get the number of Vertices */
   dir = FindMasterDir( Level, "VERTEXES");
   if (dir != NULL)
      OldNumVertexes = (int) (dir->dir.size / 4L);
   else
      OldNumVertexes = 0;
   if (OldNumVertexes > 0)
   {
      VertexUsed = GetMemory( OldNumVertexes * sizeof( int));
      for (n = 0; n < OldNumVertexes; n++)
	 VertexUsed[ n] = FALSE;
   }

   /* read in the Things data */
   dir = FindMasterDir( Level, "THINGS");
   if (dir != NULL)
      NumThings = (int) (dir->dir.size / 10L);
   else
      NumThings = 0;
   if (NumThings > 0)
   {
      Things = GetFarMemory( (unsigned long) NumThings * sizeof( struct Thing));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumThings; n++)
      {
	 BasicWadRead( dir->wadfile, &(Things[ n].xpos), 2);
	 BasicWadRead( dir->wadfile, &(Things[ n].ypos), 2);
	 BasicWadRead( dir->wadfile, &(Things[ n].angle), 2);
	 BasicWadRead( dir->wadfile, &(Things[ n].type), 2);
	 BasicWadRead( dir->wadfile, &(Things[ n].when), 2);
      }
   }

   /* read in the LineDef information */
   dir = FindMasterDir( Level, "LINEDEFS");
   if (dir != NULL)
      NumLineDefs = (int) (dir->dir.size / 14L);
   else
      NumLineDefs = 0;
   if (NumLineDefs > 0)
   {
      LineDefs = GetFarMemory( (unsigned long) NumLineDefs * sizeof( struct LineDef));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumLineDefs; n++)
      {
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].start), 2);
	 VertexUsed[ LineDefs[ n].start] = TRUE;
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].end), 2);
	 VertexUsed[ LineDefs[ n].end] = TRUE;
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].flags), 2);
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].type), 2);
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].tag), 2);
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].sidedef1), 2);
	 BasicWadRead( dir->wadfile, &(LineDefs[ n].sidedef2), 2);
      }
   }

   /* read in the SideDef information */
   dir = FindMasterDir( Level, "SIDEDEFS");
   if (dir != NULL)
      NumSideDefs = (int) (dir->dir.size / 30L);
   else
      NumSideDefs = 0;
   if (NumSideDefs > 0)
   {
      SideDefs = GetFarMemory( (unsigned long) NumSideDefs * sizeof( struct SideDef));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumSideDefs; n++)
      {
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].xoff), 2);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].yoff), 2);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].tex1), 8);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].tex2), 8);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].tex3), 8);
	 BasicWadRead( dir->wadfile, &(SideDefs[ n].sector), 2);
      }
   }

   /* read in the Vertices which are all the corners of the level, but ignore the */
   /* Vertices not used in any LineDef (they usually are at the end of the list). */
   NumVertexes = 0;
   for (n = 0; n < OldNumVertexes; n++)
      if (VertexUsed[ n])
	 NumVertexes++;
   if (NumVertexes > 0)
   {
      Vertexes = GetFarMemory( (unsigned long) NumVertexes * sizeof( struct Vertex));
      dir = FindMasterDir( Level, "VERTEXES");
      BasicWadSeek( dir->wadfile, dir->dir.start);
      MapMaxX = -32767;
      MapMaxY = -32767;
      MapMinX = 32767;
      MapMinY = 32767;
      m = 0;
      for (n = 0; n < OldNumVertexes; n++)
      {
	 BasicWadRead( dir->wadfile, &val, 2);
	 if (VertexUsed[ n])
	 {
	    if (val < MapMinX)
	       MapMinX = val;
	    if (val > MapMaxX)
	       MapMaxX = val;
	    Vertexes[ m].x = val;
	 }
	 BasicWadRead( dir->wadfile, &val, 2);
	 if (VertexUsed[ n])
	 {
	    if (val < MapMinY)
	       MapMinY = val;
	    if (val > MapMaxY)
	       MapMaxY = val;
	    Vertexes[ m].y = val;
	    m++;
	 }
      }
      if (m != NumVertexes)
	 ProgError("inconsistency in the Vertexes data\n");
   }

   if (OldNumVertexes > 0)
   {
      /* update the Vertex numbers in the LineDefs (not really necessary, but...) */
      m = 0;
      for (n = 0; n < OldNumVertexes; n++)
	 if (VertexUsed[ n])
	    VertexUsed[ n] = m++;
      ObjectsNeeded( OBJ_LINEDEFS, 0);
      for (n = 0; n < NumLineDefs; n++)
      {
	 LineDefs[ n].start = VertexUsed[ LineDefs[ n].start];
	 LineDefs[ n].end = VertexUsed[ LineDefs[ n].end];
      }
      ObjectsNeeded( 0);
      FreeMemory( VertexUsed);
   }

   /* ignore the Segs, SSectors and Nodes */

   /* read in the Sectors information */
   dir = FindMasterDir( Level, "SECTORS");
   if (dir != NULL)
      NumSectors = (int) (dir->dir.size / 26L);
   else
      NumSectors = 0;
   if (NumSectors > 0)
   {
      Sectors = GetFarMemory( (unsigned long) NumSectors * sizeof( struct Sector));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumSectors; n++)
      {
	 BasicWadRead( dir->wadfile, &(Sectors[ n].floorh), 2);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].ceilh), 2);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].floort), 8);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].ceilt), 8);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].light), 2);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].special), 2);
	 BasicWadRead( dir->wadfile, &(Sectors[ n].tag), 2);
      }
   }

   /* ignore the last entries (Reject & BlockMap) */
}



/*
   forget the level data
*/

void ForgetLevelData() /* SWAP! */
{
   /* forget the Things */
   ObjectsNeeded( OBJ_THINGS, 0);
   NumThings = 0;
   if (Things)
      FreeFarMemory( Things);
   Things = NULL;

   /* forget the Vertices */
   ObjectsNeeded( OBJ_VERTEXES, 0);
   NumVertexes = 0;
   if (Vertexes)
      FreeFarMemory( Vertexes);
   Vertexes = NULL;

   /* forget the LineDefs */
   ObjectsNeeded( OBJ_LINEDEFS, 0);
   NumLineDefs = 0;
   if (LineDefs)
      FreeFarMemory( LineDefs);
   LineDefs = NULL;

   /* forget the SideDefs */
   ObjectsNeeded( OBJ_SIDEDEFS, 0);
   NumSideDefs = 0;
   if (SideDefs)
      FreeFarMemory( SideDefs);
   SideDefs = NULL;

   /* forget the Sectors */
   ObjectsNeeded( OBJ_SECTORS, 0);
   NumSectors = 0;
   if (Sectors)
      FreeFarMemory( Sectors);
   Sectors = NULL;
   ObjectsNeeded( 0);
}



/*
   recursively save the Nodes data to a PWAD file
*/

void SaveNodes( FILE *file, NPtr node)
{
   /* Nodes tree walk: save child1, save child2, save parent */
   if ((node->child1 & 0x8000) == 0)
   {
      SaveNodes( file, node->node1);
      node->child1 = node->node1->num;
   }
   if ((node->child2 & 0x8000) == 0)
   {
      SaveNodes( file, node->node2);
      node->child2 = node->node2->num;
   }
   WriteBytes( file, &(node->x), 2L);
   WriteBytes( file, &(node->y), 2L);
   WriteBytes( file, &(node->dx), 2L);
   WriteBytes( file, &(node->dy), 2L);
   WriteBytes( file, &(node->maxy1), 2L);
   WriteBytes( file, &(node->miny1), 2L);
   WriteBytes( file, &(node->minx1), 2L);
   WriteBytes( file, &(node->maxx1), 2L);
   WriteBytes( file, &(node->maxy2), 2L);
   WriteBytes( file, &(node->miny2), 2L);
   WriteBytes( file, &(node->minx2), 2L);
   WriteBytes( file, &(node->maxx2), 2L);
   WriteBytes( file, &(node->child1), 2L);
   WriteBytes( file, &(node->child2), 2L);
   node->num = NumNodes++;
}



/*
   forget the Nodes
*/

void ForgetNodes( NPtr node)
{
   if ((node->child1 & 0x8000) == 0)
      ForgetNodes( node->node1);
   if ((node->child2 & 0x8000) == 0)
      ForgetNodes( node->node2);
   FreeFarMemory( node);
}



/*
   save the level data to a PWAD file
*/

void SaveLevelData( char *outfile) /* SWAP! */
{
   FILE   *file;
   MDirPtr dir;
   long    counter = 11;
   int     n, i, j;
   void   *data;
   long    size;
   long    dirstart;
   int    *blockptr;
   long    blocksize;
   int     blockcount;
   long    oldpos;
   Bool    newnodes;
   long    rejectsize;
   int     oldNumVertexes;

   DisplayMessage( -1, -1, "Saving data to \"%s\"...", outfile);
   LogMessage( ": Saving data to \"%s\"...\n", outfile);
   oldNumVertexes = NumVertexes;
   /* open the file */
   if ((file = fopen( outfile, "wb")) == NULL)
      ProgError( "Unable to open file \"%s\"", outfile);
   WriteBytes( file, "PWAD", 4L);     /* PWAD file */
   WriteBytes( file, &counter, 4L);   /* 11 entries */
   WriteBytes( file, &counter, 4L);   /* fix this up later */
   counter = 12L;
   dir = Level->next;

   /* output the things data */
   ObjectsNeeded( OBJ_THINGS, 0);
   for (n = 0; n < NumThings; n++)
   {
      WriteBytes( file, &(Things[ n].xpos), 2L);
      WriteBytes( file, &(Things[ n].ypos), 2L);
      WriteBytes( file, &(Things[ n].angle), 2L);
      WriteBytes( file, &(Things[ n].type), 2L);
      WriteBytes( file, &(Things[ n].when), 2L);
      counter += 10L;
   }
   dir = dir->next;

   /* update MapMinX, MapMinY, MapMaxX, MapMaxY */
   ObjectsNeeded( OBJ_VERTEXES, 0);
   MapMaxX = -32767;
   MapMaxY = -32767;
   MapMinX = 32767;
   MapMinY = 32767;
   for (n = 0; n < NumVertexes; n++)
   {
      if (Vertexes[ n].x < MapMinX)
	 MapMinX = Vertexes[ n].x;
      if (Vertexes[ n].x > MapMaxX)
	 MapMaxX = Vertexes[ n].x;
      if (Vertexes[ n].y < MapMinY)
	 MapMinY = Vertexes[ n].y;
      if (Vertexes[ n].y > MapMaxY)
	 MapMaxY = Vertexes[ n].y;
   }

   /* do we need to rebuild the Nodes, Segs and SSectors? */
   if (MadeMapChanges && (Expert || Confirm( -1, 270, "Do you want to rebuild the NODES, SEGS, SSECTORS, REJECT and BLOCKMAP?",
						      "WARNING: You won't be able to use your level if you don't do this...")))
   {
      SEPtr seglist;

      if (UseMouse)
	 HideMousePointer();
      ClearScreen();
      DrawScreenBox3D( 218, 0, ScrMaxX, 55);
      SetColor( WHITE);
      DrawScreenText( 225, 10, "Rebuilding the NODES...");
      DrawScreenBoxHollow( 225, 28, ScrMaxX - 10, 48);
      DrawScreenMeter( 225, 28, ScrMaxX - 10, 48, 0.0);
      if (UseMouse)
	 ShowMousePointer();
      seglist = NULL;
      ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].sidedef1 >= 0)
	 {
	    if (seglist)
	    {
	       LastSeg->next = GetMemory( sizeof( struct Seg));
	       LastSeg = LastSeg->next;
	    }
	    else
	    {
	       seglist = GetMemory( sizeof( struct Seg));
	       LastSeg = seglist;
	    }
	    LastSeg->next = NULL;
	    LastSeg->start = LineDefs[ n].start;
	    LastSeg->end = LineDefs[ n].end;
	    LastSeg->angle = ComputeAngle(Vertexes[ LineDefs[ n].end].x - Vertexes[ LineDefs[ n].start].x,
					  Vertexes[ LineDefs[ n].end].y - Vertexes[ LineDefs[ n].start].y);
	    LastSeg->linedef = n;
	    LastSeg->flip = 0;
	    LastSeg->dist = 0;
	 }
	 if (LineDefs[ n].sidedef2 >= 0)
	 {
	    if (seglist)
	    {
	       LastSeg->next = GetMemory( sizeof( struct Seg));
	       LastSeg = LastSeg->next;
	    }
	    else
	    {
	       seglist = GetMemory( sizeof( struct Seg));
	       LastSeg = seglist;
	    }
	    LastSeg->next = NULL;
	    LastSeg->start = LineDefs[ n].end;
	    LastSeg->end = LineDefs[ n].start;
	    LastSeg->angle = ComputeAngle(Vertexes[ LineDefs[ n].start].x - Vertexes[ LineDefs[ n].end].x,
					  Vertexes[ LineDefs[ n].start].y - Vertexes[ LineDefs[ n].end].y);
	    LastSeg->linedef = n;
	    LastSeg->flip = 1;
	    LastSeg->dist = 0;
	 }
      }
      ShowProgress( OBJ_VERTEXES);
      ShowProgress( OBJ_SIDEDEFS);
      LogMessage( ": Starting Nodes builder...\n");
      LogMessage( "\tNumber of Vertices: %d\n", NumVertexes);
      LogMessage( "\tNumber of Segs:     %d\n", NumSegs);
      ObjectsNeeded( OBJ_VERTEXES, 0);
      if (CreateNodes( &Nodes, &n, seglist) == FALSE)
      {
	 Beep();
	 Beep();
	 Beep();
         LogMessage( "\nError: CreateNodes failed!\n\n");
	 Beep();
	 Beep();
	 Beep();
      }
      LogMessage( ": Nodes created OK.\n");
      LogMessage( "\tNumber of Vertices: %d\n", NumVertexes);
      LogMessage( "\tNumber of SideDefs: %d\n", NumSideDefs);
      LogMessage( "\tNumber of Segs:     %d\n", NumSegs);
      LogMessage( "\tNumber of SSectors: %d\n", NumSSectors);
      if (UseMouse)
	 HideMousePointer();
      DrawScreenMeter( 225, 28, ScrMaxX - 10, 48, 1.0);
      if (UseMouse)
	 ShowMousePointer();
      newnodes = TRUE;
   }
   else
      newnodes = FALSE;

   /* output the LineDefs */
   ObjectsNeeded( OBJ_LINEDEFS, 0);
   for (n = 0; n < NumLineDefs; n++)
   {
      WriteBytes( file, &(LineDefs[ n].start), 2L);
      WriteBytes( file, &(LineDefs[ n].end), 2L);
      WriteBytes( file, &(LineDefs[ n].flags), 2L);
      WriteBytes( file, &(LineDefs[ n].type), 2L);
      WriteBytes( file, &(LineDefs[ n].tag), 2L);
      WriteBytes( file, &(LineDefs[ n].sidedef1), 2L);
      WriteBytes( file, &(LineDefs[ n].sidedef2), 2L);
      counter += 14L;
   }
   dir = dir->next;

   /* output the SideDefs */
   ObjectsNeeded( OBJ_SIDEDEFS, 0);
   for (n = 0; n < NumSideDefs; n++)
   {
      WriteBytes( file, &(SideDefs[ n].xoff), 2L);
      WriteBytes( file, &(SideDefs[ n].yoff), 2L);
      WriteBytes( file, &(SideDefs[ n].tex1), 8L);
      WriteBytes( file, &(SideDefs[ n].tex2), 8L);
      WriteBytes( file, &(SideDefs[ n].tex3), 8L);
      WriteBytes( file, &(SideDefs[ n].sector), 2L);
      counter += 30L;
   }
   dir = dir->next;

   if (MadeMapChanges)
   {
      /* output the Vertices */
      ObjectsNeeded( OBJ_VERTEXES, 0);
      for (n = 0; n < NumVertexes; n++)
      {
	 WriteBytes( file, &(Vertexes[ n].x), 2L);
	 WriteBytes( file, &(Vertexes[ n].y), 2L);
	 counter += 4L;
      }
   }
   else
   {
      /* copy the Vertices */
      ObjectsNeeded( 0);
      size = dir->dir.size;
      counter += size;
      BasicWadSeek( dir->wadfile, dir->dir.start);
      CopyBytes( file, dir->wadfile->fileinfo, size);
   }
   dir = dir->next;

   if (newnodes)
   {
      SEPtr curse, oldse;
      SSPtr curss, oldss;

      ObjectsNeeded( 0);
      /* output and forget the Segments */
      curse = Segs;
      while (curse)
      {
	 WriteBytes( file, &(curse->start), 2L);
	 WriteBytes( file, &(curse->end), 2L);
	 WriteBytes( file, &(curse->angle), 2L);
	 WriteBytes( file, &(curse->linedef), 2L);
	 WriteBytes( file, &(curse->flip), 2L);
	 WriteBytes( file, &(curse->dist), 2L);
	 oldse = curse;
	 curse = curse->next;
	 FreeFarMemory( oldse);
	 counter += 12L;
      }
      Segs = NULL;
      dir = dir->next;

      /* output and forget the SSectors */
      curss = SSectors;
      while (curss)
      {
	 WriteBytes( file, &(curss->num), 2L);
	 WriteBytes( file, &(curss->first), 2L);
	 oldss = curss;
	 curss = curss->next;
	 FreeFarMemory( oldss);
	 counter += 4L;
      }
      SSectors = NULL;
      dir = dir->next;

      /* output the Nodes */
      NumNodes = 0;
      SaveNodes( file, Nodes);
      counter += (long) NumNodes * 28L;
      dir = dir->next;

      /* forget the Nodes */
      ForgetNodes( Nodes);
      Nodes = NULL;
   }
   else
   {
      /* copy the Segs, SSectors and Nodes */
      for (n = 0; n < 3; n++)
      {
	 size = dir->dir.size;
	 counter += size;
	 BasicWadSeek( dir->wadfile, dir->dir.start);
	 CopyBytes( file, dir->wadfile->fileinfo, size);
	 dir = dir->next;
      }
   }

   /* output the Sectors */
   ObjectsNeeded( OBJ_SECTORS, 0);
   for (n = 0; n < NumSectors; n++)
   {
      WriteBytes( file, &(Sectors[ n].floorh), 2L);
      WriteBytes( file, &(Sectors[ n].ceilh), 2L);
      WriteBytes( file, &(Sectors[ n].floort), 8L);
      WriteBytes( file, &(Sectors[ n].ceilt), 8L);
      WriteBytes( file, &(Sectors[ n].light), 2L);
      WriteBytes( file, &(Sectors[ n].special), 2L);
      WriteBytes( file, &(Sectors[ n].tag), 2L);
      counter += 26L;
   }
   dir = dir->next;

   if (newnodes)
   {
      /* create and output the reject data */
      ObjectsNeeded( OBJ_SECTORS, 0); /* !!! */
      if (UseMouse)
	 HideMousePointer();
      DrawScreenBox3D( 218, 80, ScrMaxX, 135);
      SetColor( WHITE);
      DrawScreenText( 225, 90, "Rebuilding the REJECT data...");
      DrawScreenBoxHollow( 225, 108, ScrMaxX - 10, 128);
      DrawScreenMeter( 225, 108, ScrMaxX - 10, 128, 0.0);
      if (UseMouse)
	 ShowMousePointer();
      rejectsize = ((long) NumSectors * (long) NumSectors + 7L) / 8L;
      data = GetMemory( (size_t) rejectsize);
      for (i = 0; i < rejectsize; i++)
	 ((char *) data)[ i] = 0;
      for (i = 0; i < NumSectors; i++)
      {
	 if (UseMouse)
	    HideMousePointer();
	 DrawScreenMeter( 225, 108, ScrMaxX - 10, 128, (float) i / (float) NumSectors);
	 if (UseMouse)
	    ShowMousePointer();
	 for (j = 0; j < NumSectors; j++)
	 {
/*
	    if (Reject( i, j))
	       data[ (i * NumSectors + j) / 8] |= 1 <<
*/
	 }
      }
      if (UseMouse)
	 HideMousePointer();
      DrawScreenMeter( 225, 108, ScrMaxX - 10, 128, 1.0);
      if (UseMouse)
	 ShowMousePointer();
      WriteBytes( file, data, rejectsize);
      counter += rejectsize;
      dir = dir->next;
      FreeMemory( data);
   }
   else
   {
      /* copy the Reject data */
      ObjectsNeeded( 0);
      rejectsize = dir->dir.size;
      size = rejectsize;
      counter += size;
      BasicWadSeek( dir->wadfile, dir->dir.start);
      CopyBytes( file, dir->wadfile->fileinfo, size);
      dir = dir->next;
   }

   if (newnodes)
   {
      int mminx, mminy, mnumx, mnumy;

      /* create and output the blockmap */
      ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
      if (UseMouse)
	 HideMousePointer();
      DrawScreenBox3D( 218, 160, ScrMaxX, 215);
      SetColor( WHITE);
      DrawScreenText( 225, 170, "Rebuilding the BLOCKMAP...");
      DrawScreenBoxHollow( 225, 188, ScrMaxX - 10, 208);
      DrawScreenMeter( 225, 188, ScrMaxX - 10, 208, 0.0);
      if (UseMouse)
	 ShowMousePointer();
      mminx = (int) (MapMinX / 8 - 8) * 8;
      WriteBytes( file, &mminx, 2L);
      mminy = (int) (MapMinY / 8 - 8) * 8;
      WriteBytes( file, &mminy, 2L);
      mnumx = MapMaxX / 128 - mminx / 128 + 2;
      WriteBytes( file, &mnumx, 2L);
      mnumy = MapMaxY / 128 - mminy / 128 + 2;
      WriteBytes( file, &mnumy, 2L);
      counter += 8L;
      oldpos = ftell( file);
      blocksize = (long) (mnumx * mnumy * sizeof( int));
      blockptr = GetMemory( blocksize);
      WriteBytes( file, blockptr, blocksize);
      blocksize += 8L;
      counter += blocksize - 7L;
      blockcount = mnumx * mnumy + 4;
      for (i = 0; i < mnumy; i++)
      {
	 if (UseMouse)
	    HideMousePointer();
	 DrawScreenMeter( 225, 188, ScrMaxX - 10, 208, (float) i / (float) mnumy);
	 if (UseMouse)
	    ShowMousePointer();
	 for (j = 0; j < mnumx; j++)
	 {
	    blockptr[ mnumx * i + j] = blockcount;
	    n = 0;
	    WriteBytes( file, &n, 2L);
	    counter += 2L;
	    blocksize += 2L;
	    blockcount++;
	    for (n = 0; n < NumLineDefs; n++)
	       if (IsLineDefInside( n, mminx + j * 128, mminy + i * 128, mminx + 127 + j * 128, mminy + 127 + i * 128))
	       {
		  WriteBytes( file, &n, 2L);
		  counter += 2L;
		  blocksize += 2L;
		  blockcount++;
	       }
	    n = -1;
	    WriteBytes( file, &n, 2L);
	    counter += 2L;
	    blocksize += 2L;
	    blockcount++;
	 }
      }
      if (UseMouse)
	 HideMousePointer();
      DrawScreenMeter( 225, 188, ScrMaxX - 10, 208, 1.0);
      if (UseMouse)
	 ShowMousePointer();
      size = ftell( file);
      fseek( file, oldpos, SEEK_SET);
      WriteBytes( file, blockptr, (long) (mnumx * mnumy * sizeof( int)));
      fseek( file, size, SEEK_SET);
      if (FindMasterDir( dir, "P2_END"))
	 counter--;
      FreeMemory( blockptr);
   }
   else
   {
      /* copy the blockmap data */
      ObjectsNeeded( 0);
      blocksize = dir->dir.size;
      size = blocksize;
      counter += size;
      BasicWadSeek( dir->wadfile, dir->dir.start);
      CopyBytes( file, dir->wadfile->fileinfo, size);
      dir = dir->next;
   }


   /* output the actual directory */
   dirstart = counter;
   counter = 12L;
   size = 0L;
   dir = Level;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, &(dir->dir.name), 8L);
   dir = dir->next;

   size = (long) NumThings * 10L;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "THINGS\0\0", 8L);
   counter += size;
   dir = dir->next;

   size = (long) NumLineDefs * 14L;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "LINEDEFS", 8L);
   counter += size;
   dir = dir->next;

   size = (long) NumSideDefs * 30L;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SIDEDEFS", 8L);
   counter += size;
   dir = dir->next;

   if (MadeMapChanges)
      size = (long) NumVertexes * 4L;
   else
      size = dir->dir.size;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "VERTEXES", 8L);
   counter += size;
   dir = dir->next;

   if (newnodes)
      size = (long) NumSegs * 12L;
   else
      size = dir->dir.size;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SEGS\0\0\0\0", 8L);
   counter += size;
   dir = dir->next;

   if (newnodes)
      size = (long) NumSSectors * 4L;
   else
      size = dir->dir.size;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SSECTORS", 8L);
   counter += size;
   dir = dir->next;

   if (newnodes)
      size = (long) NumNodes * 28L;
   else
      size = dir->dir.size;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "NODES\0\0\0", 8L);
   counter += size;
   dir = dir->next;

   size = (long) NumSectors * 26L;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SECTORS\0", 8L);
   counter += size;
   dir = dir->next;

   size = rejectsize;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "REJECT\0\0", 8L);
   counter += size;
   dir = dir->next;

   size = blocksize;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "BLOCKMAP", 8L);
   counter += size;
   dir = dir->next;

   /* fix up the directory start information */
   if (fseek( file, 8L, SEEK_SET))
      ProgError( "error writing to file");
   WriteBytes( file, &dirstart, 4L);

   /* close the file */
   fclose( file);

   NumSegs = 0;
   NumSSectors = 0;
   NumNodes = 0;

   /* delete the vertices added by the Nodes builder */
   if (NumVertexes != oldNumVertexes)
   {
      ObjectsNeeded( OBJ_VERTEXES, 0);
      NumVertexes = oldNumVertexes;
      ResizeFarMemory( Vertexes, NumVertexes * sizeof( struct Vertex));
   }

   /* the file is now up to date */
   MadeChanges = FALSE;
   if (newnodes)
      MadeMapChanges = FALSE;
   ObjectsNeeded( 0);

   /* update pointers in Master Directory */
   OpenPatchWad( outfile);

   /* this should free the old "*.BAK" file */
   CloseUnusedWadFiles();

}



/*
   function used by qsort to sort the texture names
*/
int SortTextures( const void *a, const void *b)
{
   return strcmp( *((char **)a), *((char **)b));
}



/*
   read in the wall texture names
*/

void ReadWTextureNames()
{
   MDirPtr dir;
   long *offsets;
   int n;
   long val;

   printf("Reading wall texture names\n");
   dir = FindMasterDir( MasterDir, "TEXTURE1");
   BasicWadSeek( dir->wadfile, dir->dir.start);
   BasicWadRead( dir->wadfile, &val, 4);
   NumWTexture = (int) val + 1;
   /* read in the offsets for texture1 names */
   offsets = GetMemory( NumWTexture * sizeof( long));
   for (n = 1; n < NumWTexture; n++)
      BasicWadRead( dir->wadfile, &(offsets[ n]), 4);
   /* read in the actual names */
   WTexture = GetMemory( NumWTexture * sizeof( char *));
   WTexture[ 0] = GetMemory( 9 * sizeof( char));
   strcpy(WTexture[ 0], "-");
   for (n = 1; n < NumWTexture; n++)
   {
      WTexture[ n] = GetMemory( 9 * sizeof( char));
      BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ n]);
      BasicWadRead( dir->wadfile, WTexture[ n], 8);
      WTexture[ n][ 8] = '\0';
   }
   FreeMemory( offsets);
   if (Registered)
   {
      dir = FindMasterDir( MasterDir, "TEXTURE2");
      BasicWadSeek( dir->wadfile, dir->dir.start);
      BasicWadRead( dir->wadfile, &val, 4);
      /* read in the offsets for texture2 names */
      offsets = GetMemory( val * sizeof( long));
      for (n = 0; n < val; n++)
	 BasicWadRead( dir->wadfile, &(offsets[ n]), 4);
      /* read in the actual names */
      WTexture = ResizeMemory( WTexture, (NumWTexture + val) * sizeof( char *));
      for (n = 0; n < val; n++)
      {
	 WTexture[ NumWTexture + n] = GetMemory( 9 * sizeof( char));
	 BasicWadSeek( dir->wadfile, dir->dir.start + offsets[ n]);
	 BasicWadRead( dir->wadfile, WTexture[ NumWTexture + n], 8);
	 WTexture[ NumWTexture + n][ 8] = '\0';
      }
      NumWTexture += val;
      FreeMemory( offsets);
   }
   /* sort the names */
   qsort( WTexture, NumWTexture, sizeof( char *), SortTextures);
}



/*
   forget the wall texture names
*/

void ForgetWTextureNames()
{
   int n;

   /* forget all names */
   for (n = 0; n < NumWTexture; n++)
      FreeMemory( WTexture[ n]);

   /* forget the array */
   NumWTexture = 0;
   FreeMemory( WTexture);
}



/*
   read in the floor/ceiling texture names
*/

void ReadFTextureNames()
{
   MDirPtr dir;
   int n, m;

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
   if (Registered)
   {
      /* count the names */
      dir = FindMasterDir( MasterDir, "F2_START");
      dir = dir->next;
      for (n = 0; dir && strcmp(dir->dir.name, "F2_END"); n++)
	 dir = dir->next;
      /* get the actual names from master dir. */
      dir = FindMasterDir( MasterDir, "F2_START");
      dir = dir->next;
      FTexture = ResizeMemory( FTexture, (NumFTexture + n) * sizeof( char *));
      for (m = NumFTexture; m < NumFTexture + n; m++)
      {
	 FTexture[ m] = GetMemory( 9 * sizeof( char));
	 strncpy( FTexture[ m], dir->dir.name, 8);
	 FTexture[ m][ 8] = '\0';
	 dir = dir->next;
      }
      NumFTexture += n;
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
      FreeMemory( FTexture[ n]);

   /* forget the array */
   NumFTexture = 0;
   FreeMemory( FTexture);
}



/* end of file */
