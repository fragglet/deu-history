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
MDirPtr Level = NULL;		/* master dictionary entry for the level */
int NumThings = 0;		/* number of things */
TPtr Things;			/* things data */
int NumLineDefs = 0;		/* number of line defs */
LDPtr LineDefs;			/* line defs data */
int NumSideDefs = 0;		/* number of side defs */
SDPtr SideDefs;			/* side defs data */
int NumVertexes = 0;		/* number of vertexes */
VPtr Vertexes;			/* vertex data */
int NumSegs = 0;		/* number of segments */
SEPtr Segs;			/* segments data */
int NumSSectors = 0;		/* number of subsectors */
SSPtr SSectors;			/* subsectors data */
int NumNodes = 0;		/* number of nodes */
NPtr Nodes;			/* nodes data */
int NumSectors = 0;		/* number of sectors */
SPtr Sectors;			/* sectors data */
int NumWTexture = 0;		/* number of wall textures */
char **WTexture;		/* array of wall texture names */
int NumFTexture = 0;		/* number of floor/ceiling textures */
char **FTexture;		/* array of texture names */
int MaxX = -32768;		/* maximum X value of map */
int MaxY = -32768;		/* maximum Y value of map */
int MinX = 32767;		/* minimum X value of map */
int MinY = 32767;		/* minimum Y value of map */
int MoveSpeed = 20;		/* movement speed */
int SaveChanges = FALSE;	/* save changes? */
int MadeChanges = FALSE;	/* made changes? */
int MadeMapChanges = FALSE;	/* made changes that need rebuilding? */


/*
   the driving program
*/

void EditLevel( int episode, int level)
{
   WadPtr wad;
   char *outfile = NULL;
   char *dotp;

   ReadWTextureNames();
   ReadFTextureNames();
   InitGfx();
   CheckMouseDriver();
   if (episode < 1 || level < 1)
      SelectLevel( &episode, &level);

   /* TEMPORARY */
   if (episode == 2 && level == 7)
   {
      ClearScreen();
      if (! Confirm( -1, -1, "WARNING: I still have a bug to fix here...  You may get stange results",
			     "if you edit this level.  Do you want to continue anyway?"))
      {
	 TermGfx();
	 ForgetWTextureNames();
	 ForgetFTextureNames();
	 printf( "Editing aborted...   This bug will be fixed in a future release.\n");
	 return;
      }
   }

   if (episode > 0 && level > 0)
   {
      ClearScreen();
      ReadLevelData( episode, level);
      EditorLoop();
      if (SaveChanges && MadeChanges && Registered)
      {
	 /* get the name of the new WAD file */
	 outfile = GetMemory( 80);
	 if (! strcmp(Level->wadfile->filename, "DOOM.WAD"))
	    sprintf( outfile, "E%dL%d.WAD", episode, level);
	 else
	    strcpy( outfile, Level->wadfile->filename);
	 do
	 {
	    InputFileName( -1, -1, "Name of the new WAD file:", 79, outfile);
	 }
	 while (outfile[0] == '\0' || !strcmp(outfile, "DOOM.WAD"));
	 /* if the WAD file already exists, rename it to "*.BAK" */
	 for (wad = WadFileList; wad; wad = wad->next)
	    if (!stricmp( outfile, wad->filename))
	       break;
	 if (wad)
	 {
	    dotp = strrchr( wad->filename, '.');
	    if (dotp == NULL)
	       strcat( wad->filename, ".BAK");
	    else
	       strcpy( dotp, ".BAK");
	    if (rename( outfile, wad->filename) < 0)
	    {
	       if (unlink( wad->filename) < 0)
		  ProgError("cannot delete file \"%s\"\n", wad->filename);
	       if (rename( outfile, wad->filename) < 0)
		  ProgError("cannot rename \"%s\" to \"%s\"\n", outfile, wad->filename);
	    }
	 }
	 /* save the new WAD file */
	 SaveLevelData( outfile);
      }
      TermGfx();
      if (! Registered)
	 printf( "Please register DOOM if you want to be able to save your changes.\n");
      else if (! SaveChanges)
	 printf( "Editing aborted...\n");
      else if (! MadeChanges)
	 printf( "No changes made, nothing to save.\n");
      ForgetLevelData();
   }
   else
      TermGfx();
   ForgetWTextureNames();
   ForgetFTextureNames();
   if (outfile)
      OpenPatchWad( outfile);
   /* this should free the old "*.BAK" file */
   CloseUnusedWadFiles();
}


/*
   select a level
*/

void SelectLevel( int *episode, int *level)
{
   MDirPtr dir;
   char name[ 7];
   char **levels;
   int n = 0;

   dir = MasterDir;
   while (dir)
   {
      if (dir->dir.size == 0 && dir->dir.name[ 0] == 'E' && dir->dir.name[ 2] == 'M' && dir->dir.name[ 4] == '\0')
      {
	 if (n > 0)
	    levels = ResizeMemory( levels, (n + 1) * sizeof( char *));
	 else
	    levels = GetMemory( sizeof( char *));
	 levels[ n] = dir->dir.name;
	 n++;
      }
      dir = dir->next;
   }
   if (*episode < 1)
      *episode = 1;
   if (*level < 1)
      *level = 1;
   sprintf( name, "E%dM%d", *episode, *level);
   InputNameFromList( -1, -1, "Select an episode and a level number:", n, levels, name);
   free( levels);
   if (*name)
      sscanf( name, "E%dM%d", episode, level);
   else
   {
      *episode = 0;
      *level = 0;
   }
}


/*
   read in the level data
*/

