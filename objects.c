/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   OBJECTS.C - object handling routines.
*/

/* the includes */
#include "deu.h"
#include "wstructs.h"
#include "things.h"

/* the external variables from edit.c */
extern int NumThings;		/* number of things */
extern TPtr Things;		/* things data */
extern int NumLineDefs;		/* number of line defs */
extern LDPtr LineDefs;		/* line defs data */
extern int NumSideDefs;		/* number of side defs */
extern SDPtr SideDefs;		/* side defs data */
extern int NumVertexes;		/* number of vertexes */
extern VPtr Vertexes;		/* vertex data */
extern int NumSegs;		/* number of segments */
extern SEPtr Segs;		/* list of segments */
extern SEPtr LastSeg;		/* last segment in the list */
extern int NumSSectors;		/* number of subsectors */
extern SSPtr SSectors;		/* list of subsectors */
extern SSPtr LastSSector;	/* last subsector in the list */
extern int NumSectors;		/* number of sectors */
extern SPtr Sectors;		/* sectors data */
extern int MaxX;		/* maximum X value of map */
extern int MaxY;		/* maximum Y value of map */
extern int MinX;		/* minimum X value of map */
extern int MinY;		/* minimum Y value of map */
extern Bool MadeChanges;	/* made changes? */
extern Bool MadeMapChanges;	/* made changes that need rebuilding? */

/*
   get the number of objets of a given type minus one
*/
int GetMaxObjectNum( int objtype)
{
   switch (objtype)
   {
   case OBJ_THINGS:
      return NumThings - 1;
   case OBJ_LINEDEFS:
      return NumLineDefs - 1;
   case OBJ_SIDEDEFS:
      return NumSideDefs - 1;
   case OBJ_VERTEXES:
      return NumVertexes - 1;
   case OBJ_SEGS:
      return NumSegs - 1;
   case OBJ_SSECTORS:
      return NumSSectors - 1;
   case OBJ_SECTORS:
      return NumSectors - 1;
   }
   return -1;
}


/*
   check if there is something of interest inside the given box
*/

