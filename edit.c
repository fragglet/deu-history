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

void DrawMapQuick();

extern Bool InfoShown;		/* should we display the info bar? */
BCINT MoveSpeed;

int tff = TF_EASY | TF_MEDIUM | TF_HARD | TF_DM | TF_NOT_DM;

/* use LevelNameFormat & the token list to make a
   valid level name */
void MakeLevelName(SList tokens)
{
	char *format, scratch[10];
	SList flist, tlist;
	int arg;
	
	LevelName[0] = 0;
	
	flist = LevelNameFormat;
	if(!flist)
		return;
	
	format = flist->string;
	
	while(*format) {
		if(*format == '%') {
			format++;
			/* move pointer on to the width specifier */
			
			flist = flist->next;
			if(!flist)
				ProgError("cannot make level name");
			
			if(flist->string[0] != '%')
				ProgError("garbage in level name format");
			
			sscanf((flist->string) + 1, "%d", &arg);
			/* now we know which of the tokens from the list we want */
			
			tlist = tokens;
			while(arg > 1) {
				arg--;
				tlist = tlist->next;
				if(!tlist)
					ProgError("level name format specifies too many args");
			}
			
			arg = atoi(tlist->string);
			
			sprintf(scratch, "%0*d", (*format - '0'), arg); 
			format++;
			
			/* actually add it to LevelName */
			strcat(LevelName, scratch);
		}
		else {
			LevelName[strlen(LevelName) + 1] = 0;
			LevelName[strlen(LevelName)] = *format++;
		}
	}
}


/*
   the driving program
   */

void EditLevel( Bool newlevel)
{
	MoveSpeed = DefaultLargeScroll;				/* movement speed */
    ReadWTextureNames();
    ReadFTextureNames();
    InitGfx();
    CheckMouseDriver();
    if(!(*LevelName))
    	SelectLevel();
    if(*LevelName) {
		ClearMapScreen(0);
		ReadLevelData();
		if (newlevel) {
			ForgetLevelData();
			MapMinX = -4000;
			MapMinY = -4000;
			MapMaxX = 4000;
			MapMaxY = 4000;
		}
		LogMessage( ": Editing %s...\n", LevelName);
		EditorLoop();
		LogMessage( ": Finished editing %s...\n", LevelName);
		TermGfx();
		if (!Registered)
			printf( "Please register if you want to be able to save your changes.\n");
		ForgetLevelData();
		/* forget the level pointer */
		Level = NULL;
		/* lose the level name */
		LevelName[0] = 0;
    }
    else
		TermGfx();
    ForgetWTextureNames();
    ForgetFTextureNames();
}



/*
   select a level
   */

void SelectLevel()
{
    MDirPtr dir;
    char *levels[36];
    char name[9];
    int n = 0;
	
	name[8] = 0;
    for(dir = MasterDir; dir; dir = dir->next) {
    	strncpy(name, dir->dir.name, 8);
    	if(isalev(name))
    		levels[n++] = dir->dir.name;
    }
    strcpy(name, levels[0]);
    
    InputNameFromList( -1, -1, "Select A Level To Edit:", n, levels, name);
	
	strcpy(LevelName, name);
}



/*
   get the name of the new WAD file
   */


char *GetWadFileName()
{
    char *dotp;
	static char outfile[80];
    WadPtr wad;
	
    /* get the file name */
    if (! strcmp(Level->wadfile->filename, MainWad))
    	sprintf(outfile, "%s.WAD", LevelName);
    else
		strcpy( outfile, Level->wadfile->filename);
    
    do {
		InputFileName( -1, -1, "Name Of The New WAD File:", 79, outfile);
		/* escape */
		if (outfile[ 0] == '\0')
			return NULL;
    } while (!strcmp(outfile, MainWad));
	
    /* if the WAD file already exists, rename it to "*.BAK" */
    for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( outfile, wad->filename)) {
			dotp = strrchr( wad->filename, '.');
			if (dotp == NULL)
				strcat( wad->filename, ".BAK");
			else
				strcpy( dotp, ".BAK");
			
			/* need to close, then reopen: problems with SHARE.EXE */
			fclose( wad->fileinfo);
			
			CopyFile(outfile, wad->filename);
			
			wad->fileinfo = fopen( wad->filename, "rb");
			if (wad->fileinfo == NULL)
				ProgError( "Could Not Reopen File \"%s\"", wad->filename);
			strcpy(wad->filename, outfile);
			break;
		}
    return outfile;
}



/*
   display the help screen
   */

void DisplayHelp( BCINT objtype, BCINT grid) /* SWAP! */
{
    BCINT x0 = (ScrMaxX - 8 * 45) /2;
    BCINT y0 = (ScrMaxY - 10 * 41) /2;
    
    if (UseMouse)
		HideMousePointer();
    /* put in the instructions */
    DrawScreenBox3D( x0, y0, x0 + 364, y0 + 408);
    SetColor( LIGHTCYAN);
    DrawScreenText( x0 + 60, y0 + 20, "Doom Editor for Total Headcases");
    /*DrawScreenText( 269 - strlen(GetEditModeName( objtype)) * 4, y0 + 32, "- %s Editor -", GetEditModeName( objtype));*/
    SetColor( BLACK);
    DrawScreenText( x0 + 10, y0 + 60, "Use the mouse or the cursor keys to move");
    DrawScreenText( -1, -1, "around.  The map scrolls when the pointer");
    DrawScreenText( -1, -1, "reaches the edge of the screen.");
    DrawScreenText( -1, y0 + 100, "Other useful keys are:");
    if (Registered)
        DrawScreenText( -1, y0 + 115, "Q     - Quit or SaveAs - see deth.ini");
    else {
		SetColor( DARKGRAY);
        DrawScreenText( -1, y0 + 115, "Q     - Quit without saving changes");
		SetColor( BLACK);
    }
    DrawScreenText( -1, -1, "Esc   - Exit without saving changes");
    DrawScreenText( -1, -1, "Tab   - Switch to the next editing mode");
    DrawScreenText( -1, -1, "Space - Change the move/scroll speed");
    DrawScreenText( -1, -1, "+/-   - Change the map scale (current: %d)", (BCINT) (1.0 / Scale + 0.5));
	DrawScreenText( -1, -1, "C     - Clear all marks and redraw map");
	DrawScreenText( -1, -1, "D     - Toggle drag mode");
    DrawScreenText( -1, -1, "F     - Filter Things");
    DrawScreenText( -1, -1, "G     - Change the grid scale (cur.: %d)", grid);
    DrawScreenText( -1, -1, "H     - Hide the grid");
    DrawScreenText( -1, -1, "J, #  - Jump to a specific object (enter #)");
    DrawScreenText( -1, -1, "K     - Kill the grid (set grid to 0)");
    DrawScreenText( -1, -1, "M     - Mark/unmark current object (select)");
    DrawScreenText( -1, -1, "N, >  - Jump to the next object.");
	DrawScreenText( -1, -1, "O     - Copy Objects");
    DrawScreenText( -1, -1, "P, <  - Jump to the previous object.");
	DrawScreenText( -1, -1, ",     - Rotate Thing Clockwise");
	DrawScreenText( -1, -1, ".     - Rotate Thing Anti-Clockwise");
    DrawScreenText( -1, -1, "Ins   - Insert a new object");
    DrawScreenText( -1, -1, "Del   - Delete the current object");
    DrawScreenText( -1, -1, "Enter - Edit the current/selected object(s)");
    DrawScreenText( -1, y0 + 330, "Mouse buttons:");
    if (SwapButtons) {
		DrawScreenText( -1, y0 + 340, "Left  - Edit the current/selected object(s)");
		DrawScreenText( -1, -1, "Middle- Mark/unmark the current object.");
    }
    else {
		DrawScreenText( -1, y0 + 340, "Left  - Mark/unmark the current object");
		DrawScreenText( -1, -1, "Middle- Edit the current/selected object(s)");
    }
    DrawScreenText( -1, -1, "Right - Drag the current/selected object(s)");
    SetColor( YELLOW);
    DrawScreenText( -1, y0 + 390, " Press any key to return to the editor...");
    bioskey( 0);
    if (UseMouse)
		ShowMousePointer();
}



/*
   the editor main loop
   */

#define REDRAW_NONE			0
#define REDRAW_QUICK		1
#define REDRAW_ALL			2