void ReadLevelData( int episode, int level)
{
   MDirPtr dir;
   char name[ 7];
   int n, m;
   int val;

   /* find the various level information from the master dictionary */
   sprintf( name, "E%dM%d", episode, level);
   DisplayMessage( -1, -1, "Reading data for level %s...", name);
   Level = FindMasterDir( MasterDir, name);
   if (!Level)
      ProgError( "level data not found");

   /* read in the things data */
   dir = FindMasterDir( Level, "THINGS");
   NumThings = (int) (dir->dir.size / 10L);
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

   /* read in the line def information */
   dir = FindMasterDir( Level, "LINEDEFS");
   NumLineDefs = (int) (dir->dir.size / 14L);
   LineDefs = GetFarMemory( (unsigned long) NumLineDefs * sizeof( struct LineDef));
   BasicWadSeek( dir->wadfile, dir->dir.start);
   for (n = 0; n < NumLineDefs; n++)
   {
      BasicWadRead( dir->wadfile, &(LineDefs[ n].start), 2);
      BasicWadRead( dir->wadfile, &(LineDefs[ n].end), 2);
      BasicWadRead( dir->wadfile, &(LineDefs[ n].flags), 2);
      BasicWadRead( dir->wadfile, &(LineDefs[ n].type), 2);
      BasicWadRead( dir->wadfile, &(LineDefs[ n].tag), 2);
      BasicWadRead( dir->wadfile, &(LineDefs[ n].sidedef1), 2);
      BasicWadRead( dir->wadfile, &(LineDefs[ n].sidedef2), 2);
   }

   /* read in the side def information */
   dir = FindMasterDir( Level, "SIDEDEFS");
   NumSideDefs = (int) (dir->dir.size / 30L);
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

   /* read in the vertexes which are all the corners of the level */
   dir = FindMasterDir( Level, "VERTEXES");
   NumVertexes = (int) (dir->dir.size / 4L);
   Vertexes = GetFarMemory( (unsigned long) NumVertexes * sizeof( struct Vertex));
   BasicWadSeek( dir->wadfile, dir->dir.start);
   for (n = 0; n < NumVertexes; n++)
   {
      BasicWadRead( dir->wadfile, &val, 2);
      if (val < MinX)
	 MinX = val;
      if (val > MaxX)
	 MaxX = val;
      Vertexes[ n].x = val;
      BasicWadRead( dir->wadfile, &val, 2);
      if (val < MinY)
	 MinY = val;
      if (val > MaxY)
	 MaxY = val;
      Vertexes[ n].y = val;
   }

   if (Debug)
   {
      /* read in the segments information */
      dir = FindMasterDir( Level, "SEGS");
      NumSegs = (int) (dir->dir.size / 12L);
      Segs = GetFarMemory( (unsigned long) NumSegs * sizeof( struct Seg));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumSegs; n++)
      {
	 BasicWadRead( dir->wadfile, &(Segs[ n].start), 2);
	 BasicWadRead( dir->wadfile, &(Segs[ n].end), 2);
	 BasicWadRead( dir->wadfile, &(Segs[ n].angle), 2);
	 BasicWadRead( dir->wadfile, &(Segs[ n].linedef), 2);
	 BasicWadRead( dir->wadfile, &(Segs[ n].flip), 2);
	 BasicWadRead( dir->wadfile, &(Segs[ n].dist), 2);
      }

      /* read in the subsectors information */
      dir = FindMasterDir( Level, "SSECTORS");
      NumSSectors = (int) (dir->dir.size / 4L);
      SSectors = GetFarMemory( (unsigned long) NumSSectors * sizeof( struct SSector));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumSSectors; n++)
      {
	 BasicWadRead( dir->wadfile, &(SSectors[ n].num), 2);
	 BasicWadRead( dir->wadfile, &(SSectors[ n].first), 2);
      }

      /* read in the nodes information */
      dir = FindMasterDir( Level, "NODES");
      NumNodes = (int) (dir->dir.size / 28L);
      Nodes = GetFarMemory( (unsigned long) NumNodes * sizeof( struct Node));
      BasicWadSeek( dir->wadfile, dir->dir.start);
      for (n = 0; n < NumNodes; n++)
      {
	 BasicWadRead( dir->wadfile, &(Nodes[ n].x), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].y), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].dx), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].dy), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].miny1), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].maxy1), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].minx1), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].maxx1), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].miny2), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].maxy2), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].minx2), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].maxx2), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].tree1), 2);
	 BasicWadRead( dir->wadfile, &(Nodes[ n].tree2), 2);
      }
   }
   else
   {
      Segs = NULL;
      NumSegs = 0;
      SSectors = NULL;
      NumSSectors = 0;
      Nodes = NULL;
      NumNodes = 0;
   }

   /* read in the sectors information */
   dir = FindMasterDir( Level, "SECTORS");
   NumSectors = (int) (dir->dir.size / 26L);
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
   if (Things)
      farfree( Things);

   /* forget the line defs */
   NumLineDefs = 0;
   if (LineDefs)
      farfree( LineDefs);

   /* forget the side defs */
   NumSideDefs = 0;
   if (SideDefs)
      farfree( SideDefs);

   /* forget the vertices */
   NumVertexes = 0;
   if (Vertexes)
      farfree( Vertexes);
   MaxX = -32768;
   MaxY = -32768;
   MinX = 32767;
   MinY = 32767;

   /* forget the segments */
   NumSegs = 0;
   if (Segs)
      farfree( Segs);

   /* forget the subsectors */
   NumSSectors = 0;
   if (SSectors)
      farfree( SSectors);

   /* forget the nodes */
   NumNodes = 0;
   if (Nodes)
      farfree( Nodes);

   /* forget the sectors */
   NumSectors = 0;
   if (Sectors)
      farfree( Sectors);
}



/*
   save the level data to a PWAD file
*/