int GetCurObject( int objtype, int x0, int y0, int x1, int y1)
{
   int n, m, cur, curx;
   int lx0, ly0, lx1, ly1;

   cur = -1;

   switch (objtype)
   {
   case OBJ_THINGS:
      for (n = 0; n < NumThings; n++)
	 if (Things[ n].xpos >= x0 && Things[ n].xpos <= x1 && Things[ n].ypos >= y0 && Things[ n].ypos <= y1)
	 {
	    cur = n;
	    break;
	 }
      break;
   case OBJ_LINEDEFS:
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (IsLineDefInside( n, x0, y0, x1, y1))
	 {
	    cur = n;
	    break;
	 }
      }
      break;
   case OBJ_VERTEXES:
      for (n = 0; n < NumVertexes; n++)
	 if (Vertexes[ n].x >= x0 && Vertexes[ n].x <= x1 && Vertexes[ n].y >= y0 && Vertexes[ n].y <= y1)
	 {
	    cur = n;
	    break;
	 }
      break;
   case OBJ_SECTORS:
      /* hack, hack...  I look for the first LineDef crossing an horizontal half-line drawn from the cursor */
      curx = MaxX + 1;
      cur = -1;
      for (n = 0; n < NumLineDefs; n++)
	 if ((Vertexes[ LineDefs[ n].start].y > MAPY( PointerY)) != (Vertexes[ LineDefs[ n].end].y > MAPY( PointerY)))
	 {
	    lx0 = Vertexes[ LineDefs[ n].start].x;
	    ly0 = Vertexes[ LineDefs[ n].start].y;
	    lx1 = Vertexes[ LineDefs[ n].end].x;
	    ly1 = Vertexes[ LineDefs[ n].end].y;
	    m = lx0 + (int) ((long) (MAPY( PointerY) - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
	    if (m >= MAPX( PointerX) && m < curx)
	    {
	       curx = m;
	       cur = n;
	    }
	 }
      /* now look if this LineDef has a SideDef bound to one sector */
      if (cur >= 0)
      {
	 if (Vertexes[ LineDefs[ cur].start].y > Vertexes[ LineDefs[ cur].end].y)
	 {
	    if (LineDefs[ cur].sidedef1 >= 0)
	       cur = SideDefs[ LineDefs[ cur].sidedef1].sector;
	    else
	       cur = -1;
	 }
	 else
	 {
	    if (LineDefs[ cur].sidedef2 >= 0)
	       cur = SideDefs[ LineDefs[ cur].sidedef2].sector;
	    else
	       cur = -1;
	 }
      }
      else
	 cur = -1;
   }
   return cur;
}



/*
   highlight the selected object
*/

void HighlightObject( int objtype, int objnum, int color)
{
   int  n, m;

   /* use XOR mode : drawing any line twice erases it */
   setwritemode( XOR_PUT);
   setcolor( color);
   switch ( objtype)
   {
   case OBJ_THINGS:
      DrawMapLine( Things[ objnum].xpos - OBJSIZE * 2, Things[ objnum].ypos - OBJSIZE * 2, Things[ objnum].xpos - OBJSIZE * 2, Things[ objnum].ypos + OBJSIZE * 2);
      DrawMapLine( Things[ objnum].xpos - OBJSIZE * 2, Things[ objnum].ypos + OBJSIZE * 2, Things[ objnum].xpos + OBJSIZE * 2, Things[ objnum].ypos + OBJSIZE * 2);
      DrawMapLine( Things[ objnum].xpos + OBJSIZE * 2, Things[ objnum].ypos + OBJSIZE * 2, Things[ objnum].xpos + OBJSIZE * 2, Things[ objnum].ypos - OBJSIZE * 2);
      DrawMapLine( Things[ objnum].xpos + OBJSIZE * 2, Things[ objnum].ypos - OBJSIZE * 2, Things[ objnum].xpos - OBJSIZE * 2, Things[ objnum].ypos - OBJSIZE * 2);
      break;
   case OBJ_LINEDEFS:
      n = (Vertexes[ LineDefs[ objnum].start].x + Vertexes[ LineDefs[ objnum].end].x) / 2;
      m = (Vertexes[ LineDefs[ objnum].start].y + Vertexes[ LineDefs[ objnum].end].y) / 2;
      DrawMapLine( n, m, n + (Vertexes[ LineDefs[ objnum].end].y - Vertexes[ LineDefs[ objnum].start].y) / 2, m + (Vertexes[ LineDefs[ objnum].start].x - Vertexes[ LineDefs[ objnum].end].x) / 2);
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      DrawMapVector( Vertexes[ LineDefs[ objnum].start].x, Vertexes[ LineDefs[ objnum].start].y,
		     Vertexes[ LineDefs[ objnum].end].x, Vertexes[ LineDefs[ objnum].end].y);
      if (color == YELLOW && LineDefs[ objnum].tag > 0)
      {
	 for (m = 0; m < NumSectors; m++)
	    if (Sectors[ m].tag == LineDefs[ objnum].tag)
	       HighlightObject( OBJ_SECTORS, m, LIGHTRED);
      }
      setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
      break;
   case OBJ_VERTEXES:
      DrawMapLine( Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2, Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2);
      DrawMapLine( Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2, Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2);
      DrawMapLine( Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2, Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2);
      DrawMapLine( Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2, Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2);
      break;
   case OBJ_SECTORS:
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      for (n = 0; n < NumLineDefs; n++)
	 if (SideDefs[ LineDefs[ n].sidedef1].sector == objnum || SideDefs[ LineDefs[ n].sidedef2].sector == objnum)
	    DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			 Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      if (color == YELLOW && Sectors[ objnum].tag > 0)
      {
	 for (m = 0; m < NumLineDefs; m++)
	    if (LineDefs[ m].tag == Sectors[ objnum].tag)
	       HighlightObject( OBJ_LINEDEFS, m, LIGHTRED);
      }
      setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
      break;
   }
   /* restore normal write mode */
   setwritemode( COPY_PUT);
}



/*
   display the information about one object
*/

void DisplayObjectInfo( int objtype, int objnum)
{
   char texname[ 9];
   int  tag, n;

   switch (objtype)
   {
   case OBJ_THINGS:
      DrawScreenBox3D( 0, 420, 260, 479);
      if (objnum < 0)
      {
	 DrawScreenText( 60, 440, "Use the cursor to");
	 DrawScreenText( 72, 450, "select a Thing  ");
	 break;
      }
      setcolor( YELLOW);
      DrawScreenText( 5, 425, "Selected Thing (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 439, "Coordinates:  (%d, %d)", Things[ objnum].xpos, Things[ objnum].ypos);
      DrawScreenText( 5, 449, "Type:         %s", GetThingName( Things[ objnum].type));
      DrawScreenText( 5, 459, "Angle:        %s", GetAngleName( Things[ objnum].angle));
      DrawScreenText( 5, 469, "Appears when: %s", GetWhenName( Things[ objnum].when));
      break;
   case OBJ_LINEDEFS:
      DrawScreenBox3D(   0, 400, 218, 479);
      DrawScreenBox3D( 220, 400, 438, 479);
      DrawScreenBox3D( 440, 400, 639, 479);
      if (objnum >= 0)
      {
	 setcolor( YELLOW);
	 DrawScreenText( 5, 405, "Selected LineDef (#%d)", objnum);
	 setcolor( BLACK);
	 DrawScreenText( 5, 419, "Vertexes:    (#%d, #%d)", LineDefs[ objnum].start, LineDefs[ objnum].end);
	 DrawScreenText( 5, 429, "Flags:%3d    %s", LineDefs[ objnum].flags, GetLineDefFlagsName( LineDefs[ objnum].flags));
	 DrawScreenText( 5, 439, "Type: %3d %s", LineDefs[ objnum].type, GetLineDefTypeName( LineDefs[ objnum].type));
	 tag = LineDefs[ objnum].tag;
	 if (tag > 0)
	 {
	    for (n = 0; n < NumSectors; n++)
	       if (Sectors[ n].tag == tag)
		  break;
	 }
	 else
	    n = NumSectors;
	 if (n < NumSectors)
	    DrawScreenText( 5, 449, "Sector tag:  %d (#%d)", tag, n);
	 else
	    DrawScreenText( 5, 449, "Sector tag:  %d (none)", tag);
	 DrawScreenText( 5, 459, "1st SideDef: #%d", LineDefs[ objnum].sidedef1);
	 DrawScreenText( 5, 469, "2nd SideDef: #%d", LineDefs[ objnum].sidedef2);
      }
      else
      {
	setcolor( DARKGRAY);
	DrawScreenText( 25, 435, "(No LineDef selected)");
      }
      if (objnum >= 0 && LineDefs[ objnum].sidedef1 >= 0)
      {
	 setcolor( YELLOW);
	 DrawScreenText( 225, 405, "First SideDef (#%d)", LineDefs[ objnum].sidedef1);
	 setcolor( BLACK);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef1].tex3, 8);
	 DrawScreenText( 225, 419, "Normal texture: %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef1].tex1, 8);
	 DrawScreenText( 225, 429, "Upper texture:  %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef1].tex2, 8);
	 DrawScreenText( 225, 439, "Lower texture:  %s", texname);
	 DrawScreenText( 225, 449, "Tex. X offset:  %d", SideDefs[ LineDefs[ objnum].sidedef1].xoff);
	 DrawScreenText( 225, 459, "Tex. Y offset:  %d", SideDefs[ LineDefs[ objnum].sidedef1].yoff);
	 DrawScreenText( 225, 469, "Sector:         #%d", SideDefs[ LineDefs[ objnum].sidedef1].sector);
      }
      else
      {
	setcolor( DARKGRAY);
	DrawScreenText( 255, 435, "(No first SideDef)");
      }
      if (objnum >= 0 && LineDefs[ objnum].sidedef2 >= 0)
      {
	 setcolor( YELLOW);
	 DrawScreenText( 445, 405, "Second SideDef (#%d)", LineDefs[ objnum].sidedef2);
	 setcolor( BLACK);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef2].tex3, 8);
	 DrawScreenText( 445, 419, "Normal texture: %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef2].tex1, 8);
	 DrawScreenText( 445, 429, "Upper texture:  %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef2].tex2, 8);
	 DrawScreenText( 445, 439, "Lower texture:  %s", texname);
	 DrawScreenText( 445, 449, "Tex. X offset:  %d", SideDefs[ LineDefs[ objnum].sidedef2].xoff);
	 DrawScreenText( 445, 459, "Tex. Y offset:  %d", SideDefs[ LineDefs[ objnum].sidedef2].yoff);
	 DrawScreenText( 445, 469, "Sector:         #%d", SideDefs[ LineDefs[ objnum].sidedef2].sector);
      }
      else
      {
	setcolor( DARKGRAY);
	DrawScreenText( 465, 435, "(No second SideDef)");
      }
      break;
   case OBJ_VERTEXES:
      DrawScreenBox3D( 0, 450, 220, 479);
      if (objnum < 0)
      {
	 setcolor( DARKGRAY);
	 DrawScreenText( 30, 462, "(No Vertex selected)");
	 break;
      }
      setcolor( YELLOW);
      DrawScreenText( 5, 455, "Selected Vertex (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 469, "Coordinates: (%d, %d)", Vertexes[ objnum].x, Vertexes[ objnum].y);
      break;
   case OBJ_SECTORS:
      DrawScreenBox3D( 0, 390, 255, 479);
      if (objnum < 0)
      {
	setcolor( DARKGRAY);
	DrawScreenText( 48, 425, "(No Sector selected)");
	break;
      }
      setcolor( YELLOW);
      DrawScreenText( 5, 395, "Selected Sector (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 409, "Floor height:    %d", Sectors[ objnum].floorh);
      DrawScreenText( 5, 419, "Ceiling height:  %d", Sectors[ objnum].ceilh);
      texname[ 8] = '\0';
      strncpy( texname, Sectors[ objnum].floort, 8);
      DrawScreenText( 5, 429, "Floor texture:   %s", texname);
      strncpy( texname, Sectors[ objnum].ceilt, 8);
      DrawScreenText( 5, 439, "Ceiling texture: %s", texname);
      DrawScreenText( 5, 449, "Light level:     %d", Sectors[ objnum].light);
      DrawScreenText( 5, 459, "Type: %3d        %s", Sectors[ objnum].special, GetSectorTypeName( Sectors[ objnum].special));
      tag = Sectors[ objnum].tag;
      if (tag == 0)
	 n = NumLineDefs;
      else
	 for (n = 0; n < NumLineDefs; n++)
	    if (LineDefs[ n].tag == tag)
	       break;
      if (n < NumLineDefs)
	 DrawScreenText( 5, 469, "LineDef tag:     %d (#%d)", tag, n);
      else if (tag == 99 || tag == 999)
	 DrawScreenText( 5, 469, "LineDef tag:     %d (stairs?)", tag);
      else if (tag == 666)
	 DrawScreenText( 5, 469, "LineDef tag:     %d (lower@end)", tag);
      else
	 DrawScreenText( 5, 469, "LineDef tag:     %d (none)", tag);
      break;
   }
}



/*
   delete an object (*recursive*)
*/
void DeleteObject( int objtype, int objnum)
{
   int n;

   MadeChanges = TRUE;
   switch (objtype)
   {
   case OBJ_THINGS:
      /* delete the Thing */
      NumThings--;
      if (NumThings > 0)
      {
	 for (n = objnum; n < NumThings; n++)
	    Things[ n] = Things[ n + 1];
	 Things = ResizeFarMemory( Things, NumThings * sizeof( struct Thing));
      }
      else
      {
	 farfree( Things);
	 Things = NULL;
      }
      break;
   case OBJ_LINEDEFS:
      /* delete the two SideDefs bound to this LineDef */
      if (LineDefs[ objnum].sidedef1 >= 0)
	 DeleteObject( OBJ_SIDEDEFS, LineDefs[ objnum].sidedef1);
      if (LineDefs[ objnum].sidedef2 >= 0)
	 DeleteObject( OBJ_SIDEDEFS, LineDefs[ objnum].sidedef2);
      /* delete the LineDef */
      NumLineDefs--;
      if (NumLineDefs > 0)
      {
	 for (n = objnum; n < NumLineDefs; n++)
	    LineDefs[ n] = LineDefs[ n + 1];
	 LineDefs = ResizeFarMemory( LineDefs, NumLineDefs * sizeof( struct LineDef));
      }
      else
      {
	 farfree( LineDefs);
	 LineDefs = NULL;
      }
      break;
   case OBJ_SIDEDEFS:
      /* change the LineDefs references */
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].sidedef1 == objnum)
	    LineDefs[ n].sidedef1 = -1;
	 else if (LineDefs[ n].sidedef1 >= objnum)
	    LineDefs[ n].sidedef1--;
	 if (LineDefs[ n].sidedef2 == objnum)
	    LineDefs[ n].sidedef2 = -1;
	 else if (LineDefs[ n].sidedef2 >= objnum)
	    LineDefs[ n].sidedef2--;
      }
      /* delete the SideDef */
      NumSideDefs--;
      if (NumSideDefs > 0)
      {
	 for (n = objnum; n < NumSideDefs; n++)
	    SideDefs[ n] = SideDefs[ n + 1];
	 SideDefs = ResizeFarMemory( SideDefs, NumSideDefs * sizeof( struct SideDef));
      }
      else
      {
	 farfree( SideDefs);
	 SideDefs = NULL;
      }
      MadeMapChanges = TRUE;
      break;
   case OBJ_VERTEXES:
      /* delete the LineDefs bound to this Vertex and change the references */
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].start == objnum || LineDefs[ n].end == objnum)
	    DeleteObject( OBJ_LINEDEFS, n--);
	 else
	 {
	    if (LineDefs[ n].start >= objnum)
	       LineDefs[ n].start--;
	    if (LineDefs[ n].end >= objnum)
	       LineDefs[ n].end--;
	 }
      }
      /* delete the Vertex */
      NumVertexes--;
      if (NumVertexes > 0)
      {
	 for (n = objnum; n < NumVertexes; n++)
	    Vertexes[ n] = Vertexes[ n + 1];
	 Vertexes = ResizeFarMemory( Vertexes, NumVertexes * sizeof( struct Vertex));
      }
      else
      {
	 farfree( Vertexes);
	 Vertexes = NULL;
      }
      break;
   case OBJ_SECTORS:
      /* delete the SideDefs bound to this Sector and change the references */
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (SideDefs[ LineDefs[ n].sidedef1].sector == objnum)
	    DeleteObject( OBJ_SIDEDEFS, LineDefs[ n].sidedef1);
	 else if (SideDefs[ LineDefs[ n].sidedef1].sector >= objnum)
	    SideDefs[ LineDefs[ n].sidedef1].sector--;
	 if (SideDefs[ LineDefs[ n].sidedef2].sector == objnum)
	    DeleteObject( OBJ_SIDEDEFS, LineDefs[ n].sidedef2);
	 else if (SideDefs[ LineDefs[ n].sidedef2].sector >= objnum)
	    SideDefs[ LineDefs[ n].sidedef2].sector--;
      }
      /* delete the Sector */
      NumSectors--;
      if (NumSectors > 0)
      {
	 for (n = objnum; n < NumSectors; n++)
	    Sectors[ n] = Sectors[ n + 1];
	 Sectors = ResizeFarMemory( Sectors, NumSectors * sizeof( struct Sector));
      }
      else
      {
	 farfree( Sectors);
	 Sectors = NULL;
      }
      break;
   default:
      Beep();
   }
}



