/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   OBJECTS.C - object handling routines.
*/

/* the includes */
#include "deu.h"
#include "levels.h"


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

Bool IsSelected( SelPtr list, int objnum)
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


   if (objnum < 0)
      ProgError( "SelectObject called with %d (BUG!)", objnum);
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

   if (objnum < 0)
      ProgError( "UnSelectObject called with %d (BUG!)", objnum);
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
   int lx0, ly0, lx1, ly1, yy;
   int midx, midy;

   cur = -1;
   if (x1 < x0)
   {
      n = x0;
      x0 = x1;
      x1 = n;
   }
   if (y1 < y0)
   {
      n = y0;
      y0 = y1;
      y1 = n;
   }

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
   case OBJ_VERTEXES:
      for (n = 0; n < NumVertexes; n++)
	 if (Vertexes[ n].x >= x0 && Vertexes[ n].x <= x1 && Vertexes[ n].y >= y0 && Vertexes[ n].y <= y1)
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
   case OBJ_SECTORS:
      /* hack, hack...  I look for the first LineDef crossing an horizontal half-line drawn from the cursor */
      curx = MapMaxX + 1;
      cur = -1;
      midx = (x0 + x1) / 2;
      midy = (y0 + y1) / 2;
      for (n = 0; n < NumLineDefs; n++)
	 if ((Vertexes[ LineDefs[ n].start].y > midy != (Vertexes[ LineDefs[ n].end].y > midy)))
	 {
	    lx0 = Vertexes[ LineDefs[ n].start].x;
	    ly0 = Vertexes[ LineDefs[ n].start].y;
	    lx1 = Vertexes[ LineDefs[ n].end].x;
	    ly1 = Vertexes[ LineDefs[ n].end].y;
	    m = lx0 + (int) ((long) (midy - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
	    if (m >= midx && m < curx)
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
      break;
   }
   return cur;
}



/*
   select all objects inside a given box
*/

SelPtr SelectObjectsInBox( int objtype, int x0, int y0, int x1, int y1)
{
   int n, m;
   SelPtr list;

   list = NULL;
   if (x1 < x0)
   {
      n = x0;
      x0 = x1;
      x1 = n;
   }
   if (y1 < y0)
   {
      n = y0;
      y0 = y1;
      y1 = n;
   }

   switch (objtype)
   {
   case OBJ_THINGS:
      for (n = 0; n < NumThings; n++)
	 if (Things[ n].xpos >= x0 && Things[ n].xpos <= x1 && Things[ n].ypos >= y0 && Things[ n].ypos <= y1)
	    SelectObject( &list, n);
      break;
   case OBJ_VERTEXES:
      for (n = 0; n < NumVertexes; n++)
	 if (Vertexes[ n].x >= x0 && Vertexes[ n].x <= x1 && Vertexes[ n].y >= y0 && Vertexes[ n].y <= y1)
	    SelectObject( &list, n);
      break;
   case OBJ_LINEDEFS:
      for (n = 0; n < NumLineDefs; n++)
      {
	 /* the two ends of the line must be in the box */
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x < x0 || Vertexes[ m].x > x1 || Vertexes[ m].y < y0 || Vertexes[ m].y > y1)
	    continue;
	 m = LineDefs[ n].end;
	 if (Vertexes[ m].x < x0 || Vertexes[ m].x > x1 || Vertexes[ m].y < y0 || Vertexes[ m].y > y1)
	    continue;
	 SelectObject( &list, n);
      }
      break;
   case OBJ_SECTORS:
      /* hack: select all sectors... */
      for (n = 0; n < NumSectors; n++)
	 SelectObject( &list, n);
      /* ... then remove the unwanted ones from the list */
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].start;
	 if (Vertexes[ m].x < x0 || Vertexes[ m].x > x1 || Vertexes[ m].y < y0 || Vertexes[ m].y > y1)
	 {
	    m = LineDefs[ n].sidedef1;
	    if (m >= 0 && SideDefs[ m].sector >= 0)
	       UnSelectObject( &list, SideDefs[ m].sector);
	    m = LineDefs[ n].sidedef2;
	    if (m >= 0 && SideDefs[ m].sector >= 0)
	       UnSelectObject( &list, SideDefs[ m].sector);
	    continue;
	 }
	 m = LineDefs[ n].end;
	 if (Vertexes[ m].x < x0 || Vertexes[ m].x > x1 || Vertexes[ m].y < y0 || Vertexes[ m].y > y1)
	 {
	    m = LineDefs[ n].sidedef1;
	    if (m >= 0 && SideDefs[ m].sector >= 0)
	       UnSelectObject( &list, SideDefs[ m].sector);
	    m = LineDefs[ n].sidedef2;
	    if (m >= 0 && SideDefs[ m].sector >= 0)
	       UnSelectObject( &list, SideDefs[ m].sector);
	    continue;
	 }
      }
      break;
   }
   return list;
}



