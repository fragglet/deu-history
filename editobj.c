/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   EDITOBJ.C - object editing routines.
*/

/* the includes */
#include "deu.h"
#include "levels.h"


/*
   display the information about one object
*/

void DisplayObjectInfo( int objtype, int objnum)
{
   char texname[ 9];
   int  tag, n;
   int  x0, y0;

   switch (objtype)
   {
   case OBJ_THINGS:
      x0 = 0;
      y0 = ScrMaxY - 60;
      if (InfoShown)
	 y0 -= 13;
      DrawScreenBox3D( x0, y0, x0 + 260, y0 + 60);
      if (objnum < 0)
      {
	 DrawScreenText( x0 + 60, y0 + 20, "Use the cursor to");
	 DrawScreenText( x0 + 72, y0 + 30, "select a Thing  ");
	 break;
      }
      SetColor( YELLOW);
      DrawScreenText( x0 + 5, y0 + 5, "Selected Thing (#%d)", objnum);
      SetColor( BLACK);
      DrawScreenText( -1, y0 + 20, "Coordinates:  (%d, %d)", Things[ objnum].xpos, Things[ objnum].ypos);
      DrawScreenText( -1, -1, "Type:         %s", GetThingName( Things[ objnum].type));
      DrawScreenText( -1, -1, "Angle:        %s", GetAngleName( Things[ objnum].angle));
      DrawScreenText( -1, -1, "Appears when: %s", GetWhenName( Things[ objnum].when));
      break;
   case OBJ_LINEDEFS:
      x0 = 0;
      y0 = ScrMaxY - 80;
      if (InfoShown)
	 y0 -= 13;
      DrawScreenBox3D(   x0, y0, x0 + 218, y0 + 80);
      if (objnum >= 0)
      {
	 SetColor( YELLOW);
	 DrawScreenText( x0 + 5, y0 + 5, "Selected LineDef (#%d)", objnum);
	 SetColor( BLACK);
	 DrawScreenText( -1, y0 + 20, "Vertexes:    (#%d, #%d)", LineDefs[ objnum].start, LineDefs[ objnum].end);
	 DrawScreenText( -1, -1, "Flags:%3d    %s", LineDefs[ objnum].flags, GetLineDefFlagsName( LineDefs[ objnum].flags));
	 DrawScreenText( -1, -1, "Type: %3d %s", LineDefs[ objnum].type, GetLineDefTypeName( LineDefs[ objnum].type));
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
	    DrawScreenText( -1, -1, "Sector tag:  %d (#%d)", tag, n);
	 else
	    DrawScreenText( -1, -1, "Sector tag:  %d (none)", tag);
	 DrawScreenText( -1, -1, "1st SideDef: #%d", LineDefs[ objnum].sidedef1);
	 DrawScreenText( -1, -1, "2nd SideDef: #%d", LineDefs[ objnum].sidedef2);
      }
      else
      {
	SetColor( DARKGRAY);
	DrawScreenText( x0 + 25, y0 + 35, "(No LineDef selected)");
      }
      x0 = 220;
      y0 = ScrMaxY - 80;
      if (InfoShown)
	 y0 -= 13;
      DrawScreenBox3D( x0, y0, x0 + 218, y0 + 80);
      if (objnum >= 0 && LineDefs[ objnum].sidedef1 >= 0)
      {
	 SetColor( YELLOW);
	 DrawScreenText( x0 + 5, y0 + 5, "First SideDef (#%d)", LineDefs[ objnum].sidedef1);
	 SetColor( BLACK);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef1].tex3, 8);
	 DrawScreenText( -1, y0 + 20, "Normal texture: %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef1].tex1, 8);
	 DrawScreenText( -1, -1, "Upper texture:  %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef1].tex2, 8);
	 DrawScreenText( -1, -1, "Lower texture:  %s", texname);
	 DrawScreenText( -1, -1, "Tex. X offset:  %d", SideDefs[ LineDefs[ objnum].sidedef1].xoff);
	 DrawScreenText( -1, -1, "Tex. Y offset:  %d", SideDefs[ LineDefs[ objnum].sidedef1].yoff);
	 DrawScreenText( -1, -1, "Sector:         #%d", SideDefs[ LineDefs[ objnum].sidedef1].sector);
      }
      else
      {
	SetColor( DARKGRAY);
	DrawScreenText( x0 + 25, y0 + 35, "(No first SideDef)");
      }
      x0 = 440;
      y0 = ScrMaxY - 80;
      if (InfoShown)
	 y0 -= 13;
      DrawScreenBox3D( x0, y0, x0 + 200, y0 + 80);
      if (objnum >= 0 && LineDefs[ objnum].sidedef2 >= 0)
      {
	 SetColor( YELLOW);
	 DrawScreenText( x0 + 5, y0 + 5, "Second SideDef (#%d)", LineDefs[ objnum].sidedef2);
	 SetColor( BLACK);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef2].tex3, 8);
	 DrawScreenText( -1, y0 + 20, "Normal texture: %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef2].tex1, 8);
	 DrawScreenText( -1, -1, "Upper texture:  %s", texname);
	 strncpy( texname, SideDefs[ LineDefs[ objnum].sidedef2].tex2, 8);
	 DrawScreenText( -1, -1, "Lower texture:  %s", texname);
	 DrawScreenText( -1, -1, "Tex. X offset:  %d", SideDefs[ LineDefs[ objnum].sidedef2].xoff);
	 DrawScreenText( -1, -1, "Tex. Y offset:  %d", SideDefs[ LineDefs[ objnum].sidedef2].yoff);
	 DrawScreenText( -1, -1, "Sector:         #%d", SideDefs[ LineDefs[ objnum].sidedef2].sector);
      }
      else
      {
	SetColor( DARKGRAY);
	DrawScreenText( x0 + 25, y0 + 35, "(No second SideDef)");
      }
      break;
   case OBJ_VERTEXES:
      x0 = 0;
      y0 = ScrMaxY - 30;
      if (InfoShown)
	 y0 -= 13;
      DrawScreenBox3D( x0, y0, x0 + 220, y0 + 30);
      if (objnum < 0)
      {
	 SetColor( DARKGRAY);
	 DrawScreenText( x0 + 30, y0 + 12, "(No Vertex selected)");
	 break;
      }
      SetColor( YELLOW);
      DrawScreenText( x0 + 5, y0 + 5, "Selected Vertex (#%d)", objnum);
      SetColor( BLACK);
      DrawScreenText( -1, y0 + 20, "Coordinates: (%d, %d)", Vertexes[ objnum].x, Vertexes[ objnum].y);
      break;
   case OBJ_SECTORS:
      x0 = 0;
      y0 = ScrMaxY - 90;
      if (InfoShown)
	 y0 -= 13;
      DrawScreenBox3D( x0, y0, x0 + 255, y0 + 90);
      if (objnum < 0)
      {
	SetColor( DARKGRAY);
	DrawScreenText( x0 + 48, y0 + 35, "(No Sector selected)");
	break;
      }
      SetColor( YELLOW);
      DrawScreenText( x0 + 5, y0 + 5, "Selected Sector (#%d)", objnum);
      SetColor( BLACK);
      DrawScreenText( -1, y0 + 20, "Floor height:    %d", Sectors[ objnum].floorh);
      DrawScreenText( -1, -1, "Ceiling height:  %d", Sectors[ objnum].ceilh);
      texname[ 8] = '\0';
      strncpy( texname, Sectors[ objnum].floort, 8);
      DrawScreenText( -1, -1, "Floor texture:   %s", texname);
      strncpy( texname, Sectors[ objnum].ceilt, 8);
      DrawScreenText( -1, -1, "Ceiling texture: %s", texname);
      DrawScreenText( -1, -1, "Light level:     %d", Sectors[ objnum].light);
      DrawScreenText( -1, -1, "Type: %3d        %s", Sectors[ objnum].special, GetSectorTypeName( Sectors[ objnum].special));
      tag = Sectors[ objnum].tag;
      if (tag == 0)
	 n = NumLineDefs;
      else
	 for (n = 0; n < NumLineDefs; n++)
	    if (LineDefs[ n].tag == tag)
	       break;
      if (n < NumLineDefs)
	 DrawScreenText( -1, -1, "LineDef tag:     %d (#%d)", tag, n);
      else if (tag == 99 || tag == 999)
	 DrawScreenText( -1, -1, "LineDef tag:     %d (stairs?)", tag);
      else if (tag == 666)
	 DrawScreenText( -1, -1, "LineDef tag:     %d (lower@end)", tag);
      else
	 DrawScreenText( -1, -1, "LineDef tag:     %d (none)", tag);
      break;
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
   val = DisplayMenuArray( x0, y0, menutitle, num, NULL, menustr) - 1;

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
   val = DisplayMenuArray( x0, y0, menutitle, num, NULL, menustr) - 1;

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
   if (x0 < 0)
      x0 = (ScrMaxX - 25 - 8 * strlen( prompt)) / 2;
   if (y0 < 0)
      y0 = (ScrMaxY - 55) / 2;
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * strlen( prompt), y0 + 55);
   SetColor( WHITE);
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
   if (x0 < 0)
      x0 = (ScrMaxX - 25 - 8 * val) / 2;
   if (y0 < 0)
      y0 = (ScrMaxY - (allownone ? 85 : 75)) / 2;
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * val, y0 + (allownone ? 85 : 75));
   SetColor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, prompt);
   if (allownone)
      DrawScreenText( x0 + 10, y0 + 18, "or -1 for none:");
   SetColor( RED);
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
      x0 = (ScrMaxX - 25 - 8 * maxlen) / 2;
   if (y0 < 0)
      y0 = (ScrMaxY - 75) / 2;
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * maxlen, y0 + 75);
   DrawScreenText( x0 + 10, y0 + 36, "From this Vertex");
   DrawScreenText( x0 + 180, y0 + 36, "To this Vertex");
   SetColor( WHITE);
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
	 SetColor( DARKGRAY);
	 ok = FALSE;
      }
      DrawScreenText( x0 + 14, y0 + 51, "%d", *v1);
      DrawScreenBox3D( x0 + 180, y0 + 48, x0 + 241, y0 + 61);
      if (*v2 < 0 || *v2 >= NumVertexes)
      {
	 SetColor( DARKGRAY);
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

void EditObjectsInfo( int x0, int y0, int objtype, SelPtr obj)
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
      val = DisplayMenuArray( x0, y0, menustr[ 5], 5, NULL, menustr);
      for (n = 0; n < 6; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 switch (DisplayMenu( x0 + 42, y0 + 34, "Select Class",
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
	    val = DisplayThingsMenu( x0 + 84, y0 + 68, "Select Start Position Type",
				     THING_PLAYER1,
				     THING_PLAYER2,
				     THING_PLAYER3,
				     THING_PLAYER4,
				     THING_DEATHMATCH,
				     -1);
	    break;

	 case 2:
	    val = DisplayThingsMenu( x0 + 84, y0 + 78, "Select Enemy",
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
	    val = DisplayThingsMenu( x0 + 84, y0 + 88, "Select Weapon",
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
	    val = DisplayThingsMenu( x0 + 84, y0 + 98, "Select Bonus",
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
	    val = DisplayThingsMenu( x0 + 84, y0 + 108, "Select Decoration",
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
	    val = DisplayThingsMenu( x0 + 84, y0 + 118, "Select Decoration",
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
	    val = DisplayThingsMenu( x0 + 84, y0 + 128, "Select Decoration",
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
	    val = DisplayThingsMenu( x0 + 84, y0 + 138, "Select Decoration",
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
	    val = InputIntegerValue( x0 + 84, y0 + 158, 0, 9999, Things[ obj->objnum].type);
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
	 switch (DisplayMenu( x0 + 42, y0 + 44, "Select Angle",
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
	 val = DisplayMenu( x0 + 42, y0 + 54, "Choose the difficulty level(s)",
			    "D12          (Easy only)",
			    "D3           (Medium only)",
			    "D12, D3      (Easy and Medium)",
			    "D45          (Hard only)",
			    "D12, D45     (Easy and Hard)",
			    "D3, D45      (Medium and Hard)",
			    "D12, D3, D45 (Easy, Medium, Hard)",
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
	    val = InputIntegerValue( x0 + 84, y0 + 158, 1, 31, Things[ obj->objnum].when);
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
	 val = InputIntegerValue( x0 + 42, y0 + 64, MapMinX, MapMaxX, Things[ obj->objnum].xpos);
	 if (val >= MapMinX)
	 {
	    n = val - Things[ obj->objnum].xpos;
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].xpos += n;
	    MadeChanges = TRUE;
	 }
	 break;

      case 5:
	 val = InputIntegerValue( x0 + 42, y0 + 74, MapMinY, MapMaxY, Things[ obj->objnum].ypos);
	 if (val >= MapMinY)
	 {
	    n = val - Things[ obj->objnum].ypos;
	    for (cur = obj; cur; cur = cur->next)
	       Things[ cur->objnum].ypos += n;
	    MadeChanges = TRUE;
	 }
	 break;
      }
      break;

   case OBJ_VERTEXES:
      for (n = 0; n < 3; n++)
	 menustr[ n] = GetMemory( 60);
      sprintf( menustr[ 2], "Edit Vertex #%d", obj->objnum);
      sprintf( menustr[ 0], "Change X position (Current: %d)", Vertexes[ obj->objnum].x);
      sprintf( menustr[ 1], "Change Y position (Current: %d)", Vertexes[ obj->objnum].y);
      val = DisplayMenuArray( 0, 30, menustr[ 2], 2, NULL, menustr);
      for (n = 0; n < 3; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( x0 + 42, y0 + 34, MapMinX, MapMaxX, Vertexes[ obj->objnum].x);
	 if (val >= MapMinX)
	 {
	    n = val - Vertexes[ obj->objnum].x;
	    for (cur = obj; cur; cur = cur->next)
	       Vertexes[ cur->objnum].x += n;
	    MadeChanges = TRUE;
	    MadeMapChanges = TRUE;
	 }
	 break;

      case 2:
	 val = InputIntegerValue( x0 + 42, y0 + 44, MapMinY, MapMaxY, Vertexes[ obj->objnum].y);
	 if (val >= MapMinY)
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

   case OBJ_LINEDEFS:
      switch (DisplayMenu( x0, y0, "Choose the object to edit:",
			   "Edit the LineDef",
			   (LineDefs[ obj->objnum].sidedef1 >= 0) ? "Edit the 1st SideDef" : "Add a 1st SideDef",
			   (LineDefs[ obj->objnum].sidedef2 >= 0) ? "Edit the 2nd SideDef" : "Add a 2nd SideDef",
			   NULL))
      {
      case 1:
	 for (n = 0; n < 8; n++)
	    menustr[ n] = GetMemory( 60);
	 sprintf( menustr[ 7], "Edit LineDef #%d", obj->objnum);
	 sprintf( menustr[ 0], "Change Flags            (Current: %d)", LineDefs[ obj->objnum].flags);
	 sprintf( menustr[ 1], "Change Type             (Current: %d)", LineDefs[ obj->objnum].type);
	 sprintf( menustr[ 2], "Change Sector tag       (Current: %d)", LineDefs[ obj->objnum].tag);
	 sprintf( menustr[ 3], "Change Starting Vertex  (Current: #%d)", LineDefs[ obj->objnum].start);
	 sprintf( menustr[ 4], "Change Ending Vertex    (Current: #%d)", LineDefs[ obj->objnum].end);
	 sprintf( menustr[ 5], "Change 1st SideDef ref. (Current: #%d)", LineDefs[ obj->objnum].sidedef1);
	 sprintf( menustr[ 6], "Change 2nd SideDef ref. (Current: #%d)", LineDefs[ obj->objnum].sidedef2);
	 val = DisplayMenuArray( x0 + 42, y0 + 34, menustr[ 7], 7, NULL, menustr);
	 for (n = 0; n < 8; n++)
	    free( menustr[ n]);
	 switch (val)
	 {
	 case 1:
	    val = DisplayMenu( x0 + 84, y0 + 68, "Toggle the flags:",
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
	       val = InputIntegerValue( x0 + 126, y0 + 182, 0, 511, LineDefs[ obj->objnum].flags);
	       if (val >= 0)
	       {
		  for (cur = obj; cur; cur = cur->next)
		     LineDefs[ cur->objnum].flags = val;
		  MadeChanges = TRUE;
	       }
	    }
	    break;
	 case 2:
	    switch (DisplayMenu( x0 + 84, y0 + 78, "Choose a LineDef type:",
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
	       val = DisplayLineDefTypeMenu( x0 + 126, y0 + 122, NULL, /* Doors */
					     1, 26, 27, 28, 63, 29, 90, 31, 32, 34, 33, 103, 86, 2, 46, 42, 75, 3, 76, 16,
					     -1);
	       break;
	    case 3:
	       val = DisplayLineDefTypeMenu( x0 + 126, y0 + 132, NULL, /* Ceilings */
					     41, 44, 40,
					     -1);
	       break;
	    case 4:
	       val = DisplayLineDefTypeMenu( x0 + 126, y0 + 142, NULL, /* Floors */
					     102, 70, 23, 9, 21, 82, 19, 38, 37, 98, 36, 18, 20, 14, 5, 22, 59, 30, 58, 91, 56,
					     -1);
	       break;
	    case 5:
	       val = DisplayLineDefTypeMenu( x0 + 126, y0 + 152, NULL, /* Lifts & Moving things */
					     62, 88, 10, 77, 73, 74, 87, 89, 7, 8,
					     -1);
	       break;
	    case 6:
	       val = DisplayLineDefTypeMenu( x0 + 126, y0 + 162, NULL, /* Special */
					     48, 11, 52, 51, 97, 39, 13, 35, 80, 104,
					     -1);
	       break;
	    case 7:
	       val = InputIntegerValue( x0 + 126, y0 + 172, 0, 255, LineDefs[ obj->objnum].type);
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
	    val = InputIntegerValue( x0 + 84, y0 + 88, 0, 255, LineDefs[ obj->objnum].tag);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].tag = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 4:
	    val = InputObjectXRef( x0 + 84, y0 + 98, OBJ_VERTEXES, FALSE, LineDefs[ obj->objnum].start);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].start = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    break;
	 case 5:
	    val = InputObjectXRef( x0 + 84, y0 + 108, OBJ_VERTEXES, FALSE, LineDefs[ obj->objnum].end);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].end = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    break;
	 case 6:
	    val = InputObjectXRef( x0 + 84, y0 + 118, OBJ_SIDEDEFS, FALSE, LineDefs[ obj->objnum].sidedef1);
	    if (val >= 0)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].sidedef1 = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    break;
	 case 7:
	    val = InputObjectXRef( x0 + 84, y0 + 128, OBJ_SIDEDEFS, TRUE, LineDefs[ obj->objnum].sidedef2);
	    if (val >= -1)
	    {
	       for (cur = obj; cur; cur = cur->next)
		  LineDefs[ cur->objnum].sidedef2 = val;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
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
		  LineDefs[ cur->objnum].sidedef1 = NumSideDefs - 1;
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
		     LineDefs[ cur->objnum].sidedef1 = NumSideDefs - 1;
		  }
		  else if (LineDefs[ cur->objnum].sidedef2 == -1)
		  {
		     InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		     LineDefs[ cur->objnum].sidedef2 = NumSideDefs - 1;
		     LineDefs[ cur->objnum].flags = 4;
		     strncpy( SideDefs[ NumSideDefs - 1].tex3, "-", 8);
		     strncpy( SideDefs[ LineDefs[ cur->objnum].sidedef1].tex3, "-", 8);
		  }
	       break;
	    }
	 }
	 for (n = 0; n < 7; n++)
	    menustr[ n] = GetMemory( 60);
	 sprintf( menustr[ 6], "Edit SideDef #%d", sdlist->objnum);
	 texname[ 8] = '\0';
	 strncpy( texname, SideDefs[ sdlist->objnum].tex3, 8);
	 sprintf( menustr[ 0], "Change Normal Texture   (Current: %s)", texname);
	 strncpy( texname, SideDefs[ sdlist->objnum].tex1, 8);
	 sprintf( menustr[ 1], "Change Upper texture    (Current: %s)", texname);
	 strncpy( texname, SideDefs[ sdlist->objnum].tex2, 8);
	 sprintf( menustr[ 2], "Change Lower texture    (Current: %s)", texname);
	 sprintf( menustr[ 3], "Change Texture X offset (Current: %d)", SideDefs[ sdlist->objnum].xoff);
	 sprintf( menustr[ 4], "Change Texture Y offset (Current: %d)", SideDefs[ sdlist->objnum].yoff);
	 sprintf( menustr[ 5], "Change Sector ref.      (Current: #%d)", SideDefs[ sdlist->objnum].sector);
	 val = DisplayMenuArray( x0 + 42, y0 + 54, menustr[ 6], 6, NULL, menustr);
	 for (n = 0; n < 7; n++)
	    free( menustr[ n]);
	 switch (val)
	 {
	 case 1:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex3, 8);
	    ChooseWallTexture( x0 + 84, y0 + 88, "Choose a wall texture", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex3, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 2:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex1, 8);
	    ChooseWallTexture( x0 + 84, y0 + 98, "Choose a wall texture", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex1, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 3:
	    strncpy( texname, SideDefs[ sdlist->objnum].tex2, 8);
	    ChooseWallTexture( x0 + 84, y0 + 108, "Choose a wall texture", NumWTexture, WTexture, texname);
	    if (strlen(texname) > 0)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     strncpy( SideDefs[ cur->objnum].tex2, texname, 8);
	       MadeChanges = TRUE;
	    }
	    break;
	 case 4:
	    val = InputIntegerValue( x0 + 84, y0 + 118, -100, 100, SideDefs[ sdlist->objnum].xoff);
	    if (val >= -100)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     SideDefs[ cur->objnum].xoff = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 5:
	    val = InputIntegerValue( x0 + 84, y0 + 128, -100, 100, SideDefs[ sdlist->objnum].yoff);
	    if (val >= -100)
	    {
	       for (cur = sdlist; cur; cur = cur->next)
		  if (cur->objnum >= 0)
		     SideDefs[ cur->objnum].yoff = val;
	       MadeChanges = TRUE;
	    }
	    break;
	 case 6:
	    val = InputObjectXRef( x0 + 84, y0 + 138, OBJ_SECTORS, FALSE, SideDefs[ sdlist->objnum].sector);
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
      val = DisplayMenuArray( x0, y0, menustr[ 7], 7, NULL, menustr);
      for (n = 0; n < 8; n++)
	 free( menustr[ n]);
      switch (val)
      {
      case 1:
	 val = InputIntegerValue( x0 + 42, y0 + 34, -511, 511, Sectors[ obj->objnum].floorh);
	 if (val >= -511)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].floorh = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 2:
	 val = InputIntegerValue( x0 + 42, y0 + 44, -511, 511, Sectors[ obj->objnum].ceilh);
	 if (val >= -511)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].ceilh = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 3:
	 strncpy( texname, Sectors[ obj->objnum].floort, 8);
	 ChooseFloorTexture( x0 + 42, y0 + 54, "Choose a floor texture", NumFTexture, FTexture, texname);
	 if (strlen(texname) > 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       strncpy( Sectors[ cur->objnum].floort, texname, 8);
	    MadeChanges = TRUE;
	 }
	 break;
      case 4:
	 strncpy( texname, Sectors[ obj->objnum].ceilt, 8);
	 ChooseFloorTexture( x0 + 42, y0 + 64, "Choose a ceiling texture", NumFTexture, FTexture, texname);
	 if (strlen(texname) > 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       strncpy( Sectors[ cur->objnum].ceilt, texname, 8);
	    MadeChanges = TRUE;
	 }
	 break;
      case 5:
	 val = InputIntegerValue( x0 + 42, y0 + 74, 0, 255, Sectors[ obj->objnum].light);
	 if (val >= 0)
	 {
	    for (cur = obj; cur; cur = cur->next)
	       Sectors[ cur->objnum].light = val;
	    MadeChanges = TRUE;
	 }
	 break;
      case 6:
	 val = DisplayMenu( x0 + 42, y0 + 84, "Choose a special behaviour",
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
	    val = InputIntegerValue( x0 + 84, y0 + 208, 0, 255, Sectors[ obj->objnum].special);
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
	 val = InputIntegerValue( x0 + 42, y0 + 94, 0, 999, Sectors[ obj->objnum].tag);
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
   move a group of objects to a new position
   (must be called with obj = NULL before moving the objects)
*/

Bool MoveObjectsToCoords( int objtype, SelPtr obj, int newx, int newy, int grid)
{
   int        n, m;
   int        dx, dy;
   SelPtr     cur, vertices;
   static int refx, refy;

   if (grid > 0)
   {
      newx = (newx + grid / 2) & ~(grid - 1);
      newy = (newy + grid / 2) & ~(grid - 1);
   }
   /* only update the reference point? */
   if (obj == NULL)
   {
      refx = newx;
      refy = newy;
      return TRUE;
   }
   /* compute the displacement */
   dx = newx - refx;
   dy = newy - refy;
   /* nothing to do? */
   if (dx == 0 && dy == 0)
      return FALSE;

   /* move the object(s) */
   switch (objtype)
   {
      case OBJ_THINGS:
	 for (cur = obj; cur; cur = cur->next)
	    Things[ cur->objnum].xpos += dx;
	 for (cur = obj; cur; cur = cur->next)
	    Things[ cur->objnum].ypos += dy;
	 refx = newx;
	 refy = newy;
	 MadeChanges = TRUE;
	 break;
      case OBJ_VERTEXES:
	 for (cur = obj; cur; cur = cur->next)
	    Vertexes[ cur->objnum].x += dx;
	 for (cur = obj; cur; cur = cur->next)
	    Vertexes[ cur->objnum].y += dy;
	 refx = newx;
	 refy = newy;
	 MadeChanges = TRUE;
	 MadeMapChanges = TRUE;
	 break;
      case OBJ_LINEDEFS:
	 vertices = NULL;
	 for (cur = obj; cur; cur = cur->next)
	 {
	    if (!IsSelected( vertices, LineDefs[ cur->objnum].start))
	       SelectObject( &vertices, LineDefs[ cur->objnum].start);
	    if (!IsSelected( vertices, LineDefs[ cur->objnum].end))
	       SelectObject( &vertices, LineDefs[ cur->objnum].end);
	 }
	 MoveObjectsToCoords( OBJ_VERTEXES, vertices, newx, newy, grid);
	 ForgetSelection( &vertices);
	 break;
      case OBJ_SECTORS:
	 vertices = NULL;
	 for (cur = obj; cur; cur = cur->next)
	 {
	    for (n = 0; n < NumLineDefs; n++)
	       if (((m = LineDefs[ n].sidedef1) >= 0 && SideDefs[ m].sector == cur->objnum)
		|| ((m = LineDefs[ n].sidedef2) >= 0 && SideDefs[ m].sector == cur->objnum))
	       {
		  if (!IsSelected( vertices, LineDefs[ n].start))
		     SelectObject( &vertices, LineDefs[ n].start);
		  if (!IsSelected( vertices, LineDefs[ n].end))
		     SelectObject( &vertices, LineDefs[ n].end);
	       }
	 }
	 MoveObjectsToCoords( OBJ_VERTEXES, vertices, newx, newy, grid);
	 ForgetSelection( &vertices);
	 break;
   }
   return TRUE;
}



/*
   ... Yuck!
*/
int Input2Numbers( int x0, int y0, char *name1, char *name2, int v1max, int v2max, int *v1, int *v2)
{
   int  val, key;
   int  maxlen, first;
   Bool ok;
   char prompt[ 80];

   if (UseMouse)
      HideMousePointer();
   sprintf( prompt, "Give the %s and %s for the new Sector:", name1, name2);
   maxlen = strlen( prompt);
   if (x0 < 0)
      x0 = (ScrMaxX - 25 - 8 * maxlen) / 2;
   if (y0 < 0)
      y0 = (ScrMaxY - 75) / 2;
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * maxlen, y0 + 75);
   DrawScreenText( x0 + 10, y0 + 26, name1);
   DrawScreenText( x0 + 180, y0 + 26, name2);
   DrawScreenText( x0 + 10, y0 + 58, "(0-%d)", v1max);
   DrawScreenText( x0 + 180, y0 + 58, "(0-%d)", v2max);
   SetColor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, prompt);
   first = TRUE;
   key = 0;
   for (;;)
   {
      ok = TRUE;
      DrawScreenBox3D( x0 + 10, y0 + 38, x0 + 71, y0 + 51);
      if (*v1 < 0 || *v1 > v1max)
      {
	 SetColor( DARKGRAY);
	 ok = FALSE;
      }
      DrawScreenText( x0 + 14, y0 + 41, "%d", *v1);
      DrawScreenBox3D( x0 + 180, y0 + 38, x0 + 241, y0 + 51);
      if (*v2 < 0 || *v2 > v2max)
      {
	 SetColor( DARKGRAY);
	 ok = FALSE;
      }
      DrawScreenText( x0 + 184, y0 + 41, "%d", *v2);
      if (first)
	 key = InputInteger( x0 + 10, y0 + 38, v1, 0, v1max);
      else
	 key = InputInteger( x0 + 180, y0 + 38, v2, 0, v2max);
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
   find a free tag number
*/

int FindFreeTag()
{
   int  tag, n;
   Bool ok;

   tag = 1;
   ok = FALSE;
   while (! ok)
   {
      ok = TRUE;
      for (n = 0; n < NumLineDefs; n++)
	 if (LineDefs[ n].tag == tag)
	 {
	    ok = FALSE;
	    break;
	 }
      if (ok)
	 for (n = 0; n < NumSectors; n++)
	    if (Sectors[ n].tag == tag)
	    {
	       ok = FALSE;
	       break;
	    }
      tag++;
   }
   return tag;
}



/*
   insert a standard object at given position
*/

void InsertStandardObject( int x0, int y0, int xpos, int ypos)
{
   int sector;
   int choice;
   int n;
   int a, b;

   sector = GetCurObject( OBJ_SECTORS, xpos, ypos, xpos, ypos);
   if (sector >= 0)
      choice = DisplayMenu( x0, y0, "Insert a pre-defined object (inside a Sector)",
			    "Rectangle",
			    "Polygon (N sides)",
			    "Door |",
			    "Door -",
			    "Stairs |",
			    "Stairs -",
			    "Hidden stairs |",
			    "Hidden stairs -",
			    NULL);
   else
      choice = DisplayMenu( x0, y0, "Insert a pre-defined object (outside)",
			    "Rectangle",
			    "Polygon (N sides)",
			    NULL);
   /* !!!! Should also check for overlapping objects (sectors) !!!! */
   switch (choice)
   {
   case 1:
      a = 256;
      b = 128;
      if (Input2Numbers( -1, -1, "Width", "Height", 2000, 2000, &a, &b))
      {
	 xpos = xpos - a / 2;
	 ypos = ypos - b / 2;
	 InsertObject( OBJ_VERTEXES, -1, xpos, ypos);
	 InsertObject( OBJ_VERTEXES, -1, xpos + a, ypos);
	 InsertObject( OBJ_VERTEXES, -1, xpos + a, ypos + b);
	 InsertObject( OBJ_VERTEXES, -1, xpos, ypos + b);
	 if (sector < 0)
	    InsertObject( OBJ_SECTORS, -1, 0, 0);
	 for (n = 0; n < 4; n++)
	 {
	    InsertObject( OBJ_LINEDEFS, -1, 0, 0);
	    InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
	    LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs - 1;
	    if (sector >= 0)
	       SideDefs[ NumSideDefs - 1].sector = sector;
	 }
	 if (sector >= 0)
	 {
	    LineDefs[ NumLineDefs - 4].start = NumVertexes - 4;
	    LineDefs[ NumLineDefs - 4].end = NumVertexes - 3;
	    LineDefs[ NumLineDefs - 3].start = NumVertexes - 3;
	    LineDefs[ NumLineDefs - 3].end = NumVertexes - 2;
	    LineDefs[ NumLineDefs - 2].start = NumVertexes - 2;
	    LineDefs[ NumLineDefs - 2].end = NumVertexes - 1;
	    LineDefs[ NumLineDefs - 1].start = NumVertexes - 1;
	    LineDefs[ NumLineDefs - 1].end = NumVertexes - 4;
	 }
	 else
	 {
	    LineDefs[ NumLineDefs - 4].start = NumVertexes - 1;
	    LineDefs[ NumLineDefs - 4].end = NumVertexes - 2;
	    LineDefs[ NumLineDefs - 3].start = NumVertexes - 2;
	    LineDefs[ NumLineDefs - 3].end = NumVertexes - 3;
	    LineDefs[ NumLineDefs - 2].start = NumVertexes - 3;
	    LineDefs[ NumLineDefs - 2].end = NumVertexes - 4;
	    LineDefs[ NumLineDefs - 1].start = NumVertexes - 4;
	    LineDefs[ NumLineDefs - 1].end = NumVertexes - 1;
	 }
      }
      break;
   case 2:
      a = 8;
      b = 128;
      if (Input2Numbers( -1, -1, "Number of sides", "Radius", 32, 2000, &a, &b))
      {
	 InsertPolygonVertices( xpos, ypos, a, b);
	 if (sector < 0)
	    InsertObject( OBJ_SECTORS, -1, 0, 0);
	 for (n = 0; n < a; n++)
	 {
	    InsertObject( OBJ_LINEDEFS, -1, 0, 0);
	    InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
	    LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs - 1;
	    if (sector >= 0)
	       SideDefs[ NumSideDefs - 1].sector = sector;
	 }
	 if (sector >= 0)
	 {
	    LineDefs[ NumLineDefs - 1].start = NumVertexes - 1;
	    LineDefs[ NumLineDefs - 1].end = NumVertexes - a;
	    for (n = 2; n <= a; n++)
	    {
	       LineDefs[ NumLineDefs - n].start = NumVertexes - n;
	       LineDefs[ NumLineDefs - n].end = NumVertexes - n + 1;
	    }
	 }
	 else
	 {
	    LineDefs[ NumLineDefs - 1].start = NumVertexes - a;
	    LineDefs[ NumLineDefs - 1].end = NumVertexes - 1;
	    for (n = 2; n <= a; n++)
	    {
	       LineDefs[ NumLineDefs - n].start = NumVertexes - n + 1;
	       LineDefs[ NumLineDefs - n].end = NumVertexes - n;
	    }
	 }
      }
      break;
   case 3:
   case 4:
   case 5:
   case 6:
   case 7:
   case 8:
     NotImplemented();
     break;
   }
}



/*
   menu of miscellaneous operations
*/

void MiscOperations( int x0, int y0, int objtype, SelPtr obj)
{
   char   temp[ 80];
   int    val, n, m;
   SelPtr cur;

   /* check if obj != NULL */
   if (obj && objtype == OBJ_VERTEXES)
   {
      val = DisplayMenu( x0, y0, "Misc. Operations",
			 "Find first free tag number",
			 "Check if all Sectors are closed",
			 "Check all cross-references",
			 "Delete Vertex and join LineDefs",
			 NULL);
   }
   else if (obj && objtype == OBJ_LINEDEFS)
   {
      val = DisplayMenu( x0, y0, "Misc. Operations",
			 "Find first free tag number",
			 "Check if all Sectors are closed",
			 "Check all cross-references",
			 "Split LineDef (add new Vertex)",
			 "Flip LineDef",
			 "Align textures",
			 NULL);
   }
   else
   {
      val = DisplayMenu( x0, y0, "Misc. Operations",
			 "Find first free tag number",
			 "Check if all Sectors are closed",
			 "Check all cross-references",
			 NULL);
   }
   switch (val)
   {
   case 1:
      sprintf( temp, "First free tag number: %d", FindFreeTag());
      Notify( -1, -1, temp, NULL);
      break;
   case 2:
      NotImplemented();
      break;
   case 3:
      for (n = 0; n < NumLineDefs; n++)
	 if (LineDefs[ n].sidedef1 < 0)
	 {
	    Beep();
	    sprintf( temp, "ERROR: LineDef %d has no first SideDef!", n);
	    Notify( -1, -1, temp, "Doom will crash if you play with this level.");
	    return;
	 }
      cur = NULL;
      for (n = 0; n < NumVertexes; n++)
	 SelectObject( &cur, n);
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].start;
	 if (m >= 0)
	    UnSelectObject( &cur, m);
	 m = LineDefs[ n].end;
	 if (m >= 0)
	    UnSelectObject( &cur, m);
	 continue;
      }
      if (cur && (Expert || Confirm(-1, -1, "Some Vertices are not bound to any LineDef", "Do you want to delete these unused Vertices?")))
	 DeleteObjects( OBJ_VERTEXES, &cur);
      cur = NULL;
      for (n = 0; n < NumSideDefs; n++)
	 SelectObject( &cur, n);
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].sidedef1;
	 if (m >= 0)
	    UnSelectObject( &cur, m);
	 m = LineDefs[ n].sidedef2;
	 if (m >= 0)
	    UnSelectObject( &cur, m);
	 continue;
      }
      if (cur && (Expert || Confirm(-1, -1, "Some SideDefs are not bound to any LineDef", "Do you want to delete these unused SideDefs?")))
	 DeleteObjects( OBJ_SECTORS, &cur);
      cur = NULL;
      for (n = 0; n < NumSectors; n++)
	 SelectObject( &cur, n);
      for (n = 0; n < NumLineDefs; n++)
      {
	 m = LineDefs[ n].sidedef1;
	 if (m >= 0 && SideDefs[ m].sector >= 0)
	    UnSelectObject( &cur, SideDefs[ m].sector);
	 m = LineDefs[ n].sidedef2;
	 if (m >= 0 && SideDefs[ m].sector >= 0)
	    UnSelectObject( &cur, SideDefs[ m].sector);
	 continue;
      }
      if (cur && (Expert || Confirm(-1, -1, "Some Sectors are not bound to any SideDef", "Do you want to delete these unused Sectors?")))
	 DeleteObjects( OBJ_SECTORS, &cur);
      break;
   case 4:
      if (objtype == OBJ_VERTEXES)
      {
	 while (obj)
	 {
	    cur = obj;
	    obj = obj->next;
	    n = -1;
	    m = -1;
	    for (val = 0; val < NumLineDefs; val++)
	    {
	       if (LineDefs[ val].start == cur->objnum)
	       {
		  if (n == -1)
		     n = val;
		  else
		     n = -2;
	       }
	       if (LineDefs[ val].end == cur->objnum)
	       {
		  if (m == -1)
		     m = val;
		  else
		     m = -2;
	       }
	    }
	    if (n < 0 || m < 0)
	    {
	       Beep();
	       sprintf(temp, "Cannot delete Vertex %d and join the LineDefs", cur->objnum);
	       Notify( -1, -1, temp, "The Vertex must be the start of one LineDef and the end of another one");
	       continue;
	    }
	    LineDefs[ m].end = LineDefs[ n].end;
	    DeleteObject( OBJ_LINEDEFS, n);
	    DeleteObject( OBJ_VERTEXES, cur->objnum);
	 }
      }
      else
      {
	 for (cur = obj; cur; cur = cur->next)
	 {
	    n = LineDefs[ cur->objnum].start;
	    m = LineDefs[ cur->objnum].end;
	    InsertObject( OBJ_VERTEXES, -1, (Vertexes[ n].x + Vertexes[ m].x) / 2, (Vertexes[ n].y + Vertexes[ m].y) / 2);
	    InsertObject( OBJ_LINEDEFS, cur->objnum, 0, 0);
	    LineDefs[ cur->objnum].end = NumVertexes - 1;
	    LineDefs[ NumLineDefs - 1].start = NumVertexes - 1;
	    n = LineDefs[ cur->objnum].sidedef1;
	    if (n >= 0)
	    {
	       InsertObject( OBJ_SIDEDEFS, n, 0, 0);
	       LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs - 1;
	    }
	    n = LineDefs[ cur->objnum].sidedef2;
	    if (n >= 0)
	    {
	       InsertObject( OBJ_SIDEDEFS, n, 0, 0);
	       LineDefs[ NumLineDefs - 1].sidedef2 = NumSideDefs - 1;
	    }
	 }
      }
      MadeChanges = TRUE;
      MadeMapChanges = TRUE;
      break;
   case 5:
      for (cur = obj; cur; cur = cur->next)
      {
	 val = LineDefs[ cur->objnum].end;
	 LineDefs[ cur->objnum].end = LineDefs[ cur->objnum].start;
	 LineDefs[ cur->objnum].start = val;
	 val = LineDefs[ cur->objnum].sidedef1;
	 LineDefs[ cur->objnum].sidedef1 = LineDefs[ cur->objnum].sidedef2;
	 LineDefs[ cur->objnum].sidedef2 = val;
      }
      MadeChanges = TRUE;
      MadeMapChanges = TRUE;
      break;
   case 6:
      NotImplemented();
      MadeChanges = TRUE;
      break;
   }
}



/* end of file */