/*
   insert a new object
*/
void InsertObject(int objtype, int copyfrom, int xpos, int ypos)
{
   int last;

   MadeChanges = TRUE;
   switch (objtype)
   {
   case OBJ_THINGS:
      last = NumThings++;
      if (last > 0)
	 Things = ResizeFarMemory( Things, (unsigned long) NumThings * sizeof( struct Thing));
      else
	 Things = GetFarMemory( sizeof( struct Thing));
      Things[ last].xpos = xpos;
      Things[ last].ypos = ypos;
      if (copyfrom >= 0)
      {
	 Things[ last].type  = Things[ copyfrom].type;
	 Things[ last].angle = Things[ copyfrom].angle;
	 Things[ last].when  = Things[ copyfrom].when;
      }
      else
      {
	 Things[ last].type  = THING_TROOPER;
	 Things[ last].angle = 0;
	 Things[ last].when  = 0x07;
      }
      break;
   case OBJ_LINEDEFS:
      last = NumLineDefs++;
      if (last > 0)
	 LineDefs = ResizeFarMemory( LineDefs, (unsigned long) NumLineDefs * sizeof( struct LineDef));
      else
	 LineDefs = GetFarMemory( sizeof( struct LineDef));
      if (copyfrom >= 0)
      {
	 LineDefs[ last].start = LineDefs[ copyfrom].start;
	 LineDefs[ last].end = LineDefs[ copyfrom].end;
	 LineDefs[ last].flags = LineDefs[ copyfrom].flags;
	 LineDefs[ last].type = LineDefs[ copyfrom].type;
	 LineDefs[ last].tag = LineDefs[ copyfrom].tag;
      }
      else
      {
	 LineDefs[ last].start = 0;
	 LineDefs[ last].end = NumVertexes - 1;
	 LineDefs[ last].flags = 1;
	 LineDefs[ last].type = 0;
	 LineDefs[ last].tag = 0;
      }
      LineDefs[ last].sidedef1 = -1;
      LineDefs[ last].sidedef2 = -1;
      break;
   case OBJ_SIDEDEFS:
      /* SideDefs are added from the LineDefs menu, so "copyfrom" should always be -1.  But I test it anyway. */
      last = NumSideDefs++;
      if (last > 0)
	 SideDefs = ResizeFarMemory( SideDefs, (unsigned long) NumSideDefs * sizeof( struct SideDef));
      else
	 SideDefs = GetFarMemory( sizeof( struct SideDef));
      if (copyfrom >= 0)
      {
	 SideDefs[ last].xoff = SideDefs[ copyfrom].xoff;
	 SideDefs[ last].yoff = SideDefs[ copyfrom].yoff;
	 strncpy( SideDefs[ last].tex1, SideDefs[ copyfrom].tex1, 8);
	 strncpy( SideDefs[ last].tex2, SideDefs[ copyfrom].tex2, 8);
	 strncpy( SideDefs[ last].tex3, SideDefs[ copyfrom].tex3, 8);
	 SideDefs[ last].sector = SideDefs[ copyfrom].sector;
      }
      else
      {
	 SideDefs[ last].xoff = 0;
	 SideDefs[ last].yoff = 0;
	 strcpy( SideDefs[ last].tex1, "-");
	 strcpy( SideDefs[ last].tex2, "-");
	 strcpy( SideDefs[ last].tex3, "STARTAN3");
	 SideDefs[ last].sector = NumSectors - 1;
      }
      MadeMapChanges = TRUE;
      break;
   case OBJ_VERTEXES:
      last = NumVertexes++;
      if (last > 0)
	 Vertexes = ResizeFarMemory( Vertexes, (unsigned long) NumVertexes * sizeof( struct Vertex));
      else
	 Vertexes = GetFarMemory( sizeof( struct Vertex));
      Vertexes[ last].x = xpos;
      Vertexes[ last].y = ypos;
      break;
   case OBJ_SECTORS:
      last = NumSectors++;
      if (last > 0)
	 Sectors = ResizeFarMemory( Sectors, (unsigned long) NumSectors * sizeof( struct Sector));
      else
	 Sectors = GetFarMemory( sizeof( struct Sector));
      if (copyfrom >= 0)
      {
	 Sectors[ last].floorh = Sectors[ copyfrom].floorh;
	 Sectors[ last].ceilh = Sectors[ copyfrom].ceilh;
	 strncpy( Sectors[ last].floort, Sectors[ copyfrom].floort, 8);
	 strncpy( Sectors[ last].ceilt, Sectors[ copyfrom].ceilt, 8);
	 Sectors[ last].light = Sectors[ copyfrom].light;
	 Sectors[ last].special = Sectors[ copyfrom].special;
	 Sectors[ last].tag = Sectors[ copyfrom].tag;
      }
      else
      {
	 Sectors[ last].floorh = 0;
	 Sectors[ last].ceilh = 400;
	 strcpy( Sectors[ last].floort,"FLOOR4_8");
	 strcpy( Sectors[ last].ceilt, "CEIL3_5");
	 Sectors[ last].light = 255;
	 Sectors[ last].special = 0;
	 Sectors[ last].tag = 0;
      }
      break;
   default:
      Beep();
   }
}



/*
   display and execute a "things" menu
*/
int DisplayThingsMenu( int x0, int y0, char *menutitle, ...)
{
   va_list args;
   int val, num;
   int thingid[ 30];
   char *menustr[ 30];

   /* put the va_args in the menustr table */
   num = 0;
   va_start( args, menutitle);
   while ((num < 30) && ((thingid[ num] = va_arg( args, int)) >= 0))
   {
      menustr[ num] = GetThingName( thingid[ num]);
      num++;
   }
   va_end( args);

   /* display the menu */
   val = DisplayMenuArray( x0, y0, menutitle, num, menustr) - 1;

   /* return the thing id, if valid */
   if (val < 0 || val >= num)
     return -1;
   return thingid[ val];
}



/*
   display and execute a "linedef type" menu
*/
int DisplayLineDefTypeMenu( int x0, int y0, char *menutitle, ...)
{
   va_list args;
   int val, num;
   int typeid[ 30];
   char *menustr[ 30];

   /* put the va_args in the menustr table */
   num = 0;
   va_start( args, menutitle);
   while ((num < 30) && ((typeid[ num] = va_arg( args, int)) >= 0))
   {
      menustr[ num] = GetLineDefTypeLongName( typeid[ num]);
      num++;
   }
   va_end( args);

   /* display the menu */
   val = DisplayMenuArray( x0, y0, menutitle, num, menustr) - 1;

   /* return the thing id, if valid */
   if (val < 0 || val >= num)
     return -1;
   return typeid[ val];
}



/*
   ask for an object number and check for maximum valid number
   (this is just like InputIntegerValue, but with a different prompt)
*/
int InputObjectNumber( int x0, int y0, int objtype, int curobj)
{
   int val, key;
   char prompt[ 80];

   if (UseMouse)
      HideMousePointer();
   sprintf( prompt, "Enter a %s number between 0 and %d:", GetObjectTypeName( objtype), GetMaxObjectNum( objtype));
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * strlen( prompt), y0 + 55);
   setcolor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, prompt);
   val = curobj;
   while (((key = InputInteger( x0 + 10, y0 + 28, &val, 0, GetMaxObjectNum( objtype))) & 0x00FF) != 0x000D && (key & 0x00FF) != 0x001B)
      Beep();
   if (UseMouse)
      ShowMousePointer();
   return val;
}



/*
   ask for an object number and display a warning message
*/
int InputObjectXRef( int x0, int y0, int objtype, Bool allownone, int curobj)
{
   int val, key;
   char prompt[ 80];

   if (UseMouse)
      HideMousePointer();
   sprintf( prompt, "Enter a %s number between 0 and %d%c", GetObjectTypeName( objtype), GetMaxObjectNum( objtype), allownone ? ',' : ':');
   val = strlen( prompt);
   if (val < 40)
      val = 40;
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * val, y0 + (allownone ? 85 : 75));
   setcolor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, prompt);
   if (allownone)
      DrawScreenText( x0 + 10, y0 + 18, "or -1 for none:");
   setcolor( RED);
   DrawScreenText( x0 + 10, y0 + (allownone ? 60 : 50), "Warning: modifying the cross-references");
   DrawScreenText( x0 + 10, y0 + (allownone ? 70 : 60), "between some objects may crash the game.");
   val = curobj;
   while (((key = InputInteger( x0 + 10, y0 + (allownone ? 38 : 28), &val, allownone ? -1 : 0, GetMaxObjectNum( objtype))) & 0x00FF) != 0x000D && (key & 0x00FF) != 0x001B)
      Beep();
   if (UseMouse)
      ShowMousePointer();
   return val;
}



