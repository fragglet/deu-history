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
extern int NumThings;                /* number of things */
extern TPtr Things;                  /* things data */
extern int NumLineDefs;              /* number of line defs */
extern LDPtr LineDefs;               /* line defs data */
extern int NumSideDefs;              /* number of side defs */
extern SDPtr SideDefs;               /* side defs data */
extern int NumVertexes;              /* number of vertexes */
extern VPtr Vertexes;                /* vertex data */
extern int NumSegs;		     /* number of segs */
extern SEPtr Segs;                   /* segs data */
extern int NumNodes;	     	     /* number of nodes */
extern NPtr Nodes;		     /* nodes data */
extern int NumSSectors;	     	     /* number of subsectors */
extern SSPtr SSectors;               /* subsectors data */
extern int NumSectors;		     /* number of sectors */
extern SPtr Sectors;                 /* sectors data */
extern int MaxX;		     /* maximum X value of map */
extern int MaxY;		     /* maximum Y value of map */
extern int MinX;		     /* minimum X value of map */
extern int MinY;		     /* minimum Y value of map */
extern int MadeChanges;		     /* made changes? */



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
   case OBJ_NODES:
      return NumNodes - 1;
   case OBJ_SECTORS:
      return NumSectors - 1;
   case OBJ_REJECT:
      return - 1 /* NumReject? */;
   case OBJ_BLOCKMAP:
      return - 1 /* NumBlockmap? */;
   }
   return -1;
}


/*
   check if there is something of interest near the pointer
*/