void SaveLevelData( char *outfile)
{
   FILE *file;
   MDirPtr dir;
   long counter = 11;
   int n, i, j;
   void *data;
   long size;
   long dirstart;
   int *blockptr;
   long blocksize;
   int blockcount;
   long oldpos;

   DisplayMessage( -1, -1, "Saving data to \"%s\"...", outfile);
   /* open the file */
   if ((file = fopen( outfile, "wb")) == NULL)
      ProgError( "Unable to open file \"%s\"", outfile);
   WriteBytes( file, "PWAD", 4L);     /* PWAD file */
   WriteBytes( file, &counter, 4L);   /* 11 entries */
   WriteBytes( file, &counter, 4L);   /* fix this up later */
   counter = 12L;
   dir = Level->next;

   /* output the things data */
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

   /* output the linedefs */
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

   /* output the sidedefs */
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

   /* output the vertexes */
   for (n = 0; n < NumVertexes; n++)
   {
      WriteBytes( file, &(Vertexes[ n].x), 2L);
      WriteBytes( file, &(Vertexes[ n].y), 2L);
      counter += 4L;
   }
   dir = dir->next;

   if (Debug)
   {
      /* output the segs */
      for (n = 0; n < NumSegs; n++)
      {
	 WriteBytes( file, &(Segs[ n].start), 2L);
	 WriteBytes( file, &(Segs[ n].end), 2L);
	 WriteBytes( file, &(Segs[ n].angle), 2L);
	 WriteBytes( file, &(Segs[ n].linedef), 2L);
	 WriteBytes( file, &(Segs[ n].flip), 2L);
	 WriteBytes( file, &(Segs[ n].dist), 2L);
	 counter += 12L;
      }
      dir = dir->next;

      /* output the ssectors */
      for (n = 0; n < NumSSectors; n++)
      {
	 WriteBytes( file, &(SSectors[ n].num), 2L);
	 WriteBytes( file, &(SSectors[ n].first), 2L);
	 counter += 4L;
      }
      dir = dir->next;

      /* output the nodes */
      for (n = 0; n < NumNodes; n++)
      {
	 WriteBytes( file, &(Nodes[ n].x), 2L);
	 WriteBytes( file, &(Nodes[ n].y), 2L);
	 WriteBytes( file, &(Nodes[ n].dx), 2L);
	 WriteBytes( file, &(Nodes[ n].dy), 2L);
	 WriteBytes( file, &(Nodes[ n].miny1), 2L);
	 WriteBytes( file, &(Nodes[ n].maxy1), 2L);
	 WriteBytes( file, &(Nodes[ n].minx1), 2L);
	 WriteBytes( file, &(Nodes[ n].maxx1), 2L);
	 WriteBytes( file, &(Nodes[ n].miny2), 2L);
	 WriteBytes( file, &(Nodes[ n].maxy2), 2L);
	 WriteBytes( file, &(Nodes[ n].minx2), 2L);
	 WriteBytes( file, &(Nodes[ n].maxx2), 2L);
	 WriteBytes( file, &(Nodes[ n].tree1), 2L);
	 WriteBytes( file, &(Nodes[ n].tree2), 2L);
	 counter += 28L;
      }
      dir = dir->next;
   }
   else
   {
      data = GetMemory( 0x8000 + 2);
      for (n = 0; n < 3; n++)
      {
	 size = dir->dir.size;
	 BasicWadSeek( dir->wadfile, dir->dir.start);
	 while (size > 0x8000)
	 {
	    BasicWadRead( dir->wadfile, data, 0x8000);
	    WriteBytes( file, data, 0x8000);
	    size -= 0x8000;
	 }
	 BasicWadRead( dir->wadfile, data, size);
	 WriteBytes( file, data, size);
	 counter += size;
	 dir = dir->next;
      }
      free( data);
   }

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
      counter += 26L;
   }
   dir = dir->next;

   /* copy the reject data */
   data = GetMemory( 0x8000 + 2);
   size = dir->dir.size;
   BasicWadSeek( dir->wadfile, dir->dir.start);
   while (size > 0x8000)
   {
      BasicWadRead( dir->wadfile, data, 0x8000);
      WriteBytes( file, data, 0x8000);
      size -= 0x8000;
   }
   BasicWadRead( dir->wadfile, data, size);
   WriteBytes( file, data, size);
   counter += size;
   dir = dir->next;
   free( data);

   if (MadeMapChanges && (Expert || Confirm( -1, 270, "Do you want to rebuild the BLOCKMAP?", NULL)))
   {
      /* create and output the blockmap */
      DisplayMessage( -1, -1, "Rebuilding the BLOCKMAP.  Please wait a few seconds...");
      MinX = (int) (MinX / 8) * 8;
      WriteBytes( file, &MinX, 2L);
      MinY = (int) (MinY / 8) * 8;
      WriteBytes( file, &MinY, 2L);
      MaxX = MaxX / 128 - MinX / 128 + 1;
      WriteBytes( file, &MaxX, 2L);
      MaxY = MaxY / 128 - MinY / 128 + 1;
      WriteBytes( file, &MaxY, 2L);
      counter += 8L;
      oldpos = ftell( file);
      blocksize = (long) (MaxX * MaxY * sizeof( int));
      blockptr = GetMemory( blocksize);
      WriteBytes( file, blockptr, blocksize);
      blocksize += 8L;
      counter += blocksize - 7L;
      blockcount = MaxX * MaxY + 4;
      for (i = 0; i < MaxY; i++)
	 for (j = 0; j < MaxX; j++)
	    {
	       blockptr[ MaxX * i + j] = blockcount;
	       n = 0;
	       WriteBytes( file, &n, 2L);
	       counter += 2L;
	       blocksize += 2L;
	       blockcount++;
	       for (n = 0; n < NumLineDefs; n++)
		  if (IsLineDefInside( n, MinX + j * 128, MinY + i * 128, MinX + 127 + j * 128, MinY + 127 + i * 128))
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
      size = ftell( file);
      fseek( file, oldpos, SEEK_SET);
      WriteBytes( file, blockptr, (long) (MaxX * MaxY * sizeof( int)));
      fseek( file, size, SEEK_SET);
      if (FindMasterDir( dir, "P2_END"))
	 counter--;
      free( blockptr);
   }
   else
   {
      /* copy the blockmap data */
      blocksize = dir->dir.size;
      data = GetMemory( 0x8000 + 2);
      size = blocksize;
      BasicWadSeek( dir->wadfile, dir->dir.start);
      while (size > 0x8000)
      {
	 BasicWadRead( dir->wadfile, data, 0x8000);
	 WriteBytes( file, data, 0x8000);
	 size -= 0x8000;
      }
      BasicWadRead( dir->wadfile, data, size);
      WriteBytes( file, data, size);
      counter += size;
      dir = dir->next;
      free( data);
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

   size = (long) NumVertexes * 4L;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "VERTEXES", 8L);
   counter += size;
   dir = dir->next;

   if (Debug)
   {
      size = (long) NumSegs * 12L;
      WriteBytes( file, &counter, 4L);
      WriteBytes( file, &size, 4L);
      WriteBytes( file, "SEGS\0\0\0\0", 8L);
      counter += size;
      dir = dir->next;

      size = (long) NumSSectors * 4L;
      WriteBytes( file, &counter, 4L);
      WriteBytes( file, &size, 4L);
      WriteBytes( file, "SSECTORS", 8L);
      counter += size;
      dir = dir->next;

      size = (long) NumNodes * 28L;
      WriteBytes( file, &counter, 4L);
      WriteBytes( file, &size, 4L);
      WriteBytes( file, "NODES\0\0\0", 8L);
      counter += size;
      dir = dir->next;
   }
   else
   {
      for (n = 0; n < 3; n++)
      {
	 size = dir->dir.size;
	 WriteBytes( file, &counter, 4L);
	 WriteBytes( file, &size, 4L);
	 WriteBytes( file, &(dir->dir.name), 8L);
	 counter += size;
	 dir = dir->next;
      }
   }


   size = (long) NumSectors * 26L;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "SECTORS\0", 8L);
   counter += size;
   dir = dir->next;

   size = dir->dir.size;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, &(dir->dir.name), 8L);
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
   NumWTexture = val + 1;
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
   free( offsets);
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
      free( offsets);
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
      free( WTexture[ n]);

   /* forget the array */
   NumWTexture = 0;
   free( WTexture);
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
      free( FTexture[ n]);

   /* forget the array */
   NumFTexture = 0;
   free( FTexture);
}



