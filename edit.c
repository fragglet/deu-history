/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   EDIT.C - Editor routines.
*/

/* the includes */
#include "deu.h"
#include "levels.h"
extern Bool InfoShown;		/* should we display the info bar? */
#ifdef CIRRUS_PATCH
extern char HWCursor[];		/* Cirrus hardware cursor data */
#endif /* CIRRUS_PATCH */

int  MoveSpeed = 20;		/* movement speed */


/*
   the driving program
*/

void EditLevel( int episode, int mission, Bool newlevel)
{
   ReadWTextureNames();
   ReadFTextureNames();
   InitGfx();
   CheckMouseDriver();
   if (episode < 1 || mission < 1)
      SelectLevel( &episode, &mission);
   if (episode > 0 && mission > 0)
   {
      ClearScreen();
      ReadLevelData( episode, mission);
      if (newlevel)
      {
	 ForgetLevelData();
	 MapMinX = -4000;
	 MapMinY = -4000;
	 MapMaxX = 4000;
	 MapMaxY = 4000;
      }
      LogMessage( ": Editing E%dM%d...\n", episode, mission);
      EditorLoop( episode, mission);
      LogMessage( ": Finished editing E%dM%d...\n", episode, mission);
      TermGfx();
      if (! Registered)
	 printf( "Please register DOOM if you want to be able to save your changes.\n");
      ForgetLevelData();
      /* forget the level pointer */
      Level = NULL;
   }
   else
      TermGfx();
   ForgetWTextureNames();
   ForgetFTextureNames();
}



/*
   select a level
*/

void SelectLevel( int *episode, int *mission)
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
	 if (n == 0)
	    levels = GetMemory( sizeof( char *));
	 else
	    levels = ResizeMemory( levels, (n + 1) * sizeof( char *));
	 levels[ n] = dir->dir.name;
	 n++;
      }
      dir = dir->next;
   }
   if (*episode < 1)
      *episode = 1;
   if (*mission < 1)
      *mission = 1;
   sprintf( name, "E%dM%d", *episode, *mission);
   InputNameFromList( -1, -1, "Select an episode and a mission number:", n, levels, name);
   FreeMemory( levels);
   if (*name)
      sscanf( name, "E%dM%d", episode, mission);
   else
   {
      *episode = 0;
      *mission = 0;
   }
}



/*
   get the name of the new WAD file
*/

char *GetWadFileName( int episode, int mission)
{
   char *outfile = GetMemory( 80);
   char *dotp;
   WadPtr wad;

   /* get the file name */
   if (! strcmp(Level->wadfile->filename, MainWad))
      sprintf( outfile, "E%dM%d.WAD", episode, mission);
   else
      strcpy( outfile, Level->wadfile->filename);
   do
   {
      InputFileName( -1, -1, "Name of the new WAD file:", 79, outfile);
   }
   while (!strcmp(outfile, MainWad));
   /* escape */
   if (outfile[ 0] == '\0')
   {
      FreeMemory( outfile);
      return NULL;
   }
   /* if the WAD file already exists, rename it to "*.BAK" */
   for (wad = WadFileList; wad; wad = wad->next)
      if (!stricmp( outfile, wad->filename))
      {
	 dotp = strrchr( wad->filename, '.');
	 if (dotp == NULL)
	    strcat( wad->filename, ".BAK");
	 else
	    strcpy( dotp, ".BAK");
	 /* need to close, then reopen: problems with SHARE.EXE */
	 fclose( wad->fileinfo);
	 if (rename( outfile, wad->filename) < 0)
	 {
	    if (unlink( wad->filename) < 0)
	       ProgError( "could not delete file \"%s\"", wad->filename);
	    if (rename( outfile, wad->filename) < 0)
	       ProgError( "could not rename \"%s\" to \"%s\"", outfile, wad->filename);
	 }
	 wad->fileinfo = fopen( wad->filename, "rb");
	 if (wad->fileinfo == NULL)
	    ProgError( "could not reopen file \"%s\"", wad->filename);
	 break;
      }
   return outfile;
}



/*
   display the help screen
 */

void DisplayHelp( int objtype, int grid) /* SWAP! */
{
   int x0 = 137;
   int y0 = 50;

   if (UseMouse)
      HideMousePointer();
   /* put in the instructions */
   DrawScreenBox3D( x0, y0, x0 + 364, y0 + 355);
   SetColor( LIGHTCYAN);
   DrawScreenText( x0 + 100, y0 + 20, "Doom Editor Utility");
   DrawScreenText( 269 - strlen(GetEditModeName( objtype)) * 4, y0 + 32, "- %s Editor -", GetEditModeName( objtype));
   SetColor( BLACK);
   DrawScreenText( x0 + 10, y0 + 60, "Use the mouse or the cursor keys to move");
   DrawScreenText( -1, -1, "around.  The map scrolls when the pointer");
   DrawScreenText( -1, -1, "reaches the edge of the screen.");
   DrawScreenText( -1, y0 + 100, "Other useful keys are:");
   if (Registered)
      DrawScreenText( -1, y0 + 115, "Q     - Quit, saving changes");
   else
   {
      SetColor( DARKGRAY);
      DrawScreenText( -1, y0 + 115, "Q     - Quit without saving changes");
      SetColor( BLACK);
   }
   DrawScreenText( -1, -1, "Esc   - Exit without saving changes");
   DrawScreenText( -1, -1, "Tab   - Switch to the next editing mode");
   DrawScreenText( -1, -1, "Space - Change the move/scroll speed");
   DrawScreenText( -1, -1, "+/-   - Change the map scale (current: %d)", (int) (1.0 / Scale + 0.5));
   DrawScreenText( -1, -1, "G     - Change the grid scale (cur.: %d)", grid);
   DrawScreenText( -1, -1, "N, >  - Jump to the next object.");
   DrawScreenText( -1, -1, "P, <  - Jump to the previous object.");
   DrawScreenText( -1, -1, "J, #  - Jump to a specific object (enter #)");
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
   DrawScreenText( -1, y0 + 320, "Please read DEU.TXT for more information");
   SetColor( YELLOW);
   DrawScreenText( -1, y0 + 340, "Press any key to return to the editor...");
   bioskey( 0);
   if (UseMouse)
      ShowMousePointer();
}



/*
   the editor main loop
*/

