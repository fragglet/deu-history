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
int  MoveSpeed = 20;		/* movement speed */
Bool InfoShown = FALSE;		/* should we display the info bar? */


/*
   the driving program
*/

void EditLevel( int episode, int level, Bool newlevel)
{
   ReadWTextureNames();
   ReadFTextureNames();
   InitGfx();
   CheckMouseDriver();
   if (episode < 1 || level < 1)
      SelectLevel( &episode, &level);
   if (episode > 0 && level > 0)
   {
      ClearScreen();
      ReadLevelData( episode, level);
      if (newlevel)
      {
	 ForgetLevelData();
	 MapMinX = -4000;
	 MapMinY = -4000;
	 MapMaxX = 4000;
	 MapMaxY = 4000;
      }
      EditorLoop( episode, level);
      TermGfx();
      if (! Registered)
	 printf( "Please register DOOM if you want to be able to save your changes.\n");
      else if (! MadeChanges)
	 printf( "No changes made, nothing to save.\n");
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
   get the name of the new WAD file
*/

char *GetWadFileName( int episode, int level)
{
   char *outfile = GetMemory( 80);
   char *dotp;
   WadPtr wad;

   /* get the file name */
   if (! strcmp(Level->wadfile->filename, MainWad))
      sprintf( outfile, "E%dL%d.WAD", episode, level);
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
      free( outfile);
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

void DisplayHelp( int objtype, int grid)
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
   DrawScreenText( -1, -1, "+/-   - Change the map scale (current: %d)", Scale);
   DrawScreenText( -1, -1, "G     - Change the grid scale (cur.: %d)", grid);
   if (GetCurObject( objtype, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4)) >= 0)
      SetColor( DARKGRAY);
   DrawScreenText( -1, -1, "N, >  - Jump to the next object.");
   DrawScreenText( -1, -1, "P, <  - Jump to the previous object.");
   DrawScreenText( -1, -1, "J, #  - Jump to a specific object (enter #)");
   SetColor( BLACK);
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

void EditorLoop( int episode, int level)
{
   int    EditMode = OBJ_THINGS;
   int    CurObject = -1;
   int    OldObject = -1;
   Bool   RedrawMap = TRUE;
   Bool   RedrawObj = FALSE;
   Bool   DragObject = FALSE;
   int    key, altkey, buttons, oldbuttons;
   int    GridScale = 0;
   SelPtr Selected = NULL;
   char   keychar;
   int    SelBoxX, SelBoxY;
   Bool   StretchSelBox = FALSE;

   OrigX = (MapMinX + MapMaxX) / 2;
   OrigY = (MapMinY + MapMaxY) / 2;
   if (InitialScale < 1)
      InitialScale = 1;
   else if (InitialScale > 20)
      InitialScale = 20;
   Scale = InitialScale;
   MadeChanges = FALSE;
   MadeMapChanges = FALSE;
   PointerX = ScrCenterX;
   PointerY = ScrCenterY;
   if (UseMouse)
   {
      ResetMouseLimits();
      SetMouseCoords(PointerX, PointerY);
      ShowMousePointer();
      oldbuttons = 0;
   }
   else
      FakeCursor = TRUE;

   for (;;)
   {
      key = 0;

      /* get mouse position and button status */
      if (UseMouse)
      {
	 if (FakeCursor)
	    DrawPointer();
	 GetMouseCoords( &PointerX, &PointerY, &buttons);
	 if (FakeCursor)
	    DrawPointer();
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
	    else if (PointerX < 244)
	       key = 0x1800; /* 'O' */
	    else if (PointerX < ScrMaxX - 43)
	       Beep();
	    else
	       key = 0x2300; /* 'H' */
	 }
	 else
	 {
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
		  if (EditMode == OBJ_THINGS && CurObject >= 0)
		     MoveObjectsToCoords( EditMode, NULL, Things[ CurObject].xpos, Things[ CurObject].ypos, 0);
		  else if (EditMode == OBJ_VERTEXES && CurObject >= 0)
		     MoveObjectsToCoords( EditMode, NULL, Vertexes[ CurObject].x, Vertexes[ CurObject].y, 0);
		  else
		     MoveObjectsToCoords( EditMode, NULL, MAPX( PointerX), MAPY( PointerY), GridScale);
		  break;
	       case 3:
	       case 4:
		  if (SwapButtons)
		     key = 'M';
		  else
		     key = 0x000D;	/* Press middle button = Edit ('Enter') */
		  break;
	       default:
		  if (StretchSelBox) /* Release left button = End Selection Box */
		     key = 'M';
		  DragObject = FALSE; /* Release right button = End Drag */
		  break;
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
      if (RedrawMap && FakeCursor)
	 DrawPointer();

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
	    {
	       key = PullDownMenu( 18, 19,
				   "  Save     F2", 0x001B,    (int) 'S',
				   "X Print      ", -1,        (int) 'P',
				   "  Quit      Q", (int) 'Q', (int) 'Q',
				   NULL);
	       RedrawMap = TRUE;
	    }

	    else if ((key & 0xFF00) == 0x1200)  /* Scan code for E */
	    {
	       key = PullDownMenu( 66, 19,
				   "X Copy object(s)       ", -1,        (int) 'C',
				   "  Add object        Ins", 0x5200,    (int) 'A',
				   "  Delete object(s)  Del", 0x5300,    (int) 'D',
				   "  Next object         N", (int) 'N', (int) 'N',
				   "  Prev object         P", (int) 'P', (int) 'P',
				   "  Jump to object...   J", (int) 'J', (int) 'J',
				   NULL);
	       RedrawMap = TRUE;
	    }

	    else if ((key & 0xFF00) == 0x1F00)  /* Scan code for S */
	    {
	       key = PullDownMenu( 114, 19,
				   "X Find/Change       F3", -1, (int) 'F',
				   "X Repeat last find    ", -1, (int) 'R',
				   NULL);
	       RedrawMap = TRUE;
	    }

	    else if ((key & 0xFF00) == 0x1800)  /* Scan code for O */
	    {
	       key = PullDownMenu( 178, 19,
				   "Things              T", (int) 'T', (int) 'T',
				   "Linedefs+Sidedefs   L", (int) 'L', (int) 'L',
				   "Vertexes            V", (int) 'V', (int) 'V',
				   "Sectors             S", (int) 'S', (int) 'S',
				   "Next Mode         Tab", 0x0009,    (int) 'N',
				   "Last Mode   Shift+Tab", 0x0f00,    (int) 'L',
				   NULL);
	       RedrawMap = TRUE;
	    }

	    else if ((key & 0xFF00) == 0x2300)  /* Scan code for H */
	    {
	       key = PullDownMenu( ScrMaxX - 195, 19,
				   "X Getting Started     ", -1,        (int) 'G',
				   "  Keyboard & Mouse  F1", 0x3b00,    (int) 'K',
				   (InfoShown ?
				   "û Info Box           I"
				   :
				   "  Info Box           I"
				   ),                          (int) 'I', (int) 'I',
				   "X About...            ", -1,        (int) 'A',
				   NULL);
	       RedrawMap = TRUE;
	    }
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
	 if (FakeCursor)
	    DrawPointer();

	 /* user wants to exit */
	 if (keychar == 'Q')
	 {
	    ForgetSelection( &Selected);
	    if (Registered && MadeChanges)
	    {
	       char *outfile;

	       outfile = GetWadFileName( episode, level);
	       if (outfile)
	       {
		  SaveLevelData( outfile);
		  break;
	       }
	       else
		  RedrawMap = TRUE;
	    }
	    else
	       break;
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

	    outfile = GetWadFileName( episode, level);
	    if (outfile)
	       SaveLevelData( outfile);
	    RedrawMap = TRUE;
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

	 /* user wants to get the menu of misc. ops */
	 else if ((key & 0xFF00) == 0x4400) /* 'F10' */
	 {
	    if (Selected)
	       MiscOperations( -1, -1, EditMode, Selected);
	    else
	    {
	       if (CurObject >= 0)
		  SelectObject( &Selected, CurObject);
	       MiscOperations( -1, -1, EditMode, Selected);
	       if (CurObject >= 0)
		  UnSelectObject( &Selected, CurObject);
	    }
	    CurObject = -1;
	    RedrawMap = TRUE;
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	 }

	 /* user wants to display/hide the info box */
	 else if (keychar == 'I')
	 {
	    InfoShown = !InfoShown;
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the scale */
	 else if ((keychar == '+' || keychar == '=') && Scale > 1)
	 {
	    OrigX += (PointerX - ScrCenterX) * Scale;
	    OrigY += (ScrCenterY - PointerY) * Scale;
	    Scale--;
	    OrigX -= (PointerX - ScrCenterX) * Scale;
	    OrigY -= (ScrCenterY - PointerY) * Scale;
	    RedrawMap = TRUE;
	 }
	 else if ((keychar == '-' || keychar == '_') && Scale < 20)
	 {
	    OrigX += (PointerX - ScrCenterX) * Scale;
	    OrigY += (ScrCenterY - PointerY) * Scale;
	    Scale++;
	    OrigX -= (PointerX - ScrCenterX) * Scale;
	    OrigY -= (ScrCenterY - PointerY) * Scale;
	    RedrawMap = TRUE;
	 }

	 /* user wants to move */
	 else if ((key & 0xFF00) == 0x4800 && (PointerY - MoveSpeed) >= 0)
	 {
	    PointerY -= MoveSpeed;
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY);
	 }
	 else if ((key & 0xFF00) == 0x5000 && (PointerY + MoveSpeed) <= ScrMaxY)
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
	 else if ((key & 0xFF00) == 0x4D00 && (PointerX + MoveSpeed) <= ScrMaxX)
	 {
	    PointerX += MoveSpeed;
	    if (UseMouse)
	       SetMouseCoords( PointerX, PointerY);
	 }

	 /* user wants so scroll the map */
	 else if ((key & 0xFF00) == 0x4900 && MAPY( ScrCenterY) < MapMaxY)
	 {
	    OrigY += MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x5100 && MAPY( ScrCenterY) > MapMinY)
	 {
	    OrigY -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x4700 && MAPX( ScrCenterX) > MapMinX)
	 {
	    OrigX -= MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }
	 else if ((key & 0xFF00) == 0x4F00 && MAPX( ScrCenterX) < MapMaxX)
	 {
	    OrigX += MoveSpeed * 2 * Scale;
	    RedrawMap = TRUE;
	 }

	 /* user wants to change the movement speed */
	 else if (keychar == ' ')
	    MoveSpeed = MoveSpeed == 1 ? 20 : 1;

	 /* user wants to change the edit mode */
	 else if ((key & 0x00FF) == 0x0009 || (key & 0xFF00) == 0x0F00 || keychar == 'T' || keychar == 'V' || keychar == 'L' || keychar == 'S')
	 {
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
	    else if (keychar == 'T')
	       EditMode = OBJ_THINGS;
	    else if (keychar == 'V')
	       EditMode = OBJ_VERTEXES;
	    else if (keychar == 'L')
	       EditMode = OBJ_LINEDEFS;
	    else if (keychar == 'S')
	       EditMode = OBJ_SECTORS;
	    /* unselect all */
	    ForgetSelection( &Selected);
	    if (GetMaxObjectNum( EditMode) >= 0)
	       CurObject = 0;
	    else
	       CurObject = -1;
	    OldObject = -1;
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user wants to display or hide the grid */
	 else if (keychar == 'G')
	 {
	    if (GridScale == 0)
	       GridScale = 256;
	    else if (GridScale > 8)
	       GridScale /= 2;
	    else
	       GridScale = 0;
	    RedrawMap = TRUE;
	 }
	 else if (keychar == 'H')
	 {
	    GridScale = 0;
	    RedrawMap = TRUE;
	 }

	 /* user wants to toggle drag mode */
	 else if (keychar == 'D')
	 {
	    StretchSelBox = FALSE;
	    if (DragObject)
	       DragObject = FALSE;
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
	 else if ((keychar == 'N' || keychar == '>') && GetCurObject( EditMode, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4)) < 0)
	 {
	    if (CurObject < GetMaxObjectNum( EditMode))
	       CurObject++;
	    else if (GetMaxObjectNum( EditMode) >= 0)
	       CurObject = 0;
	    else
	       CurObject = -1;
	    RedrawObj = TRUE;
	 }
	 else if ((keychar == 'P' || keychar == '<') && GetCurObject( EditMode, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4)) < 0)
	 {
	    if (CurObject > 0)
	       CurObject--;
	    else
	       CurObject = GetMaxObjectNum( EditMode);
	    RedrawObj = TRUE;
	 }
	 else if ((keychar == 'J' || keychar == '#') && GetCurObject( EditMode, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4)) < 0)
	 {
	    OldObject = InputObjectNumber( -1, -1, EditMode, CurObject);
	    if (OldObject >= 0)
	       CurObject = OldObject;
	    else
	       OldObject = CurObject;
	    RedrawMap = TRUE;
	 }

	 /* user wants to mark/unmark an object or a group of objects */
	 else if (keychar == 'M')
	 {
	    if (StretchSelBox)
	    {
	       /* select all objects in the selection box */
	       StretchSelBox = FALSE;
	       RedrawMap = TRUE;
	       ForgetSelection( &Selected);
	       Selected = SelectObjectsInBox( EditMode, SelBoxX, SelBoxY, MAPX( PointerX), MAPY( PointerY));
	       if (Selected)
		  CurObject = Selected->objnum;
	       else
		  CurObject = -1;
	       if (Selected && !Quiet)
	       {
		  sound( 440);
		  delay( 10);
		  nosound();
	       }
	    }
	    else if ((bioskey( 2) & 0x03) == 0x00)  /* no shift keys */
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
		  if (Selected && !Quiet)
		  {
		     sound( 440);
		     delay( 10);
		     nosound();
		  }
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
	       EditMode = OBJ_LINEDEFS;
	       for (cur = Selected; cur->next; cur = cur->next)
	       {
		  InsertObject( EditMode, -1, 0, 0);
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
		  InsertObject( EditMode, -1, 0, 0);
		  CurObject = GetMaxObjectNum( EditMode);
		  for (cur = Selected; cur; cur = cur->next)
		  {
		     InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		     SideDefs[ NumSideDefs - 1].sector = CurObject;
		     if (LineDefs[ cur->objnum].sidedef1 >= 0)
		     {
			LineDefs[ cur->objnum].sidedef2 = NumSideDefs - 1;
			LineDefs[ cur->objnum].flags = 4;
			strncpy( SideDefs[ NumSideDefs - 1].tex3, "-", 8);
			strncpy( SideDefs[ LineDefs[ cur->objnum].sidedef1].tex3, "-", 8);
		     }
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
	    }
	    DragObject = FALSE;
	    StretchSelBox = FALSE;
	    RedrawMap = TRUE;
	 }

	 /* user likes music */
	 else if (key)
	    Beep();

	 /* redraw the (keyboard) pointer */
	 if (FakeCursor)
	    DrawPointer();
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
	       OrigY += MoveSpeed * 2 * Scale;
	       RedrawMap = TRUE;
	    }
	 }
	 if (PointerY >= ScrMaxY - (UseMouse ? 8 : 20))
	 {
	    if (! UseMouse)
	       PointerY -= MoveSpeed;
	    if (MAPY( ScrCenterY) > MapMinY)
	    {
	       OrigY -= MoveSpeed * 2 * Scale;
	       RedrawMap = TRUE;
	    }
	 }
	 if (PointerX <= (UseMouse ? 8 : 20))
	 {
	    if (! UseMouse)
	       PointerX += MoveSpeed;
	    if (MAPX( ScrCenterX) > MapMinX)
	    {
	       OrigX -= MoveSpeed * 2 * Scale;
	       RedrawMap = TRUE;
	    }
	 }
	 if (PointerX >= ScrMaxX - (UseMouse ? 8 : 20))
	 {
	    if (! UseMouse)
	       PointerX -= MoveSpeed;
	    if (MAPX( ScrCenterX) < MapMaxX)
	    {
	       OrigX += MoveSpeed * 2 * Scale;
	       RedrawMap = TRUE;
	    }
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
   if (editmode == OBJ_VERTEXES)
   {
      SetColor( LIGHTGRAY);
      for (n = 0; n < NumLineDefs; n++)
	 DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
			Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
   }
   else if (editmode == OBJ_THINGS)
      for (n = 0; n < NumLineDefs; n++)
      {
	 if (LineDefs[ n].flags & 1)
	    SetColor( WHITE);
	 else
	    SetColor( LIGHTGRAY);
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
      }
   else if (editmode == OBJ_LINEDEFS)
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
   else if (editmode == OBJ_SECTORS)
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
	 DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
		      Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
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
   setcolor( WHITE);
   DrawScreenText( 20,  4, "File  Edit  Search  Options");
   DrawScreenText( 20,  6, "_     _     _       _      ");
   DrawScreenText( ScrMaxX - 45, 4, "Help");
   DrawScreenText( ScrMaxX - 45, 6, "_   ");

   /* draw the bottom line, if needed */
   if (InfoShown)
   {
      DrawScreenBox3D( 0, ScrMaxY - 11, ScrMaxX, ScrMaxY);
      DrawScreenText( 5, ScrMaxY - 8, "Editing %s on %s", GetEditModeName( editmode), Level->dir.name);
      DrawScreenText( ScrCenterX, ScrMaxY - 8, "Free mem: %lu", farcoreleft());
      DrawScreenText( ScrMaxX - 140, ScrMaxY - 8, "Press F1 for Help");
   }
}



/* end of file */