/*
   ask for two vertex numbers and check for maximum valid number
*/
int Input2VertexNumbers( int x0, int y0, char *prompt1, int *v1, int *v2)
{
   int  val, key;
   int  maxlen, first;
   Bool ok;
   char prompt2[ 80];

   if (UseMouse)
      HideMousePointer();
   sprintf( prompt2, "Enter two numbers between 0 and %d:", NumVertexes - 1);
   if (strlen( prompt1) > strlen( prompt2))
      maxlen = strlen( prompt1);
   else
      maxlen = strlen( prompt2);
   if (x0 < 0)
      x0 = 307 - 4 * maxlen;
   if (y0 < 0)
      y0 = 202;
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * maxlen, y0 + 75);
   DrawScreenText( x0 + 10, y0 + 36, "From this Vertex");
   DrawScreenText( x0 + 180, y0 + 36, "To this Vertex");
   setcolor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, prompt1);
   DrawScreenText( x0 + 10, y0 + 18, prompt2);
   first = TRUE;
   key = 0;
   for (;;)
   {
      ok = TRUE;
      DrawScreenBox3D( x0 + 10, y0 + 48, x0 + 71, y0 + 61);
      if (*v1 < 0 || *v1 >= NumVertexes)
      {
	 setcolor( DARKGRAY);
	 ok = FALSE;
      }
      DrawScreenText( x0 + 14, y0 + 51, "%d", *v1);
      DrawScreenBox3D( x0 + 180, y0 + 48, x0 + 241, y0 + 61);
      if (*v2 < 0 || *v2 >= NumVertexes)
      {
	 setcolor( DARKGRAY);
	 ok = FALSE;
      }
      DrawScreenText( x0 + 184, y0 + 51, "%d", *v2);
      if (first)
	 key = InputInteger( x0 + 10, y0 + 48, v1, 0, NumVertexes - 1);
      else
	 key = InputInteger( x0 + 180, y0 + 48, v2, 0, NumVertexes - 1);
      if ((key & 0xFF00) == 0x4B00 || (key & 0xFF00) == 0x4D00 || (key & 0x00FF) == 0x0009 || (key & 0xFF00) == 0x0F00)
	 first = !first;
      else if ((key & 0x00FF) == 0x001B)
	 break;
      else if ((key & 0x00FF) == 0x000D)
      {
	 if (first)
	    first = FALSE;
	 else if (ok)
	    break;
	 else
	    Beep();
      }
      else
	 Beep();
   }
   if (UseMouse)
      ShowMousePointer();
   return ((key & 0x00FF) == 0x000D);
}



/*
   edit an object or a group of objects
*/