void EditorLoop( int episode, int mission) /* SWAP! */
{
   int    EditMode = OBJ_THINGS;
   int    CurObject = -1;
   int    OldObject = -1;
   Bool   RedrawMap = TRUE;
   Bool   RedrawObj = FALSE;
   Bool   DragObject = FALSE;
   int    key, altkey, buttons, oldbuttons;
   int    GridScale = 0;
   Bool   GridShown = TRUE;
   SelPtr Selected = NULL;
   char   keychar;
   int    SelBoxX = 0;
   int    SelBoxY = 0;
   int    OldPointerX = 0;
   int    OldPointerY = 0;
   Bool   StretchSelBox = FALSE;
   Bool   ShowRulers = FALSE;

   MadeChanges = FALSE;
   MadeMapChanges = FALSE;
   if (InitialScale < 1)
      InitialScale = 1;
   else if (InitialScale > 20)
      InitialScale = 20;
   Scale = (float) (1.0 / InitialScale);
   CenterMapAroundCoords( (MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);
   if (UseMouse)
   {
      ResetMouseLimits();
      SetMouseCoords( PointerX, PointerY);
      ShowMousePointer();
#ifdef CIRRUS_PATCH
      if (CirrusCursor == TRUE)
      {
	 SetHWCursorMap( HWCursor);
	 SetHWCursorPos( PointerX, PointerY);
      }
#endif /* CIRRUS_PATCH */
      oldbuttons = 0;
   }
   else
      FakeCursor = TRUE;

   for (;;)
   {
      key = 0;
      altkey = 0;

      /* get mouse position and button status */
      if (UseMouse)
      {
	 if (FakeCursor || ShowRulers)
	 {
	    HideMousePointer();
	    DrawPointer( ShowRulers);
	    ShowMousePointer();
	 }
	 GetMouseCoords( &PointerX, &PointerY, &buttons);
	 if (FakeCursor || ShowRulers)
	 {
	    HideMousePointer();
	    DrawPointer( ShowRulers);
	    ShowMousePointer();
	 }
	 if ( buttons == 1 && PointerY < 17)
	 {
	    /* kluge for the menu bar */
	    altkey = 0x08;
	    if (PointerX < 12)
	       Beep();
	    else if (PointerX < 60)
	       key = 0x2100; /* 'F' */
	    else if (PointerX < 108)
	       key = 0x1200; /* 'E' */
	    else if (PointerX < 172)
	       key = 0x1f00; /* 'S' */
	    else if (PointerX < 228)
	       key = 0x3200; /* 'M' */
	    else if (PointerX < 276)
	       key = 0x1700; /* 'I' */
	    else if (PointerX < 348)
	       key = 0x1800; /* 'O' */
	    else if (PointerX < 406)
	       key = 0x2E00; /* 'C' */
	    else if (PointerX < ScrMaxX - 43)
	       Beep();
	    else
	       key = 0x2300; /* 'H' */
	 }
	 else
	 {
	    if (buttons != oldbuttons)
	    {
	       switch (buttons)
	       {
	       case 1:
		  if (SwapButtons)
		     key = 0x000D;
		  else
		     key = 'M';      /* Press left button = Mark/Unmark ('M') */
		  break;
	       case 2:
		  if (! DragObject)
		     key = 'D';      /* Press right button = Drag */
		  break;
	       case 3:
	       case 4:
		  if (SwapButtons)
		     key = 'M';
		  else
		     key = 0x000D;   /* Press middle button = Edit ('Enter') */
		  break;
	       default:
		  if (StretchSelBox) /* Release left button = End Selection Box */
		     key = 'M';
		  if (DragObject)    /* Release right button = End Drag */
		     key = 'D';
		  break;
	       }
	       altkey = bioskey( 2);
	    }
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
	    /* current object must be first in the list */
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
	    if (MoveObjectsToCoords( EditMode, Selected, MAPX( PointerX), MAPY( PointerY), GridScale))
	       RedrawMap = TRUE;
	    if (forgetit)
	       ForgetSelection( &Selected);
	 }
	 else
	 {
	    Beep();
	    DragObject = FALSE;
	 }
      }
      else if (StretchSelBox)
      {
	 int x = MAPX( PointerX);
	 int y = MAPY( PointerY);

	 /* draw selection box */
	 SetColor( CYAN);
	 setwritemode( XOR_PUT);
	 if (UseMouse)
	    HideMousePointer();
	 DrawMapLine( SelBoxX, SelBoxY, SelBoxX, y);
	 DrawMapLine( SelBoxX, y, x, y);
	 DrawMapLine( x, y, x, SelBoxY);
	 DrawMapLine( x, SelBoxY, SelBoxX, SelBoxY);
	 if (UseMouse)
	    ShowMousePointer();
	 delay( 50);
	 if (UseMouse)
	    HideMousePointer();
	 DrawMapLine( SelBoxX, SelBoxY, SelBoxX, y);
	 DrawMapLine( SelBoxX, y, x, y);
	 DrawMapLine( x, y, x, SelBoxY);
	 DrawMapLine( x, SelBoxY, SelBoxX, SelBoxY);
	 setwritemode( COPY_PUT);
	 if (UseMouse)
	    ShowMousePointer();
      }
      else if (!RedrawObj)
      {
	 /* check if there is something near the pointer */
	 OldObject = CurObject;
	 if ((bioskey( 2) & 0x03) == 0x00)  /* no shift keys */
	    CurObject = GetCurObject( EditMode, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4));
	 if (CurObject < 0)
	    CurObject = OldObject;
      }

      /* draw the map */
      if (RedrawMap)
      {
	 if (UseMouse)
	    HideMousePointer();
	 DrawMap( EditMode, GridScale, GridShown);
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
	    PlaySound( 50, 10);
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

      /* redraw the pointer if necessary */
      if (RedrawMap && (FakeCursor || ShowRulers))
      {
	 if (UseMouse)
	    HideMousePointer();
	 DrawPointer( ShowRulers);
	 if (UseMouse)
	    ShowMousePointer();
      }

      /* display the current pointer coordinates */
      if (RedrawMap || PointerX != OldPointerX || PointerY != OldPointerY)
      {
	 SetColor( LIGHTGRAY);
	 DrawScreenBox( ScrMaxX - 170, 4, ScrMaxX - 50, 12);
	 SetColor( BLUE);
	 DrawScreenText( ScrMaxX - 170, 4, "%d, %d", MAPX( PointerX), MAPY( PointerY));
	 OldPointerX = PointerX;
	 OldPointerY = PointerY;
      }

      /* the map is up to date */
      RedrawMap = FALSE;

      /* get user input */
      if (bioskey( 1) || key)
      {
	 if (! key)
	 {
	    key = bioskey( 0);
	    altkey = bioskey( 2);
	 }

	 /* user wants to access the drop-down menus */
	 if (altkey & 0x08)    /* if alt is pressed... */
	 {
	    if ((key & 0xFF00) == 0x2100)       /* Scan code for F */
	       key = PullDownMenu( 18, 19,
				   "Save         F2", 0x3C00,    (int) 'S', 1,
				   "Save As ExMx F3", 0x3D00,    (int) 'A', 6,
				   "Print          ", -1,        (int) 'P', -1,
				   "Quit          Q", (int) 'Q', (int) 'Q', 1,
				   NULL);
	    else if ((key & 0xFF00) == 0x1200)  /* Scan code for E */
	    {
	       key = PullDownMenu( 66, 19,
				   "Copy object(s)      O", (int) 'O', (int) 'C', 1,
				   "Add object        Ins", 0x5200,    (int) 'A', 1,
				   "Delete object(s)  Del", 0x5300,    (int) 'D', 1,
				   ((EditMode == OBJ_VERTEXES) ?
				   NULL :
				   "Preferences        F5"), 0x3F00,   (int) 'P', 1,
				   NULL);
	    }
	    else if ((key & 0xFF00) == 0x1F00)  /* Scan code for S */
	       key = PullDownMenu( 114, 19,
				   "Find/Change       F4", -1,        (int) 'F', -1,
				   "Repeat last find    ", -1,        (int) 'R', -1,
				   "Next object        N", (int) 'N', (int) 'N', 1,
				   "Prev object        P", (int) 'P', (int) 'P', 1,
				   "Jump to object...  J", (int) 'J', (int) 'J', 1,
				   NULL);
	    else if ((key & 0xFF00) == 0x3200)  /* Scan code for M */
	       key = PullDownMenu( 178, 19,
				   ((EditMode == OBJ_THINGS) ?
				   "û Things              T" :
				   "  Things              T"), (int) 'T', (int) 'T', 3,
				   ((EditMode == OBJ_LINEDEFS) ?
				   "û Linedefs+Sidedefs   L" :
				   "  Linedefs+Sidedefs   L"), (int) 'L', (int) 'L', 3,
				   ((EditMode == OBJ_VERTEXES) ?
				   "û Vertexes            V" :
				   "  Vertexes            V"), (int) 'V', (int) 'V', 3,
				   ((EditMode == OBJ_SECTORS) ?
				   "û Sectors             S" :
				   "  Sectors             S"), (int) 'S', (int) 'S', 3,
				   "  Next mode         Tab",  0x0009,    (int) 'N', 3,
				   "  Last mode   Shift+Tab",  0x0F00,    (int) 'L', 3,
				   "  3D Preview          3",  (int) '3', (int) '3', -1,
				   NULL);
	    else if ((key & 0xFF00) == 0x1700)  /* Scan code for I */
	    {
	       key = 0;
	       /* code duplicated from 'F8' - I hate to do that */
	       if (Selected)
		  MiscOperations( 234, 19, EditMode, &Selected);
	       else
	       {
		  if (CurObject >= 0)
		     SelectObject( &Selected, CurObject);
		  MiscOperations( 234, 19, EditMode, &Selected);
		  if (CurObject >= 0)
		     UnSelectObject( &Selected, CurObject);
	       }
	       CurObject = -1;
	       DragObject = FALSE;
	       StretchSelBox = FALSE;
	    }
	    else if ((key & 0xFF00) == 0x1800)  /* Scan code for O */
	    {
	       int savednum, i;

	       key = 0;
	       /* don't want to create the object behind the menu bar... */
	       if (PointerY < 20)
	       {
		  PointerX = ScrCenterX;
		  PointerY = ScrCenterY;
	       }
	       /* code duplicated from 'F9' - I hate to do that */
	       savednum = NumLineDefs;
	       InsertStandardObject( 282, 19, MAPX( PointerX), MAPY( PointerY));
	       if (NumLineDefs > savednum)
	       {
		  ForgetSelection( &Selected);
		  EditMode = OBJ_LINEDEFS;
		  for (i = savednum; i < NumLineDefs; i++)
		     SelectObject( &Selected, i);
		  CurObject = NumLineDefs - 1;
		  OldObject = -1;
		  DragObject = FALSE;
		  StretchSelBox = FALSE;
	       }
	    }
	    else if ((key & 0xFF00) == 0x2E00)  /* Scan code for C */
	    {
	       key = 0;
	       CheckLevel( 354, 19);
	    }
	    else if ((key & 0xFF00) == 0x2300)  /* Scan code for H */
	       key = PullDownMenu( ScrMaxX, 19,
				   "  Keyboard & mouse  F1",  0x3B00,    (int) 'K', 3,
				   (InfoShown ?
				   "û Info bar           I" :
				   "  Info bar           I"), (int) 'I', (int) 'I', 3,
				   "  About DEU...        ",  -1,        (int) 'A', -1,
				   NULL);
	    else
	    {
	       Beep();
	       key = 0;
	    }
	    RedrawMap = TRUE;
	 }

	 /* User wants to do the impossible. */
	 if (key == -1)
	 {
	    NotImplemented();
	    RedrawMap = TRUE;
	 }

	 /* simplify the checks later on */
	 if (isprint(key & 0x00ff))
	    keychar = toupper(key);
	 else
	    keychar = '\0';

	 /* erase the (keyboard) pointer */
	 if (FakeCursor || ShowRulers)
	 {
	    HideMousePointer();
	    DrawPointer( ShowRulers);
	    ShowMousePointer();
	 }

	 /* user wants to exit */
	 if (keychar == 'Q')
	 {
	    ForgetSelection( &Selected);
	    if (CheckStartingPos())
	    {
	       if (Registered && MadeChanges)
	       {
		  char *outfile;

		  outfile = GetWadFileName( episode, mission);
		  if (outfile)
		  {
		     SaveLevelData( outfile);
		     break;
		  }
	       }
	       else
		  break;
	    }
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0x00FF) == 0x001B) /* 'Esc' */
	 {
	    if (DragObject)
	       DragObject = FALSE;
	    else if (StretchSelBox)
	       StretchSelBox = FALSE;
	    else
	    {
	       ForgetSelection( &Selected);
	       if (!MadeChanges || Confirm(-1, -1, "You have unsaved changes.  Do you really want to quit?", NULL))
		  break;
	       RedrawMap = TRUE;
	    }
	 }

	 /* user is lost */
	 else if ((key & 0xFF00) == 0x3B00) /* 'F1' */
	 {
	    DisplayHelp( EditMode, GridScale);
	    RedrawMap = TRUE;
	 }

	 /* user wants to save the level data */
	 else if ((key & 0xFF00) == 0x3C00 && Registered) /* 'F2' */
	 {
	    char *outfile;

	    if (CheckStartingPos())
	    {
	       outfile = GetWadFileName( episode, mission);
	       if (outfile)
		  SaveLevelData( outfile);
	    }
	    RedrawMap = TRUE;
	 }

	 /* user wants to save and change the episode and mission numbers */
	 else if ((key & 0xFF00) == 0x3D00 && Registered) /* 'F3' */
	 {
	    char *outfile;
	    int   e, m;
	    MDirPtr newLevel, oldl, newl;
	    char name[ 7];

	    if (CheckStartingPos())
	    {
	       outfile = GetWadFileName( episode, mission);
	       if (outfile)
	       {
		  e = episode;
		  m = mission;
		  SelectLevel( &e, &m);
		  if (e > 0 && m > 0 && (e != episode || m != mission))
		  {
		     /* horrible but it works... */
		     episode = e;
		     mission = m;
		     sprintf( name, "E%dM%d", episode, mission);
		     newLevel = FindMasterDir( MasterDir, name);
		     oldl = Level;
		     newl = newLevel;
		     for (m = 0; m < 11; m++)
		     {
			newl->wadfile = oldl->wadfile;
			if (m > 0)
			   newl->dir = oldl->dir;
			/*
			if (!strcmp( outfile, oldl->wadfile->filename))
			{
			   oldl->wadfile = WadFileList;
			   oldl->dir = lost...
			}
			*/
			oldl = oldl->next;
			newl = newl->next;
		     }
		     Level = newLevel;
		  }
		  SaveLevelData( outfile);
	       }
	    }
	    RedrawMap = TRUE;
	 }

	 /* user wants to get the 'Preferences' menu */
	 else if ((key & 0xFF00) == 0x3F00) /* 'F5' */
	 {
	    Preferences( -1, -1);
	    RedrawMap = TRUE;
	 }
	 /* user wants to get the menu of misc. ops */
	 else if ((key & 0xFF00) == 0x4200) /* 'F8' */
	 {
	    if (Selected)
	       MiscOperations( -1, -1, EditMode, &Selected);
	    else
	    {
	       if (CurObject >= 0)
		  SelectObject( &Selected, CurObject);
	       MiscOperations( -1, -1, EditMode, &Selected);
	       if (CurObject >= 0)
		  UnSelectObject( &Selected, CurObject);
	    }
	    CurObject = -1;
	    RedrawMap = TRUE;
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	 }

	 /* user wants to insert a standard shape */
	 else if ((key & 0xFF00) == 0x4300) /* 'F9' */
	 {
	    int savednum, i;

	    savednum = NumLineDefs;
	    InsertStandardObject( -1, -1, MAPX( PointerX), MAPY( PointerY));
	    if (NumLineDefs > savednum)
	    {
	       ForgetSelection( &Selected);
	       EditMode = OBJ_LINEDEFS;
	       for (i = savednum; i < NumLineDefs; i++)
		  SelectObject( &Selected, i);
	       CurObject = NumLineDefs - 1;
	       OldObject = -1;
	       DragObject = FALSE;
	       StretchSelBox = FALSE;
	    }
	    RedrawMap = TRUE;
	 }

	 /* user wants to check his level */
	 else if ((key & 0xFF00) == 0x4400) /* 'F10' */
	 {
	    CheckLevel( -1, -1);
	    RedrawMap = TRUE;
	 }

	 /* user wants to display/hide the info box */
	 else if (keychar == 'I')
	 {
	    InfoShown = !InfoShown;
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the scale */
	 else if ((keychar == '+' || keychar == '=') && Scale < 4.0)
	 {
	    OrigX += (int) ((PointerX - ScrCenterX) / Scale);
	    OrigY += (int) ((ScrCenterY - PointerY) / Scale);
	    if (Scale < 1.0)
	       Scale = 1.0 / ((1.0 / Scale) - 1.0);
	    else
	       Scale = Scale * 2.0;
	    OrigX -= (int) ((PointerX - ScrCenterX) / Scale);
	    OrigY -= (int) ((ScrCenterY - PointerY) / Scale);
	    RedrawMap = TRUE;
	 }
	 else if ((keychar == '-' || keychar == '_') && Scale > 0.05)
	 {
	    OrigX += (int) ((PointerX - ScrCenterX) / Scale);
	    OrigY += (int) ((ScrCenterY - PointerY) / Scale);
	    if (Scale < 1.0)
	       Scale = 1.0 / ((1.0 / Scale) + 1.0);
	    else
	       Scale = Scale / 2.0;
	    OrigX -= (int) ((PointerX - ScrCenterX) / Scale);
	    OrigY -= (int) ((ScrCenterY - PointerY) / Scale);
	    RedrawMap = TRUE;
	 }

	 /* user wants to set the scale directly */
	 else if (keychar >= '0' && keychar <= '9')
	 {
	    OrigX += (int) ((PointerX - ScrCenterX) / Scale);
	    OrigY += (int) ((ScrCenterY - PointerY) / Scale);
	    if (keychar == '0')
	       Scale = 0.1;
	    else
	       Scale = 1.0 / (keychar - '0');
	    OrigX -= (int) ((PointerX - ScrCenterX) / Scale);
	    OrigY -= (int) ((ScrCenterY - PointerY) / Scale);
	    RedrawMap = TRUE;
	 }

	 /* user wants to move */
	 else if ((key & 0xFF00) == 0x4800 && (PointerY - MoveSpeed) >= 0)
	 {
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY - MoveSpeed);
	    else
	       PointerY -= MoveSpeed;
	 }
	 else if ((key & 0xFF00) == 0x5000 && (PointerY + MoveSpeed) <= ScrMaxY)
	 {
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY + MoveSpeed);
	    else
	       PointerY += MoveSpeed;
	 }
	 else if ((key & 0xFF00) == 0x4B00 && (PointerX - MoveSpeed) >= 0)
	 {
	    if (UseMouse)
	       SetMouseCoords( PointerX - MoveSpeed, PointerY);
	    else
	       PointerX -= MoveSpeed;
	 }
	 else if ((key & 0xFF00) == 0x4D00 && (PointerX + MoveSpeed) <= ScrMaxX)
	 {
	    if (UseMouse)
	       SetMouseCoords( PointerX + MoveSpeed, PointerY);
	    else
	       PointerX += MoveSpeed;
	 }

	 /* user wants to scroll the map (scroll one half page at a time) */
	 else if ((key & 0xFF00) == 0x4900 && MAPY( ScrCenterY) < MapMaxY)
	 {
	    OrigY += (int) (ScrCenterY / Scale);
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x5100 && MAPY( ScrCenterY) > MapMinY)
	 {
	    OrigY -= (int) (ScrCenterY / Scale);
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x4700 && MAPX( ScrCenterX) > MapMinX)
	 {
	    OrigX -= (int) (ScrCenterX / Scale);
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x4F00 && MAPX( ScrCenterX) < MapMaxX)
	 {
	    OrigX += (int) (ScrCenterX / Scale);
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the movement speed */
	 else if (keychar == ' ')
	    MoveSpeed = MoveSpeed == 1 ? 20 : 1;

	 /* user wants to change the edit mode */
	 else if ((key & 0x00FF) == 0x0009 || (key & 0xFF00) == 0x0F00 || keychar == 'T' || keychar == 'V' || keychar == 'L' || keychar == 'S')
	 {
	    int    PrevMode = EditMode;
	    SelPtr NewSel;

	    if ((key & 0x00FF) == 0x0009) /* 'Tab' */
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
		  EditMode = OBJ_THINGS;
		  break;
	       }
	    }
	    else if ((key & 0xFF00) == 0x0F00) /* 'Shift-Tab' */
	    {
	       switch (EditMode)
	       {
	       case OBJ_THINGS:
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
	       }
	    }
	    else
	    {
	       if (keychar == 'T')
		  EditMode = OBJ_THINGS;
	       else if (keychar == 'V')
		  EditMode = OBJ_VERTEXES;
	       else if (keychar == 'L')
		  EditMode = OBJ_LINEDEFS;
	       else if (keychar == 'S')
		  EditMode = OBJ_SECTORS;
	       /* unselect all */
	       ForgetSelection( &Selected);
	    }

	    /* special cases for the selection list... */
	    if (Selected)
	    {
	       /* select all LineDefs bound to the selected Sectors */
	       if (PrevMode == OBJ_SECTORS && EditMode == OBJ_LINEDEFS)
	       {
		  int l, sd;

		  ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
		  NewSel = NULL;
		  for (l = 0; l < NumLineDefs; l++)
		  {
		     sd = LineDefs[ l].sidedef1;
		     if (sd >= 0 && IsSelected( Selected, SideDefs[ sd].sector))
			SelectObject( &NewSel, l);
		     else
		     {
			sd = LineDefs[ l].sidedef2;
			if (sd >= 0 && IsSelected( Selected, SideDefs[ sd].sector))
			   SelectObject( &NewSel, l);
		     }
		  }
		  ForgetSelection( &Selected);
		  Selected = NewSel;
	       }
	       /* select all Vertices bound to the selected LineDefs */
	       else if (PrevMode == OBJ_LINEDEFS && EditMode == OBJ_VERTEXES)
	       {
		  ObjectsNeeded( OBJ_LINEDEFS, 0);
		  NewSel = NULL;
		  while (Selected)
		  {
		     if (!IsSelected( NewSel, LineDefs[ Selected->objnum].start))
			SelectObject( &NewSel, LineDefs[ Selected->objnum].start);
		     if (!IsSelected( NewSel, LineDefs[ Selected->objnum].end))
			SelectObject( &NewSel, LineDefs[ Selected->objnum].end);
		     UnSelectObject( &Selected, Selected->objnum);
		  }
		  Selected = NewSel;
	       }
	       /* select all Sectors that have their LineDefs selected */
	       else if (PrevMode == OBJ_LINEDEFS && EditMode == OBJ_SECTORS)
	       {
		  int l, sd;

		  ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
		  NewSel = NULL;
		  /* select all Sectors... */
		  for (l = 0; l < NumSectors; l++)
		     SelectObject( &NewSel, l);
		  /* ... then unselect those that should not be in the list */
		  for (l = 0; l < NumLineDefs; l++)
		     if (!IsSelected( Selected, l))
		     {
			sd = LineDefs[ l].sidedef1;
			if (sd >= 0)
			   UnSelectObject( &NewSel, SideDefs[ sd].sector);
			sd = LineDefs[ l].sidedef2;
			if (sd >= 0)
			   UnSelectObject( &NewSel, SideDefs[ sd].sector);
		     }
		  ForgetSelection( &Selected);
		  Selected = NewSel;
	       }
	       /* select all LineDefs that have both ends selected */
	       else if (PrevMode == OBJ_VERTEXES && EditMode == OBJ_LINEDEFS)
	       {
		  int l;

		  ObjectsNeeded( OBJ_LINEDEFS, 0);
		  NewSel = NULL;
		  for (l = 0; l < NumLineDefs; l++)
		     if (IsSelected( Selected, LineDefs[ l].start) && IsSelected( Selected, LineDefs[ l].end))
			SelectObject( &NewSel, l);
		  ForgetSelection( &Selected);
		  Selected = NewSel;
	       }
	       /* unselect all */
	       else
		  ForgetSelection( &Selected);
	    }
	    if (GetMaxObjectNum( EditMode) >= 0 && Select0 == TRUE)
	       CurObject = 0;
	    else
	       CurObject = -1;
	    OldObject = -1;
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the grid scale */
	 else if (keychar == 'G')
	 {
	    if ((altkey & 0x03) == 0x00)  /* no shift keys */
	    {
	       if (GridScale == 0)
		  GridScale = 256;
	       else if (GridScale > 8)
		  GridScale /= 2;
	       else
		  GridScale = 0;
	    }
	    else
	    {
	       if (GridScale == 0)
		  GridScale = 8;
	       else if (GridScale < 256)
		  GridScale *= 2;
	       else
		  GridScale = 0;
	    }
	    RedrawMap = TRUE;
	 }

	 /* user wants to display or hide the grid */
	 else if (keychar == 'H')
	 {
	    if ((altkey & 0x03) != 0x00)  /* shift key pressed */
	       GridScale = 0;
	    else
	       GridShown = !GridShown;
	    RedrawMap = TRUE;
	 }

	 /* user wants to toggle the rulers */
	 else if (keychar == 'R')
	    ShowRulers = !ShowRulers;

	 /* user wants to toggle drag mode */
	 else if (keychar == 'D')
	 {
	    StretchSelBox = FALSE;
	    if (DragObject)
	    {
	       DragObject = FALSE;
	       if (EditMode == OBJ_VERTEXES)
	       {
		  if (Selected == NULL && CurObject >= 0)
		  {
		     SelectObject( &Selected, CurObject);
		     if (AutoMergeVertices( &Selected))
			RedrawMap = TRUE;
		     ForgetSelection( &Selected);
		  }
		  else
		     if (AutoMergeVertices( &Selected))
			RedrawMap = TRUE;
	       }
	       else if (EditMode == OBJ_LINEDEFS)
	       {
		  SelPtr NewSel, cur;

		  ObjectsNeeded( OBJ_LINEDEFS, 0);
		  NewSel = NULL;
		  if (Selected == NULL && CurObject >= 0)
		  {
		     SelectObject( &NewSel, LineDefs[ CurObject].start);
		     SelectObject( &NewSel, LineDefs[ CurObject].end);
		  }
		  else
		  {
		     for (cur = Selected; cur; cur = cur->next)
		     {
			if (!IsSelected( NewSel, LineDefs[ cur->objnum].start))
			   SelectObject( &NewSel, LineDefs[ cur->objnum].start);
			if (!IsSelected( NewSel, LineDefs[ cur->objnum].end))
			   SelectObject( &NewSel, LineDefs[ cur->objnum].end);
		     }
		  }
		  if (AutoMergeVertices( &NewSel))
		     RedrawMap = TRUE;
		  ForgetSelection( &NewSel);
	       }
	    }
	    else
	    {
	       DragObject = TRUE;
	       if (EditMode == OBJ_THINGS && CurObject >= 0)
		  MoveObjectsToCoords( EditMode, NULL, Things[ CurObject].xpos, Things[ CurObject].ypos, 0);
	       else if (EditMode == OBJ_VERTEXES && CurObject >= 0)
		  MoveObjectsToCoords( EditMode, NULL, Vertexes[ CurObject].x, Vertexes[ CurObject].y, 0);
	       else
		  MoveObjectsToCoords( EditMode, NULL, MAPX( PointerX), MAPY( PointerY), GridScale);
	    }
	 }

	 /* user wants to select the next or previous object */
	 else if (keychar == 'N' || keychar == '>')
	 {
	    if (CurObject < GetMaxObjectNum( EditMode))
	       CurObject++;
	    else if (GetMaxObjectNum( EditMode) >= 0)
	       CurObject = 0;
	    else
	       CurObject = -1;
	    RedrawMap = TRUE;
	 }
	 else if (keychar == 'P' || keychar == '<')
	 {
	    if (CurObject > 0)
	       CurObject--;
	    else
	       CurObject = GetMaxObjectNum( EditMode);
	    RedrawMap = TRUE;
	 }
	 else if (keychar == 'J' || keychar == '#')
	 {
	    OldObject = InputObjectNumber( -1, -1, EditMode, CurObject);
	    if (OldObject >= 0)
	    {
	       CurObject = OldObject;
	       GoToObject( EditMode, CurObject);
	    }
	    else
	       OldObject = CurObject;
	    RedrawMap = TRUE;
	 }

	 /* user wants to mark/unmark an object or a group of objects */
	 else if (keychar == 'M')
	 {
	    if (StretchSelBox)
	    {
	       SelPtr oldsel;

	       /* select all objects in the selection box */
	       StretchSelBox = FALSE;
	       RedrawMap = TRUE;
	       /* additive selection box or not? */
	       if (AdditiveSelBox == FALSE)
		  ForgetSelection( &Selected);
	       else
		  oldsel = Selected;
	       Selected = SelectObjectsInBox( EditMode, SelBoxX, SelBoxY, MAPX( PointerX), MAPY( PointerY));
	       if (AdditiveSelBox == TRUE)
		  while (oldsel != NULL)
		  {
		     if (! IsSelected( Selected, oldsel->objnum))
			SelectObject( &Selected, oldsel->objnum);
		     UnSelectObject( &oldsel, oldsel->objnum);
		  }
	       if (Selected)
	       {
		  CurObject = Selected->objnum;
		  PlaySound( 440, 10);
	       }
	       else
		  CurObject = -1;
	    }
	    else if ((altkey & 0x03) == 0x00)  /* no shift keys */
	    {
	       if (CurObject >= 0)
	       {
		  /* mark or unmark one object */
		  if (IsSelected( Selected, CurObject))
		     UnSelectObject( &Selected, CurObject);
		  else
		     SelectObject( &Selected, CurObject);
		  if (UseMouse)
		     HideMousePointer();
		  HighlightObject( EditMode, CurObject, GREEN);
		  if (UseMouse)
		     ShowMousePointer();
		  if (Selected)
		     PlaySound( 440, 10);
		  DragObject = FALSE;
	       }
	       else
		  Beep();
	    }
	    else
	    {
	       /* begin "stretch selection box" mode */
	       SelBoxX = MAPX( PointerX);
	       SelBoxY = MAPY( PointerY);
	       StretchSelBox = TRUE;
	       DragObject = FALSE;
	    }
	 }

	 /* user wants to clear all marks and redraw the map */
	 else if (keychar == 'C')
	 {
	    ForgetSelection( &Selected);
	    RedrawMap = TRUE;
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	 }

	 /* user wants to copy a group of objects */
	 else if (keychar == 'O' && CurObject >= 0)
	 {
	    /* copy the object(s) */
	    if (Selected == NULL)
	       SelectObject( &Selected, CurObject);
	    CopyObjects( EditMode, Selected);
	    /* enter drag mode */
	    DragObject = TRUE;
	    CurObject = Selected->objnum;
	    if (EditMode == OBJ_THINGS)
	       MoveObjectsToCoords( EditMode, NULL, Things[ CurObject].xpos, Things[ CurObject].ypos, 0);
	    else if (EditMode == OBJ_VERTEXES)
	       MoveObjectsToCoords( EditMode, NULL, Vertexes[ CurObject].x, Vertexes[ CurObject].y, 0);
	    else
	       MoveObjectsToCoords( EditMode, NULL, MAPX( PointerX), MAPY( PointerY), GridScale);
	    RedrawMap = TRUE;
	    StretchSelBox = FALSE;
	 }

	 /* user wants to edit the current object */
	 else if ((key & 0x00FF) == 0x000D && CurObject >= 0) /* 'Enter' */
	 {
	    if (Selected)
	       EditObjectsInfo( 0, 30, EditMode, Selected);
	    else
	    {
	       SelectObject( &Selected, CurObject);
	       EditObjectsInfo( 0, 30, EditMode, Selected);
	       UnSelectObject( &Selected, CurObject);
	    }
	    RedrawMap = TRUE;
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	 }

	 /* user wants to delete the current object */
	 else if ((key & 0xFF00) == 0x5300 && CurObject >= 0) /* 'Del' */
	 {
	    if (EditMode == OBJ_THINGS || Expert || Confirm( -1, -1,
				(Selected ? "Do you really want to delete these objects?" : "Do you really want to delete this object?"),
				(Selected ? "This will also delete the objects bound to them." : "This will also delete the objects bound to it.")))
	    {
	       if (Selected)
		  DeleteObjects( EditMode, &Selected);
	       else
		  DeleteObject( EditMode, CurObject);
	       CurObject = -1;
	    }
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to insert a new object */
	 else if ((key & 0xFF00) == 0x5200) /* 'Ins' */
	 {
	    SelPtr cur;

	    /* first special case: if several Vertices are selected, add new LineDefs */
	    if (EditMode == OBJ_VERTEXES && Selected != NULL && Selected->next != NULL)
	    {
	       int firstv;

	       ObjectsNeeded( OBJ_LINEDEFS, 0);
	       if (Selected->next->next != NULL)
		  firstv = Selected->objnum;
	       else
		  firstv = -1;
	       EditMode = OBJ_LINEDEFS;
	       /* create LineDefs between the Vertices */
	       for (cur = Selected; cur->next; cur = cur->next)
	       {
		  /* check if there is already a LineDef between the two Vertices */
		  for (CurObject = 0; CurObject < NumLineDefs; CurObject++)
		     if ((LineDefs[ CurObject].start == cur->next->objnum && LineDefs[ CurObject].end == cur->objnum)
		      || (LineDefs[ CurObject].end == cur->next->objnum && LineDefs[ CurObject].start == cur->objnum))
			break;
		  if (CurObject < NumLineDefs)
		     cur->objnum = CurObject;
		  else
		  {
		     InsertObject( OBJ_LINEDEFS, -1, 0, 0);
		     CurObject = NumLineDefs - 1;
		     LineDefs[ CurObject].start = cur->next->objnum;
		     LineDefs[ CurObject].end = cur->objnum;
		     cur->objnum = CurObject;
		  }
	       }
	       /* close the polygon if there are more than 2 Vertices */
	       if (firstv >= 0 && (altkey & 0x03) != 0x00)  /* shift key pressed */
	       {
		  for (CurObject = 0; CurObject < NumLineDefs; CurObject++)
		     if ((LineDefs[ CurObject].start == firstv && LineDefs[ CurObject].end == cur->objnum)
		      || (LineDefs[ CurObject].end == firstv && LineDefs[ CurObject].start == cur->objnum))
			break;
		  if (CurObject < NumLineDefs)
		     cur->objnum = CurObject;
		  else
		  {
		     InsertObject( OBJ_LINEDEFS, -1, 0, 0);
		     CurObject = NumLineDefs - 1;
		     LineDefs[ CurObject].start = firstv;
		     LineDefs[ CurObject].end = cur->objnum;
		     cur->objnum = CurObject;
		  }
	       }
	       else
		  UnSelectObject( &Selected, cur->objnum);
	    }
	    /* second special case: if several LineDefs are selected, add new SideDefs and one Sector */
	    else if (EditMode == OBJ_LINEDEFS && Selected != NULL)
	    {
	       ObjectsNeeded( OBJ_LINEDEFS, 0);
	       for (cur = Selected; cur; cur = cur->next)
		  if (LineDefs[ cur->objnum].sidedef1 >= 0 && LineDefs[ cur->objnum].sidedef2 >= 0)
		  {
		     char msg[ 80];

		     Beep();
		     sprintf( msg, "LineDef #%d already has two SideDefs", cur->objnum);
		     Notify( -1, -1, "Error: cannot add the new Sector", msg);
		     break;
		  }
	       if (cur == NULL)
	       {
		  EditMode = OBJ_SECTORS;
		  InsertObject( OBJ_SECTORS, -1, 0, 0);
		  CurObject = NumSectors - 1;
		  for (cur = Selected; cur; cur = cur->next)
		  {
		     InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		     SideDefs[ NumSideDefs - 1].sector = CurObject;
		     ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
		     if (LineDefs[ cur->objnum].sidedef1 >= 0)
		     {
			int s;

			s = SideDefs[ LineDefs[ cur->objnum].sidedef1].sector;
			if (s >= 0)
			{
			   Sectors[ CurObject].floorh = Sectors[ s].floorh;
			   Sectors[ CurObject].ceilh = Sectors[ s].ceilh;
			   strncpy( Sectors[ CurObject].floort, Sectors[ s].floort, 8);
			   strncpy( Sectors[ CurObject].ceilt, Sectors[ s].ceilt, 8);
			   Sectors[ CurObject].light = Sectors[ s].light;
			}
			LineDefs[ cur->objnum].sidedef2 = NumSideDefs - 1;
			LineDefs[ cur->objnum].flags = 4;
			strncpy( SideDefs[ NumSideDefs - 1].tex3, "-", 8);
			strncpy( SideDefs[ LineDefs[ cur->objnum].sidedef1].tex3, "-", 8);
		     }
		     else
			LineDefs[ cur->objnum].sidedef1 = NumSideDefs - 1;
		  }
		  ForgetSelection( &Selected);
		  SelectObject( &Selected, CurObject);
	       }
	    }
	    /* normal case: add a new object of the current type */
	    else
	    {
	       ForgetSelection( &Selected);
	       if (GridScale > 0)
		  InsertObject( EditMode, CurObject, (MAPX( PointerX) + GridScale / 2) & ~(GridScale - 1), (MAPY( PointerY) + GridScale / 2) & ~(GridScale - 1));
	       else
		  InsertObject( EditMode, CurObject, MAPX( PointerX), MAPY( PointerY));
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
	       else if (EditMode == OBJ_VERTEXES)
	       {
		  SelectObject( &Selected, CurObject);
		  if (AutoMergeVertices( &Selected))
		     RedrawMap = TRUE;
		  ForgetSelection( &Selected);
	       }
	    }
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user likes music */
	 else if (key)
	    Beep();

	 /* redraw the (keyboard) pointer */
	 if (FakeCursor || ShowRulers)
	 {
	    HideMousePointer();
	    DrawPointer( ShowRulers);
	    ShowMousePointer();
	 }
      }

      /* check if Scroll Lock is off */
      if ((bioskey( 2) & 0x10) == 0x00)
      {
	 /* move the map if the pointer is near the edge of the screen */
	 if (PointerY <= (UseMouse ? 2 : 20))
	 {
	    if (! UseMouse)
	       PointerY += MoveSpeed;
	    if (MAPY( ScrCenterY) < MapMaxY)
	    {
	       OrigY += (int) (MoveSpeed * 2.0 / Scale);
	       RedrawMap = TRUE;
	    }
	 }
	 if (PointerY >= ScrMaxY - (UseMouse ? 8 : 20))
	 {
	    if (! UseMouse)
	       PointerY -= MoveSpeed;
	    if (MAPY( ScrCenterY) > MapMinY)
	    {
	       OrigY -= (int) (MoveSpeed * 2.0 / Scale);
	       RedrawMap = TRUE;
	    }
	 }
	 if (PointerX <= (UseMouse ? 8 : 20))
	 {
	    if (! UseMouse)
	       PointerX += MoveSpeed;
	    if (MAPX( ScrCenterX) > MapMinX)
	    {
	       OrigX -= (int) (MoveSpeed * 2.0 / Scale);
	       RedrawMap = TRUE;
	    }
	 }
	 if (PointerX >= ScrMaxX - (UseMouse ? 8 : 20))
	 {
	    if (! UseMouse)
	       PointerX -= MoveSpeed;
	    if (MAPX( ScrCenterX) < MapMaxX)
	    {
	       OrigX += (int) (MoveSpeed * 2.0 / Scale);
	       RedrawMap = TRUE;
	    }
	 }
      }
   }
}



/*
   draw the actual game map
*/

void DrawMap( int editmode, int grid, Bool drawgrid) /* SWAP! */
{
   int  n, m;

   /* clear the screen */
   ClearScreen();

   /* draw the grid */
   if (drawgrid == TRUE && grid > 0)
   {
      int mapx0 = MAPX( 0) & ~(grid - 1);
      int mapx1 = (MAPX( ScrMaxX) + grid) & ~(grid - 1);
      int mapy0 = (MAPY( ScrMaxY) - grid) & ~(grid - 1);
      int mapy1 = MAPY( 0) & ~(grid - 1);

      SetColor( BLUE);
      for (n = mapx0; n <= mapx1; n += grid)
	 DrawMapLine( n, mapy0, n, mapy1);
      for (n = mapy0; n <= mapy1; n += grid)
	 DrawMapLine( mapx0, n, mapx1, n);
   }

   /* draw the linedefs to form the map */
   switch (editmode)
   {
   case OBJ_THINGS:
      ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].flags & 1)
	    SetColor( WHITE);
	 else
	    SetColor( LIGHTGRAY);
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }
      break;
   case OBJ_VERTEXES:
      ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
      SetColor( LIGHTGRAY);
      for (n = 0; n < NumLineDefs; n++)
	 DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      break;
   case OBJ_LINEDEFS:
      ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].type > 0)
	 {
	    if (LineDefs[ n].tag > 0)
	       SetColor( LIGHTMAGENTA);
	    else
	       SetColor( LIGHTGREEN);
	 }
	 else if (LineDefs[ n].tag > 0)
	    SetColor( LIGHTRED);
	 else if (LineDefs[ n].flags & 1)
	    SetColor( WHITE);
	 else
	    SetColor( LIGHTGRAY);
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }
      break;
   case OBJ_SECTORS:
      ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
      for (n = 0; n < NumLineDefs; n++)
      {
	 if ((m = LineDefs[ n].sidedef1) < 0 || (m = SideDefs[ m].sector) < 0)
	    SetColor( LIGHTRED);
	 else
	 {
	    if (Sectors[ m].tag > 0)
	       SetColor( LIGHTGREEN);
	    else if (Sectors[ m].special > 0)
	       SetColor( LIGHTCYAN);
	    else if (LineDefs[ n].flags & 1)
	       SetColor( WHITE);
	    else
	       SetColor( LIGHTGRAY);
	    if ((m = LineDefs[ n].sidedef2) >= 0)
	    {
	       if ((m = SideDefs[ m].sector) < 0)
		  SetColor( LIGHTRED);
	       else if (Sectors[ m].tag > 0)
		  SetColor( LIGHTGREEN);
	       else if (Sectors[ m].special > 0)
		  SetColor( LIGHTCYAN);
	    }
	 }
	 ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }
      break;
   }

   /* draw in the vertices */
   if (editmode == OBJ_VERTEXES)
   {
      SetColor( LIGHTGREEN);
      for (n = 0; n < NumVertexes; n++)
      {
	 DrawMapLine( Vertexes[ n].x - OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x + OBJSIZE, Vertexes[ n].y + OBJSIZE);
	 DrawMapLine( Vertexes[ n].x + OBJSIZE, Vertexes[ n].y - OBJSIZE, Vertexes[ n].x - OBJSIZE, Vertexes[ n].y + OBJSIZE);
      }
   }

   /* draw in the things */
   ObjectsNeeded( OBJ_THINGS, 0);
   if (editmode == OBJ_THINGS)
   {
      for (n = 0; n < NumThings; n++)
      {
	 m = GetThingRadius( Things[ n].type);
	 SetColor( GetThingColour( Things[ n].type));
	 DrawMapLine( Things[ n].xpos - m, Things[ n].ypos, Things[ n].xpos + m, Things[ n].ypos);
	 DrawMapLine( Things[ n].xpos, Things[ n].ypos - m, Things[ n].xpos, Things[ n].ypos + m);
	 DrawMapCircle( Things[ n].xpos, Things[ n].ypos, m);
      }
   }
   else
   {
      SetColor( LIGHTGRAY);
      for (n = 0; n < NumThings; n++)
      {
	 DrawMapLine( Things[ n].xpos - OBJSIZE, Things[ n].ypos, Things[ n].xpos + OBJSIZE, Things[ n].ypos);
	 DrawMapLine( Things[ n].xpos, Things[ n].ypos - OBJSIZE, Things[ n].xpos, Things[ n].ypos + OBJSIZE);
      }
   }

   /* draw in the title bar */
   DrawScreenBox3D( 0, 0, ScrMaxX, 16);
   SetColor( WHITE);
   DrawScreenText( 20,  4, "File  Edit  Search  Modes  Misc  Objects  Check");
   DrawScreenText( 20,  6, "_     _     _       _       _    _        _    ");
   DrawScreenText( ScrMaxX - 45, 4, "Help");
   DrawScreenText( ScrMaxX - 45, 6, "_   ");

   /* draw the bottom line, if needed */
   if (InfoShown)
   {
      DrawScreenBox3D( 0, ScrMaxY - 11, ScrMaxX, ScrMaxY);
      if (MadeMapChanges == TRUE)
	 DrawScreenText( 5, ScrMaxY - 8, "Editing %s on %s #", GetEditModeName( editmode), Level->dir.name);
      else if (MadeChanges == TRUE)
	 DrawScreenText( 5, ScrMaxY - 8, "Editing %s on %s *", GetEditModeName( editmode), Level->dir.name);
      else
	 DrawScreenText( 5, ScrMaxY - 8, "Editing %s on %s", GetEditModeName( editmode), Level->dir.name);
      if (Scale < 1.0)
	 DrawScreenText( ScrMaxX - 176, ScrMaxY - 8, "Scale: 1/%d  Grid: %d", (int) (1.0 / Scale + 0.5), grid);
      else
	 DrawScreenText( ScrMaxX - 176, ScrMaxY - 8, "Scale: %d/1  Grid: %d", (int) Scale, grid);
      if (farcoreleft() < 50000L)
      {
	 if (farcoreleft() < 20000L)
	    SetColor( LIGHTRED);
	 else
	    SetColor( RED);
      }
      DrawScreenText( ScrCenterX - ((editmode == OBJ_LINEDEFS) ? 10 : 50), ScrMaxY - 8, "Free mem: %lu", farcoreleft());
   }
}