/*
   display the help screen
 */

void DisplayHelp( int objtype, int grid)
{
   int x0 = 137;
   int y0 = 50;

   if (UseMouse)
      HideMousePointer();
   /* put in the instructions */
   DrawScreenBox3D( x0, y0, x0 + 364, y0 + 355);
   setcolor( LIGHTCYAN);
   DrawScreenText( x0 + 100, y0 + 20, "Doom Editor Utility");
   DrawScreenText( 269 - strlen(GetEditModeName( objtype)) * 4, y0 + 32, "- %s Editor -", GetEditModeName( objtype));
   setcolor( BLACK);
   DrawScreenText( x0 + 10, y0 + 60, "Use the mouse or the cursor keys to move");
   DrawScreenText( -1, -1, "around.  The map scrolls when the pointer");
   DrawScreenText( -1, -1, "reaches the edge of the screen.");
   DrawScreenText( -1, y0 + 100, "Other useful keys are:");
   if (Registered)
      DrawScreenText( -1, y0 + 115, "Q     - Quit, saving changes");
   else
   {
      setcolor( DARKGRAY);
      DrawScreenText( -1, y0 + 115, "Q     - Quit without saving changes");
      setcolor( BLACK);
   }
   DrawScreenText( -1, -1, "Esc   - Exit without saving changes");
   DrawScreenText( -1, -1, "Tab   - Switch to the next editing mode");
   DrawScreenText( -1, -1, "Space - Change the move/scroll speed");
   DrawScreenText( -1, -1, "+/-   - Change the map scale (current: %d)", Scale);
   DrawScreenText( -1, -1, "G     - Change the grid scale (cur.: %d)", grid);
   if (GetCurObject( objtype) >= 0)
      setcolor( DARKGRAY);
   DrawScreenText( -1, -1, "N, >  - Jump to the next object.");
   DrawScreenText( -1, -1, "P, <  - Jump to the previous object.");
   DrawScreenText( -1, -1, "J, #  - Jump to a specific object (enter #)");
   setcolor( BLACK);
   DrawScreenText( -1, -1, "M     - Mark/unmark current object (select)");
   if (objtype == OBJ_THINGS || objtype == OBJ_VERTEXES)
      DrawScreenText( -1, -1, "D     - Toggle drag mode");
   else
      DrawScreenText( -1, -1, "C     - Clear all marks and redraw map");
   DrawScreenText( -1, -1, "Ins   - Insert a new object");
   DrawScreenText( -1, -1, "Del   - Delete the current object");
   DrawScreenText( -1, -1, "Enter - Edit the current/selected object(s)");
   DrawScreenText( -1, y0 + 265, "Mouse buttons:");
   if (SwapButtons)
   {
      DrawScreenText( -1, y0 + 280, "Left  - Edit the current/selected object(s)");
      DrawScreenText( -1, -1, "Middle- Mark/unmark the current object.");
   }
   else
   {
      DrawScreenText( -1, y0 + 280, "Left  - Mark/unmark the current object");
      DrawScreenText( -1, -1, "Middle- Edit the current/selected object(s)");
   }
   DrawScreenText( -1, -1, "Right - Drag the current/selected object(s)");
   DrawScreenText( -1, y0 + 320, "Please read MANUAL.DOC for more information");
   setcolor( YELLOW);
   DrawScreenText( -1, y0 + 340, "Press any key to return to the editor...");
   bioskey( 0);
   if (UseMouse)
      ShowMousePointer();
}