void EditObjectInfo( int objtype, SelPtr obj)
{
   char  *menustr[ 30];
   char   texname[ 9];
   int    n, val;
   SelPtr cur, sdlist;

   if (obj == NULL)
      return;
   switch (objtype)
   {
   case OBJ_THINGS:
      for (n = 0; n < 6; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 5], "Edit Thing #%d", obj->objnum);
      sprintf( menustr[ 0], "Change Type          (Current: %s)", GetThingName( Things[ obj->objnum].type));
      sprintf( menustr[ 1], "Change Angle         (Current: %s)", GetAngleName( Things[ obj->objnum].angle));
      sprintf( menustr[ 2], "Change When Appears  (Current: %s)", GetWhenName( Things[ obj->objnum].when));
      sprintf( menustr[ 3], "Change X position    (Current: %d)", Things[ obj->objnum].xpos);
      sprintf( menustr[ 4], "Change Y position    (Current: %d)", Things[ obj->objnum].ypos);
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
			      "Decoration (light sources)",
			      "Decoration (dead bodies)",
			      "Decoration (hanging bodies)",
			      "Teleport landing",
			      "(Enter a decimal value)",
			      NULL))
	 {
	 case 1:
	    val = DisplayThingsMenu( 84, 98, "Select Start Position Type",
				     THING_PLAYER1,
				     THING_PLAYER2,
				     THING_PLAYER3,
				     THING_PLAYER4,
				     THING_DEATHMATCH,
				     -1);
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
				     THING_CACODEMON,
				     THING_CYBERDEMON,
				     THING_SPIDERBOSS,
				     -1);
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
				     -1);
	    break;

	 case 4:
	    val = DisplayThingsMenu( 84, 128, "Select Bonus",
				     THING_REDCARD,
				     THING_YELLOWCARD,
				     THING_BLUECARD,
				     THING_REDSKULLKEY,
				     THING_YELLOWSKULLKEY,
				     THING_BLUESKULLKEY,
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
				     THING_LITEAMP,
				     THING_BESERK,
				     THING_INVULN,
				     -1);
	    break;

	 case 5:
	    val = DisplayThingsMenu( 84, 138, "Select Decoration",
				     THING_BARREL,
				     THING_TECHCOLUMN,
				     THING_TGREENPILLAR,
				     THING_TREDPILLAR,
				     THING_SGREENPILLAR,
				     THING_SREDPILLAR,
				     THING_PILLARHEART,
				     THING_PILLARSKULL,
				     THING_EYEINSYMBOL,
				     THING_BROWNSTUB,
				     THING_GREYTREE,
				     THING_BROWNTREE,
				     -1);
	    break;

	 case 6:
	    val = DisplayThingsMenu( 84, 148, "Select Decoration",
				     THING_CANDLE,
				     THING_LAMP,
				     THING_CANDELABRA,
				     THING_TBLUETORCH,
				     THING_TGREENTORCH,
				     THING_TREDTORCH,
				     THING_SBLUETORCH,
				     THING_SGREENTORCH,
				     THING_SREDTORCH,
				     -1);
	    break;

	 case 7:
	    val = DisplayThingsMenu( 84, 158, "Select Decoration",
				     THING_DEADPLAYER,
				     THING_DEADTROOPER,
				     THING_DEADSARGEANT,
				     THING_DEADIMP,
				     THING_DEADDEMON,
				     THING_DEADCACODEMON,
				     THING_BONES,
				     THING_BONES2,
				     THING_POOLOFBLOOD,
				     THING_SKULLTOPPOLE,
				     THING_HEADSKEWER,
				     THING_PILEOFSKULLS,
				     THING_IMPALEDBODY,
				     THING_IMPALEDBODY2,
				     THING_SKULLSINFLAMES,
				     -1);
	    break;

	 case 8:
	    val = DisplayThingsMenu( 84, 168, "Select Decoration",
				     THING_HANGINGSWAYING,
				     THING_HANGINGARMSOUT,
				     THING_HANGINGONELEG,
				     THING_HANGINGTORSO,
				     THING_HANGINGLEG,
				     THING_HANGINGSWAYING2,
				     THING_HANGINGARMSOUT2,
				     THING_HANGINGONELEG2,
				     THING_HANGINGTORSO2,
				     THING_HANGINGLEG2,
				     -1);
	    break;

	 case 9:
	    val = THING_TELEPORT;
	    break;

	 case 10:
	    val = InputIntegerValue( 84, 188, 0, 9999, Things[ obj->objnum].type);
	    break;

	 default:
	    Beep();
	    return;
	 }
	 if (val >= 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].type = val;
	    MadeChanges = TRUE;
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
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 90;
	    MadeChanges = TRUE;
	    break;
	 case 2:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 45;
	    MadeChanges = TRUE;
	    break;
	 case 3:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 0;
	    MadeChanges = TRUE;
	    break;
	 case 4:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 315;
	    MadeChanges = TRUE;
	    break;
	 case 5:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 270;
	    MadeChanges = TRUE;
	    break;
	 case 6:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 225;
	    MadeChanges = TRUE;
	    break;
	 case 7:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 180;
	    MadeChanges = TRUE;
	    break;
	 case 8:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].angle = 135;
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
			    "Toggle \"Deaf/Ambush\" bit",
			    "Toggle \"Multi-player only\" bit",
			    "(Enter a decimal value)",
			    NULL);
	 switch (val)
	 {
	 case 1:
	 case 2:
	 case 3:
	 case 4:
	 case 5:
	 case 6:
	 case 7:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].when = val;
	    MadeChanges = TRUE;
	    break;
	 case 8:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].when ^= 0x08;
	    MadeChanges = TRUE;
	    break;
	 case 9:
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].when ^= 0x10;
	    MadeChanges = TRUE;
	    break;
	 case 10:
	    val = InputIntegerValue( 84,  188, 1, 31, Things[ obj->objnum].when);
	    if (val > 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  Things[ cur->objnum].when = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 }
	 break;

      case 4:
	 val = InputIntegerValue( 42, 94, MinX, MaxX, Things[ obj->objnum].xpos);
	 if (val >= MinX)
	 {
	    n = val - Things[ obj->objnum].xpos;
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].xpos += n;
	    MadeChanges = TRUE;
	 }
	 break;

      case 5:
	 val = InputIntegerValue( 42, 104, MinY, MaxY, Things[ obj->objnum].ypos);
	 if (val >= MinY)
	 {
	    n = val - Things[ obj->objnum].ypos;
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].ypos += n;
	    MadeChanges = TRUE;
	 }
	 break;
      }
      break;

   case OBJ_LINEDEFS:
      switch (DisplayMenu( 0, 30, "Choose the object to edit:",
			   "Edit the LineDef",
			   (LineDefs[ obj->objnum].sidedef1 >= 0) ? "Edit the 1st SideDef" : "Add a 1st SideDef",
			   (LineDefs[ obj->objnum].sidedef2 >= 0) ? "Edit the 2nd SideDef" : "Add a 2nd SideDef",
			   NULL))
      {
      case 1:
	 for (n = 0; n < 9; n++)
	    menustr[ n] = GetMemory( 60);
	 sprintf( menustr[ 8], "Edit LineDef #%d", obj->objnum);
	 sprintf( menustr[ 0], "Change Flags            (Current: %d)", LineDefs[ obj->objnum].flags);
	 sprintf( menustr[ 1], "Change Type             (Current: %d)", LineDefs[ obj->objnum].type);
	 sprintf( menustr[ 2], "Change Sector tag       (Current: %d)", LineDefs[ obj->objnum].tag);
	 sprintf( menustr[ 3], "Change Starting Vertex  (Current: #%d)", LineDefs[ obj->objnum].start);
	 sprintf( menustr[ 4], "Change Ending Vertex    (Current: #%d)", LineDefs[ obj->objnum].end);
	 sprintf( menustr[ 5], "Change 1st SideDef ref. (Current: #%d)", LineDefs[ obj->objnum].sidedef1);
	 sprintf( menustr[ 6], "Change 2nd SideDef ref. (Current: #%d)", LineDefs[ obj->objnum].sidedef2);
	 sprintf( menustr[ 7], "Flip LineDef");
	 val = DisplayMenuArray( 42, 64, menustr[ 8], 8, menustr);
	 for (n = 0; n < 9; n++)
	    free( menustr[ n]);
	 switch (val)
	 {
	 case 1:
	    val = DisplayMenu( 84, 98, "Toggle the flags:",
			       GetLineDefFlagsLongName(0x01),
			       GetLineDefFlagsLongName(0x02),
			       GetLineDefFlagsLongName(0x04),
			       GetLineDefFlagsLongName(0x08),
			       GetLineDefFlagsLongName(0x10),
			       GetLineDefFlagsLongName(0x20),
			       GetLineDefFlagsLongName(0x40),
			       GetLineDefFlagsLongName(0x80),
			       GetLineDefFlagsLongName(0x100),
			       "(Enter a decimal value)",
			       NULL);
	    if (val >= 1 && val <= 9)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].flags ^= 0x01 << (val - 1);
	       MadeChanges = TRUE;
	    }
	    else if (val == 10)
	    {
	       val = InputIntegerValue( 126, 212, 0, 511, LineDefs[ obj->objnum].flags);
	       if (val >= 0)
	       {
		  for (cur = obj; cur; cur = cur->next)
		     LineDefs[ cur->objnum].flags = val;
		  MadeChanges = TRUE;
	       }
	    }
	    break;
	 case 2:
	    switch (DisplayMenu( 84, 108, "Choose a LineDef type:",
				 "Normal",
				 "Doors...",
				 "Ceilings...",
				 "Floors...",
				 "Lifts & Moving things...",
				 "Special...",
				 "(Enter a decimal value)",
				 NULL))
	    {
	    case 1:
	       val = 0;
	       break;
	    case 2:
	       val = DisplayLineDefTypeMenu( 126, 152, NULL, /* Doors */
					     1, 26, 27, 28, 63, 29, 90, 31, 32, 34, 33, 103, 86, 2, 46, 42, 75, 3, 76, 16,
					     -1);
	       break;
	    case 3:
	       val = DisplayLineDefTypeMenu( 126, 162, NULL, /* Ceilings */
					     41, 44, 40,
					     -1);
	       break;
	    case 4:
	       val = DisplayLineDefTypeMenu( 126, 172, NULL, /* Floors */
					     102, 70, 23, 9, 21, 82, 19, 38, 37, 98, 36, 18, 20, 14, 5, 22, 59, 30, 58, 91, 56,
					     -1);
	       break;
	    case 5:
	       val = DisplayLineDefTypeMenu( 126, 182, NULL, /* Lifts & Moving things */
					     62, 88, 10, 77, 73, 74, 87, 89, 7, 8,
					     -1);
	       break;
	    case 6:
	       val = DisplayLineDefTypeMenu( 126, 192, NULL, /* Special */
					     48, 11, 52, 51, 97, 39, 13, 35, 80, 104,
					     -1);
	       break;
	    case 7:
	       val = InputIntegerValue( 126, 202, 0, 255, LineDefs[ obj->objnum].type);
	       break;
	    default:
	       val = -1;
	    }
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].type = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 3:
	    val = InputIntegerValue( 84, 118, 0, 255, LineDefs[ obj->objnum].tag);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].tag = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 4:
	    val = InputObjectXRef( 84, 128, OBJ_VERTEXES, FALSE, LineDefs[ obj->objnum].start);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].start = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    break;
	 case 5:
	    val = InputObjectXRef( 84, 138, OBJ_VERTEXES, FALSE, LineDefs[ obj->objnum].end);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].end = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    break;
	 case 6:
	    val = InputObjectXRef( 84, 148, OBJ_SIDEDEFS, FALSE, LineDefs[ obj->objnum].sidedef1);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].sidedef1 = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    break;
	 case 7:
	    val = InputObjectXRef( 84, 158, OBJ_SIDEDEFS, TRUE, LineDefs[ obj->objnum].sidedef2);
	    if (val >= -1)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].sidedef2 = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    break;
	 case 8:
	    NotImplemented();
	    break;
	 }
	 break;

      /* edit or add the first SideDef */
      case 2:
	 if (LineDefs[ obj->objnum].sidedef1 >= 0)
	 {
	    /* build a new selection list with the first SideDefs */
	    objtype = OBJ_SIDEDEFS;
	    sdlist = NULL;
	    for (cur = obj; cur; cur = cur->next)
	       if (LineDefs[ cur->objnum].sidedef1 >= 0)
		  SelectObject( &sdlist, LineDefs[ cur->objnum].sidedef1);
	 }
	 else
	 {
	    /* add a new first SideDef */
	    for (cur = obj; cur; cur = cur->next)
	       if (LineDefs[ cur->objnum].sidedef1 == -1)
	       {
		  InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		  LineDefs[ cur->objnum].sidedef1 = GetMaxObjectNum( OBJ_SIDEDEFS);
	       }
	    break;
	 }
	 /* no break here */

      /* edit or add the second SideDef */
      case 3:
	 if (objtype != OBJ_SIDEDEFS)
	 {
	    if (LineDefs[ obj->objnum].sidedef2 >= 0)
	    {
	       /* build a new selection list with the second (or first) SideDefs */
	       objtype = OBJ_SIDEDEFS;
	       sdlist = NULL;
	       for (cur = obj; cur; cur = cur->next)
		  if (LineDefs[ cur->objnum].sidedef2 >= 0)
		     SelectObject( &sdlist, LineDefs[ cur->objnum].sidedef2);
		  else if (LineDefs[ cur->objnum].sidedef1 >= 0)
		     SelectObject( &sdlist, LineDefs[ cur->objnum].sidedef1);
	    }
	    else
	    {
	       /* add a new second (or first) SideDef */
	       for (cur = obj; cur; cur = cur->next)
		  if (LineDefs[ cur->objnum].sidedef1 == -1)
		  {
		     InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		     LineDefs[ cur->objnum].sidedef1 = GetMaxObjectNum( OBJ_SIDEDEFS);
		  }
		  else if (LineDefs[ cur->objnum].sidedef2 == -1)
		  {
		     InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		     LineDefs[ cur->objnum].sidedef2 = GetMaxObjectNum( OBJ_SIDEDEFS);
		  }
	       break;
	    }
	 }
	 for (n = 0; n < 10; n++)
	    menustr[ n] = GetMemory( 60);
	 sprintf( menustr[ 9], "Edit SideDef #%d", sdlist->objnum);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ sdlist->objnum].tex3, 8);
	 sprintf( menustr[ 0], "Change Normal Texture   (Current: %s)", texname);
	 sprintf( menustr[ 1], "Display Normal Texture");
	 strncpy( texname, SideDefs[ sdlist->objnum].tex1, 8);
	 sprintf( menustr[ 2], "Change Upper texture    (Current: %s)", texname);
	 sprintf( menustr[ 3], "Display Upper texture");
	 strncpy( texname, SideDefs[ sdlist->objnum].tex2, 8);
	 sprintf( menustr[ 4], "Change Lower texture    (Current: %s)", texname);
	 sprintf( menustr[ 5], "Display Lower texture");
	 sprintf( menustr[ 6], "Change Texture X offset (Current: %d)", SideDefs[ sdlist->objnum].xoff);
	 sprintf( menustr[ 7], "Change Texture Y offset (Current: %d)", SideDefs[ sdlist->objnum].yoff);
	 sprintf( menustr[ 8], "Change Sector ref.      (Current: #%d)", SideDefs[ sdlist->objnum].sector);
	 val = DisplayMenuArray( 42, 84, menustr[ 9], 9, menustr);
	 for (n = 0; n < 10; n++)
	    free( menustr[ n]);
	 switch (val)
	 {
	 case 1:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex3, 8);
	    InputNameFromList( 84, 118, "Enter a wall texture name:", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex3, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 2:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex3, 8);
	    ChooseWallTexture( 84, 128, "Choose a wall texture", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex3, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 3:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex1, 8);
	    InputNameFromList( 84, 138, "Enter a wall texture name:", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex1, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 4:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex1, 8);
	    ChooseWallTexture( 84, 148, "Choose a wall texture", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex1, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 5:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex2, 8);
	    InputNameFromList( 84, 158, "Enter a wall texture name:", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex2, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 6:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex2, 8);
	    ChooseWallTexture( 84, 168, "Choose a wall texture", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex2, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 7:
	    val = InputIntegerValue( 84, 178, -100, 100, SideDefs[ sdlist->objnum].xoff);
	    if (val >= -100)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     SideDefs[ cur->objnum].xoff = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 8:
	    val = InputIntegerValue( 84, 188, -100, 100, SideDefs[ sdlist->objnum].yoff);
	    if (val >= -100)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     SideDefs[ cur->objnum].yoff = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 9:
	    val = InputObjectXRef( 84, 198, OBJ_SECTORS, FALSE, SideDefs[ sdlist->objnum].sector);
	    if (val >= 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     SideDefs[ cur->objnum].sector = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 }
	 ForgetSelection( &sdlist);
	 break;

      }
      break;

   case OBJ_VERTEXES:
      for (n = 0; n < 3; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 2], "Edit Vertex #%d", obj->objnum);
      sprintf( menustr[ 0], "Change X position (Current: %d)", Vertexes[ obj->objnum].x);
      sprintf( menustr[ 1], "Change Y position (Current: %d)", Vertexes[ obj->objnum].y);
      val = DisplayMenuArray( 0, 30, menustr[ 2], 2, menustr);
      for (n = 0; n < 3; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( 42, 94, MinX, MaxX, Vertexes[ obj->objnum].x);
	 if (val >= MinX)
	 {
	    n = val - Vertexes[ obj->objnum].x;
	    for (cur = obj; cur; cur = cur->next)
	       Vertexes[ cur->objnum].x += n;
	    MadeChanges = TRUE;
	    MadeMapChanges = TRUE;
	 }
	 break;

      case 2:
	 val = InputIntegerValue( 42, 104, MinY, MaxY, Vertexes[ obj->objnum].y);
	 if (val >= MinY)
	 {
	    n = val - Vertexes[ obj->objnum].y;
	    for (cur = obj; cur; cur = cur->next)
	       Vertexes[ cur->objnum].y += n;
	    MadeChanges = TRUE;
	    MadeMapChanges = TRUE;
	 }
	 break;
      }
      break;

   case OBJ_SECTORS:
      for (n = 0; n < 8; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 7], "Edit Sector #%d", obj->objnum);
      sprintf( menustr[ 0], "Change Floor height     (Current: %d)", Sectors[ obj->objnum].floorh);
      sprintf( menustr[ 1], "Change Ceiling height   (Current: %d)", Sectors[ obj->objnum].ceilh);
      texname[ 8] = '\0';
      strncpy( texname, Sectors[ obj->objnum].floort, 8);
      sprintf( menustr[ 2], "Change Floor texture    (Current: %s)", texname);
      strncpy( texname, Sectors[ obj->objnum].ceilt, 8);
      sprintf( menustr[ 3], "Change Ceiling texture  (Current: %s)", texname);
      sprintf( menustr[ 4], "Change Light level      (Current: %d)", Sectors[ obj->objnum].light);
      sprintf( menustr[ 5], "Change Type             (Current: %d)", Sectors[ obj->objnum].special);
      sprintf( menustr[ 6], "Change LineDef tag      (Current: %d)", Sectors[ obj->objnum].tag);
      val = DisplayMenuArray( 0, 30, menustr[ 7], 7, menustr);
      for (n = 0; n < 8; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( 42, 64, -511, 511, Sectors[ obj->objnum].floorh);
	 if (val >= -511)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].floorh = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 2:
	 val = InputIntegerValue( 42, 74, -511, 511, Sectors[ obj->objnum].ceilh);
	 if (val >= -511)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].ceilh = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 3:
	 strncpy( texname, Sectors[ obj->objnum].floort, 8);
	 ChooseFloorTexture( 42, 84, "Choose a floor texture", NumFTexture, FTexture, texname);
	 if (strlen(texname) > 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       strncpy( Sectors[ cur->objnum].floort, texname, 8);
	    MadeChanges = TRUE;
	 }
	 break;
      case 4:
	 strncpy( texname, Sectors[ obj->objnum].ceilt, 8);
	 ChooseFloorTexture( 42, 94, "Choose a ceiling texture", NumFTexture, FTexture, texname);
	 if (strlen(texname) > 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       strncpy( Sectors[ cur->objnum].ceilt, texname, 8);
	    MadeChanges = TRUE;
	 }
	 break;
      case 5:
	 val = InputIntegerValue( 42, 124, 0, 255, Sectors[ obj->objnum].light);
	 if (val >= 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].light = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 6:
	 val = DisplayMenu( 42, 134, "Choose a special behaviour",
			    GetSectorTypeLongName(0),
			    GetSectorTypeLongName(1),
			    GetSectorTypeLongName(2),
			    GetSectorTypeLongName(3),
			    GetSectorTypeLongName(4),
			    GetSectorTypeLongName(5),
			    GetSectorTypeLongName(7),
			    GetSectorTypeLongName(8),
			    GetSectorTypeLongName(9),
			    GetSectorTypeLongName(10),
			    GetSectorTypeLongName(11),
			    GetSectorTypeLongName(12),
			    GetSectorTypeLongName(13),
			    GetSectorTypeLongName(14),
			    GetSectorTypeLongName(16),
			    "(Enter a decimal value)",
			    NULL);
	 switch (val)
	 {
	 case 1:
	 case 2:
	 case 3:
	 case 4:
	 case 5:
	 case 6:
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].special = val - 1;
	    MadeChanges = TRUE;
	    break;
	 case 7:
	 case 8:
	 case 9:
	 case 10:
	 case 11:
	 case 12:
	 case 13:
	 case 14:
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].special = val;
	    MadeChanges = TRUE;
	    break;
	 case 15:
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].special = 16;
	    MadeChanges = TRUE;
	    break;
	 case 16:
	    val = InputIntegerValue( 84, 258, 0, 255, Sectors[ obj->objnum].special);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  Sectors[ cur->objnum].special = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 }
	 break;
      case 7:
	 val = InputIntegerValue( 42, 144, 0, 999, Sectors[ obj->objnum].tag);
	 if (val >= 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].tag = val;
	    MadeChanges = TRUE;
	 }
	 break;
      }
      break;
   }
}