/*
   center the map around the given coords
*/

void CenterMapAroundCoords( int xpos, int ypos)
{
   OrigX = xpos;
   OrigY = ypos;
   PointerX = ScrCenterX;
   PointerY = ScrCenterY;
}



/*
   center the map around the object and zoom in if necessary
*/

void GoToObject( int objtype, int objnum) /* SWAP! */
{
   int   xpos, ypos;
   int   xpos2, ypos2;
   int   n;
   int   sd1, sd2;
   float oldscale;

   GetObjectCoords( objtype, objnum, &xpos, &ypos);
   CenterMapAroundCoords( xpos, ypos);
   oldscale = Scale;

   /* zoom in until the object can be selected */
   while (Scale < 4.0 && GetCurObject( objtype, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4)) != objnum)
   {
      if (Scale < 1.0)
	 Scale = 1.0 / ((1.0 / Scale) - 1.0);
      else
	 Scale = Scale * 2.0;
   }

   /* Special case for Sectors: if several Sectors are one inside another, then    */
   /* zooming in on the center won't help.  So I choose a LineDef that borders the */
   /* Sector, move a few pixels towards the inside of the Sector, then zoom in.    */
   if (objtype == OBJ_SECTORS && GetCurObject( OBJ_SECTORS, OrigX, OrigY, OrigX, OrigY) != objnum)
   {
      /* restore the Scale */
      Scale = oldscale;
      for (n = 0; n < NumLineDefs; n++)
      {
	 ObjectsNeeded( OBJ_LINEDEFS, 0);
	 sd1 = LineDefs[ n].sidedef1;
	 sd2 = LineDefs[ n].sidedef2;
	 ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 if (sd1 >= 0 && SideDefs[ sd1].sector == objnum)
	    break;
	 if (sd2 >= 0 && SideDefs[ sd2].sector == objnum)
	    break;
      }
      if (n < NumLineDefs)
      {
	 GetObjectCoords( OBJ_LINEDEFS, n, &xpos2, &ypos2);
	 n = ComputeDist( abs( xpos - xpos2), abs( ypos - ypos2)) / 7;
	 if (n <= 1)
	   n = 2;
	 xpos = xpos2 + (xpos - xpos2) / n;
	 ypos = ypos2 + (ypos - ypos2) / n;
	 CenterMapAroundCoords( xpos, ypos);
	 /* zoom in until the sector can be selected */
	 while (Scale > 4.0 && GetCurObject( OBJ_SECTORS, OrigX, OrigY, OrigX, OrigY) != objnum)
	 {
	    if (Scale < 1.0)
	       Scale = 1.0 / ((1.0 / Scale) - 1.0);
	    else
	       Scale = Scale / 2.0;
	 }
      }
   }
   if (UseMouse)
      SetMouseCoords( PointerX, PointerY);
}



/* end of file */