void EditorLoop()
{
    BCINT  EditMode = OBJ_THINGS;
    BCINT  CurObject = -1;
    BCINT  OldObject = -1;
    BCINT  RedrawMap = REDRAW_ALL;
    BCINT  LastRedraw = REDRAW_NONE;
    Bool   RedrawObj = FALSE;
    Bool   DragObject = FALSE;
    int	key, altkey;
    BCINT  buttons; 
    BCINT  oldbuttons = 0;
    BCINT  GridScale = 0;
    Bool   GridShown = TRUE;
    SelPtr Selected = NULL;
    char   keychar;
    BCINT  SelBoxX = 0;
    BCINT  SelBoxY = 0;
    Bool   StretchSelBox = FALSE;
    Bool   ShowRulers = FALSE;
    BCINT  OldPointerX = 0;
    BCINT  OldPointerY = 0;
    
    MadeChanges = FALSE;
    MadeMapChanges = FALSE;
    if (InitialScale < 1)
		InitialScale = 1;
    else if (InitialScale > 20)
		InitialScale = 20;
    Scale = (float) (1.0 / InitialScale);
    CenterMapAroundCoords( (MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);
    if (UseMouse) {
		ResetMouseLimits();
		SetMouseCoords( PointerX, PointerY);
		ShowMousePointer();
		oldbuttons = 0;
    }
    else
		FakeCursor = TRUE;
    
    for (;;) {
		key = 0;
		altkey = 0;
		
		/* get mouse position and button status */
		if (UseMouse) {
			if (FakeCursor || ShowRulers) {
				HideMousePointer();
				DrawPointer( ShowRulers);
				ShowMousePointer();
			}
			GetMouseCoords( &PointerX, &PointerY, &buttons);
			if (FakeCursor || ShowRulers) {
				HideMousePointer();
				DrawPointer( ShowRulers);
				ShowMousePointer();
			}
			if ( buttons == 1 && PointerY < 16) {
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
			else {
				if (buttons != oldbuttons) {
					switch (buttons) {
					case 1:
						if (SwapButtons)
							key = 0x000D;
						else
							key = 'M';	  /* Press left button = Mark/Unmark ('M') */
						break;
					case 2:
						if (! DragObject)
							key = 'D';	  /* Press right button = Drag */
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
						if (DragObject)	/* Release right button = End Drag */
							key = 'D';
						break;
					}
					altkey = bioskey( 2);
				}
			}
			oldbuttons = buttons;
		}
		
		/* drag object(s) */
		if (DragObject) {
			BCINT forgetit = FALSE;
			
			if (IsSelected( Selected, CurObject) == FALSE)
				ForgetSelection( &Selected);
			else if (Selected->objnum != CurObject) {
				/* current object must be first in the list */
				UnSelectObject( &Selected, CurObject);
				SelectObject( &Selected, CurObject);
			}
			if (Selected == NULL && CurObject >= 0) {
				SelectObject( &Selected, CurObject);
				forgetit = TRUE;
			}
			if (Selected) {
				if (MoveObjectsToCoords( EditMode, Selected, MAPX( PointerX), MAPY( PointerY), GridScale))
					RedrawMap = REDRAW_ALL;
				if (forgetit)
					ForgetSelection( &Selected);
			}
			else {
				Beep();
				DragObject = FALSE;
			}
		}
		else if (StretchSelBox) {
			BCINT x = MAPX( PointerX);
			BCINT y = MAPY( PointerY);
			
			/* draw selection box */
			SetColor( CYAN);
			setwritemode( XOR_PUT);
			DrawMapLine( SelBoxX, SelBoxY, SelBoxX, y);
			DrawMapLine( SelBoxX, y, x, y);
			DrawMapLine( x, y, x, SelBoxY);
			DrawMapLine( x, SelBoxY, SelBoxX, SelBoxY);
			delay( 50);
			DrawMapLine( SelBoxX, SelBoxY, SelBoxX, y);
			DrawMapLine( SelBoxX, y, x, y);
			DrawMapLine( x, y, x, SelBoxY);
			DrawMapLine( x, SelBoxY, SelBoxX, SelBoxY);
			setwritemode( COPY_PUT);
		}
		else if (!RedrawObj) {
			/* check if there is something near the pointer */
			OldObject = CurObject;
			if ((bioskey( 2) & 0x03) == 0x00)  /* no shift keys */
				CurObject = GetCurObject( EditMode, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4));
			if (CurObject < 0)
				CurObject = OldObject;
		}
		
		/* draw the map */
		if((RedrawMap == REDRAW_NONE) &&
		   (LastRedraw == REDRAW_QUICK))
			RedrawMap = REDRAW_ALL;
		
		if (RedrawMap == REDRAW_ALL) {
			DrawMap( EditMode, GridScale, GridShown);
			HighlightSelection( EditMode, Selected);
		}
		else if(RedrawMap == REDRAW_QUICK)
			DrawMapQuick();
		
		/* highlight the current object and display the information box */
		if (RedrawMap == REDRAW_ALL || CurObject != OldObject || RedrawObj) {
			RedrawObj = FALSE;
			if (!RedrawMap && OldObject >= 0)
				HighlightObject( EditMode, OldObject, YELLOW); 
			if (CurObject != OldObject) {
				PlaySound( 50, 10);
				OldObject = CurObject;
			}
			if (CurObject >= 0)
				HighlightObject( EditMode, CurObject, YELLOW);
			if (bioskey( 1)) /* speedup */
				RedrawObj = TRUE;
			else
				DisplayObjectInfo( EditMode, CurObject);
		}
		if (RedrawMap && (FakeCursor || ShowRulers)) {
			if (UseMouse)
				HideMousePointer();
			DrawPointer( ShowRulers);
			if (UseMouse)
				ShowMousePointer();
		}
		
		/* display the current pointer coordinates */
		if (RedrawMap == REDRAW_ALL || PointerX != OldPointerX || PointerY != OldPointerY) {
			SetColor(DARKGRAY);
			DrawScreenBox( ScrMaxX - 170, 5, ScrMaxX - 70, 12);
			SetColor( BLUE);
			DrawScreenText( ScrMaxX - 170, 5, "%d, %d", MAPX( PointerX), MAPY( PointerY));
			OldPointerX = PointerX;
			OldPointerY = PointerY;
		}
		
		/* the map is up to date */
		LastRedraw = RedrawMap;
		RedrawMap = REDRAW_NONE;
		
		/* get user input */
		if (bioskey( 1) || key) {
			if (! key) {
				key = bioskey( 0);
				altkey = bioskey( 2);
			}
			
			/* user wants to access the drop-down menus */
			if (altkey & 0x08) {   /* if alt is pressed... */
				if ((key & 0xFF00) == 0x2100)	   /* Scan code for F */
					key = PullDownMenu( -1, -1,
									   "Save           F2", 0x3C00,    (BCINT) 'S', 1,
									   "Save As...     F3", 0x3D00,    (BCINT) 'A', 6,
									   "Quit            Q", (BCINT) 'Q', (BCINT) 'Q', 1,
									   NULL); 
				else if ((key & 0xFF00) == 0x1200) {  /* Scan code for E */
					
					key = PullDownMenu( -1, -1,
									   "Copy Object(s)      O", (BCINT) 'O', (BCINT) 'C', 1,
									   "Add Object        Ins", 0x5200,    (BCINT) 'A', 1,
									   "Delete Object(s)  Del", 0x5300,    (BCINT) 'D', 1,
									   ((EditMode == OBJ_VERTEXES) ?
										NULL :
										"Preferences        F5"), 0x3F00,   (BCINT) 'P', 1,
									   NULL);
				}
				else if ((key & 0xFF00) == 0x1F00)  /* Scan code for S */
					key = PullDownMenu( -1, -1,
									   "Find/Mark         F4", 0x3e00,      (BCINT) 'F', 1,
									   "Repeat Last Find   A", (BCINT) 'A', (BCINT) 'R', 1,
									   "Next Object        N", (BCINT) 'N', (BCINT) 'N', 1,
									   "Prev Object        P", (BCINT) 'P', (BCINT) 'P', 1,
									   "Jump To Object...  J", (BCINT) 'J', (BCINT) 'J', 1,
									   NULL);
				else if ((key & 0xFF00) == 0x3200)  /* Scan code for M */
					key = PullDownMenu( -1, -1,
									   ((EditMode == OBJ_THINGS) ?
										"û Things              T" :
										"  Things              T"), (BCINT) 'T', (BCINT) 'T', 3,
									   ((EditMode == OBJ_LINEDEFS) ?
										"û LineDefs+SideDefs   L" :
										"  LineDefs+SideDefs   L"), (BCINT) 'L', (BCINT) 'L', 3,
									   ((EditMode == OBJ_VERTEXES) ?
										"û Vertexes            V" :
										"  Vertexes            V"), (BCINT) 'V', (BCINT) 'V', 3,
									   ((EditMode == OBJ_SECTORS) ?
										"û Sectors             S" :
										"  Sectors             S"), (BCINT) 'S', (BCINT) 'S', 3,
									   "  Next Mode         Tab",  0x0009,    (BCINT) 'N', 3,
									   "  Last Mode   Shift+Tab",  0x0F00,    (BCINT) 'L', 3,
									   NULL);
				else if ((key & 0xFF00) == 0x1700) { /* Scan code for I */
					key = 0;
					/* code duplicated from 'F8' - I hate to do that */
					if (Selected)
						MiscOperations( 234, 17, EditMode, &Selected);
					else {
						if (CurObject >= 0)
							SelectObject( &Selected, CurObject);
						MiscOperations( 234, 17, EditMode, &Selected);
						if (CurObject >= 0)
							UnSelectObject( &Selected, CurObject);
					}
					CurObject = -1;
					DragObject = FALSE;
					StretchSelBox = FALSE;
				}
				else if ((key & 0xFF00) == 0x1800) { /* Scan code for O */
					BCINT savednum, i;
					
					key = 0;
					/* don't want to create the object behind the menu bar... */
					if (PointerY < 20) {
						PointerX = ScrCenterX;
						PointerY = ScrCenterY;
					}
					/* code duplicated from 'F9' - I hate to do that */
					savednum = NumLineDefs;
					InsertStandardObject( 282, 17, MAPX( PointerX), MAPY( PointerY));
					if (NumLineDefs > savednum) {
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
				else if ((key & 0xFF00) == 0x2E00) { /* Scan code for C */
					key = 0;
					CheckLevel( 354, 17);
				}
				else if ((key & 0xFF00) == 0x2300)  /* Scan code for H */
					key = PullDownMenu( -1, -1,
									   "  Keyboard & Mouse  F1",  0x3B00,    (BCINT) 'K', 3,
									   (InfoShown ?
										"û Info Bar           I" :
										"  Info Bar           I"), (BCINT) 'I', (BCINT) 'I', 3,
									   "  About DETH...       ",  -1,	(BCINT) 'A', 3 ,
									   NULL);
				else {
					Beep();
					key = 0;
				}
				RedrawMap = REDRAW_ALL;
			}
			
			/* User wants to do the impossible. */
			if (key == -1) {
				NotImplemented();
				RedrawMap = REDRAW_ALL;
			}
			
			
			/* simplify the checks later on */
			if (isprint(key & 0x00ff)) 
				keychar = toupper(key & 0x00ff);
			else
				keychar = '\0';
			
			
			/* erase the (keyboard) pointer */
			if (FakeCursor || ShowRulers) {
				HideMousePointer();
				DrawPointer( ShowRulers);
				ShowMousePointer();
			}
			
			if(key == 0x3e00 && EditMode == OBJ_THINGS) {
				FindThing(0, &Selected);
				RedrawMap = REDRAW_ALL;
			}
			else if(keychar == 'A' && EditMode == OBJ_THINGS) {
				FindThing(1, NULL);
				RedrawMap = REDRAW_ALL;
			}

			else if(keychar == 'F' && EditMode == OBJ_THINGS) {
				tff = SetThingFlagFilter(tff);
				RedrawMap = REDRAW_ALL;
			}
			
			
			/* user wants to exit            AJB   */
			else if (keychar == 'Q' && QisQuit == TRUE) {
				if (!MadeChanges || Confirm(-1, -1,
											"You Have Unsaved Changes.  Do You Really Want To Quit?",
											NULL))
					break;
				RedrawMap = REDRAW_ALL;
			} 
			else if ((key & 0x00FF) == 0x001B) { /* 'Esc' */
				if (DragObject)
					DragObject = FALSE;						  
				else if (StretchSelBox)
					StretchSelBox = FALSE;
				else {
					ForgetSelection( &Selected);
					if (!MadeChanges || Confirm(-1, -1,
												"You Have Unsaved Changes.  Do You Really Want To Quit?",
												NULL))
						break;
					RedrawMap = REDRAW_ALL;
				}
			}
			
			/* user is lost */
			else if ((key & 0xFF00) == 0x3B00) { /* 'F1' */
				DisplayHelp( EditMode, GridScale);
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to save the level data */
			else if (((key & 0xFF00) == 0x3C00 || (keychar == 'Q')) && Registered) { /* 'F2' */
				char *outfile = GetWadFileName();
				if (outfile)
					SaveLevelData( outfile);
				RedrawMap = REDRAW_ALL;
				DrawMapQuick();
			}
			
			/* user wants to save and change the episode and mission numbers */
			else if ((key & 0xFF00) == 0x3D00 && Registered) { /* 'F3' */
				char *outfile;
				MDirPtr newLevel, oldl, newl;
				char name[ 9];
				BCINT i;
				
				if (CheckStartingPos()) {
					outfile = GetWadFileName();
					if (outfile) {
						SelectLevel();
						if (*LevelName) {
							/* horrible but it works... */
							sprintf( name, "%s", LevelName);
							newLevel = FindMasterDir( MasterDir, name);
							oldl = Level;
							newl = newLevel;
							for (i = 0; i < 11; i++) {
								newl->wadfile = oldl->wadfile;
								if (i > 0)
									newl->dir = oldl->dir;
								oldl = oldl->next;
								newl = newl->next;
							}
							Level = newLevel;
						}
						SaveLevelData( outfile);
					}
				}
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to get the 'Preferences' menu */
			else if ((key & 0xFF00) == 0x3F00) { /* 'F5' */
				Preferences( -1, -1);
				RedrawMap = REDRAW_ALL;
			}

			
			/* user wants to get the menu of misc. ops */
			else if ((key & 0xFF00) == 0x4200) { /* 'F8' */
				if (Selected)
					MiscOperations( -1, -1, EditMode, &Selected);
				else {
					if (CurObject >= 0)
						SelectObject( &Selected, CurObject);
					MiscOperations( -1, -1, EditMode, &Selected);
					if (CurObject >= 0)
						UnSelectObject( &Selected, CurObject);
				}
				CurObject = -1;
				RedrawMap = REDRAW_ALL;
				DragObject = FALSE;
				StretchSelBox = FALSE;
			}
			
			/* user wants to insert a standard shape */
			else if ((key & 0xFF00) == 0x4300) { /* 'F9' */
				BCINT savednum, i;
				
				savednum = NumLineDefs;
				InsertStandardObject( -1, -1, MAPX( PointerX), MAPY( PointerY));
				if (NumLineDefs > savednum) {
					ForgetSelection( &Selected);
					EditMode = OBJ_LINEDEFS;
					for (i = savednum; i < NumLineDefs; i++)
						SelectObject( &Selected, i);
					CurObject = NumLineDefs - 1;
					OldObject = -1;
					DragObject = FALSE;
					StretchSelBox = FALSE;
				}
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to check his level */
			else if ((key & 0xFF00) == 0x4400) { /* 'F10' */
				CheckLevel( -1, -1);
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to display/hide the info box */
			else if (keychar == 'I') {
				InfoShown = !InfoShown;
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to change the scale */
			/*AJB*/
			else if ((keychar == '+' || keychar == '=') && Scale < 16.0) { 
				OrigX += (BCINT) ((PointerX - ScrCenterX) / Scale);
				OrigY += (BCINT) ((ScrCenterY - PointerY) / Scale);
				if (Scale < 1.0)
					Scale = 1.0 / ((1.0 / Scale) - 1.0);
				else
					Scale = Scale * 2.0;
				OrigX -= (BCINT) ((PointerX - ScrCenterX) / Scale);
				OrigY -= (BCINT) ((ScrCenterY - PointerY) / Scale);
				RedrawMap = REDRAW_ALL;
			}
			else if ((keychar == '-' || keychar == '_') && Scale > 0.05) {
				OrigX += (BCINT) ((PointerX - ScrCenterX) / Scale);
				OrigY += (BCINT) ((ScrCenterY - PointerY) / Scale);
				if (Scale < 1.0)
					Scale = 1.0 / ((1.0 / Scale) + 1.0);
				else
					Scale = Scale / 2.0;
				OrigX -= (BCINT) ((PointerX - ScrCenterX) / Scale);
				OrigY -= (BCINT) ((ScrCenterY - PointerY) / Scale);
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to set the scale directly */
			else if (keychar >= '0' && keychar <= '9') {
				OrigX += (BCINT) ((PointerX - ScrCenterX) / Scale);
				OrigY += (BCINT) ((ScrCenterY - PointerY) / Scale);
				if (keychar == '0')
					Scale = 0.1;
				else
					Scale = 1.0 / (keychar - '0');
				OrigX -= (BCINT) ((PointerX - ScrCenterX) / Scale);
				OrigY -= (BCINT) ((ScrCenterY - PointerY) / Scale);
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to move */
			else if ((key & 0xFF00) == 0x4800 && (PointerY - MoveSpeed) >= 0) {
				if (UseMouse)
					SetMouseCoords( PointerX, PointerY - MoveSpeed);
				else
					PointerY -= MoveSpeed;
			}
			else if ((key & 0xFF00) == 0x5000 && (PointerY + MoveSpeed) <= ScrMaxY) {
				if (UseMouse)
					SetMouseCoords( PointerX, PointerY + MoveSpeed);
				else
					PointerY += MoveSpeed;
			}
			else if ((key & 0xFF00) == 0x4B00 && (PointerX - MoveSpeed) >= 0) {
				if (UseMouse)
					SetMouseCoords( PointerX - MoveSpeed, PointerY);
				else
					PointerX -= MoveSpeed;
			}
			else if ((key & 0xFF00) == 0x4D00 && (PointerX + MoveSpeed) <= ScrMaxX) {
				if (UseMouse)
					SetMouseCoords( PointerX + MoveSpeed, PointerY);
				else
					PointerX += MoveSpeed;
			}
			
			/* user wants so scroll the map */
			else if ((key & 0xFF00) == 0x4900 && MAPY( ScrCenterY) < MapMaxY) {
				OrigY += (BCINT) (ScrCenterY / Scale);
				RedrawMap = REDRAW_ALL;
			}
			else if ((key & 0xFF00) == 0x5100 && MAPY( ScrCenterY) > MapMinY) {
				OrigY -= (BCINT) (ScrCenterY / Scale);
				RedrawMap = REDRAW_ALL;
			}
			else if ((key & 0xFF00) == 0x4700 && MAPX( ScrCenterX) > MapMinX) {
				OrigX -= (BCINT) (ScrCenterX / Scale);
				RedrawMap = REDRAW_ALL;
			}
			else if ((key & 0xFF00) == 0x4F00 && MAPX( ScrCenterX) < MapMaxX) {
				OrigX += (BCINT) (ScrCenterX / Scale);
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to change the movement speed */
			else if (keychar == ' ')
				MoveSpeed = MoveSpeed == 2 ? DefaultLargeScroll : 2;
			
			/* user wants to change the edit mode */
			else if ((key & 0x00FF) == 0x0009 ||
					 (key & 0xFF00) == 0x0F00 ||
					 keychar == 'T' || keychar == 'V'
					 || keychar == 'L' || keychar == 'S') {
				BCINT PrevMode = EditMode;
				SelPtr NewSel;
				
				if ((key & 0x00FF) == 0x0009) { /* 'Tab' */
					switch (EditMode) {
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
				else if ((key & 0xFF00) == 0x0F00) { /* 'Shift-Tab' */
					switch (EditMode) {
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
				else {
					if (keychar == 'T')
						EditMode = OBJ_THINGS;
					else if (keychar == 'V')
						EditMode = OBJ_VERTEXES;
					else if (keychar == 'L')
						EditMode = OBJ_LINEDEFS;
					else if (keychar == 'S')
						EditMode = OBJ_SECTORS;
					ForgetSelection( &Selected);
				}
				
				/* special cases for the selection list... */
				if (Selected) {
					/* select all LineDefs bound to the selected Sectors */
					if (PrevMode == OBJ_SECTORS && EditMode == OBJ_LINEDEFS) {
						int l, sd;
						
						ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
						NewSel = NULL;
						for (l = 0; l < NumLineDefs; l++) {
							sd = LineDefs[ l].sidedef1;
							if (sd >= 0 && IsSelected( Selected, SideDefs[ sd].sector))
								SelectObject( &NewSel, l);
							else {
								sd = LineDefs[ l].sidedef2;
								if (sd >= 0 && IsSelected( Selected, SideDefs[ sd].sector))
									SelectObject( &NewSel, l);
							}
						}
						ForgetSelection( &Selected);
						Selected = NewSel;
					}
					/* select all Vertices bound to the selected LineDefs */
					else if (PrevMode == OBJ_LINEDEFS && EditMode == OBJ_VERTEXES) {
						ObjectsNeeded( OBJ_LINEDEFS, 0);
						NewSel = NULL;
						while (Selected) {
							if (!IsSelected( NewSel, LineDefs[ Selected->objnum].start))
								SelectObject( &NewSel, LineDefs[ Selected->objnum].start);
							if (!IsSelected( NewSel, LineDefs[ Selected->objnum].end))
								SelectObject( &NewSel, LineDefs[ Selected->objnum].end);
							UnSelectObject( &Selected, Selected->objnum);
						}
						Selected = NewSel;
					}
					/* select all Sectors that have their LineDefs selected */
					else if (PrevMode == OBJ_LINEDEFS && EditMode == OBJ_SECTORS) {
						int l, sd;
						
						ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
						NewSel = NULL;
						/* select all Sectors... */
						for (l = 0; l < NumSectors; l++)
							SelectObject( &NewSel, l);
						/* ... then unselect those that should not be in the list */
						for (l = 0; l < NumLineDefs; l++)
							if (!IsSelected( Selected, l)) {
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
					else if (PrevMode == OBJ_VERTEXES && EditMode == OBJ_LINEDEFS) {
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
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to change the grid scale */
			else if (keychar == 'G') {
				if ((altkey & 0x03) == 0x00) { /* no shift keys */
					if (GridScale == 0)
						GridScale = 512;
					else if (GridScale > 4)
						GridScale /= 2;
					else
						GridScale = 0;
				}
				else {
					if (GridScale == 0)
						GridScale = 4;
					else if (GridScale < 512)
						GridScale *= 2;
					else
						GridScale = 0;
				}
				RedrawMap = REDRAW_ALL;
			}
			else if (keychar == 'K') {
				if ((altkey & 0x03) == 0x00)  /* no shift keys */
					GridScale = 0;
				RedrawMap = REDRAW_ALL;
			}
			else if (keychar == 'H') {
				if ((altkey & 0x03) != 0x00)  /* shift key pressed */
					GridScale = 0;
				else
					GridShown = !GridShown;
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to toggle the rulers */
			else if (keychar == 'R')
				ShowRulers = !ShowRulers;
			
			
			/* user wants to toggle drag mode */
			else if (keychar == 'D') {
				StretchSelBox = FALSE;
				if (DragObject) {
					DragObject = FALSE;
					if (EditMode == OBJ_VERTEXES) {
						if (Selected == NULL && CurObject >= 0) {
							SelectObject( &Selected, CurObject);
							if (AutoMergeVertices( &Selected))
								RedrawMap = REDRAW_ALL;
							ForgetSelection( &Selected);
						}
						else
							if (AutoMergeVertices( &Selected))
								RedrawMap = REDRAW_ALL;
					}
					else if (EditMode == OBJ_LINEDEFS) {
						SelPtr NewSel, cur;
						
						ObjectsNeeded( OBJ_LINEDEFS, 0);
						NewSel = NULL;
						if (Selected == NULL && CurObject >= 0) {
							SelectObject( &NewSel, LineDefs[ CurObject].start);
							SelectObject( &NewSel, LineDefs[ CurObject].end);
						}
						else {
							for (cur = Selected; cur; cur = cur->next) {
								if (!IsSelected( NewSel, LineDefs[ cur->objnum].start))
									SelectObject( &NewSel, LineDefs[ cur->objnum].start);
								if (!IsSelected( NewSel, LineDefs[ cur->objnum].end))
									SelectObject( &NewSel, LineDefs[ cur->objnum].end);
							}
						}
						if (AutoMergeVertices( &NewSel))
							RedrawMap = REDRAW_ALL;
						ForgetSelection( &NewSel);
					}
				}
				else {
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
			else if (keychar == 'N' || keychar == '>') {
				if (CurObject < GetMaxObjectNum( EditMode))
					CurObject++;
				else if (GetMaxObjectNum( EditMode) >= 0)
					CurObject = 0;
				else
					CurObject = -1;
				RedrawMap = REDRAW_ALL;
			}
			else if (keychar == 'P' || keychar == '<') {
				if (CurObject > 0)
					CurObject--;
				else
					CurObject = GetMaxObjectNum( EditMode);
				RedrawMap = REDRAW_ALL;
			}
			else if (keychar == 'J' || keychar == '#') {
				OldObject = InputObjectNumber( -1, -1, EditMode, CurObject);
				if (OldObject >= 0) {
					CurObject = OldObject;
					GoToObject( EditMode, CurObject);
				}
				else
					OldObject = CurObject;
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to mark/unmark an object or a group of objects */
			else if (keychar == 'M') {
				if (StretchSelBox) {
					SelPtr oldsel;
					
					/* select all objects in the selection box */
					StretchSelBox = FALSE;
					RedrawMap = REDRAW_ALL;
					/* additive selection box or not? */
					if (AdditiveSelBox == FALSE)
						ForgetSelection( &Selected);
					else
						oldsel = Selected;
					Selected = SelectObjectsInBox( EditMode, SelBoxX, SelBoxY, MAPX( PointerX), MAPY( PointerY));
					if (AdditiveSelBox == TRUE)
						while (oldsel != NULL) {
							if (! IsSelected( Selected, oldsel->objnum))
								SelectObject( &Selected, oldsel->objnum);
							UnSelectObject( &oldsel, oldsel->objnum);
						}
					if (Selected) {
						CurObject = Selected->objnum;
						PlaySound( 440, 10);
					}
					else
						CurObject = -1;
				}
				else if ((altkey & 0x03) == 0x00)  { /* no shift keys */
					if (CurObject >= 0) {
						/* mark or unmark one object */
						if (IsSelected( Selected, CurObject))
							UnSelectObject( &Selected, CurObject);
						else
							SelectObject( &Selected, CurObject);
						HighlightObject( EditMode, CurObject, GREEN);
						if (Selected)
							PlaySound( 440, 10);
						DragObject = FALSE;
					}
					else
						Beep();
				}
				else {
					/* begin "stretch selection box" mode */
					SelBoxX = MAPX( PointerX);
					SelBoxY = MAPY( PointerY);
					StretchSelBox = TRUE;
					DragObject = FALSE;
				}
			}
			
			/* user wants to clear all marks and redraw the map */
			else if (keychar == 'C') {
				ForgetSelection( &Selected);
				RedrawMap = REDRAW_ALL;
				DragObject = FALSE;
				StretchSelBox = FALSE;
			}
			
			/* user wants to copy a group of objects */
			else if (keychar == 'O' && CurObject >= 0) {
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
				RedrawMap = REDRAW_ALL;
				StretchSelBox = FALSE;
			}

			else if (key == 5140) { /* CTRL 'T' */
				char msg[ 80];
				sprintf( msg, "Next Free Tag Number: %d", FindFreeTag());
				Notify( -1, -1, msg, NULL); 
				RedrawMap = REDRAW_ALL;
			}

			else if ((key == 9226 || key == 8196) && EditMode == OBJ_VERTEXES) { /* CTRL 'J or D' */
				DeleteVerticesJoinLineDefs( Selected);
				ForgetSelection ( &Selected);
				RedrawMap = REDRAW_ALL;
			}

			else if ((key == 9226 || key == 8196) && EditMode == OBJ_LINEDEFS) { /* CTRL 'J or D' */
				DeleteLineDefsJoinSectors ( &Selected);
				RedrawMap = REDRAW_ALL; 
			}

			else if (key == 12054 && EditMode == OBJ_LINEDEFS) { /* CTRL 'V' */
				SplitLineDefs( Selected);
				RedrawMap = REDRAW_ALL;
			}
			
			else if (key == 8454 && EditMode == OBJ_LINEDEFS) { /* CTRL 'F' */
				FlipLineDefs( Selected, TRUE);
				RedrawMap = REDRAW_ALL;
			}

			else if (key == 7955 && EditMode == OBJ_LINEDEFS) { /* CTRL 'S' */
				FlipLineDefs( Selected, FALSE);
				RedrawMap = REDRAW_ALL; 
			}


			else if (key == 12813 && EditMode == OBJ_VERTEXES) { /* CTRL 'M' */
				MergeVertices ( &Selected);
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 11544 && EditMode == OBJ_LINEDEFS) { /* CTRL 'X' */
				AlignTexturesX ( &Selected, TRUE, FALSE);
				RedrawMap = REDRAW_ALL; 
			}

			else if (key == 11640 && EditMode == OBJ_LINEDEFS) { /* 'X' */
				AlignTexturesX ( &Selected, FALSE, FALSE);
				RedrawMap = REDRAW_ALL; 
			}

			else if (key == 5401 && EditMode == OBJ_LINEDEFS) { /* CTRL 'Y' */
				AlignTexturesY ( &Selected, TRUE, FALSE);
				RedrawMap = REDRAW_ALL; 
			}

			else if (key == 5497 && EditMode == OBJ_LINEDEFS) { /* 'Y' */
				AlignTexturesY ( &Selected, FALSE, FALSE);
				RedrawMap = REDRAW_ALL; 
			}

			else if (key == 12813 && EditMode == OBJ_LINEDEFS) { /* CTRL 'M' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 0;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 6416 && EditMode == OBJ_LINEDEFS) { /* CTRL 'P' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 1;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 768 && EditMode == OBJ_LINEDEFS) { /* CTRL '2' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 2;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 5653 && EditMode == OBJ_LINEDEFS) { /* CTRL 'U' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 3;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 9740 && EditMode == OBJ_LINEDEFS) { /* CTRL 'L' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 4;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 4882 && EditMode == OBJ_LINEDEFS) { /* CTRL 'R' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 5;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 12290 && EditMode == OBJ_LINEDEFS) { /* CTRL 'B' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 6;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 8968 && EditMode == OBJ_LINEDEFS) {  /* CTRL 'H' */ 
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 7;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			}  

			else if (key == 6159 && EditMode == OBJ_LINEDEFS) { /* CTRL 'O' */
				SelPtr cur;
				for(cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].flags ^= 0x01 << 8;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL; 
			} 

			else if (key == 12558 && EditMode == OBJ_LINEDEFS) { /* CTRL 'N' */
				SelPtr cur;
				for (cur = Selected; cur; cur = cur->next) {
					LineDefs[ cur->objnum].type = 0;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL;
			}

			else if (key == 12558 && EditMode == OBJ_SECTORS) { /* CTRL 'N' */
				SelPtr cur;
				for (cur = Selected; cur; cur = cur->next) {
					Sectors[ cur->objnum].special = 0;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL;
			}

			else if (key == 8196 && EditMode == OBJ_THINGS) { /* CTRL 'D' */
				SelPtr cur;
				for (cur = Selected; cur; cur = cur->next) {
					Things[ cur->objnum].when ^= 0x08;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL;
			}

			else if (key == 12813 && EditMode == OBJ_THINGS) {  /* CTRL 'M' */ 
				SelPtr cur;
				for (cur = Selected; cur; cur = cur->next) {
					Things[ cur->objnum].when ^= 0x10;
				}
				MadeChanges = TRUE;
				RedrawMap = REDRAW_ALL;
			}


			else if (key == 7681 && EditMode == OBJ_LINEDEFS) { /* CTRL 'A' */
				if (( Selected)->next == NULL || ( Selected)->next->next != NULL) {
					Beep();
					Notify( -1, -1, "You Must Select Exactly Two LineDefs", NULL);
					RedrawMap = REDRAW_ALL;
				}
				else {
					SplitLineDefsAndSector( ( Selected)->objnum, ( Selected)->next->objnum);
					ForgetSelection( &Selected);
					RedrawMap = REDRAW_ALL;
				}
			}

			else if (key == 7681 && EditMode == OBJ_VERTEXES) { /* CTRL 'A' */
				if (( Selected)->next == NULL || ( Selected)->next->next != NULL) {
					Beep();
					Notify( -1, -1, "You Must Select Exactly Two Vertices", NULL);
					RedrawMap = REDRAW_ALL;
				}
				else {
					SplitSector( ( Selected)->objnum, ( Selected)->next->objnum);
					ForgetSelection( &Selected);
					RedrawMap = REDRAW_ALL;
				}
			}







			/* user wants to rotate things */
			else if((keychar == ',') || (keychar == '.')) {
				if(EditMode == OBJ_THINGS) {
					BCINT temp, rot = (keychar == ',') ? 45 : 315;
					SelPtr cur;
					
					for(cur = Selected; cur; cur = cur->next) {
						if(cur->objnum != CurObject) {
							temp = (Things[cur->objnum].angle + rot) % 360;
							Things[cur->objnum].angle = temp;
						}
					}
					
					temp = (Things[CurObject].angle + rot) % 360;
					Things[CurObject].angle = temp;
					
					RedrawMap = REDRAW_ALL;
				}
			}
			
			
			/* user wants to edit the current object */
			else if ((key & 0x00FF) == 0x000D && CurObject >= 0) { /* 'Enter' */
				if (Selected)
					EditObjectsInfo( 0, 30, EditMode, Selected);
				else {
					SelectObject( &Selected, CurObject);
					EditObjectsInfo( 0, 30, EditMode, Selected);
					UnSelectObject( &Selected, CurObject);
				}
				RedrawMap = REDRAW_ALL;
				DragObject = FALSE;
				StretchSelBox = FALSE;
			}
			
			/* user wants to delete the current object */
			else if ((key & 0xFF00) == 0x5300 && CurObject >= 0) { /* 'Del' */
				if (EditMode == OBJ_THINGS || Expert || Confirm( -1, -1,
																(Selected ? "Do You Really Want To Delete These Objects?" : "Do You Really Want To Delete This Object?"),
																(Selected ? "This Will Also Delete The Objects Bound To Them." : "This Will Also Delete The Objects Bound To It."))) {
					if (Selected)
						DeleteObjects( EditMode, &Selected);
					else
						DeleteObject( EditMode, CurObject);
					CurObject = -1;
				}
				DragObject = FALSE;
				StretchSelBox = FALSE;
				RedrawMap = REDRAW_ALL;
			}
			
			/* user wants to insert a new object */
			else if ((key & 0xFF00) == 0x5200) { /* 'Ins' */
				SelPtr cur;
				
				/* first special case: if several Vertices are selected, add new LineDefs */
				if (EditMode == OBJ_VERTEXES && Selected != NULL && Selected->next != NULL) {
					BCINT firstv;
					
					ObjectsNeeded( OBJ_LINEDEFS, 0);
					if (Selected->next->next != NULL)
						firstv = Selected->objnum;
					else
						firstv = -1;
					EditMode = OBJ_LINEDEFS;
					/* create LineDefs between the Vertices */
					for (cur = Selected; cur->next; cur = cur->next) {
						/* check if there is already a LineDef between the two Vertices */
						for (CurObject = 0; CurObject < NumLineDefs; CurObject++)
							if ((LineDefs[ CurObject].start == cur->next->objnum && LineDefs[ CurObject].end == cur->objnum)
								|| (LineDefs[ CurObject].end == cur->next->objnum && LineDefs[ CurObject].start == cur->objnum))
								break;
						if (CurObject < NumLineDefs)
							cur->objnum = CurObject;
						else {
							InsertObject( OBJ_LINEDEFS, -1, 0, 0);
							CurObject = NumLineDefs - 1;
							LineDefs[ CurObject].start = cur->next->objnum;
							LineDefs[ CurObject].end = cur->objnum;
							cur->objnum = CurObject;
						}
					}
					/* close the polygon if there are more than 2 Vertices */
					if (firstv >= 0 && (altkey & 0x03) != 0x00)  { /* shift key pressed */
						for (CurObject = 0; CurObject < NumLineDefs; CurObject++)
							if ((LineDefs[ CurObject].start == firstv && LineDefs[ CurObject].end == cur->objnum)
								|| (LineDefs[ CurObject].end == firstv && LineDefs[ CurObject].start == cur->objnum))
								break;
						if (CurObject < NumLineDefs)
							cur->objnum = CurObject;
						else {
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
				else if (EditMode == OBJ_LINEDEFS && Selected != NULL) {
					ObjectsNeeded( OBJ_LINEDEFS, 0);
					for (cur = Selected; cur; cur = cur->next)
						if (LineDefs[ cur->objnum].sidedef1 >= 0 && LineDefs[ cur->objnum].sidedef2 >= 0) {
							char msg[ 80];
							
							Beep();
							sprintf( msg, "LineDef #%d Already Has Two SideDefs", cur->objnum);
							Notify( -1, -1, "Error: Cannot Add The New Sector", msg);
							break;
						}
					if (cur == NULL) {
						EditMode = OBJ_SECTORS;
						InsertObject( OBJ_SECTORS, -1, 0, 0);
						CurObject = NumSectors - 1;
						for (cur = Selected; cur; cur = cur->next) {
							InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
							SideDefs[ NumSideDefs - 1].sector = CurObject;
							ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
							if (LineDefs[ cur->objnum].sidedef1 >= 0) {
								BCINT s;
								
								s = SideDefs[ LineDefs[ cur->objnum].sidedef1].sector;
								if (s >= 0) {
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
				else {
					ForgetSelection( &Selected);
					if (GridScale > 0)
						InsertObject( EditMode, CurObject, (BCINT) ((int)(MAPX( PointerX) + GridScale / 2)) & ((int)(~(GridScale - 1))), (BCINT) ((int)(MAPY( PointerY) + GridScale / 2)) & (int)(~(GridScale - 1)));
					else
						InsertObject( EditMode, CurObject, MAPX( PointerX), MAPY( PointerY));
					CurObject = GetMaxObjectNum( EditMode);
					if (EditMode == OBJ_LINEDEFS) {
						if (! Input2VertexNumbers( -1, -1, "Choose The Two Vertices For The New LineDef",
												  &(LineDefs[ CurObject].start), &(LineDefs[ CurObject].end))) {
							DeleteObject( EditMode, CurObject);
							CurObject = -1;
						}
					}
					else if (EditMode == OBJ_VERTEXES) {
						SelectObject( &Selected, CurObject);
						if (AutoMergeVertices( &Selected))
							RedrawMap = REDRAW_ALL;
						ForgetSelection( &Selected);
					}
				}
				DragObject = FALSE;
				StretchSelBox = FALSE;
				RedrawMap = REDRAW_ALL;
			}
			
			/* user likes music */
			else if (key) {
				Beep();
			}
			/*
				SetColor( BLUE);
				DrawScreenText( ScrMaxX -370 , 5, "%d", key);  */
			
			/* redraw the (keyboard) pointer */
			if (FakeCursor || ShowRulers) {
				HideMousePointer();
				DrawPointer( ShowRulers);
				ShowMousePointer();
			}
		}
		
		/* check if Scroll Lock is off */
		if ((bioskey( 2) & 0x10) == 0x00) {
			/* move the map if the pointer is near the edge of the screen */
			if (PointerY <= (UseMouse ? 0 : 20)) {
				if (! UseMouse)
					PointerY += MoveSpeed;
				if (MAPY( ScrCenterY) < MapMaxY) {								   
					OrigY += (BCINT) (MoveSpeed * 2.0 / Scale);
					RedrawMap = REDRAW_QUICK;
				}
			}
			if (PointerY >= ScrMaxY - (UseMouse ? 0 : 20)) {
				if (! UseMouse)
					PointerY -= MoveSpeed;
				if (MAPY( ScrCenterY) > MapMinY) {
					OrigY -= (BCINT) (MoveSpeed * 2.0 / Scale);
					RedrawMap = REDRAW_QUICK;
				}
			}
			if (PointerX <= (UseMouse ? 0 : 20)) {
				if (! UseMouse)   
					PointerX += MoveSpeed;
				if (MAPX( ScrCenterX) > MapMinX) {
					OrigX -= (BCINT) (MoveSpeed * 2.0 / Scale);
					RedrawMap = REDRAW_QUICK;
				}
			}
			if (PointerX >= ScrMaxX - (UseMouse ? 0 : 20)) {
				if (! UseMouse)
					PointerX -= MoveSpeed;
				if (MAPX( ScrCenterX) < MapMaxX) {
					OrigX += (BCINT) (MoveSpeed * 2.0 / Scale);
					RedrawMap = REDRAW_QUICK;
				}
			}
		}
    }
}


void DrawMapQuick()
{
    BCINT n;
    
	/*AJB*/
	if (InfoShown) {
		ClearMapScreen(13);
		setviewport(0, 0, ScrMaxX, ScrMaxY -13, TRUE);
	}
	else {
		ClearMapScreen(0);
		setviewport(0, 0, ScrMaxX, ScrMaxY, TRUE);
	}
    ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
    for (n = 0; n < NumLineDefs; n++) {
		SetColor(LIGHTGRAY);
		DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
					Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
    }
    
    /* draw in the things */
	SetColor(DARKGRAY);
	for (n = 0; n < NumThings; n++) {
		DrawMapLine( Things[ n].xpos - ( OBJSIZE * 2 ), Things[ n].ypos, Things[ n].xpos + ( OBJSIZE * 2 ), Things[ n].ypos);
		DrawMapLine( Things[ n].xpos, Things[ n].ypos - ( OBJSIZE * 2 ), Things[ n].xpos, Things[ n].ypos + ( OBJSIZE * 2 ));
	}
	setviewport(0, 0, ScrMaxX, ScrMaxY, TRUE);
}


/*
   draw the actual game map							
   */

void DrawMap( BCINT editmode, BCINT grid, Bool drawgrid) /* SWAP! */
{
    BCINT  n, m, max;
    
    /* clear the screen */
	/*AJB*/
	if (InfoShown)
		ClearMapScreen(13);
	else
		ClearMapScreen(0);
    ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);

	if (InfoShown)
		max = 13;
	else
		max = 0;

    
    /* draw the grid */
    if (drawgrid == TRUE && grid > 0) {					   
		BCINT mapx1 = (BCINT)(((int)(MAPX( ScrMaxX) + grid)) & ((int)~(grid - 1)));
		BCINT mapx0 = (BCINT)(((int)MAPX( 0)) & ((int)~(grid - 1)));
		BCINT mapy1 = (BCINT)(((int)MAPY( 17)) & ((int)~(grid - 1)));
		BCINT mapy0 = (BCINT)(((int)(MAPY( ScrMaxY - max) - grid)) & ((int)~(grid - 1)));
		
		SetColor(DARKBLUE);
		for (n = mapx0; n <= mapx1; n += grid)
			DrawMapLine( n, mapy0, n, mapy1);
		for (n = mapy0; n <= mapy1; n += grid)
			DrawMapLine( mapx0, n, mapx1, n);
    }
    
    /* draw the linedefs to form the map */
    switch (editmode) {
    case OBJ_THINGS:
		ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
		/*AJB*/
		for (n = 0; n < NumLineDefs; n++) {
			if (LineDefs[ n].flags & 1)
				SetColor(LIGHTGRAY);
			else if (LineDefs[ n].flags & 2)
				SetColor(LINEDEFNOPASS);
			else if (LineDefs[ n].flags & 32)
				SetColor(LINEDEFSECRET);
			else if (LineDefs[ n].flags & 64)
				SetColor(LINEDEFNOSOUND);
			else if (LineDefs[ n].flags & 128)
				SetColor(LINEDEFNOMAP);
			else
				SetColor(DARKGRAY);
			DrawMapLine(Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
						Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
		}
		break;
    case OBJ_VERTEXES:
		ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
		SetColor(DARKGRAY);
		for (n = 0; n < NumLineDefs; n++)
			DrawMapVector( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
						  Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
		break;
    case OBJ_LINEDEFS:
		ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
		for (n = 0; n < NumLineDefs; n++) {
			if (LineDefs[ n].type > 0) {
				if (LineDefs[ n].tag > 0)
					SetColor( LINEDEFTAGGED);
				else
					SetColor( LIGHTGREEN);
			}
			/*AJB*/
			else if (LineDefs[ n].tag > 0)
				SetColor( LINEDEFTAGGED);
			else if (LineDefs[ n].flags & 1)
				SetColor(LIGHTGRAY);
			else if (LineDefs[ n].flags & 2)
				SetColor(LINEDEFNOPASS);
			else if (LineDefs[ n].flags & 32)
				SetColor(LINEDEFSECRET);
			else if (LineDefs[ n].flags & 64)
				SetColor(LINEDEFNOSOUND);
			else if (LineDefs[ n].flags & 128)
				SetColor(LINEDEFNOMAP);
			else
				SetColor(DARKGRAY);
			DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
						Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
		}
		break;
    case OBJ_SECTORS:
		ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
		for (n = 0; n < NumLineDefs; n++) {
			if ((m = LineDefs[ n].sidedef1) < 0 || (m = SideDefs[ m].sector) < 0)
				SetColor( CYAN);
			else {
				/*AJB*/
				if (Sectors[ m].tag > 0)
					SetColor( SECTORTAGGED);
				else if (Sectors[ m].special == 7 || Sectors[ m].special == 5 || Sectors[ m].special == 16 || Sectors[ m].special == 4 || Sectors[ m].special == 11 || Sectors[ m].special == 6)
				 	SetColor( SECTORPAINFUL);
				else if (Sectors[ m].special == 9)
					SetColor( SECTORSECRET);
				else if (Sectors[ m].special == 2 || Sectors[ m].special == 3 || Sectors[ m].special == 13 || Sectors[ m].special == 12 || Sectors[ m].special == 1 || Sectors[ m].special == 8 || Sectors[ m].special == 17) 
					SetColor( SECTORLIGHT);
				else if (LineDefs[ n].flags & 1)
					SetColor(LIGHTGRAY);
				else if (LineDefs[ n].flags & 2)
					SetColor(LINEDEFNOPASS);
				else if (LineDefs[ n].flags & 32)
					SetColor(LINEDEFSECRET);
				else if (LineDefs[ n].flags & 64)
					SetColor(LINEDEFNOSOUND);
				else if (LineDefs[ n].flags & 128)
					SetColor(LINEDEFNOMAP);
				else
					SetColor(DARKGRAY);

				if ((m = LineDefs[ n].sidedef2) >= 0) {
					if ((m = SideDefs[ m].sector) < 0)
						SetColor( LIGHTBLUE);
					else if (Sectors[ m].tag > 0)
						SetColor( SECTORTAGGED);
					else if (Sectors[ m].special == 7 || Sectors[ m].special == 5 || Sectors[ m].special == 16 || Sectors[ m].special == 4 || Sectors[ m].special == 11 || Sectors[ m].special == 6)
						SetColor( SECTORPAINFUL);
					else if (Sectors[ m].special == 9)
						SetColor( SECTORSECRET);
					else if (Sectors[ m].special == 2 || Sectors[ m].special == 3 || Sectors[ m].special == 13 || Sectors[ m].special == 12 || Sectors[ m].special == 1 || Sectors[ m].special == 8 || Sectors[ m].special == 17) 
						SetColor( SECTORLIGHT);
				}
			}
			ObjectsNeeded( OBJ_LINEDEFS, OBJ_VERTEXES, 0);
			DrawMapLine( Vertexes[ LineDefs[ n].start].x, Vertexes[ LineDefs[ n].start].y,
						Vertexes[ LineDefs[ n].end].x, Vertexes[ LineDefs[ n].end].y);
		}
		break;
    }
    
    /* draw in the vertices */
    if (editmode == OBJ_VERTEXES) {
		SetColor( LIGHTGREEN);
		for (n = 0; n < NumVertexes; n++) {
			DrawMapLine( Vertexes[ n].x - ( OBJSIZE / 4 ), Vertexes[ n].y - ( OBJSIZE / 4 ), Vertexes[ n].x + ( OBJSIZE / 4 ), Vertexes[ n].y - ( OBJSIZE / 4 ));
			DrawMapLine( Vertexes[ n].x + ( OBJSIZE / 4 ), Vertexes[ n].y - ( OBJSIZE / 4 ), Vertexes[ n].x + ( OBJSIZE / 4 ), Vertexes[ n].y + ( OBJSIZE / 4 ));
			DrawMapLine( Vertexes[ n].x - ( OBJSIZE / 4 ), Vertexes[ n].y - ( OBJSIZE / 4 ), Vertexes[ n].x - ( OBJSIZE / 4 ), Vertexes[ n].y + ( OBJSIZE / 4 ));
			DrawMapLine( Vertexes[ n].x - ( OBJSIZE / 4 ), Vertexes[ n].y + ( OBJSIZE / 4 ), Vertexes[ n].x + ( OBJSIZE / 4 ), Vertexes[ n].y + ( OBJSIZE / 4 ));
		}
    }
    
    /* draw in the things */
    ObjectsNeeded( OBJ_THINGS, 0);
    if (editmode == OBJ_THINGS) {
		for (n = 0; n < NumThings; n++) {
			if(!(Things[n].when & tff))
				continue;
			/* skip if the filter says so */

			if(!((Things[n].when & TF_DM) ?
				 (tff & TF_DM) : (tff & TF_NOT_DM)))
				continue;
			/* clear as mud:
			   if this is a deathmatch thing and we are not displaying
			   deathmatch things, skip it.
			   same for non-deathmatch things. */
			
			m = GetThingRadius(Things[n].type);
			SetColor( GetThingColour(Things[n].type));
			if (ThingAngle == FALSE) {
				DrawMapLine( Things[n].xpos - m , Things[n].ypos, Things[n].xpos + m, Things[n].ypos);
				DrawMapLine( Things[n].xpos, Things[n].ypos - m, Things[n].xpos, Things[n].ypos + m);
			}
			if (ThingAngle == TRUE) {
				switch(Things[n].angle) {
				case 0:
					DrawMapVector(Things[n].xpos - m, Things[n].ypos,
								  Things[n].xpos + m, Things[n].ypos);
					break;
				case 180:
					DrawMapVector(Things[n].xpos + m, Things[n].ypos,
								  Things[n].xpos - m, Things[n].ypos);
					break;
				case 90:
					DrawMapVector(Things[n].xpos, Things[n].ypos - m,
								  Things[n].xpos, Things[n].ypos + m);
					break;
				case 270:
					DrawMapVector(Things[n].xpos, Things[n].ypos + m,
								  Things[n].xpos, Things[n].ypos - m);
					break;
				case 45:
					DrawMapVector(Things[n].xpos - m / 1.4, Things[n].ypos - m / 1.4,
								  Things[n].xpos + m / 1.4, Things[n].ypos + m / 1.4);
					break;
				case 315:
					DrawMapVector(Things[n].xpos - m / 1.4, Things[n].ypos + m / 1.4,
								  Things[n].xpos + m / 1.4, Things[n].ypos - m / 1.4);
					break;
				case 225:
					DrawMapVector(Things[n].xpos + m / 1.4, Things[n].ypos + m / 1.4,
								  Things[n].xpos - m / 1.4, Things[n].ypos - m / 1.4);
					break;
				case 135:
					DrawMapVector(Things[n].xpos + m / 1.4, Things[n].ypos - m / 1.4,
								  Things[n].xpos - m / 1.4, Things[n].ypos + m / 1.4);
				}
			}
			DrawMapCircle( Things[ n].xpos, Things[ n].ypos, m);
		}
    }
    else {
		SetColor(DARKGRAY);
		for (n = 0; n < NumThings; n++) {
			DrawMapLine( Things[ n].xpos - ( OBJSIZE * 2 ), Things[ n].ypos, Things[ n].xpos + ( OBJSIZE * 2 ), Things[ n].ypos);
			DrawMapLine( Things[ n].xpos, Things[ n].ypos - ( OBJSIZE * 2 ), Things[ n].xpos, Things[ n].ypos + ( OBJSIZE * 2 ));
		}
    }
    
    /* draw in the title bar */
    DrawScreenBox3D( 0, 0, ScrMaxX, 16);
    SetColor( WHITE);
    DrawScreenText( 7,  4, "File  Edit  Search  Mode  Misc(F8) Objects(F9) Check(F10)");
    DrawScreenText( 7,  6, "_     _     _       _      _       _           _");
    DrawScreenText( ScrMaxX - 66, 4, "Help(F1)");
    DrawScreenText( ScrMaxX - 66, 6, "_"); 
    
    /* draw the bottom line, if needed */
    if (InfoShown) {
		DrawScreenBox3D( 0, ScrMaxY - 12, ScrMaxX, ScrMaxY);
		if (MadeMapChanges == TRUE)
			DrawScreenText( 5, ScrMaxY - 9, "Editing %s On %s #", GetEditModeName( editmode), Level->dir.name);
		else if (MadeChanges == TRUE)
			DrawScreenText( 5, ScrMaxY - 9, "Editing %s On %s *", GetEditModeName( editmode), Level->dir.name);
		else
			DrawScreenText( 5, ScrMaxY - 9, "Editing %s On %s", GetEditModeName( editmode), Level->dir.name);
		if (Scale < 1.0)
			DrawScreenText( ScrMaxX - 176, ScrMaxY - 9, "Scale: 1/%d  Grid: %d", (BCINT) (1.0 / Scale + 0.5), grid);
		else
			DrawScreenText( ScrMaxX - 176, ScrMaxY - 9, "Scale: %d/1  Grid: %d", (BCINT) Scale, grid);
		if (farcoreleft() < 50000L) {
			if (farcoreleft() < 20000L)
				SetColor( CYAN);
			else
				SetColor( RED);
		}
		DrawScreenText( ScrCenterX - ((editmode == OBJ_LINEDEFS) ? 10 : 50), ScrMaxY - 9, "Free Mem: %lu", farcoreleft());
    }
}



/*
   center the map around the given coords
   */

void CenterMapAroundCoords( BCINT xpos, BCINT ypos)
{
    OrigX = xpos;
    OrigY = ypos;
    PointerX = ScrCenterX;
    PointerY = ScrCenterY;
}



/*
   center the map around the object and zoom in if necessary
   */

void GoToObject( BCINT objtype, BCINT objnum) /* SWAP! */
{
    BCINT xpos, ypos;
    BCINT xpos2, ypos2;
    BCINT n;
    BCINT sd1, sd2;
    float oldscale;
    
    GetObjectCoords( objtype, objnum, &xpos, &ypos);
    CenterMapAroundCoords( xpos, ypos);
    oldscale = Scale;
    
    /* zoom in until the object can be selected */
    while (Scale < 4.0 && GetCurObject( objtype, MAPX( PointerX - 4), MAPY( PointerY - 4), MAPX( PointerX + 4), MAPY( PointerY + 4)) != objnum) {
		if (Scale < 1.0)
			Scale = 1.0 / ((1.0 / Scale) - 1.0);
		else
			Scale = Scale * 2.0;
    }
    /* Special case for Sectors: if several Sectors are one inside another, then	 */
    /* zooming in on the center won't help.  So I choose a LineDef that borders the  */
    /* the Sector and move a few pixels towards the inside of the Sector.			*/
    if (objtype == OBJ_SECTORS && GetCurObject( OBJ_SECTORS, OrigX, OrigY, OrigX, OrigY) != objnum) {
		/* restore the Scale */
		Scale = oldscale;
		for (n = 0; n < NumLineDefs; n++) {
			ObjectsNeeded( OBJ_LINEDEFS, 0);
			sd1 = LineDefs[ n].sidedef1;
			sd2 = LineDefs[ n].sidedef2;
			ObjectsNeeded( OBJ_SIDEDEFS, 0);
			if (sd1 >= 0 && SideDefs[ sd1].sector == objnum)
				break;
			if (sd2 >= 0 && SideDefs[ sd2].sector == objnum)
				break;
		}
		if (n < NumLineDefs) {
			GetObjectCoords( OBJ_LINEDEFS, n, &xpos2, &ypos2);
			n = ComputeDist( abs( xpos - xpos2), abs( ypos - ypos2)) / 7;
			if (n <= 1)
				n = 2;
			xpos = xpos2 + (xpos - xpos2) / n;
			ypos = ypos2 + (ypos - ypos2) / n;
			CenterMapAroundCoords( xpos, ypos);
			/* zoom in until the sector can be selected */
			while (Scale > 4.0 && GetCurObject( OBJ_SECTORS, OrigX, OrigY, OrigX, OrigY) != objnum) {
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



void FindThing(int mode, SelPtr *List)
{
	static int thing_no = 0, last = -1;
	static BCINT type = -1;
	int action = 0;
	Bool mark = FALSE;
	static thing_class *c = NULL;
	thing_type *t = NULL;
	
	if(List) {
		action = DisplayMenu(-1, -1, "Find/Mark",
							 "Find Things",
							 "Find & Mark Things",
							 "Mark By Thing Class",NULL);

		if(action == 2)
			mark = TRUE;
		
		if(action == 3) {
			c = SelectThingClass();
			if(c)
				for(t = c->types; t; t = t->next)
					for(thing_no = 0; thing_no < NumThings; thing_no++)
						if(Things[thing_no].type == t->type)
							SelectObject(List, thing_no);
			return;
		}

 		if(action == 0)
			return;

	}
	
	if(mode == 0 || type == -1) {
		type = SelectThingType();
		thing_no = 0;
	}
	
	if(type == -1)
		return;
	
	if(mark) {
		for(thing_no = 0; thing_no < NumThings; thing_no++)
			if(Things[thing_no].type == type)
				SelectObject(List, thing_no);
		
		return;
	}

	thing_no = last + 1;
	if(last == -1)
		last = 0;
		
	do {
		if(thing_no == NumThings)
			thing_no = 0;
		else {
			if(Things[thing_no].type == type) {
				last = thing_no;
				GoToObject(OBJ_THINGS, thing_no);
				return;
			}
			thing_no++;
		}
	} while(thing_no != last);
}


void CopyFile(const char *from, const char *to)
{
	FILE *f = NULL, *t = NULL;
	char *buffer;
	long bytes, left;
	
	if(!((buffer = malloc(65536)) &&
	     (f = fopen(from, "rb")) &&
	     (t = fopen(to, "wb"))))
		ProgError("Unable To Copy File");
	
	fseek(f, 0, SEEK_END);
	left = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	while(left > 0) {
		bytes = fread(buffer, 1, 65536, f);
		fwrite(buffer, 1, bytes, t);
		left -= bytes;
	}
	
	fclose(f);
	fclose(t);
	
	free(buffer);
} 

/* end of file */