/*
   move an object or a group of objects to a new position
*/

void MoveObjectToCoords( int objtype, SelPtr obj, int x, int y)
{
   int    d;
   SelPtr cur;

   switch (objtype)
   {
      case OBJ_THINGS:
	 d = x - Things[ obj->objnum].xpos;
	 for (cur = obj; cur; cur = cur ->next)
	    Things[ cur->objnum].xpos += d;
	 d = y - Things[ obj->objnum].ypos;
	 for (cur = obj; cur; cur = cur ->next)
	    Things[ cur->objnum].ypos += d;
	 MadeChanges = TRUE;
	 break;
      case OBJ_VERTEXES:
	 d = x - Vertexes[ obj->objnum].x;
	 for (cur = obj; cur; cur = cur ->next)
	    Vertexes[ cur->objnum].x += d;
	 d = y - Vertexes[ obj->objnum].y;
	 for (cur = obj; cur; cur = cur ->next)
	    Vertexes[ cur->objnum].y += d;
	 MadeChanges = TRUE;
	 MadeMapChanges = TRUE;
	 break;
   }
}



/*
   check if a (part of a) LineDef is inside a given block
*/

Bool IsLineDefInside( int ldnum, int x0, int y0, int x1, int y1)
{
   int lx0 = Vertexes[ LineDefs[ ldnum].start].x;
   int ly0 = Vertexes[ LineDefs[ ldnum].start].y;
   int lx1 = Vertexes[ LineDefs[ ldnum].end].x;
   int ly1 = Vertexes[ LineDefs[ ldnum].end].y;
   int i;

   /* do you like mathematics? */
   if (lx0 >= x0 && lx0 <= x1 && ly0 >= y0 && ly0 <= y1)
      return TRUE; /* the LineDef start is entirely inside the square */
   if (lx1 >= x0 && lx1 <= x1 && ly1 >= y0 && ly1 <= y1)
      return TRUE; /* the LineDef end is entirely inside the square */
   if ((ly0 > y0) != (ly1 > y0))
   {
      i = lx0 + (int) ( (long) (y0 - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
      if (i >= x0 && i <= x1)
	 return TRUE; /* the LineDef crosses the y0 side (left) */
   }
   if ((ly0 > y1) != (ly1 > y1))
   {
      i = lx0 + (int) ( (long) (y1 - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
      if (i >= x0 && i <= x1)
	 return TRUE; /* the LineDef crosses the y1 side (right) */
   }
   if ((lx0 > x0) != (lx1 > x0))
   {
      i = ly0 + (int) ( (long) (x0 - lx0) * (long) (ly1 - ly0) / (long) (lx1 - lx0));
      if (i >= y0 && i <= y1)
	 return TRUE; /* the LineDef crosses the x0 side (down) */
   }
   if ((lx0 > x1) != (lx1 > x1))
   {
      i = ly0 + (int) ( (long) (x1 - lx0) * (long) (ly1 - ly0) / (long) (lx1 - lx0));
      if (i >= y0 && i <= y1)
	 return TRUE; /* the LineDef crosses the x1 side (up) */
   }
   return FALSE;
}



/*
   display some informations while the user is waiting
*/
void ShowProgress( int objtype)
{
   switch (objtype)
   {
   case OBJ_VERTEXES:
      DrawScreenBox3D( 0, 0, 200, 22);
      DrawScreenText( 10, 8, "Number of Vertices: %d", NumVertexes);
      break;
   case OBJ_SIDEDEFS:
      DrawScreenBox3D( 0, 30, 200, 52);
      DrawScreenText( 10, 38, "Number of SideDefs: %d", NumSideDefs);
      break;
   case OBJ_SSECTORS:
      DrawScreenBox3D( 0, 60, 200, 92);
      DrawScreenText( 10, 68, "Number of Segs:     %d", NumSegs);
      DrawScreenText( 10, 78, "Number of SSectors: %d", NumSSectors);
      break;
   }
}



/*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*DEBUG*/
void ShowSeg( SEPtr seg, int color)
{
   int n, m;

   setwritemode( XOR_PUT);
   setcolor( color);
   n = (Vertexes[ seg->start].x + Vertexes[ seg->end].x) / 2;
   m = (Vertexes[ seg->start].y + Vertexes[ seg->end].y) / 2;
   DrawMapLine( n, m, n + (Vertexes[ seg->end].y - Vertexes[ seg->start].y) / 2, m + (Vertexes[ seg->start].x - Vertexes[ seg->end].x) / 2);
   setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
   DrawMapVector( Vertexes[ seg->start].x, Vertexes[ seg->start].y,
		  Vertexes[ seg->end].x, Vertexes[ seg->end].y);
   setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
   setwritemode( COPY_PUT);
}



/*
   check if a list of Segs should be divided in smaller parts (by a nodeline)
*/

Bool NeedFurtherDivision( SEPtr seglist)
{
   SEPtr curseg, refseg;
   int   sector;

   /* the sector number must be the same for all Segs */
   if (seglist->flip)
      sector = SideDefs[ LineDefs[ seglist->linedef].sidedef2].sector;
   else
      sector = SideDefs[ LineDefs[ seglist->linedef].sidedef1].sector;
   for (curseg = seglist->next; curseg; curseg = curseg->next)
   {
      if (curseg->flip)
      {
	 if (SideDefs[ LineDefs[ curseg->linedef].sidedef2].sector != sector)
	    return TRUE;
      }
      else
      {
	 if (SideDefs[ LineDefs[ curseg->linedef].sidedef1].sector != sector)
	    return TRUE;
      }
   }
   /* the angle between two successive Segs must be <= 32767 */
   for (refseg = seglist; refseg; refseg = refseg->next)
   {
      for (curseg = seglist; curseg; curseg = curseg->next)
	 if (curseg->start == refseg->end && (unsigned int) (refseg->angle - curseg->angle) > (unsigned int) 32767)
	    return TRUE;
   }
   /* no need to split the list: these Segs can be put in a SSector */
   return FALSE;
}



/*
   find the point of intersection for two lines
*/

Bool ComputeIntersection( int *x, int *y, SEPtr seg1, SEPtr seg2)
{
   /* floating-point required because long integers cause errors */
   double x1  = Vertexes[ seg1->start].x;
   double y1  = Vertexes[ seg1->start].y;
   double dx1 = Vertexes[ seg1->end].x - Vertexes[ seg1->start].x;
   double dy1 = Vertexes[ seg1->end].y - Vertexes[ seg1->start].y;
   double x2  = Vertexes[ seg2->start].x;
   double y2  = Vertexes[ seg2->start].y;
   double dx2 = Vertexes[ seg2->end].x - Vertexes[ seg2->start].x;
   double dy2 = Vertexes[ seg2->end].y - Vertexes[ seg2->start].y;
   double d;

   d = dy1 * dx2 - dx1 * dy2;
   if (d != 0.0)
   {
      x1 = y1 * dx1 - x1 * dy1;
      x2 = y2 * dx2 - x2 * dy2;
      /* (*x, *y) = intersection */
      *x = (int) ((dx1 * x2 - dx2 * x1) / d);
      *y = (int) ((dy1 * x2 - dy2 * x1) / d);
      /* check if the intersection is not at one end of a Seg */
      if ((*x != Vertexes[ seg1->start].x || *y != Vertexes[ seg1->start].y) && (*x != Vertexes[ seg1->end].x || *y != Vertexes[ seg1->end].y)
       && (*x != Vertexes[ seg2->start].x || *y != Vertexes[ seg2->start].y) && (*x != Vertexes[ seg2->end].x || *y != Vertexes[ seg2->end].y))
	 return TRUE; /* intersection OK */
      else
	 return FALSE; /* not a real intersection point (round-off error in a previous operation) */
   }
   else
      return FALSE; /* parallel lines */
}



/*
   create a SSector from a list of Segs
*/

int CreateSSector( SEPtr seglist)
{
   /* update the SSectors list */
   NumSSectors++;
   if (SSectors)
   {
      LastSSector->next = GetMemory( sizeof( struct SSector));
      LastSSector = LastSSector->next;
   }
   else
   {
      SSectors = GetMemory( sizeof( struct SSector));
      LastSSector = SSectors;
   }
   LastSSector->next = NULL;
   /* number of first Segment in this SubSector */
   LastSSector->first = NumSegs;
   /* update the Segs list */
   if (Segs == NULL)
      Segs = seglist;
   else
      LastSeg->next = seglist;
   NumSegs++;
   for (LastSeg = seglist; LastSeg->next; LastSeg = LastSeg->next)
      NumSegs++;
   /* total number of Segments in this SubSector */
   LastSSector->num = NumSegs - LastSSector->first;
   /* while the user is waiting... */
   ShowProgress( OBJ_SSECTORS);
   /* return the number of this SubSector */
   return NumSSectors - 1;
}



/*
   create all the Nodes from a list of Segs
*/

NPtr CreateNodes( SEPtr seglist)
{
   NPtr   node;
   SEPtr  nodeline, curseg;
   int    splits, minsplits;
   int    num1, num2, mindiff;
   long   a, b, c, d;
   SEPtr  segs1, lastseg1;
   SEPtr  segs2, lastseg2;
   /* ***DEBUG*** */
   static SEPtr lastnodeline = NULL;

   /* new Node */
   node = GetMemory( sizeof( struct Node));

   /* find nodeline - brute force: try with all Segs */
   minsplits = 32767;
   mindiff = 32767;
   segs1 = NULL;
   for (nodeline = seglist; nodeline; nodeline = nodeline->next)
   {
      /* compute x, y, dx, dy */
      long x = Vertexes[ nodeline->start].x;
      long y = Vertexes[ nodeline->start].y;
      long dx = Vertexes[ nodeline->end].x - Vertexes[ nodeline->start].x;
      long dy = Vertexes[ nodeline->end].y - Vertexes[ nodeline->start].y;
      /* compute number of splits */
      splits = 0;
      num1 = 0;
      num2 = 0;
      for (curseg = seglist; curseg; curseg = curseg->next)
      {
	 if (curseg == nodeline)
	 {
	    num1++;
	    continue;
	 }
	 a = ((long) Vertexes[ curseg->start].x - x) * dy;
	 b = ((long) Vertexes[ curseg->start].y - y) * dx;
	 c = ((long) Vertexes[ curseg->end].x - x) * dy;
	 d = ((long) Vertexes[ curseg->end].y - y) * dx;
	 if ((a != b) && (c != d) && ((a > b) != (c > d)))
	 {
	    int newx, newy;
	    /* we should have an intersection, but... */
	    /* check for round-off errors (intersection of long diagonal lines) */
	    if (ComputeIntersection( &newx, &newy, nodeline, curseg))
	    {
	       splits++; /* one more split */
	       num1++;
	       num2++;
	    }
	 }
	 else if ((a > b) || ((a == b) && (c > d))
		  || ((a == b) && (c == d) && ((dx > 0) == ((Vertexes[ curseg->end].x - Vertexes[ curseg->start].x) > 0)) && ((dy > 0) == ((Vertexes[ curseg->end].y - Vertexes[ curseg->start].y) > 0))))
	    num1++; /* one more Seg on the first (right) side */
	 else
	    num2++; /* one more Seg on the second (left) side */
	 if (splits > minsplits)
	    break;  /* don't waste time */
      }
      /* there must be at least one Seg on each side */
      if (num1 > 0 && num2 > 0)
      {
	 /* now, num1 = difference in number of Segs between two sides */
	 if (num1 > num2)
	    num1 = num1 - num2;
	 else
	    num1 = num2 - num1;
	 /* minimal number of splits = candidate for nodeline */
	 if (splits < minsplits || (splits == minsplits && num1 < mindiff))
	 {
	    minsplits = splits; /* minimal number of splits */
	    mindiff = num1; /* minimal difference between the two sides */
	    segs1 = nodeline; /* save the nodeline */
	 }
      }
   }

   if (segs1)
      nodeline = segs1;
   else
      ProgError( "nodeline not found (check if all sectors are closed)");

   /* ***DEBUG*** */
   if (nodeline == lastnodeline)
      ProgError( "nodeline picked twice (this is a BUG!)");
   lastnodeline = nodeline;

   /* compute x, y, dx, dy */
   node->x = Vertexes[ nodeline->start].x;
   node->y = Vertexes[ nodeline->start].y;
   node->dx = Vertexes[ nodeline->end].x - Vertexes[ nodeline->start].x;
   node->dy = Vertexes[ nodeline->end].y - Vertexes[ nodeline->start].y;

   /* split seglist in segs1 and segs2 */
   segs1 = NULL;
   segs2 = NULL;
   while (seglist)
   {
      curseg = seglist;
      seglist = seglist->next;
      a = (long) (Vertexes[ curseg->start].x - node->x) * (long) (node->dy);
      b = (long) (Vertexes[ curseg->start].y - node->y) * (long) (node->dx);
      c = (long) (Vertexes[ curseg->end].x - node->x) * (long) (node->dy);
      d = (long) (Vertexes[ curseg->end].y - node->y) * (long) (node->dx);
      if ((a > b) || ((a == b) && (c > d))
	  || ((a == b) && (c == d) && ((node->dx > 0) == ((Vertexes[ curseg->end].x - Vertexes[ curseg->start].x) > 0)) && ((node->dy > 0) == ((Vertexes[ curseg->end].y - Vertexes[ curseg->start].y) > 0))))
      {
	 /* the starting Vertex is on the first side (right) of the nodeline */
	 if (segs1)
	 {
	    lastseg1->next = curseg;
	    lastseg1 = lastseg1->next;
	 }
	 else
	 {
	    segs1 = curseg;
	    lastseg1 = segs1;
	 }
	 lastseg1->next = NULL;
	 if (c < d)
	 {
	    int newx, newy;

	    /* the ending Vertex is on the other side: split the Seg in two */
	    if (ComputeIntersection( &newx, &newy, nodeline, curseg))
	    {
	       InsertObject( OBJ_VERTEXES, -1, newx, newy);
	       if (segs2)
	       {
		  lastseg2->next = GetFarMemory( sizeof( struct Seg));
		  lastseg2 = lastseg2->next;
	       }
	       else
	       {
		  segs2 = GetFarMemory( sizeof( struct Seg));
		  lastseg2 = segs2;
	       }
	       lastseg2->next = NULL;
	       lastseg2->start = NumVertexes - 1;
	       lastseg2->end = lastseg1->end;
	       lastseg2->angle = lastseg1->angle;
	       lastseg2->linedef = lastseg1->linedef;
	       lastseg2->flip = lastseg1->flip;
	       lastseg2->dist = lastseg1->dist + ComputeDist( newx - Vertexes[ lastseg1->start].x, newy - Vertexes[ lastseg1->start].y);
	       lastseg1->end = NumVertexes - 1;
	       ShowProgress( OBJ_VERTEXES);
	    }
	 }
      }
      else
      {
	 /* the starting Vertex is on the second side (left) of the nodeline */
	 if (segs2)
	 {
	    lastseg2->next = curseg;
	    lastseg2 = lastseg2->next;
	 }
	 else
	 {
	    segs2 = curseg;
	    lastseg2 = segs2;
	 }
	 lastseg2->next = NULL;
	 if (c > d)
	 {
	    int newx, newy;

	    /* the ending Vertex is on the other side: split the Seg in two */
	    if (ComputeIntersection( &newx, &newy, nodeline, curseg))
	    {
	       InsertObject( OBJ_VERTEXES, -1, newx, newy);
	       if (segs1)
	       {
		  lastseg1->next = GetFarMemory( sizeof( struct Seg));
		  lastseg1 = lastseg1->next;
	       }
	       else
	       {
		  segs1 = GetFarMemory( sizeof( struct Seg));
		  lastseg1 = segs1;
	       }
	       lastseg1->next = NULL;
	       lastseg1->start = NumVertexes - 1;
	       lastseg1->end = lastseg2->end;
	       lastseg1->angle = lastseg2->angle;
	       lastseg1->linedef = lastseg2->linedef;
	       lastseg1->flip = lastseg2->flip;
	       lastseg1->dist = lastseg2->dist + ComputeDist( newx - Vertexes[ lastseg2->start].x, newy - Vertexes[ lastseg2->start].y);
	       lastseg2->end = NumVertexes - 1;
	       ShowProgress( OBJ_VERTEXES);
	    }
	 }
      }
   }

   /* now, we should have all the Segs in segs1 and segs2 */
   if (segs1 == NULL || segs2 == NULL)
      ProgError("cannot split the Segs list (this is a BUG!)");

   /* compute minx1, miny1, maxx1, maxy1 */
   node->maxx1 = -32768;
   node->maxy1 = -32768;
   node->minx1 = 32767;
   node->miny1 = 32767;
   for (curseg = segs1; curseg; curseg = curseg->next)
   {
      if (Vertexes[ curseg->start].x < node->minx1)
	 node->minx1 = Vertexes[ curseg->start].x;
      if (Vertexes[ curseg->start].x > node->maxx1)
	 node->maxx1 = Vertexes[ curseg->start].x;
      if (Vertexes[ curseg->start].y < node->miny1)
	 node->miny1 = Vertexes[ curseg->start].y;
      if (Vertexes[ curseg->start].y > node->maxy1)
	 node->maxy1 = Vertexes[ curseg->start].y;
      if (Vertexes[ curseg->end].x < node->minx1)
	 node->minx1 = Vertexes[ curseg->end].x;
      if (Vertexes[ curseg->end].x > node->maxx1)
	 node->maxx1 = Vertexes[ curseg->end].x;
      if (Vertexes[ curseg->end].y < node->miny1)
	 node->miny1 = Vertexes[ curseg->end].y;
      if (Vertexes[ curseg->end].y > node->maxy1)
	 node->maxy1 = Vertexes[ curseg->end].y;
   }

   /* create Nodes or SSectors from segs1 */
   if (NeedFurtherDivision( segs1))
   {
      node->node1 = CreateNodes( segs1);
      node->child1 = 0;
   }
   else
   {
      node->node1 = NULL;
      node->child1 = CreateSSector( segs1) | 0x8000;
   }

   /* compute minx2, miny2, maxx2, maxy2 */
   node->maxx2 = -32768;
   node->maxy2 = -32768;
   node->minx2 = 32767;
   node->miny2 = 32767;
   for (curseg = segs2; curseg; curseg = curseg->next)
   {
      if (Vertexes[ curseg->start].x < node->minx2)
	 node->minx2 = Vertexes[ curseg->start].x;
      if (Vertexes[ curseg->start].x > node->maxx2)
	 node->maxx2 = Vertexes[ curseg->start].x;
      if (Vertexes[ curseg->start].y < node->miny2)
	 node->miny2 = Vertexes[ curseg->start].y;
      if (Vertexes[ curseg->start].y > node->maxy2)
	 node->maxy2 = Vertexes[ curseg->start].y;
      if (Vertexes[ curseg->end].x < node->minx2)
	 node->minx2 = Vertexes[ curseg->end].x;
      if (Vertexes[ curseg->end].x > node->maxx2)
	 node->maxx2 = Vertexes[ curseg->end].x;
      if (Vertexes[ curseg->end].y < node->miny2)
	 node->miny2 = Vertexes[ curseg->end].y;
      if (Vertexes[ curseg->end].y > node->maxy2)
	 node->maxy2 = Vertexes[ curseg->end].y;
   }

   /* create Nodes or SSectors from segs2 */
   if (NeedFurtherDivision( segs2))
   {
      node->node2 = CreateNodes( segs2);
      node->child2 = 0;
   }
   else
   {
      node->node2 = NULL;
      node->child2 = CreateSSector( segs2) | 0x8000;
   }
   /* this Node is OK */
   return node;
}


/*
   IF YOU ARE WRITING A DOOM EDITOR, PLEASE READ THIS:

   I spent a lot of time writing the Nodes builder.  There are some bugs in
   it, but most of the code is OK.  If you steal any ideas from this program,
   put a prominent message in your own editor to make it CLEAR that some
   original ideas were taken from DEU.  Thanks.

   While everyone was talking about LineDefs, I had the idea of taking only
   the Segs into account, and creating the Segs directly from the SideDefs.
   Also, dividing the list of Segs in two after each call to CreateNodes makes
   the algorithm faster.  I use several other tricks, such as looking at the
   two ends of a Seg to see on which side of the nodeline it lies or if it
   should be split in two.  I took me a lot of time and efforts to do this.

   I give this algorithm to whoever wants to use it, but with this condition:
   if your program uses some of the ideas from DEU or the whole algorithm, you
   MUST tell it to the user.  And if you post a message with all or parts of
   this algorithm in it, please post this notice also.  I don't want to speak
   legalese; I hope that you understand me...  I kindly give the sources of my
   program to you: please be kind with me...

   If you need more information about this, here is my E-mail address:
   quinet@montefiore.ulg.ac.be (Rapha‰l Quinet).

   Short description of the algorithm:
     1 - Create one Seg for each SideDef: pick each LineDef in turn.  If it
	 has a "first" SideDef, then create a normal Seg.  If it has a
	 "second" SideDef, then create a flipped Seg.
     2 - Call CreateNodes with the current list of Segs.  The list of Segs is
	 the only argument to CreateNodes.
     3 - Save the Nodes, Segs and SSectors to disk.  Start with the leaves of
	 the Nodes tree and continue up to the root (last Node).

   CreateNodes does the following:
     1 - Pick a nodeline amongst the Segs (minimize the number of splits and
	 keep the tree as balanced as possible).
     2 - Move all Segs on the right of the nodeline in a list (segs1) and do
	 the same for all Segs on the left of the nodeline (in segs2).
     3 - If the first list (segs1) contains references to more than one
	 Sector or if the angle between two adjacent Segs is greater than
	 180ø, then call CreateNodes with this (smaller) list.  Else, create
	 a SubSector with all these Segs.
     4 - Do the same for the second list (segs2).
     5 - Return the new node (its two children are already OK).

   Each time CreateSSector is called, the Segs are put in a global list.
   When there is no more Seg in CreateNodes' list, then they are all in the
   global list and ready to be saved to disk.
*/


/* end of file */
