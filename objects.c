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
extern int NumSSectors;	     	     /* number of subsectors */
extern SSPtr SSectors;               /* subsectors data */
extern int NumSectors;		     /* number of sectors */
extern SPtr Sectors;                 /* sectors data */

/*
   what are we editing?
*/
char *GetObjectType( int editmode)
{
   switch (editmode)
   {
   case OBJ_THINGS:
      return "Things";
   case OBJ_LINEDEFS:
   case OBJ_SIDEDEFS:
      return "LineDefs & SideDefs";
   case OBJ_VERTEXES:
      return "Vertexes";
   case OBJ_SEGS:
      return "Segments";
   case OBJ_SSECTORS:
      return "Sub-sectors";
   case OBJ_NODES:
      return "Nodes";
   case OBJ_SECTORS:
      return "Sectors";
   case OBJ_REJECT:
      return "Reject data";
   case OBJ_BLOCKMAP:
      return "Blockmap";
   }
   return "< Bug! >";
}


/*
   check if there is something of interest near the pointer
   (*bug: some lines cannot be selected with this method*)
*/

int GetCurObject( int objtype)
{
   int  x0 = MAPX(PointerX) - OBJSIZE;
   int  x1 = MAPX(PointerX) + OBJSIZE;
   int  y0 = MAPY(PointerY) - OBJSIZE;
   int  y1 = MAPY(PointerY) + OBJSIZE;
   int  n, m, cur;

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
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    cur = n;
	    break;
	 }
      }
      break;
/*
   case OBJ_SIDEDEFS:
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    cur = LineDefs[ n].sidedef1;
	    break;
	 }
      }
      break;
*/
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
   case OBJ_SECTORS:
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x >= x0 && Vertexes[ m].x <= x1 && Vertexes[ m].y >= y0 && Vertexes[ m].y <= y1)
	 {
	    cur = SideDefs[ LineDefs[ n].sidedef1].sector;
	    break;
	 }
      }
      break;
   }
   return cur;
}



/*
   highlight the selected object
*/

void HighlightObject( int objtype, int objnum)
{
   int  n;

   /* use XOR mode : drawing any line twice erases it */
   setwritemode( XOR_PUT);
   setcolor( YELLOW);
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
   case OBJ_SECTORS:
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      for (n = 0; n < NumLineDefs; n++)
	 if (SideDefs[ LineDefs[ n].sidedef1].sector == objnum || SideDefs[ LineDefs[ n].sidedef2].sector == objnum)
	    DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			 Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
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
      if (objnum < 0)
	break;
      setcolor( YELLOW);
      DrawScreenText( 5, 405, "Selected LineDef (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 419, "Vertexes:    (#%d, #%d)", LineDefs[ objnum].start, LineDefs[ objnum].end);
      /* (*"flags1" and "flags2" should be replaced by a descriptive text string*) */
      DrawScreenText( 5, 429, "Flags 1:     %d", LineDefs[ objnum].flags1);
      DrawScreenText( 5, 439, "Flags 2:     %d", LineDefs[ objnum].flags2);
      DrawScreenText( 5, 449, "Sector tag:  %d", LineDefs[ objnum].tag);
      DrawScreenText( 5, 459, "1st SideDef: #%d", LineDefs[ objnum].sidedef1);
      DrawScreenText( 5, 469, "2nd SideDef: #%d", LineDefs[ objnum].sidedef2);
      if (LineDefs[ objnum].sidedef1 >= 0)
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
	DrawScreenText( 225, 435, "(No sidedef information)");
      if (LineDefs[ objnum].sidedef2 >= 0)
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
	DrawScreenText( 445, 435, "(No sidedef information)");
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
      DrawScreenBox3D( 0, 410, 218, 479);
      if (objnum < 0)
	break;
      setcolor( YELLOW);
      DrawScreenText( 5, 415, "Selected LineDef (#%d)", objnum);
      setcolor( BLACK);
      DrawScreenText( 5, 429, "Vertexes:    (#%d, #%d)", Segs[ objnum].start, Segs[ objnum].end);
      DrawScreenText( 5, 439, "Angle:       %d", LineDefs[ objnum].tag);
      /* (*"flags1" and "flags2" should be replaced by a descriptive text string*) */
      DrawScreenText( 5, 449, "Flags 1:     %d", Segs[ objnum].flip);
      DrawScreenText( 5, 459, "Flags 2:     %d", Segs[ objnum].flags);
      DrawScreenText( 5, 469, "LineDef:     #%d", Segs[ objnum].linedef);
      break;
   case OBJ_SECTORS:
      DrawScreenBox3D( 0, 390, 220, 479);
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
      /* (*"special" should be replaced by a text string*) */
      DrawScreenText( 5, 459, "Special flags:   %d", Sectors[ objnum].special);
      DrawScreenText( 5, 469, "LineDef tag:     %d", Sectors[ objnum].tag);
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
   insert a new object (*if Num<Obj>s == 0, then copyfrom = -1!*)
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
      Things[ last].type  = Things[ copyfrom].type;
      Things[ last].angle = Things[ copyfrom].angle;
      Things[ last].when  = Things[ copyfrom].when;
      break;
/*
   case OBJ_VERTEXES:
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