/*
   the editor main loop
*/

void EditorLoop()
{
   int    EditMode = OBJ_THINGS;
   int    CurObject = -1;
   int    OldObject = -1;
   int    RedrawMap = TRUE;
   int    RedrawObj = FALSE;
   int    DragObject = FALSE;
   int    key, buttons, oldbuttons;
   int    GridScale = 0;
   SelPtr Selected = NULL;

   OrigX = (MinX + MaxX) / 2;
   OrigY = (MinY + MaxY) / 2;
   Scale = 10;
   SaveChanges = FALSE;
   MadeChanges = FALSE;
   MadeMapChanges = FALSE;
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
	       if (SwapButtons)
		  key = 0x000D;
	       else
		  key = 'M'; /* Press left button = Mark/Unmark ('M') */
	       break;
	    case 2:
	       DragObject = TRUE; /* Press right button = Drag */
	       break;
	    case 3:
	    case 4:
	       if (SwapButtons)
		  key = 'M';
	       else
		  key = 0x000D;	/* Press middle button = Edit ('Enter') */
	       break;
	    default:
	       DragObject = FALSE; /* Release right button = End Drag */
	       break;
	    }
	 oldbuttons = buttons;
      }

      /* drag object(s) */
      if (DragObject)
      {
	 int forgetit = FALSE;

	 if (IsSelected( Selected, CurObject) == FALSE)
	    ForgetSelection( &Selected);
	 else if (Selected->objnum != CurObject)
	 {
	    UnSelectObject( &Selected, CurObject);
	    SelectObject( &Selected, CurObject);
	 }
	 if (Selected == NULL && CurObject >= 0)
	 {
	    SelectObject( &Selected, CurObject);
	    forgetit = TRUE;
	 }
	 if (Selected)
	 {
	    if (GridScale > 0)
	       MoveObjectToCoords( EditMode, Selected,
				   (MAPX( PointerX) / GridScale) * GridScale,
				   (MAPY( PointerY) / GridScale) * GridScale);
	    else
	       MoveObjectToCoords( EditMode, Selected, MAPX( PointerX), MAPY( PointerY));
	    if (forgetit)
	       ForgetSelection( &Selected);
	    RedrawMap = TRUE;
	 }
	 else
	 {
	    Beep();
	    DragObject = FALSE;
	 }
      }
      else if (!RedrawObj)
      {
	 /* check if there is something near the pointer */
	 OldObject = CurObject;
	 if ((bioskey( 2) & 0x03) == 0x00)  /* no shift keys */
	    CurObject = GetCurObject( EditMode);
	 if (CurObject < 0)
	    CurObject = OldObject;
      }

      /* draw the map */
      if (RedrawMap)
      {
	 if (UseMouse)
	    HideMousePointer();
	 DrawMap( EditMode, GridScale);
	 HighlightSelection( EditMode, Selected);
	 if (UseMouse)
	    ShowMousePointer();
      }

      /* highlight the current object and display the information box */
      if (RedrawMap || CurObject != OldObject || RedrawObj)
      {
	 RedrawObj = FALSE;
	 if (UseMouse)
	    HideMousePointer();
	 if (!RedrawMap && OldObject >= 0)
	    HighlightObject( EditMode, OldObject, YELLOW);
	 if (CurObject != OldObject)
	 {
	    if (! Quiet)
	    {
	       sound( 50);
	       delay( 10);
	       nosound();
	    }
	    OldObject = CurObject;
	 }
	 if (CurObject >= 0)
	    HighlightObject( EditMode, CurObject, YELLOW);
	 if (bioskey( 1)) /* speedup */
	    RedrawObj = TRUE;
	 else
	    DisplayObjectInfo( EditMode, CurObject);
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
	    ForgetSelection( &Selected);
	    SaveChanges = Registered;
	    break;
	 }
	 else if ((key & 0x00FF) == 0x001B)
	 {
	    ForgetSelection( &Selected);
	    if (!MadeChanges || Confirm(90, 230, "You have unsaved changes.  Do you really want to quit?", NULL))
	       break;
	    RedrawMap = TRUE;
	 }

	 /* user is lost */
	 else if ((key & 0xFF00) == 0x3B00)
	 {
	    DisplayHelp( EditMode, GridScale);
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the scale */
	 else if (((key & 0x00FF) == '+' || (key & 0x00FF) == '=') && Scale > 1)
	 {
	    OrigX += (PointerX - 319) * Scale;
	    OrigY += (239 - PointerY) * Scale;
	    Scale--;
	    OrigX -= (PointerX - 319) * Scale;
	    OrigY -= (239 - PointerY) * Scale;
	    RedrawMap = TRUE;
	 }
	 else if (((key & 0x00FF) == '-' || (key & 0x00FF) == '_') && Scale < 20)
	 {
	    OrigX += (PointerX - 319) * Scale;
	    OrigY += (239 - PointerY) * Scale;
	    Scale++;
	    OrigX -= (PointerX - 319) * Scale;
	    OrigY -= (239 - PointerY) * Scale;
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

	 /* user wants so scroll the map */
	 else if ((key & 0xFF00) == 0x4900 && MAPY(239) < MaxY)
	 {
	    OrigY += MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x5100 && MAPY(239) > MinY)
	 {
	    OrigY -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x4700 && MAPX(319) > MinX)
	 {
	    OrigX -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x4F00 && MAPX(319) < MaxX)
	 {
	    OrigX += MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the movement speed */
	 else if ((key & 0x00FF) == ' ')
	    MoveSpeed = MoveSpeed == 1 ? 20 : 1;

	 /* user wants to change the edit mode */
	 else if ((key & 0x00FF) == 0x0009 || (key & 0xFF00) == 0x0F00
	       || (key & 0x00FF) == 'T' || (key & 0x00FF) == 't'
	       || (key & 0x00FF) == 'V' || (key & 0x00FF) == 'v'
	       || (key & 0x00FF) == 'L' || (key & 0x00FF) == 'l'
	       || (key & 0x00FF) == 'S' || (key & 0x00FF) == 's')
	 {
	    if ((key & 0x00FF) == 0x0009)
	    {
	       switch (EditMode)
	       {
	       case OBJ_THINGS:
		  EditMode = OBJ_VERTEXES;
		  break;
	       case OBJ_VERTEXES:
		  EditMode = OBJ_LINEDEFS;
		  break;
	       case OBJ_LINEDEFS:
		  EditMode = OBJ_SECTORS;
		  break;
	       case OBJ_SECTORS:
		  if (Debug)
		     EditMode = OBJ_SEGS;
		  else
		     EditMode = OBJ_THINGS;
		  break;
	       case OBJ_SEGS:
		  EditMode = OBJ_NODES;
		  break;
	       case OBJ_NODES:
		  EditMode = OBJ_THINGS;
		  break;
	       }
	    }
	    else if ((key & 0xFF00) == 0x0F00)
	    {
	       switch (EditMode)
	       {
	       case OBJ_THINGS:
		  if (Debug)
		     EditMode = OBJ_NODES;
		  else
		     EditMode = OBJ_SECTORS;
		  break;
	       case OBJ_VERTEXES:
		  EditMode = OBJ_THINGS;
		  break;
	       case OBJ_LINEDEFS:
		  EditMode = OBJ_VERTEXES;
		  break;
	       case OBJ_SECTORS:
		  EditMode = OBJ_LINEDEFS;
		  break;
	       case OBJ_SEGS:
		  EditMode = OBJ_SECTORS;
		  break;
	       case OBJ_NODES:
		  EditMode = OBJ_SEGS;
		  break;
	       }
	    }
	    else if ((key & 0x00FF) == 'T' || (key & 0x00FF) == 't')
	       EditMode = OBJ_THINGS;
	    else if ((key & 0x00FF) == 'V' || (key & 0x00FF) == 'v')
	       EditMode = OBJ_VERTEXES;
	    else if ((key & 0x00FF) == 'L' || (key & 0x00FF) == 'l')
	       EditMode = OBJ_LINEDEFS;
	    else if ((key & 0x00FF) == 'S' || (key & 0x00FF) == 's')
	       EditMode = OBJ_SECTORS;
	    /* unselect all */
	    ForgetSelection( &Selected);
	    if (GetMaxObjectNum( EditMode) >= 0)
	       CurObject = 0;
	    else
	       CurObject = -1;
	    OldObject = -1;
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to display or hide the grid */
	 else if ((key & 0x00FF) == 'G' || (key & 0x00FF) == 'g')
	 {
	    if (GridScale == 0)
	       GridScale = 256;
	    else if (GridScale > 16)
	       GridScale /= 2;
	    else
	       GridScale = 0;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0x00FF) == 'H' || (key & 0x00FF) == 'h')
	 {
	    GridScale = 0;
	    RedrawMap = TRUE;
	 }

	 /* user wants to toggle drag mode */
	 else if ((key & 0x00FF) == 'D' || (key & 0x00FF) == 'd')
	    DragObject = !DragObject;

	 /* user wants to select the next or previous object */
	 else if (((key & 0x00FF) == 'N' || (key & 0x00FF) == 'n' || (key & 0x00FF) == '>') && GetCurObject( EditMode) < 0)
	 {
	    if (CurObject < GetMaxObjectNum( EditMode))
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
	       CurObject = GetMaxObjectNum( EditMode);
	    RedrawObj = TRUE;
	 }
	 else if (((key & 0x00FF) == 'J' || (key & 0x00FF) == 'j' || (key & 0x00FF) == '#') && GetCurObject( EditMode) < 0)
	 {
	    OldObject = InputObjectNumber( 140, 200, EditMode, CurObject);
	    if (OldObject >= 0)
	       CurObject = OldObject;
	    else
	       OldObject = CurObject;
	    RedrawMap = TRUE;
	 }

	 /* user wants to disable the mouse (DEBUG) */
	 else if (Debug && (key & 0x00FF) == 'K')
	 {
	    if (UseMouse)
	       HideMousePointer();
	    UseMouse = FALSE;
	 }

	 /* user wants to mark/unmark an object */
	 else if (((key & 0x00FF) == 'M' || (key & 0x00FF) == 'm') && CurObject >= 0)
	 {
	    if (UseMouse)
	       HideMousePointer();
	    if (IsSelected( Selected, CurObject))
	       UnSelectObject( &Selected, CurObject);
	    else
	       SelectObject( &Selected, CurObject);
	    HighlightObject( EditMode, CurObject, GREEN);
	    if (UseMouse)
	       ShowMousePointer();
	    if (! Quiet)
	    {
	       sound( 440);
	       delay( 10);
	       nosound();
	    }
	    DragObject = FALSE;
	 }

	 /* user wants to clear all marks and redraw the map */
	 else if ((key & 0x00FF) == 'C' || (key & 0x00FF) == 'c')
	 {
	    ForgetSelection( &Selected);
	    RedrawMap = TRUE;
	    DragObject = FALSE;
	 }

	 /* user wants to edit the current object */
	 else if ((key & 0x00FF) == 0x000D && CurObject >= 0)
	 {
	    if (Selected)
	       EditObjectInfo( EditMode, Selected);
	    else
	    {
	       SelectObject( &Selected, CurObject);
	       EditObjectInfo( EditMode, Selected);
	       UnSelectObject( &Selected, CurObject);
	    }
	    RedrawMap = TRUE;
	    DragObject = FALSE;
	 }

	 /* user wants to delete the current object */
	 else if ((key & 0xFF00) == 0x5300 && CurObject >= 0)
	 {
	    ForgetSelection( &Selected); /* (*should be changed!*) */
	    if (EditMode == OBJ_LINEDEFS || EditMode == OBJ_VERTEXES || EditMode == OBJ_SECTORS)
	    {
	       if (Expert || Confirm( -1, -1, "Do you really want to delete this object?", "This will also delete the objects bound to it."))
		  DeleteObject( EditMode, CurObject);
	    }
	    else
	       DeleteObject( EditMode, CurObject);
	    CurObject = -1;
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to insert a new object */
	 else if ((key & 0xFF00) == 0x5200)
	 {
	    SelPtr cur;

	    /* first special case: if several Vertices are selected, add new LineDefs */
	    if (EditMode == OBJ_VERTEXES && Selected != NULL && Selected->next != NULL)
	    {
	       EditMode = OBJ_LINEDEFS;
	       for (cur = Selected; cur->next; cur = cur->next)
	       {
		  InsertObject( EditMode, -1, GridScale);
		  CurObject = GetMaxObjectNum( EditMode);
		  LineDefs[ CurObject].start = cur->next->objnum;
		  LineDefs[ CurObject].end = cur->objnum;
		  cur->objnum = CurObject;
	       }
	       UnSelectObject( &Selected, cur->objnum);
	    }
	    /* second special case: if several LineDefs are selected, add new SideDefs and one Sector */
	    else if (EditMode == OBJ_LINEDEFS && Selected != NULL)
	    {
	       for (cur = Selected; cur; cur = cur->next)
		  if (LineDefs[ cur->objnum].sidedef1 >= 0 && LineDefs[ cur->objnum].sidedef2 >= 0)
		  {
		     Beep();
		     break;
		  }
	       if (cur == NULL)
	       {
		  EditMode = OBJ_SECTORS;
		  InsertObject( EditMode, -1, GridScale);
		  CurObject = GetMaxObjectNum( EditMode);
		  for (cur = Selected; cur; cur = cur->next)
		  {
		     InsertObject( OBJ_SIDEDEFS, -1, GridScale);
		     SideDefs[ NumSideDefs - 1].sector = CurObject;
		     if (LineDefs[ cur->objnum].sidedef1 >= 0)
			LineDefs[ cur->objnum].sidedef2 = NumSideDefs - 1;
		     else
			LineDefs[ cur->objnum].sidedef1 = NumSideDefs - 1;
		  }
		  ForgetSelection( &Selected);
	       }
	    }
	    /* normal case: add a new object of the current type */
	    else
	    {
	       ForgetSelection( &Selected);
	       InsertObject( EditMode, CurObject, GridScale);
	       CurObject = GetMaxObjectNum( EditMode);
	       if (EditMode == OBJ_LINEDEFS)
	       {
		  if (! Input2VertexNumbers( -1, -1, "Choose the two vertices for the new LineDef",
					     &(LineDefs[ CurObject].start), &(LineDefs[ CurObject].end)))
		  {
		     DeleteObject( EditMode, CurObject);
		     CurObject = -1;
		  }
	       }
	    }
	    DragObject = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user likes music */
	 else
	    Beep();

	 /* redraw the (keyboard) pointer */
	 if (! UseMouse)
	    DrawPointer();
      }

      /* move the map if the pointer is near the edge of the screen */
      if (PointerY <= 20)
      {
	 if (! UseMouse)
	    PointerY += MoveSpeed;
	 if (MAPY(239) < MaxY)
	 {
	    OrigY += MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
      }
      if (PointerY >= 469)
      {
	 if (! UseMouse)
	    PointerY -= MoveSpeed;
	 if (MAPY(239) > MinY)
	 {
	    OrigY -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
      }
      if (PointerX <= 20)
      {
	 if (! UseMouse)
	    PointerX += MoveSpeed;
	 if (MAPX(319) > MinX)
	 {
	    OrigX -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
      }
      if (PointerX >= 619)
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

void DrawMap( int editmode, int grid)
{
   int  n, m;
   char texname[9];

   /* clear the screen */
   ClearScreen();

   /* draw the grid */
   if (grid > 0)
   {
      setcolor( BLUE);
      for (n = MinX; n <= MaxX; n += grid)
	 DrawMapLine( n, MinY, n, MaxY);
      for (n = MinY; n <= MaxY; n += grid)
	 DrawMapLine( MinX, n, MaxX, n);
   }

   /* draw the linedefs to form the map */
   if (editmode == OBJ_VERTEXES || editmode == OBJ_NODES)
   {
      setcolor( LIGHTGRAY);
      for (n = 0; n < NumLineDefs; n++)
	 DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
   }
   else if (editmode == OBJ_THINGS)
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].flags & 1)
	    setcolor( WHITE);
	 else
	    setcolor( LIGHTGRAY);
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }
   else if (editmode == OBJ_LINEDEFS)
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].type > 0)
	 {
	    if (LineDefs[ n].tag > 0)
	       setcolor( LIGHTMAGENTA);
	    else
	       setcolor( LIGHTGREEN);
	 }
	 else if (LineDefs[ n].tag > 0)
	    setcolor( LIGHTRED);
	 else if (LineDefs[ n].flags & 1)
	    setcolor( WHITE);
	 else
	    setcolor( LIGHTGRAY);
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }
   else if (editmode == OBJ_SECTORS)
      for (n = 0; n < NumLineDefs; n++)
      {
	 if ((m = LineDefs[ n].sidedef1) < 0 || (m = SideDefs[ m].sector) < 0)
	    setcolor( LIGHTRED);
	 else
	 {
	    if (Sectors[ m].tag > 0)
	       setcolor( LIGHTGREEN);
	    else if (Sectors[ m].special > 0)
	       setcolor( LIGHTCYAN);
	    else if (LineDefs[ n].flags & 1)
	       setcolor( WHITE);
	    else
	       setcolor( LIGHTGRAY);
	    if ((m = LineDefs[ n].sidedef2) >= 0)
	    {
	       if ((m = SideDefs[ m].sector) < 0)
		  setcolor( LIGHTRED);
	       else if (Sectors[ m].tag > 0)
		  setcolor( LIGHTGREEN);
	       else if (Sectors[ m].special > 0)
		  setcolor( LIGHTCYAN);
	    }
	 }
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }

   /* draw the segs (DEBUG) */
   if (editmode == OBJ_SEGS)
   {
      for (n = 0; n < NumSegs; n++)
      {
	 setcolor( LIGHTGREEN + (Segs[ n].flip & 1));
	 DrawMapLine( Vertexes[ Segs[ n].start].x, Vertexes[ Segs[ n].start].y,
		      Vertexes[ Segs[ n].end].x, Vertexes[ Segs[ n].end].y);
	 /* (*TEST*)
	 setcolor( LIGHTRED + (Segs[ n].flip & 1));
	 DrawMapArrow( Vertexes[ Segs[ n].start].x, Vertexes[ Segs[ n].start].y,
		       Segs[ n].angle);
	 */
      }
   }

   /* draw in the vertices */
   if (editmode == OBJ_VERTEXES || editmode == OBJ_SEGS)
   {
      setcolor( LIGHTGREEN);
      for (n = 0; n < NumVertexes; n++)
      {
	 DrawMapLine( Vertexes[ n].x - OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x + OBJSIZE, Vertexes[ n].y + OBJSIZE);
	 DrawMapLine( Vertexes[ n].x + OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x - OBJSIZE, Vertexes[ n].y + OBJSIZE);
      }
   }
   if (Debug && editmode == OBJ_LINEDEFS)
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

   /* draw in the nodes */
   if (editmode == OBJ_NODES)
   {
      for (n = 0; n < NumNodes; n++)
      {
/*
	 setcolor( LIGHTRED);
	 DrawMapLine( Nodes[ n].x, Nodes[ n].y, Nodes[ n].x + Nodes[ n].dx, Nodes[ n].y + Nodes[n ].dy);
*/
	 setcolor( LIGHTGREEN);
	 DrawMapLine( Nodes[ n].x - OBJSIZE, Nodes[ n].y - OBJSIZE, Nodes[ n].x + OBJSIZE, Nodes[ n].y + OBJSIZE);
	 DrawMapLine( Nodes[ n].x + OBJSIZE, Nodes[ n].y - OBJSIZE, Nodes[ n].x - OBJSIZE, Nodes[ n].y + OBJSIZE);
      }
   }

   /* draw in the title bar */
   DrawScreenBox3D( 0, 0, 639, 12);
   setcolor( WHITE);
   DrawScreenText( 5, 3, "Editing %s on %s", GetEditModeName( editmode), Level->dir.name);
   DrawScreenText( 320, 3, "Free mem: %lu", farcoreleft());
   DrawScreenText( 499, 3, "Press F1 for Help");
}



/*
   highlight the selected objects
*/

void HighlightSelection( int objtype, SelPtr list)
{
   SelPtr cur;

   if (list == NULL)
      return;
   for (cur = list; cur; cur = cur->next)
      HighlightObject( objtype, cur->objnum, GREEN);
}



/*
   test if an object is in the selection list
*/

int IsSelected( SelPtr list, int objnum)
{
   SelPtr cur;

   for (cur = list; cur; cur = cur->next)
      if (cur->objnum == objnum)
	 return TRUE;
   return FALSE;
}



/*
   add an object to the selection list
*/

void SelectObject( SelPtr *list, int objnum)
{
   SelPtr cur;


   if (objnum < 0) /* TEMPORARY */
   {
      Beep();
      printf(" Error: SelectObject < 0\n");
      sleep( 3);
      Beep();
   }
   cur = GetMemory( sizeof( struct SelectionList));
   cur->next = *list;
   cur->objnum = objnum;
   *list = cur;
}



/*
   remove an object from the selection list
*/

void UnSelectObject( SelPtr *list, int objnum)
{
   SelPtr cur, prev;

   if (objnum < 0) /* TEMPORARY */
   {
      Beep();
      printf(" Error: UnSelectObject < 0\n");
      sleep( 3);
      Beep();
   }
   prev = NULL;
   cur = *list;
   while (cur)
   {
      if (cur->objnum == objnum)
      {
	 if (prev)
	    prev->next = cur->next;
	 else
	    *list = cur->next;
	 free( cur);
	 if (prev)
	    cur = prev->next;
	 else
	    cur = NULL;
      }
      else
      {
	 prev = cur;
	 cur = cur->next;
      }
   }
}



/*
   forget the selection list
*/

void ForgetSelection( SelPtr *list)
{
   SelPtr cur, prev;

   cur = *list;
   while (cur)
   {
      prev = cur;
      cur = cur->next;
      free( prev);
   }
   *list = NULL;
}



/* end of file */