int GetCurObject( int objtype)
{
   int  x0 = MAPX(PointerX) - OBJSIZE;
   int  x1 = MAPX(PointerX) + OBJSIZE;
   int  y0 = MAPY(PointerY) - OBJSIZE;
   int  y1 = MAPY(PointerY) + OBJSIZE;
   int  n, m, cur, curx;


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
	 if (IsLineDefInside( n, (double) x0, (double) y0, (double) x1, (double) y1))
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
   case OBJ_SEGS:
      for (n = 0; n < NumSegs; n++)
      {
	 m = Segs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    cur = n;
	    break;
	 }
      }
      break;
   case OBJ_NODES:
      for (n = 0; n < NumNodes; n++)
	 if (Nodes[ n].x >= x0 && Nodes[ n].x <= x1 && Nodes[ n].y >= y0 && Nodes[ n].y <= y1)
	 {
	    cur = n;
	    break;
	 }
      break;
   case OBJ_SECTORS:
      curx = MaxX + 1;
      cur = -1;
      for (n = 0; n < NumLineDefs; n++)
	 if ((Vertexes[ LineDefs[ n].start].y > MAPY( PointerY)) != (Vertexes[ LineDefs[ n].end].y > MAPY( PointerY)))
	 {
	    double lx0 = (double) Vertexes[ LineDefs[ n].start].x;
	    double ly0 = (double) Vertexes[ LineDefs[ n].start].y;
	    double lx1 = (double) Vertexes[ LineDefs[ n].end].x;
	    double ly1 = (double) Vertexes[ LineDefs[ n].end].y;

	    m = (int) (lx0 + ((double) MAPY( PointerY) - ly0) * (lx1 - lx0) / (ly1 - ly0));
	    if (m >= MAPX( PointerX) && m < curx)
	    {
	       curx = m;
	       cur = n;
	    }
	 }
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
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      DrawMapVector( Vertexes[ LineDefs[ objnum].start].x, Vertexes[ LineDefs[ objnum].start].y,
		     Vertexes[ LineDefs[ objnum].end].x, Vertexes[ LineDefs[ objnum].end].y);
      if (LineDefs[ objnum].tag > 0)
      {
	 setcolor( color - 1);
	 for (m = 0; m < NumSectors; m++)
	    if (Sectors[ m].tag == LineDefs[ objnum].tag)
	       for (n = 0; n < NumLineDefs; n++)
		  if (SideDefs[ LineDefs[ n].sidedef1].sector == m || SideDefs[ LineDefs[ n].sidedef2].sector == m)
		     DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
				  Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }
      setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
      break;
   case OBJ_VERTEXES:
      DrawMapLine( Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2, Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2);
      DrawMapLine( Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2, Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2);
      DrawMapLine( Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y + OBJSIZE * 2, Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2);
      DrawMapLine( Vertexes[ objnum].x + OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2, Vertexes[ objnum].x - OBJSIZE * 2, Vertexes[ objnum].y - OBJSIZE * 2);
      break;
   case OBJ_SEGS:
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      DrawMapVector( Vertexes[ Segs[ objnum].start].x, Vertexes[ Segs[ objnum].start].y,
		     Vertexes[ Segs[ objnum].end].x, Vertexes[ Segs[ objnum].end].y);
      setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
      break;
   case OBJ_SSECTORS:
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      for (n = SSectors[ objnum].first; n < SSectors[ objnum].first + SSectors[ objnum].num; n++)
	 DrawMapVector( Vertexes[ Segs[ n].start].x, Vertexes[ Segs[ n].start].y,
			Vertexes[ Segs[ n].end].x, Vertexes[ Segs[ n].end].y);
      setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
      break;
   case OBJ_NODES:
      if (color == YELLOW)
      {
	 if (Nodes[ objnum].tree1 & 0x8000)
	    HighlightObject( OBJ_SSECTORS, Nodes[ objnum].tree1 & 0x7FFFF, CYAN);
	 else
	    HighlightObject( OBJ_NODES, Nodes[ objnum].tree1, LIGHTCYAN);
	 setwritemode( XOR_PUT);
	 if (Nodes[ objnum].tree2 & 0x8000)
	    HighlightObject( OBJ_SSECTORS, Nodes[ objnum].tree2 & 0x7FFFF, GREEN);
	 else
	    HighlightObject( OBJ_NODES, Nodes[ objnum].tree2, LIGHTGREEN);
	 setwritemode( XOR_PUT);
	 setcolor( LIGHTCYAN);
	 DrawMapLine( Nodes[ objnum].minx1, Nodes[ objnum].miny1, Nodes[ objnum].minx1, Nodes[ objnum].maxy1);
	 DrawMapLine( Nodes[ objnum].minx1, Nodes[ objnum].maxy1, Nodes[ objnum].maxx1, Nodes[ objnum].maxy1);
	 DrawMapLine( Nodes[ objnum].maxx1, Nodes[ objnum].maxy1, Nodes[ objnum].maxx1, Nodes[ objnum].miny1);
	 DrawMapLine( Nodes[ objnum].maxx1, Nodes[ objnum].miny1, Nodes[ objnum].minx1, Nodes[ objnum].miny1);
	 setcolor( LIGHTGREEN);
	 DrawMapLine( Nodes[ objnum].minx2, Nodes[ objnum].miny2, Nodes[ objnum].minx2, Nodes[ objnum].maxy2);
	 DrawMapLine( Nodes[ objnum].minx2, Nodes[ objnum].maxy2, Nodes[ objnum].maxx2, Nodes[ objnum].maxy2);
	 DrawMapLine( Nodes[ objnum].maxx2, Nodes[ objnum].maxy2, Nodes[ objnum].maxx2, Nodes[ objnum].miny2);
	 DrawMapLine( Nodes[ objnum].maxx2, Nodes[ objnum].miny2, Nodes[ objnum].minx2, Nodes[ objnum].miny2);
	 setcolor( color);
      }
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      DrawMapVector( Nodes[ objnum].x, Nodes[ objnum].y, Nodes[ objnum].x + Nodes[ objnum].dx, Nodes[ objnum].y + Nodes[ objnum].dy);
      setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
      break;
   case OBJ_SECTORS:
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      for (n = 0; n < NumLineDefs; n++)
	 if (SideDefs[ LineDefs[ n].sidedef1].sector == objnum || SideDefs[ LineDefs[ n].sidedef2].sector == objnum)
	    DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			 Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      if (Sectors[ objnum].tag > 0)
      {
	 setcolor( color - 1);
	 for (m = 0; m < NumLineDefs; m++)
	    if (LineDefs[ m].tag == Sectors[ objnum].tag)
	       DrawMapVector( Vertexes[ LineDefs[ m].start].x, Vertexes[ LineDefs[ m].start].y,
			      Vertexes[ LineDefs[ m].end].x, Vertexes[ LineDefs[ m].end].y);
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
	 DrawScreenText( 5, 429, "Flags:       <%s>", GetLineDefFlagsName( LineDefs[ objnum].flags));
	 DrawScreenText( 5, 439, "Type:        %s", GetLineDefTypeName( LineDefs[ objnum].type));
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
	 DrawScreenText( 225, 429, "Texture above:  %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef1].tex2, 8);
	 DrawScreenText( 225, 439, "Texture below:  %s", texname);
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
	 DrawScreenText( 445, 429, "Texture above:  %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef2].tex2, 8);
	 DrawScreenText( 445, 439, "Texture below:  %s", texname);
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
	 break;
      setcolor( YELLOW);
      DrawScreenText( 5, 455, "Selected Vertex (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 469, "Coordinates: (%d, %d)", Vertexes[ objnum].x, Vertexes[ objnum].y);
      break;
   case OBJ_SEGS:
      DrawScreenBox3D( 0, 410, 225, 479);
      if (objnum < 0)
	break;
      setcolor( YELLOW);
      DrawScreenText( 5, 415, "Selected Segment (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 429, "Vertexes:    (#%d, #%d)", Segs[ objnum].start, Segs[ objnum].end);
      DrawScreenText( 5, 439, "Angle:       %d (%dø)", Segs[ objnum].angle, (int) (Segs[ objnum].angle * 0.0055));
      /* (*"flags1" and "flags2" should be replaced by a descriptive text string*) */
      DrawScreenText( 5, 449, "Flip flag:   %d", Segs[ objnum].flip);
      DrawScreenText( 5, 459, "Distance:    %d", Segs[ objnum].dist);
      DrawScreenText( 5, 469, "LineDef:     #%d", Segs[ objnum].linedef);
      break;
   case OBJ_NODES:
      DrawScreenBox3D( 0, 410, 340, 479);
      if (objnum < 0)
	break;
      setcolor( YELLOW);
      DrawScreenText( 5, 415, "Selected Node (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 429, "Coordinates: (%d, %d) + (%d, %d)", Nodes[ objnum].x, Nodes[ objnum].y, Nodes[ objnum].dx, Nodes[ objnum].dy);
      DrawScreenText( 5, 439, "Rectangle 1: %d, %d, %d, %d", Nodes[ objnum].minx1, Nodes[ objnum].miny1, Nodes[ objnum].maxx1, Nodes[ objnum].maxy1);
      DrawScreenText( 5, 449, "Rectangle 2: %d, %d, %d, %d", Nodes[ objnum].minx2, Nodes[ objnum].miny2, Nodes[ objnum].maxx2, Nodes[ objnum].maxy2);
      if (Nodes[ objnum].tree1 & 0x8000)
	 DrawScreenText( 5, 459, "Tree 1:      #%d (SSector)", Nodes[ objnum].tree1 & 0x7FFF);
      else
	 DrawScreenText( 5, 459, "Tree 1:      #%d (Node)", Nodes[ objnum].tree1);
      if (Nodes[ objnum].tree2 & 0x8000)
	 DrawScreenText( 5, 469, "Tree 2:      #%d (SSector)", Nodes[ objnum].tree2 & 0x7FFF);
      else
	 DrawScreenText( 5, 469, "Tree 2:      #%d (Node)", Nodes[ objnum].tree2);
      break;
   case OBJ_SECTORS:
      DrawScreenBox3D( 0, 390, 250, 479);
      if (objnum < 0)
	break;
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
      DrawScreenText( 5, 459, "Type:            %s", GetSectorTypeName( Sectors[ objnum].special));
      tag = Sectors[ objnum].tag;
      if (tag > 0)
      {
	 for (n = 0; n < NumLineDefs; n++)
	    if (LineDefs[ n].tag == tag)
	       break;
      }
      else
	 n = NumLineDefs;
      if (n < NumLineDefs)
	 DrawScreenText( 5, 469, "LineDef tag:     %d (#%d)", tag, n);
      else
	 DrawScreenText( 5, 469, "LineDef tag:     %d (none)", tag);
      break;
   }
}



/*
   delete an object (*should be recursive if a vertex is deleted*)
*/
void DeleteObject( int objtype, int objnum)
{
   int n;

   switch (objtype)
   {
   case OBJ_THINGS:
      NumThings--;
      for (n = objnum; n < NumThings; n++)
	Things[ n] = Things[ n + 1];
      Things = ResizeMemory( Things, NumThings * sizeof( struct Thing));
      break;
/*
   case OBJ_VERTEXES:
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
void InsertObject(int objtype, int copyfrom)
{
   int last;

   switch (objtype)
   {
   case OBJ_THINGS:
      last = NumThings++;
      Things = ResizeMemory( Things, NumThings * sizeof( struct Thing));
      Things[ last].xpos  = MAPX(PointerX);
      Things[ last].ypos  = MAPY(PointerY);
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
      LineDefs = ResizeMemory( LineDefs, NumLineDefs * sizeof( struct LineDef));
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
      SideDefs = ResizeMemory( SideDefs, NumSideDefs * sizeof( struct SideDef));
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
      break;
   case OBJ_VERTEXES:
      last = NumVertexes++;
      Vertexes = ResizeMemory( Vertexes, NumVertexes * sizeof( struct Vertex));
      Vertexes[ last].x  = MAPX(PointerX);
      Vertexes[ last].y  = MAPY(PointerY);
      break;
   case OBJ_SEGS:
      last = NumSegs++;
      Segs = ResizeMemory( Segs, NumSegs * sizeof( struct Seg));
      if (copyfrom >= 0)
      {
	 Segs[ last].start = Segs[ copyfrom].start;
	 Segs[ last].end = Segs[ copyfrom].end;
	 Segs[ last].angle = ComputeAngle(Vertexes[ Segs[ last].end].x - Vertexes[ Segs[ last].start].x,
					  Vertexes[ Segs[ last].end].y - Vertexes[ Segs[ last].start].y);
	 Segs[ last].linedef = Segs[ copyfrom].linedef;
	 Segs[ last].flip = Segs[ copyfrom].flip;
	 Segs[ last].dist = Segs[ copyfrom].dist;
      }
      else
      {
	 Segs[ last].start = 0;
	 Segs[ last].end = NumVertexes - 1;
	 Segs[ last].angle = ComputeAngle(Vertexes[ Segs[ last].end].x - Vertexes[ Segs[ last].start].x,
					  Vertexes[ Segs[ last].end].y - Vertexes[ Segs[ last].start].y);
	 Segs[ last].linedef = 0;
	 Segs[ last].flip = 0;
	 Segs[ last].dist = 0;
      }
      break;
   case OBJ_SSECTORS:
      last = NumSSectors++;
      SSectors = ResizeMemory( SSectors, NumSSectors * sizeof( struct SSector));
      SSectors[ last].num = 1;
      SSectors[ last].first = NumSegs - 1;
      break;
   case OBJ_SECTORS:
      last = NumSectors++;
      Sectors = ResizeMemory( Sectors, NumSectors * sizeof( struct Sector));
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
      if (typeid[ num] >= 9999)
	 menustr[ num] = "Enter a decimal value";
      else
	 menustr[ num] = GetLineDefTypeName( typeid[ num]);
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
int InputObjectXRef( int x0, int y0, int objtype, int allownone, int curobj)
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
			      "Decoration (light sources)",
			      "Decoration (dead bodies)",
			      "Decoration (hanging bodies)",
			      "Teleport landing",
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
				     THING_PILLARHEARTH,
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
	    val = InputIntegerValue( 84, 188, 0, 9999, Things[ objnum].type);
	    break;

	 default:
	    Beep();
	    return;
	 }
	 if (val >= 0)
	 {
	    Things[ objnum].type = val;
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
	    Things[ objnum].angle = 90;
	    MadeChanges = TRUE;
	    break;
	 case 2:
	    Things[ objnum].angle = 45;
	    MadeChanges = TRUE;
	    break;
	 case 3:
	    Things[ objnum].angle = 0;
	    MadeChanges = TRUE;
	    break;
	 case 4:
	    Things[ objnum].angle = 315;
	    MadeChanges = TRUE;
	    break;
	 case 5:
	    Things[ objnum].angle = 270;
	    MadeChanges = TRUE;
	    break;
	 case 6:
	    Things[ objnum].angle = 225;
	    MadeChanges = TRUE;
	    break;
	 case 7:
	    Things[ objnum].angle = 180;
	    MadeChanges = TRUE;
	    break;
	 case 8:
	    Things[ objnum].angle = 135;
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
	 sprintf( menustr[ 0], "Change Flags            (Current: %d)", LineDefs[ objnum].flags);
	 sprintf( menustr[ 1], "Change Type             (Current: %d)", LineDefs[ objnum].type);
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
	    val = InputIntegerValue( 84, 98, 0, 255, LineDefs[ objnum].flags);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].flags = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 2:
	    val = DisplayLineDefTypeMenu( 84, 108, "Choose a LineDef type:",
					  0, 1, 2, 7, 8, 11, 18, 20, 26, 27, 28, 32, 33, 34, 35, 36, 39, 46, 48, 51, 62, 70, 77, 88, 97,
					  9999,
					  -1);
	    if (val == 9999)
	       val = InputIntegerValue( 126, 392, 0, 255, LineDefs[ objnum].type);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].type = val;
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
	    val = InputObjectXRef( 84, 128, OBJ_VERTEXES, FALSE, LineDefs[ objnum].start);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].start = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 5:
	    val = InputObjectXRef( 84, 138, OBJ_VERTEXES, FALSE, LineDefs[ objnum].end);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].end = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 6:
	    val = InputObjectXRef( 84, 148, OBJ_SIDEDEFS, FALSE, LineDefs[ objnum].sidedef1);
	    if (val >= 0)
	    {
	       LineDefs[ objnum].sidedef1 = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 7:
	    val = InputObjectXRef( 84, 158, OBJ_SIDEDEFS, TRUE, LineDefs[ objnum].sidedef2);
	    if (val >= -1)
	    {
	       LineDefs[ objnum].sidedef2 = val;
	       MadeChanges = TRUE;
	    }
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
	    InputNameFromList( 84, 118, "Enter a wall texture name:", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       strncpy( SideDefs[ objnum].tex3, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 2:
	    strncpy( texname, SideDefs[ objnum].tex1, 8);
	    InputNameFromList( 84, 128, "Enter a wall texture name:", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       strncpy( SideDefs[ objnum].tex1, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 3:
	    strncpy( texname, SideDefs[ objnum].tex2, 8);
	    InputNameFromList( 84, 138, "Enter a wall texture name:", NumWTexture, WTexture, texname);
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
	    val = InputObjectXRef( 84, 168, OBJ_SECTORS, FALSE, SideDefs[ objnum].sector);
	    if (val >= 0)
	    {
	       SideDefs[ objnum].sector = val;
	       MadeChanges = TRUE;
	    }
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
      sprintf( menustr[ 0], "Change Flip flag       (Current: %d)", Segs[ objnum].flip);
      sprintf( menustr[ 1], "Change Distance        (Current: %d)", Segs[ objnum].dist);
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
	 val = InputIntegerValue( 42, 64, 0, 255, Segs[ objnum].dist);
	 if (val >= 0)
	 {
	    Segs[ objnum].dist = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 3:
	 NotImplemented(); /* (*ask for confirmation*) */
      case 4:
	 val = InputObjectXRef( 42, 74, OBJ_VERTEXES, FALSE, Segs[ objnum].start);
	 if (val >= 0)
	 {
	    Segs[ objnum].start = val;
	    Segs[ objnum].angle = ComputeAngle(Vertexes[ Segs[ objnum].end].x - Vertexes[ Segs[ objnum].start].x,
					       Vertexes[ Segs[ objnum].end].y - Vertexes[ Segs[ objnum].start].y);
	    MadeChanges = TRUE;
	 }
	 break;
      case 5:
	 val = InputObjectXRef( 42, 84, OBJ_VERTEXES, FALSE, Segs[ objnum].end);
	 if (val >= 0)
	 {
	    Segs[ objnum].end = val;
	    Segs[ objnum].angle = ComputeAngle(Vertexes[ Segs[ objnum].end].x - Vertexes[ Segs[ objnum].start].x,
					       Vertexes[ Segs[ objnum].end].y - Vertexes[ Segs[ objnum].start].y);
	    MadeChanges = TRUE;
	 }
	 break;
      case 6:
	 val = InputObjectXRef( 42, 94, OBJ_LINEDEFS, FALSE, Segs[ objnum].linedef);
	 if (val >= 0)
	 {
	    Segs[ objnum].linedef = val;
	    MadeChanges = TRUE;
	 }
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
      sprintf( menustr[ 5], "Change Type            (Current: %d)", Sectors[ objnum].special);
      sprintf( menustr[ 6], "Change LineDef tag     (Current: %d)", Sectors[ objnum].tag);
      val = DisplayMenuArray( 0, 30, menustr[ 7], 7, menustr);
      for (n = 0; n < 8; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( 42, 64, -512, 512, Sectors[ objnum].floorh);
	 if (val >= -264)
	 {
	    Sectors[ objnum].floorh = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 2:
	 val = InputIntegerValue( 42, 74, -512, 512, Sectors[ objnum].ceilh);
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



/*
   check if a (part of a) LineDef is inside a given block
*/

int IsLineDefInside( int ldnum, double x0, double y0, double x1, double y1)
{
   double lx0 = (double) Vertexes[ LineDefs[ ldnum].start].x;
   double ly0 = (double) Vertexes[ LineDefs[ ldnum].start].y;
   double lx1 = (double) Vertexes[ LineDefs[ ldnum].end].x;
   double ly1 = (double) Vertexes[ LineDefs[ ldnum].end].y;
   double i;

   /* do you like mathematics? */
   if ((ly0 > y0) != (ly1 > y0))
   {
      i = lx0 + (y0 - ly0) * (lx1 - lx0) / (ly1 - ly0);
      if (i >= x0 && i <= x1)
	 return TRUE;
   }
   if ((ly0 > y1) != (ly1 > y1))
   {
      i = lx0 + (y1 - ly0) * (lx1 - lx0) / (ly1 - ly0);
      if (i >= x0 && i <= x1)
	 return TRUE;
   }
   if ((lx0 > x0) != (lx1 > x0))
   {
      i = ly0 + (x0 - lx0) * (ly1 - ly0) / (lx1 - lx0);
      if (i >= y0 && i <= y1)
	 return TRUE;
   }
   if ((lx0 > x1) != (lx1 > x1))
   {
      i = ly0 + (x1 - lx0) * (ly1 - ly0) / (lx1 - lx0);
      if (i >= y0 && i <= y1)
	 return TRUE;
   }
   return FALSE;
}



/* end of file */
