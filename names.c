/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   NAMES.C - Object name and type routines.
*/

/* the includes */
#include "deu.h"


/*
   get the name of an object type
*/
char *GetObjectTypeName( int objtype)
{
   switch (objtype)
   {
   case OBJ_THINGS:
      return "Thing";
   case OBJ_LINEDEFS:
      return "LineDef";
   case OBJ_SIDEDEFS:
      return "SideDef";
   case OBJ_VERTEXES:
      return "Vertex";
   case OBJ_SEGS:
      return "Segment";
   case OBJ_SSECTORS:
      return "SSector";
   case OBJ_NODES:
      return "Node";
   case OBJ_SECTORS:
      return "Sector";
   case OBJ_REJECT:
      return "Reject";
   case OBJ_BLOCKMAP:
      return "Blockmap";
   }
   return "< Bug! >";
}



/*
   what are we editing?
*/
char *GetEditModeName( int objtype)
{
   switch (objtype)
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
   get a short (13 char.) description of the type of a linedef
*/

char *GetLineDefTypeName( int type)
{
   static char temp[ 20];

   switch (type)
   {
   case 0:
      return "Normal";
   case 1:
      return "Door";
   case 2:
      return "Raise floor";
   case 7:
   case 8:
      return "Raise stairs";
   case 11:
      return "End level";
   case 18:
      return "Raise floor";
   case 20:
      return "Raise floor 2";
   case 26:
      return "Blue door";
   case 27:
      return "Yellow door";
   case 28:
      return "Red door";
   case 32:
      return "Blue door 2";
   case 33:
      return "Yellow door 2";
   case 34:
      return "Red door 2";
   case 35:
      return "Lights out!";
   case 36:
      return "Lower floor";
   case 39:
      return "Teleport";
   case 46:
      return "Door - shoot";
   case 48:
      return "Animated wall";
   case 51:
      return "Secret level";
   case 62:
      return "Raise lift";
   case 70:
      return "Lower floor 2";
   case 77:
      return "Crushing ceil";
   case 88:
      return "Lower lift";
   case 97:
      return "Teleport 2";
   }

   sprintf( temp, "Unknown <%d>", type);
   return temp;
}



/*
   get a short description of the flags of a linedef
   (*should be changed - use text strings instead of binary*)
*/

char *GetLineDefFlagsName( int flags)
{
   int n;
   static char temp[ 20];

   for (n = 0; n < 8; n++)
   {
      if (flags & 0x01)
	 temp[ 7 - n] = '1';
      else
	 temp[ 7 - n] = '0';
      flags /= 2;
   }
   temp[ 8] = 0;
   return temp;
}



/*
   get a short (13 char.) description of the type of a sector
*/

char *GetSectorTypeName( int type)
{
   static char temp[ 20];

   switch (type)
   {
   case 0:
      return "Normal";
   case 1:
      return "Blinks random.";
   case 2:
      return "Quick pulsate";
   case 3:
      return "Blinks";
   case 4:
      return "Puslate & -20%";
   case 5:
      return "-10% health";
   case 7:
      return "-5% health";
   case 9:
      return "Secret";
   case 11:
      return "-20% health";
   }

   sprintf( temp, "Unknown <%d>", type);
   return temp;
}

/* end of file */