/*
   highlight the selected object
*/

void HighlightObject( int objtype, int objnum, int color)
{
   int  n, m;

   /* use XOR mode : drawing any line twice erases it */
   setwritemode( XOR_PUT);
   SetColor( color);
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
      DrawMapLine( n, m, n + (Vertexes[ LineDefs[ objnum].end].y - Vertexes[ LineDefs[ objnum].start].y) / 3, m + (Vertexes[ LineDefs[ objnum].start].x - Vertexes[ LineDefs[ objnum].end].x) / 3);
      setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
      DrawMapVector( Vertexes[ LineDefs[ objnum].start].x, Vertexes[ LineDefs[ objnum].start].y,
		     Vertexes[ LineDefs[ objnum].end].x, Vertexes[ LineDefs[ objnum].end].y);
      if (color != LIGHTRED && LineDefs[ objnum].tag > 0)
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
      if (color != LIGHTRED && Sectors[ objnum].tag > 0)
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
   delete an object
*/

void DeleteObject( int objtype, int objnum)
{
   SelPtr list;

   list = NULL;
   SelectObject( &list, objnum);
   DeleteObjects( objtype, &list);
}



/*
   delete a group of objects (*recursive*)
*/

void DeleteObjects( int objtype, SelPtr *list)
{
   int    n, objnum;
   SelPtr cur;

   MadeChanges = TRUE;
   switch (objtype)
   {
   case OBJ_THINGS:
      while (*list)
      {
	 objnum = (*list)->objnum;
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
	 for (cur = (*list)->next; cur; cur = cur->next)
	    if (cur->objnum > objnum)
	       cur->objnum--;
	 UnSelectObject( list, objnum);
      }
      break;
   case OBJ_VERTEXES:
      while (*list)
      {
	 objnum = (*list)->objnum;
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
	 for (cur = (*list)->next; cur; cur = cur->next)
	    if (cur->objnum > objnum)
	       cur->objnum--;
	 UnSelectObject( list, objnum);
      }
      break;
   case OBJ_LINEDEFS:
      while (*list)
      {
	 objnum = (*list)->objnum;
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
	 for (cur = (*list)->next; cur; cur = cur->next)
	    if (cur->objnum > objnum)
	       cur->objnum--;
	 UnSelectObject( list, objnum);
      }
      break;
   case OBJ_SIDEDEFS:
      while (*list)
      {
	 objnum = (*list)->objnum;
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
	 for (cur = (*list)->next; cur; cur = cur->next)
	    if (cur->objnum > objnum)
	       cur->objnum--;
	 UnSelectObject( list, objnum);
      }
      MadeMapChanges = TRUE;
      break;
   case OBJ_SECTORS:
      while (*list)
      {
	 objnum = (*list)->objnum;
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
	 for (cur = (*list)->next; cur; cur = cur->next)
	    if (cur->objnum > objnum)
	       cur->objnum--;
	 UnSelectObject( list, objnum);
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
      Vertexes[ last].x = xpos & ~7;
      Vertexes[ last].y = ypos & ~7;
      MadeMapChanges = TRUE;
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



/* end of file */
