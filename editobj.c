/*   Doom Editor for Total Headcases, by Simon Oke and Antony Burden.
	 
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

void DisplayObjectInfo( BCINT objtype, BCINT objnum) /* SWAP! */
{
    char texname[ 9];
    BCINT  tag, n;
    BCINT  sd1 = -1, sd2 = -1, s1 = -1, s2 = -1;
    BCINT  x0, y0;
    
    ObjectsNeeded( objtype, 0);
    switch (objtype) {
    case OBJ_THINGS:
		x0 = 0;
		/* ajb */
		y0 = ScrMaxY - 72;
		if (InfoShown)
			y0 -= 13;
		/* ajb */
		DrawScreenBox3D( x0, y0, x0 + 360, y0 + 72);
		if (objnum < 0) {
			/* ajb 2 */
			DrawScreenText( x0 + 112, y0 + 30, "Use The Cursor To");
			DrawScreenText( x0 + 124, y0 + 40, "Select A Thing  ");
			break;
		}
		SetColor( YELLOW);
		DrawScreenText( x0 + 5, y0 + 5, "Selected Thing (#%d)", objnum);
		/* ajb 2 */
		SetColor( RED);
		DrawScreenText( -1, y0 + 20, "Thing Number:  (%4d)", Things[ objnum].type);
		SetColor( BLACK);
		DrawScreenText( -1, -1, "Coordinates:   (%d, %d)", Things[ objnum].xpos, Things[ objnum].ypos);
        DrawScreenText( -1, -1, "Type:          %s", GetThingName( Things[ objnum].type));
        DrawScreenText( -1, -1, "Angle:         %s", GetAngleName( Things[ objnum].angle));
		DrawScreenText( -1, -1, "Flags:         %s", GetWhenName( Things[ objnum].when));
		break;
		
    case OBJ_LINEDEFS:
		x0 = 0;
		y0 = ScrMaxY - 80;
		if (InfoShown)
			y0 -= 13;
		DrawScreenBox3D(   x0, y0, x0 + 238, y0 + 80);
		if (objnum >= 0) {
			SetColor( YELLOW);
			DrawScreenText( x0 + 5, y0 + 5, "Selected LineDef (#%d)", objnum);
			SetColor( BLACK);
            DrawScreenText( -1, y0 + 20, "Flags:%3d    %s", LineDefs[ objnum].flags, GetLineDefFlagsName( LineDefs[ objnum].flags));
			DrawScreenText( -1, -1, "Type: %3d %s", LineDefs[ objnum].type, GetLineDefTypeName( LineDefs[ objnum].type));
			sd1 = LineDefs[ objnum].sidedef1;
			sd2 = LineDefs[ objnum].sidedef2;
			tag = LineDefs[ objnum].tag;
			ObjectsNeeded( OBJ_SIDEDEFS, OBJ_SECTORS, 0);
			s1 = LineDefs[ objnum].start;
			s2 = LineDefs[ objnum].end;
			ObjectsNeeded( OBJ_VERTEXES, 0);
			n = ComputeDist( Vertexes[ s2].x - Vertexes[ s1].x, Vertexes[ s2].y - Vertexes[ s1].y);
			DrawScreenText( x0 + 160, y0 + 60, "Length:");
			DrawScreenText( x0 + 170, y0 + 70, "%d", n);
			if (tag > 0) {
				for (n = 0; n < NumSectors; n++)
					if (Sectors[ n].tag == tag)
						break;
			}
			else
				n = NumSectors;
			if (n < NumSectors)
				DrawScreenText( x0 + 5, y0 + 40, "Sector Tag:  %d (#%d)", tag, n);
			else
				DrawScreenText( x0 + 5, y0 + 40, "Sector Tag:  %d (none)", tag);
            DrawScreenText( -1, -1, "Vertexes:    (#%d, #%d)", s1, s2);
			DrawScreenText( -1, -1, "1st SideDef: #%d", sd1);
			DrawScreenText( -1, -1, "2nd SideDef: #%d", sd2);
			if (sd1 >= 0)
				s1 = SideDefs[ sd1].sector;
			else
				s1 = -1;
			if (sd2 >= 0)
				s2 = SideDefs[ sd2].sector;
			else
				s2 = -1;
		}
		else {
			SetColor(LIGHTGRAY);
			DrawScreenText( x0 + 25, y0 + 35, "(No LineDef Selected)");
		}
		x0 = 239;
		y0 = ScrMaxY - 80;
		if (InfoShown)
			y0 -= 13;
		DrawScreenBox3D( x0, y0, x0 + 200, y0 + 80);
		if (objnum >= 0 && sd1 >= 0) {
			SetColor( YELLOW);
			DrawScreenText( x0 + 5, y0 + 5, "First SideDef (#%d)", sd1);
			SetColor( BLACK);
			texname[ 8] = '\0';
			strncpy( texname, SideDefs[ sd1].tex3, 8);
			DrawScreenText( -1, y0 + 20, "Normal Texture: %s", texname);
			strncpy( texname, SideDefs[ sd1].tex1, 8);
			if (s1 >= 0 && s2 >= 0 && Sectors[ s1].ceilh > Sectors[ s2].ceilh) {
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					SetColor( RED);
			}
			else
				SetColor(GRAY);
			DrawScreenText( -1, -1, "Upper Texture:  %s", texname);
			SetColor( BLACK);
			strncpy( texname, SideDefs[ sd1].tex2, 8);
			if (s1 >= 0 && s2 >= 0 && Sectors[ s1].floorh < Sectors[ s2].floorh) {
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					SetColor( RED);
			}
			else
				SetColor(GRAY);
			DrawScreenText( -1, -1, "Lower Texture:  %s", texname);
			SetColor( BLACK);
			DrawScreenText( -1, -1, "Tex. X Offset:  %d", SideDefs[ sd1].xoff);
			DrawScreenText( -1, -1, "Tex. Y Offset:  %d", SideDefs[ sd1].yoff);
            DrawScreenText( -1, -1, "Sector:         #%d", s1);
		}
		else {
			SetColor(LIGHTGRAY);
			DrawScreenText( x0 + 25, y0 + 35, "(No First SideDef)");
		}
		x0 = 440;
		y0 = ScrMaxY - 80;
		if (InfoShown)
			y0 -= 13;
		DrawScreenBox3D( x0, y0, x0 + 200, y0 + 80);
		if (objnum >= 0 && sd2 >= 0) {
			SetColor( YELLOW);
			DrawScreenText( x0 + 5, y0 + 5, "Second SideDef (#%d)", sd2);
			SetColor( BLACK);
			texname[ 8] = '\0';
			strncpy( texname, SideDefs[ sd2].tex3, 8);
			
			DrawScreenText( -1, y0 + 20, "Normal Texture: %s", texname);
			strncpy( texname, SideDefs[ sd2].tex1, 8);
			if (s1 >= 0 && s2 >= 0 && Sectors[ s2].ceilh > Sectors[ s1].ceilh) {
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					SetColor( RED);
			}
			else
				SetColor(GRAY);
			DrawScreenText( -1, -1, "Upper Texture:  %s", texname);
			SetColor( BLACK);
			strncpy( texname, SideDefs[ sd2].tex2, 8);
			if (s1 >= 0 && s2 >= 0 && Sectors[ s2].floorh < Sectors[ s1].floorh) {
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					SetColor( RED);
			}
			else
				SetColor(GRAY);
			DrawScreenText( -1, -1, "Lower Texture:  %s", texname);
			SetColor( BLACK);
			DrawScreenText( -1, -1, "Tex. X Offset:  %d", SideDefs[ sd2].xoff);
			DrawScreenText( -1, -1, "Tex. Y Offset:  %d", SideDefs[ sd2].yoff);
            DrawScreenText( -1, -1, "Sector:         #%d", s2);
		}
		else {
			SetColor(LIGHTGRAY);
			DrawScreenText( x0 + 25, y0 + 35, "(No Second SideDef)");
		}
		break;
    case OBJ_VERTEXES:
		x0 = 0;
		y0 = ScrMaxY - 30;
		if (InfoShown)
			y0 -= 13;
		DrawScreenBox3D( x0, y0, x0 + 220, y0 + 30);
		if (objnum < 0) {
			SetColor(LIGHTGRAY);
			DrawScreenText( x0 + 30, y0 + 12, "(No Vertex Selected)");
			break;
		}
		SetColor( YELLOW);
		DrawScreenText( x0 + 5, y0 + 5, "Selected Vertex (#%d)", objnum);
		SetColor( BLACK);
		DrawScreenText( -1, y0 + 20, "Coordinates: (%d, %d)", Vertexes[ objnum].x, Vertexes[ objnum].y);
		break;
    case OBJ_SECTORS:
		x0 = 0;
		y0 = ScrMaxY - 100;
		if (InfoShown)
			y0 -= 13;
		DrawScreenBox3D( x0, y0, x0 + 300, y0 + 100);
		if (objnum < 0) {
			SetColor(LIGHTGRAY);
			DrawScreenText( x0 + 73, y0 + 48, "(No Sector Selected)");
			break;
		}
		SetColor( YELLOW);
		DrawScreenText( x0 + 5, y0 + 5, "Selected Sector (#%d)", objnum);
		SetColor( BLACK);
		DrawScreenText( -1, y0 + 20, "Floor Height:    %d", Sectors[ objnum].floorh);
		DrawScreenText( -1, -1, "Ceiling Height:  %d", Sectors[ objnum].ceilh);
		DrawScreenText( -1, -1, "Sector Height:   %d", Sectors[ objnum].ceilh - Sectors[objnum].floorh);
		
		texname[ 8] = '\0';
		strncpy( texname, Sectors[ objnum].floort, 8);
		DrawScreenText( -1, -1, "Floor Texture:   %s", texname);
		strncpy( texname, Sectors[ objnum].ceilt, 8);
		DrawScreenText( -1, -1, "Ceiling Texture: %s", texname);
		DrawScreenText( -1, -1, "Light Level:     %d", Sectors[ objnum].light);
		DrawScreenText( -1, -1, "Type: %3d        %s", Sectors[ objnum].special, GetSectorTypeName( Sectors[ objnum].special));
		tag = Sectors[ objnum].tag;
		ObjectsNeeded( OBJ_LINEDEFS, 0);
		if (tag == 0)
			n = NumLineDefs;
		else
			for (n = 0; n < NumLineDefs; n++)
				if (LineDefs[ n].tag == tag)
					break;
		if (n < NumLineDefs)
            DrawScreenText( -1, -1, "LineDef Tag:     %d (#%d)", tag, n);
		else if (tag == 99 || tag == 999)
            DrawScreenText( -1, -1, "LineDef Tag:     %d (Stairs?)", tag);
		else if (tag == 666)
            DrawScreenText( -1, -1, "LineDef Tag:     %d (Lower at End)", tag);
		else
            DrawScreenText( -1, -1, "LineDef Tag:     %d (none)", tag);
		break;
    }
}


/*
   ask for an object number and check for maximum valid number
   (this is just like InputIntegerValue, but with a different prompt)
   */

BCINT InputObjectNumber( BCINT x0, BCINT y0, BCINT objtype, BCINT curobj)
{
    BCINT val, key;
    char prompt[ 80];
    
    if (UseMouse)
		HideMousePointer();
    sprintf( prompt, "Enter A %s Value Between 0 And %d:", GetObjectTypeName( objtype), GetMaxObjectNum( objtype));
    if (x0 < 0)
		x0 = (ScrMaxX - 25 - 8 * strlen( prompt)) / 2;
    if (y0 < 0)
		y0 = (ScrMaxY - 55) / 2;
    DrawScreenBox3D( x0, y0, x0 + 25 + 8 * strlen( prompt), y0 + 55);
    SetColor( WHITE);
    DrawScreenText( x0 + 10, y0 + 8, prompt);
    val = curobj;
    while (((key = InputInteger( -1, -1, &val, 0, GetMaxObjectNum( objtype))) & 0x00FF) != 0x000D && (key & 0x00FF) != 0x001B)
		Beep();
    if (UseMouse)
		ShowMousePointer();
    return val;
}



/*
   ask for an object number and display a warning message
   */

BCINT InputObjectXRef( BCINT x0, BCINT y0, BCINT objtype, Bool allownone, BCINT curobj)
{
    BCINT val, key;
    char prompt[ 80];
    
    if (UseMouse)
		HideMousePointer();
    sprintf( prompt, "Enter A %s Value Between 0 And %d%c", GetObjectTypeName( objtype), GetMaxObjectNum( objtype), allownone ? ',' : ':');
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
    DrawScreenText( x0 + 10, y0 + (allownone ? 60 : 50), "Warning: Modifying The Cross-References");
    DrawScreenText( x0 + 10, y0 + (allownone ? 70 : 60), "Between Some Objects May Crash The Game.");
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

Bool Input2VertexNumbers( BCINT x0, BCINT y0, char *prompt1, BCINT *v1, BCINT *v2)
{
    BCINT  key;
    BCINT  maxlen, first;
    Bool ok;
    char prompt2[ 80];
    
    if (UseMouse)
		HideMousePointer();
    sprintf( prompt2, "Enter Two Numbers Between 0 And %d:", NumVertexes - 1);
    if (strlen( prompt1) > strlen( prompt2))
		maxlen = strlen( prompt1);
    else
		maxlen = strlen( prompt2);
    if (x0 < 0)
		x0 = (ScrMaxX - 25 - 8 * maxlen) / 2;
    if (y0 < 0)
		y0 = (ScrMaxY - 75) / 2;
    DrawScreenBox3D( x0, y0, x0 + 25 + 8 * maxlen, y0 + 75);
    DrawScreenText( x0 + 10, y0 + 36, "From This Vertex");
    DrawScreenText( x0 + 180, y0 + 36, "To This Vertex");
    SetColor( WHITE);
    DrawScreenText( x0 + 10, y0 + 8, prompt1);
    DrawScreenText( x0 + 10, y0 + 18, prompt2);
    first = TRUE;
    key = 0;
    for (;;) {
		ok = TRUE;
		DrawScreenBox3D( x0 + 10, y0 + 48, x0 + 71, y0 + 61);
		if (*v1 < 0 || *v1 >= NumVertexes)
			{
				SetColor( DARKGRAY);
				ok = FALSE;
			}
		DrawScreenText( x0 + 14, y0 + 51, "%d", *v1);
		DrawScreenBox3D( x0 + 180, y0 + 48, x0 + 241, y0 + 61);
		if (*v2 < 0 || *v2 >= NumVertexes) {
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
		else if ((key & 0x00FF) == 0x000D) {
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
 */

char *GetTaggedLineDefFlag( BCINT linedefnum, BCINT flagndx)
{
    static char ldstr[ 9][ 50];
    
    if ((LineDefs[ linedefnum].flags & (0x01 << (flagndx - 1))) != 0)
		strcpy( ldstr[ flagndx - 1], "\04 ");
    else
		strcpy( ldstr[ flagndx - 1], "  ");
    strcat( ldstr[ flagndx - 1], GetLineDefFlagsLongName( 0x01 << (flagndx - 1)));
    return ldstr[ flagndx - 1];
}



/*
   edit an object or a group of objects
   */

void EditObjectsInfo( BCINT x0, BCINT y0, BCINT objtype, SelPtr obj) /* SWAP! */
{
    char  *menustr[ 30];
    BCINT	dummy[ 30];
    char   texname[ 9];
    BCINT	n, val;
    SelPtr cur, sdlist;
    
    ObjectsNeeded( objtype, 0);
    if (obj == NULL)
		return;
    switch (objtype) {
    case OBJ_THINGS:
		for (n = 0; n < 6; n++)
			menustr[ n] = (char*)GetMemory( 60);
		sprintf( menustr[ 5], "Edit Thing #%d", obj->objnum);
        sprintf( menustr[ 0], "Change Type          (Current: %s)", GetThingName( Things[ obj->objnum].type));
        sprintf( menustr[ 1], "Change Angle         (Current: %s)", GetAngleName( Things[ obj->objnum].angle));
		sprintf( menustr[ 2], "Change Flags         (Current: %s)", GetWhenName( Things[ obj->objnum].when));
        sprintf( menustr[ 3], "Change X Position    (Current: %d)", Things[ obj->objnum].xpos);
        sprintf( menustr[ 4], "Change Y Position    (Current: %d)", Things[ obj->objnum].ypos);
		val = DisplayMenuArray( -1, -1, menustr[ 5], 5, NULL, menustr, dummy);
		for (n = 0; n < 6; n++)
			FreeMemory( menustr[ n]);
		switch (val) {
		case 1:
			/* change current & selected things to user-selected type */

			val = SelectThingType();
			
			if(val != -1) {
				for (cur = obj; cur; cur = cur->next)
					Things[ cur->objnum].type = val;
				MadeChanges = TRUE;
			}
		break;

		case 2:
			switch (DisplayMenu(-1, -1, "Select Angle",
								"North",
								"North East",
								"East",
								"South East",
								"South",
								"South West",
								"West",
								"North West",
								NULL)) {
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
			val = DisplayMenu( -1, -1, "Choose The Difficulty Level(s)",
                              "12     Easy",
                              "  3    Medium",
                              "123    Easy And Medium",
							  "   45  Hard",
							  "12 45  Easy And Hard",
							  "  345  Medium And Hard",
							  "12345  Easy, Medium And Hard",
							  "Toggle Deaf Monster",
							  "Toggle Multi-Player",
							  "(Enter A Decimal Value)",
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
						Things[ cur->objnum].when = (Things[ cur->objnum].when & 0x18) | val;
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
					val = InputIntegerValue( -1, -1, 1, 31, Things[ obj->objnum].when);
					if (val > 0) {
						for (cur = obj; cur; cur = cur->next)
							Things[ cur->objnum].when = val;
						MadeChanges = TRUE;
					}
					break;
				}
			break;
			
		case 4:
			val = InputIntegerValue( -1, -1, MapMinX, MapMaxX, Things[ obj->objnum].xpos);
			if (val >= MapMinX) {
				n = val - Things[ obj->objnum].xpos;
				for (cur = obj; cur; cur = cur->next)
					Things[ cur->objnum].xpos += n;
				MadeChanges = TRUE;
			}
			break;
			
		case 5:
			val = InputIntegerValue( -1, -1, MapMinY, MapMaxY, Things[ obj->objnum].ypos);
			if (val >= MapMinY) {
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
			menustr[ n] = (char*)GetMemory( 60);
		sprintf( menustr[ 2], "Edit Vertex #%d", obj->objnum);
		sprintf( menustr[ 0], "Change X Position (Current: %d)", Vertexes[ obj->objnum].x);
		sprintf( menustr[ 1], "Change Y Position (Current: %d)", Vertexes[ obj->objnum].y);
		val = DisplayMenuArray( -1, -1, menustr[ 2], 2, NULL, menustr, dummy);
		for (n = 0; n < 3; n++)
			FreeMemory( menustr[ n]);
		switch (val) {
		case 1:
			val = InputIntegerValue( -1, -1, min( MapMinX, -10000), max( MapMaxX, 10000), Vertexes[ obj->objnum].x);
			if (val >= min( MapMinX, -10000)) {
				n = val - Vertexes[ obj->objnum].x;
				for (cur = obj; cur; cur = cur->next)
					Vertexes[ cur->objnum].x += n;
				MadeChanges = TRUE;
				MadeMapChanges = TRUE;
			}
			break;
			
		case 2:
			val = InputIntegerValue( -1, -1, min( MapMinY, -10000), max( MapMaxY, 10000), Vertexes[ obj->objnum].y);
			if (val >= min( MapMinY, -10000)) {
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
		switch (DisplayMenu( -1, -1, "Choose The Object To Edit:",
							"Edit The LineDef",
							(LineDefs[ obj->objnum].sidedef1 >= 0) ? "Edit The 1st SideDef" : "Add A 1st SideDef",
							(LineDefs[ obj->objnum].sidedef2 >= 0) ? "Edit The 2nd SideDef" : "Add A 2nd SideDef",
							NULL)) {
		case 1:
			for (n = 0; n < 8; n++)
				menustr[ n] = (char*)GetMemory( 60);
			sprintf( menustr[ 7], "Edit LineDef #%d", obj->objnum);
            sprintf( menustr[ 0], "Change Flags            (Current: %d)", LineDefs[ obj->objnum].flags);
            sprintf( menustr[ 1], "Change Type             (Current: %d)", LineDefs[ obj->objnum].type);
            sprintf( menustr[ 2], "Change Sector Tag       (Current: %d)", LineDefs[ obj->objnum].tag);
			sprintf( menustr[ 3], "Change Starting Vertex  (Current: #%d)", LineDefs[ obj->objnum].start);
            sprintf( menustr[ 4], "Change Ending Vertex    (Current: #%d)", LineDefs[ obj->objnum].end);
			sprintf( menustr[ 5], "Change 1st SideDef Ref. (Current: #%d)", LineDefs[ obj->objnum].sidedef1);
			sprintf( menustr[ 6], "Change 2nd SideDef Ref. (Current: #%d)", LineDefs[ obj->objnum].sidedef2);
			val = DisplayMenuArray( -1 , -1, menustr[ 7], 7, NULL, menustr, dummy);
			for (n = 0; n < 8; n++)
				FreeMemory( menustr[ n]);
			switch (val) {
			case 1:
				do {
					MadeChanges = FALSE;
					val = DisplayMenu( -1, -1, "Toggle the flags:",
									  GetTaggedLineDefFlag( obj->objnum, 1),
									  GetTaggedLineDefFlag( obj->objnum, 2),
									  GetTaggedLineDefFlag( obj->objnum, 3),
									  GetTaggedLineDefFlag( obj->objnum, 4),
									  GetTaggedLineDefFlag( obj->objnum, 5),
									  GetTaggedLineDefFlag( obj->objnum, 6),
									  GetTaggedLineDefFlag( obj->objnum, 7),
									  GetTaggedLineDefFlag( obj->objnum, 8),
									  GetTaggedLineDefFlag( obj->objnum, 9),
									  "(Enter a Decimal Value)",
									  NULL);
					if (val >= 1 && val <= 9) {
						for (cur = obj; cur; cur = cur->next)
							LineDefs[ cur->objnum].flags ^= 0x01 << (val - 1);
						MadeChanges = TRUE;
					}
					else if (val == 10) {										   
						val = InputIntegerValue( -1, -1, 0, 511, LineDefs[ obj->objnum].flags);
						if (val >= 0) {
							for (cur = obj; cur; cur = cur->next)
								LineDefs[ cur->objnum].flags = val;
							MadeChanges = TRUE;
						}
					}
				} while (MadeChanges == TRUE);
				break;
			case 2:
				{
					ld_class *c;
					ld_type *t;
					char *menuarr[30];
					BCINT dummy[30];
					int i;
					Bool input_integer = FALSE;
					
					c = Linedef_classes;
					for(i = 0; (c && i < 30); i++) {
						menuarr[i] = c->name;
						c = c->next;
					}
					menuarr[i++] = "(Enter A Decimal Value)";
					
					val = DisplayMenuArray( -1, -1, "Choose A LineDef Type:",
										   i, NULL, menuarr, dummy);
					if(val == 0)
						val = -1;
					
					if(i == val) {
						input_integer = TRUE;
						val = InputIntegerValue(-1, -1, 0, 32767, 0);
					}
					
					if(val > 0) {
						if(!input_integer) {
							c = Linedef_classes;
							while(--val)
								c = c->next;
							/* now c points to the class we are
							   going to display a submenu for */
							
							t = c->types;
							for(i = 0; (t && i < 30); t = t->next) {
								menuarr[i++] = t->longname;
							}
							
							if(i == 1) {
								val = c->types->type;
							}
							else {
								val = DisplayMenuArray(-1, -1, "Choose A LineDef Type:",
													   i, NULL, menuarr, dummy);
								if(val == 0)
									val = -1;
							}
						}
						
						if (val >= 0) {
							if(!input_integer) {
								t = c->types;
								while(--val)
									t = t->next;
								val = t->type;
							}
							
							for (cur = obj; cur; cur = cur->next)
								LineDefs[ cur->objnum].type = val;
							MadeChanges = TRUE;
						}
					}
				}
				break;
			case 3:
				if (LineDefs[ obj->objnum].tag == 0) {
					val = InputIntegerValue( -1, -1, 0, 255, FindFreeTag());
				}
				else {
					val = InputIntegerValue( -1, -1, 0, 255, LineDefs[ obj->objnum].tag);
				}
				if (val >= 0) {
					for (cur = obj; cur; cur = cur->next)
						LineDefs[ cur->objnum].tag = val;
					MadeChanges = TRUE;
				}
				break;
			case 4:
				val = InputObjectXRef( -1, -1, OBJ_VERTEXES, FALSE, LineDefs[ obj->objnum].start);
				if (val >= 0) {
					for (cur = obj; cur; cur = cur->next)
						LineDefs[ cur->objnum].start = val;
					MadeChanges = TRUE;
					MadeMapChanges = TRUE;
				}
				break;
			case 5:
				val = InputObjectXRef( -1, -1, OBJ_VERTEXES, FALSE, LineDefs[ obj->objnum].end);
				if (val >= 0) {
					for (cur = obj; cur; cur = cur->next)
						LineDefs[ cur->objnum].end = val;
					MadeChanges = TRUE;
					MadeMapChanges = TRUE;
				}
				break;
			case 6:
				val = InputObjectXRef( -1, -1, OBJ_SIDEDEFS, TRUE, LineDefs[ obj->objnum].sidedef1);
				if (val >= -1) {
					for (cur = obj; cur; cur = cur->next)
						LineDefs[ cur->objnum].sidedef1 = val;
					MadeChanges = TRUE;
					MadeMapChanges = TRUE;
				}
				break;
			case 7:
				val = InputObjectXRef( -1, -1, OBJ_SIDEDEFS, TRUE, LineDefs[ obj->objnum].sidedef2);
				if (val >= -1) {
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
			ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
			if (LineDefs[ obj->objnum].sidedef1 >= 0) {
				/* build a new selection list with the first SideDefs */
				objtype = OBJ_SIDEDEFS;
				sdlist = NULL;
				for (cur = obj; cur; cur = cur->next)
					if (LineDefs[ cur->objnum].sidedef1 >= 0)
						SelectObject( &sdlist, LineDefs[ cur->objnum].sidedef1);
			}
			else {
				/* add a new first SideDef */
				for (cur = obj; cur; cur = cur->next)
					if (LineDefs[ cur->objnum].sidedef1 == -1) {
						InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
						LineDefs[ cur->objnum].sidedef1 = NumSideDefs - 1;
					}
				break;
			}
			/* no break here */
			
			/* edit or add the second SideDef */
		case 3:
			if (objtype != OBJ_SIDEDEFS) {
				if (LineDefs[ obj->objnum].sidedef2 >= 0) {
					/* build a new selection list with the second (or first) SideDefs */
					objtype = OBJ_SIDEDEFS;
					sdlist = NULL;
					for (cur = obj; cur; cur = cur->next)
						if (LineDefs[ cur->objnum].sidedef2 >= 0)
							SelectObject( &sdlist, LineDefs[ cur->objnum].sidedef2);
						else if (LineDefs[ cur->objnum].sidedef1 >= 0)
							SelectObject( &sdlist, LineDefs[ cur->objnum].sidedef1);
				}
				else {
					/* add a new second (or first) SideDef */
					for (cur = obj; cur; cur = cur->next)
						if (LineDefs[ cur->objnum].sidedef1 == -1) {
							InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
							ObjectsNeeded( OBJ_LINEDEFS, 0);
							LineDefs[ cur->objnum].sidedef1 = NumSideDefs - 1;
						}
						else if (LineDefs[ cur->objnum].sidedef2 == -1) {
							n = LineDefs[ cur->objnum].sidedef1;
							InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
							strncpy( SideDefs[ NumSideDefs - 1].tex3, "-", 8);
							strncpy( SideDefs[ n].tex3, "-", 8);
							ObjectsNeeded( OBJ_LINEDEFS, 0);
							LineDefs[ cur->objnum].sidedef2 = NumSideDefs - 1;
							LineDefs[ cur->objnum].flags = 4;
						}
					break;
				}
			}
			ObjectsNeeded( OBJ_SIDEDEFS, 0);
			for (n = 0; n < 7; n++)
				menustr[ n] = (char*)GetMemory( 60);
			sprintf( menustr[ 6], "Edit SideDef #%d", sdlist->objnum);
			texname[ 8] = '\0';
			strncpy( texname, SideDefs[ sdlist->objnum].tex3, 8);
			sprintf( menustr[ 0], "Change Normal Texture   (Current: %s)", texname);
			strncpy( texname, SideDefs[ sdlist->objnum].tex1, 8);
            sprintf( menustr[ 1], "Change Upper Texture    (Current: %s)", texname);
			strncpy( texname, SideDefs[ sdlist->objnum].tex2, 8);
            sprintf( menustr[ 2], "Change Lower Texture    (Current: %s)", texname);
			sprintf( menustr[ 3], "Change Texture X Offset (Current: %d)", SideDefs[ sdlist->objnum].xoff);
			sprintf( menustr[ 4], "Change Texture Y Offset (Current: %d)", SideDefs[ sdlist->objnum].yoff);
            sprintf( menustr[ 5], "Change Sector Ref.      (Current: #%d)", SideDefs[ sdlist->objnum].sector);
			val = DisplayMenuArray( -1, -1, menustr[ 6], 6, NULL, menustr, dummy);
			for (n = 0; n < 7; n++)
				FreeMemory( menustr[ n]);
			switch (val) {
			case 1:
				strncpy( texname, SideDefs[ sdlist->objnum].tex3, 8);
				ObjectsNeeded( 0);
				ChooseWallTexture( -1 , -1, "Choose A Wall Texture", NumWTexture, WTexture, texname);
				ObjectsNeeded( OBJ_SIDEDEFS, 0);
				if (strlen(texname) > 0) {
					for (cur = sdlist; cur; cur = cur->next)
						if (cur->objnum >= 0)
							strncpy( SideDefs[ cur->objnum].tex3, texname, 8);
					MadeChanges = TRUE;
				}
				break;
			case 2:
				strncpy( texname, SideDefs[ sdlist->objnum].tex1, 8);
				ObjectsNeeded( 0);
				ChooseWallTexture( -1, -1, "Choose A Wall Texture", NumWTexture, WTexture, texname);
				ObjectsNeeded( OBJ_SIDEDEFS, 0);
				if (strlen(texname) > 0) {
					for (cur = sdlist; cur; cur = cur->next)
						if (cur->objnum >= 0)
							strncpy( SideDefs[ cur->objnum].tex1, texname, 8);
					MadeChanges = TRUE;
				}
				break;
			case 3:
				strncpy( texname, SideDefs[ sdlist->objnum].tex2, 8);
				ObjectsNeeded( 0);
				ChooseWallTexture( -1, -1, "Choose A Wall Texture", NumWTexture, WTexture, texname);
				ObjectsNeeded( OBJ_SIDEDEFS, 0);
				if (strlen(texname) > 0) {
					for (cur = sdlist; cur; cur = cur->next)
						if (cur->objnum >= 0)
							strncpy( SideDefs[ cur->objnum].tex2, texname, 8);
					MadeChanges = TRUE;
				}
				break;
			case 4:
				val = InputIntegerValue( -1, -1, -255, 255, SideDefs[ sdlist->objnum].xoff);
				if (val >= -255) {
					for (cur = sdlist; cur; cur = cur->next)
						if (cur->objnum >= 0)
							SideDefs[ cur->objnum].xoff = val;
					MadeChanges = TRUE;
				}
				break;
			case 5:
				val = InputIntegerValue( -1, -1, -255, 255, SideDefs[ sdlist->objnum].yoff);
				if (val >= -255) {
					for (cur = sdlist; cur; cur = cur->next)
						if (cur->objnum >= 0)
							SideDefs[ cur->objnum].yoff = val;
					MadeChanges = TRUE;
				}
				break;
			case 6:
				val = InputObjectXRef( -1, -1, OBJ_SECTORS, FALSE, SideDefs[ sdlist->objnum].sector);
				if (val >= 0) {
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
			menustr[ n] = (char*)GetMemory( 60);
		sprintf( menustr[ 7], "Edit Sector #%d", obj->objnum);
        sprintf( menustr[ 0], "Change Floor Height     (Current: %d)", Sectors[ obj->objnum].floorh);
		sprintf( menustr[ 1], "Change Ceiling Height   (Current: %d)", Sectors[ obj->objnum].ceilh);
		texname[ 8] = '\0';
		strncpy( texname, Sectors[ obj->objnum].floort, 8);
        sprintf( menustr[ 2], "Change Floor Texture    (Current: %s)", texname);
		strncpy( texname, Sectors[ obj->objnum].ceilt, 8);
		sprintf( menustr[ 3], "Change Ceiling Texture  (Current: %s)", texname);
        sprintf( menustr[ 4], "Change Light Level      (Current: %d)", Sectors[ obj->objnum].light);
        sprintf( menustr[ 5], "Change Type             (Current: %d)", Sectors[ obj->objnum].special);
        sprintf( menustr[ 6], "Change LineDef Tag      (Current: %d)", Sectors[ obj->objnum].tag);
		val = DisplayMenuArray( -1, -1, menustr[ 7], 7, NULL, menustr, dummy);
		for (n = 0; n < 8; n++)
			FreeMemory( menustr[ n]);
		switch (val) {
		case 1:
			for (n = 0; n < 3; n++)
				menustr[ n] = (char*)GetMemory( 60);
			sprintf( menustr[ 2], "Edit Sector #%d", obj->objnum);
			sprintf( menustr[ 0], "Change Floor Height To");
			sprintf( menustr[ 1], "Change Floor Height By");
			val = DisplayMenuArray( -1, -1, menustr[ 2], 2, NULL, menustr, dummy);
			for (n = 0; n < 3; n++)
				FreeMemory( menustr[ n]);
			switch (val) {
			case 1:
				val = InputIntegerValue( -1, -1, -16384, 16383, Sectors[ obj->objnum].floorh);
				if (val >= -16384) {
					for (cur = obj; cur; cur = cur->next)
						Sectors[ cur->objnum].floorh = val;
					MadeChanges = TRUE;
				}
				break;
			case 2:
				val = InputIntegerValue( -1, -1, -1024, 1024, 0);
				if (val >= -1024) {
					for (cur = obj; cur; cur = cur->next)
						Sectors[ cur->objnum].floorh = Sectors[ cur->objnum].floorh + val;
					MadeChanges = TRUE;
				}
				break;
			}
			break;
		case 2:
			for (n = 0; n < 3; n++)
				menustr[ n] = (char*)GetMemory( 60);
			sprintf( menustr[ 2], "Edit Sector #%d", obj->objnum);
			sprintf( menustr[ 0], "Change Ceiling Height To");
			sprintf( menustr[ 1], "Change Ceiling Height By");
			val = DisplayMenuArray( -1, -1, menustr[ 2], 2, NULL, menustr, dummy);
			for (n = 0; n < 3; n++)
				FreeMemory( menustr[ n]);
			switch (val) {
			case 1:
				val = InputIntegerValue( -1, -1, -16384, 16383, Sectors[ obj->objnum].ceilh);
				if (val >= -16384) {
					for (cur = obj; cur; cur = cur->next)
						Sectors[ cur->objnum].ceilh = val;
					MadeChanges = TRUE;
				}
				break;
			case 2:
				val = InputIntegerValue( -1, -1, -1024, 1024, 0);
				if (val >= -1024) {
					for (cur = obj; cur; cur = cur->next)
						Sectors[ cur->objnum].ceilh = Sectors[ cur->objnum].ceilh + val;
					MadeChanges = TRUE;
				}
				break;
			}
			break;
		case 3:
			strncpy( texname, Sectors[ obj->objnum].floort, 8);
			ObjectsNeeded( 0);
			ChooseFloorTexture( -1, -1, "Choose A Floor Texture", NumFTexture, FTexture, texname);
			ObjectsNeeded( OBJ_SECTORS, 0);
			if (strlen(texname) > 0) {
				for (cur = obj; cur; cur = cur->next)
					strncpy( Sectors[ cur->objnum].floort, texname, 8);
				MadeChanges = TRUE;
			}
			break;
		case 4:
			strncpy( texname, Sectors[ obj->objnum].ceilt, 8);
			ObjectsNeeded( 0);
			ChooseFloorTexture( -1, -1, "Choose A Ceiling Texture", NumFTexture, FTexture, texname);
			ObjectsNeeded( OBJ_SECTORS, 0);
			if (strlen(texname) > 0) {
				for (cur = obj; cur; cur = cur->next)
					strncpy( Sectors[ cur->objnum].ceilt, texname, 8);
				MadeChanges = TRUE;
			}
			break;
		case 5:
			for (n = 0; n < 3; n++)
				menustr[ n] = (char*)GetMemory( 60);
			sprintf( menustr[ 2], "Edit Sector #%d", obj->objnum);
			sprintf( menustr[ 0], "Change Light Level To");
			sprintf( menustr[ 1], "Change Light Level By");
			val = DisplayMenuArray( -1, -1, menustr[ 2], 2, NULL, menustr, dummy);
			for (n = 0; n < 3; n++)
				FreeMemory( menustr[ n]);
			switch (val) {
			case 1:
				val = InputIntegerValue( -1, -1, 0, 255, Sectors[ obj->objnum].light);
				if (val >= 0) {
					for (cur = obj; cur; cur = cur->next)
						Sectors[ cur->objnum].light = val;
					MadeChanges = TRUE;
				}
				break;
			case 2:
				val = InputIntegerValue( -1, -1, -255, 255, 0);
				if (val >= -255) {
					for (cur = obj; cur; cur = cur->next) {
						Sectors[ cur->objnum].light = Sectors[ cur->objnum].light + val;
						if (Sectors[ cur->objnum].light <= -1) {
							Sectors[ cur->objnum].light = 0;
						}
						if (Sectors[ cur->objnum].light >= 256) {
							Sectors[ cur->objnum].light = 255;
						}
					}
					MadeChanges = TRUE;
				}
				break;
			}
			break;
		case 6:
			/* Choose a sector type */
			{
				sector_class *c;
				sector_type *t;
				char *menuarr[30];
				BCINT dummy[30];
				int i;
				Bool input_integer = FALSE;
				
				c = Sector_classes;
				for(i = 0; (c && i < 29); i++) {
					menuarr[i] = c->name;
					c = c->next;
				}
				menuarr[i++] = "(Enter a Decimal Value)";
				
				val = -1;
				val = DisplayMenuArray( -1 , -1, "Choose A Sector Type:",
									   i, NULL, menuarr, dummy);
				
				if(val == i) {
					input_integer = TRUE;
					val = InputIntegerValue(-1, -1, 0, 32767, 0);
				}
				
				if(val > 0) {
					if(!input_integer) {
						c = Sector_classes;
						while(--val)
							c = c->next;
						/* now c points to the class we are
						   going to display a submenu for */
						
						t = c->types;
						for(i = 0; (t && i < 30); i++) {
							menuarr[i] = t->longname;
							t = t->next;
						}
						
						if(i == 1) {
							val = c->types->type;
						}
						else {
							val = -1;
							val = DisplayMenuArray(-1, -1, "Choose A Sector Type:",
												   i, NULL, menuarr, dummy);
						}
					}
					
					if (val >= 0) {
						if(!input_integer) {
							t = c->types;
							while(--val)
								t = t->next;
							val = t->type;
						}
						
						for (cur = obj; cur; cur = cur->next)
							Sectors[ cur->objnum].special = val;
						MadeChanges = TRUE;
					}
				}
			}
			break;
		case 7:
			if (Sectors[ obj->objnum].tag == 0) {
				val = InputIntegerValue( -1, -1, 0, 999, FindFreeTag());
			}
			else {
				val = InputIntegerValue( -1, -1, 0, 999, Sectors[ obj->objnum].tag);
			}
			if (val >= 0) {
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
   Yuck!  Dirty piece of code...
   */

Bool Input2Numbers( BCINT x0, BCINT y0, char *name1, char *name2, BCINT v1max, BCINT v2max, BCINT *v1, BCINT *v2)
{
    BCINT  key;
    BCINT  maxlen, first;
    Bool ok;
    char prompt[ 80];
    
    if (UseMouse)
		HideMousePointer();
    sprintf( prompt, "Give The %s And %s For The Object:", name1, name2);
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
    for (;;) {
		ok = TRUE;
		DrawScreenBox3D( x0 + 10, y0 + 38, x0 + 71, y0 + 51);
		if (*v1 < 0 || *v1 > v1max) {
			SetColor( DARKGRAY);
			ok = FALSE;
		}
		DrawScreenText( x0 + 14, y0 + 41, "%d", *v1);
		DrawScreenBox3D( x0 + 180, y0 + 38, x0 + 241, y0 + 51);
		if (*v2 < 0 || *v2 > v2max) {
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
		else if ((key & 0x00FF) == 0x000D) {
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
   display number of objects, etc.
   */

void Statistics( BCINT x0, BCINT y0)
{
    if (x0 < 0)
		x0 = (ScrMaxX - 270) / 2;
    if (y0 < 0)
		y0 = (ScrMaxY - 100) / 2;
    if (UseMouse)
		HideMousePointer();
    DrawScreenBox3D( x0, y0, x0 + 270, y0 + 100);
    SetColor( WHITE);
    DrawScreenText( x0 + 10, y0 + 5, "Statistics");
    if (Things == NULL)
		SetColor( DARKGRAY);
    else
		SetColor( BLACK);
    DrawScreenText( -1, y0 + 25, "Number Of Things:   %4d (%lu K)",
				   NumThings, ((unsigned long) NumThings * sizeof( struct Thing) + 512L) / 1024L);
    if (Vertexes == NULL)
		SetColor( DARKGRAY);
    else
		SetColor( BLACK);
    DrawScreenText( -1, -1, "Number Of Vertices: %4d (%lu K)",
				   NumVertexes, ((unsigned long) NumVertexes * sizeof( struct Vertex) + 512L) / 1024L);
    if (LineDefs == NULL)
		SetColor( DARKGRAY);
    else
		SetColor( BLACK);
    DrawScreenText( -1, -1, "Number Of LineDefs: %4d (%lu K)",
				   NumLineDefs, ((unsigned long) NumLineDefs * sizeof( struct LineDef) + 512L) / 1024L);
    if (SideDefs == NULL)
		SetColor( DARKGRAY);
    else
		SetColor( BLACK);
    DrawScreenText( -1, -1, "Number Of SideDefs: %4d (%lu K)",
				   NumSideDefs, ((unsigned long) NumSideDefs * sizeof( struct SideDef) + 512L) / 1024L);
    if (Sectors == NULL)
		SetColor( DARKGRAY);
    else
		SetColor( BLACK);
    DrawScreenText( -1, -1, "Number Of Sectors:  %4d (%lu K)",
				   NumSectors, ((unsigned long) NumSectors * sizeof( struct Sector) + 512L) / 1024L);
    SetColor( YELLOW);
    DrawScreenText( x0 + 10, y0 + 85, "Press Any Key To Continue...");
    bioskey( 0);
    if (UseMouse)
		ShowMousePointer();
}



/*
   display a message while the user is waiting...
   */

void CheckingObjects( BCINT x0, BCINT y0)
{
    if (UseMouse)
		HideMousePointer();
    if (x0 < 0)
		x0 = (ScrMaxX - 172) / 2;
    if (y0 < 0)
		y0 = (ScrMaxY - 30) / 2;
    DrawScreenBox3D( x0, y0, x0 + 172, y0 + 30);
    DrawScreenText( x0 + 10, y0 + 5, "Checking Objects...");
    DrawScreenText( x0 + 10, y0 + 15, "   Please wait");
    if (UseMouse)
		ShowMousePointer();
}


/*
   display a message, then ask if the check should continue (prompt2 may be NULL)
   */

Bool CheckFailed( BCINT x0, BCINT y0, char *prompt1, char *prompt2, Bool fatal)
{
    BCINT key;
    BCINT maxlen;
    
    if (UseMouse)
		HideMousePointer();
    if (fatal == TRUE)
		maxlen = 44;
    else
		maxlen = 27;
    if (strlen( prompt1) > maxlen)
		maxlen = strlen( prompt1);
    if (prompt2 != NULL && strlen( prompt2) > maxlen)
		maxlen = strlen( prompt2);
    if (x0 < 0)
		x0 = (ScrMaxX - 22 - 8 * maxlen) / 2;
    if (y0 < 0)
		y0 = (ScrMaxY - (prompt2 ? 73 : 63)) / 2;
    DrawScreenBox3D( x0, y0, x0 + 22 + 8 * maxlen, y0 + (prompt2 ? 73 : 63));
    SetColor( RED);
    DrawScreenText( x0 + 10, y0 + 8, "Verification Failed:");
    Beep();
    SetColor( WHITE);
    DrawScreenText( x0 + 10, y0 + 18, prompt1);
    LogMessage( "\t%s\n", prompt1);
    if (prompt2 != NULL) {
		DrawScreenText( x0 + 10, y0 + 28, prompt2);
		LogMessage( "\t%s\n", prompt2);
    }
    if (fatal == TRUE) {
		DrawScreenText( x0 + 10, y0 + (prompt2 ? 38 : 28), "The Game Will Crash If You Play With This Level.");
		SetColor( YELLOW);
		DrawScreenText( x0 + 10, y0 + (prompt2 ? 58 : 48), "Press Any Key To See The Object");
		LogMessage( "\n");
    }
    else {
		SetColor( YELLOW);
		DrawScreenText( x0 + 10, y0 + (prompt2 ? 48 : 38), "Press Esc To See The Object,");
		DrawScreenText( x0 + 10, y0 + (prompt2 ? 58 : 48), "Or Any Other Key To Continue");
    }
    key = bioskey( 0);
    if ((key & 0x00FF) != 0x001B) {
		DrawScreenBox3D( x0, y0, x0 + 22 + 8 * maxlen, y0 + (prompt2 ? 73 : 63));
		DrawScreenText( x0 + 10 + 4 * (maxlen - 26), y0 + 28, "Verifying Other Objects...");
    }
    if (UseMouse)
		ShowMousePointer();
    
    return ((key & 0x00FF) == 0x001B);
}



/*
   check if all sectors are closed
   */

void CheckSectors( void) /* SWAP! */
{
    BCINT	  s, n, sd;
    char huge *ends;
    char	   msg1[ 80], msg2[80];
    
    CheckingObjects( -1, -1);
    LogMessage( "\nVerifying Sectors...\n");
    ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
    ends = GetFarMemory( NumVertexes * sizeof( char));
    for (s = 0; s < NumSectors; s++) {
		/* clear the "ends" array */
		for (n = 0; n < NumVertexes; n++)
			ends[ n] = 0;
		/* for each SideDef bound to the Sector, store a "1" in the "ends" */
		/* array for its starting Vertex, and a "2" for its ending Vertex  */
		for (n = 0; n < NumLineDefs; n++) {
			sd = LineDefs[ n].sidedef1;
			if (sd >= 0 && SideDefs[ sd].sector == s) {
				ends[ LineDefs[ n].start] |= 1;
				ends[ LineDefs[ n].end] |= 2;
			}
			sd = LineDefs[ n].sidedef2;
			if (sd >= 0 && SideDefs[ sd].sector == s) {
				ends[ LineDefs[ n].end] |= 1;
				ends[ LineDefs[ n].start] |= 2;
			}
		}
		/* every entry in the "ends" array should be "0" or "3" */
		for (n = 0; n < NumVertexes; n++) {
			if (ends[ n] == 1) {
				sprintf( msg1, "Sector #%d Is Not Closed!", s);
				sprintf( msg2, "There Is No SideDef Ending At Vertex #%d", n);
				if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_VERTEXES, n);
					return;
				}
			}
			if (ends[ n] == 2) {
				sprintf( msg1, "Sector #%d is not closed!", s);
				sprintf( msg2, "There Is No SideDef Starting At Vertex #%d", n);
				if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_VERTEXES, n);
					return;
				}
			}
		}
    }
    FreeFarMemory( ends);
    
    /*
       Note from RQ:
       This is a very simple idea, but it works!  The first test (above)
       checks that all Sectors are closed.  But if a closed set of LineDefs
       is moved out of a Sector and has all its "external" SideDefs pointing
       to that Sector instead of the new one, then we need a second test.
       That's why I check if the SideDefs facing each other are bound to
       the same Sector.
       
       Other note from RQ:
       Nowadays, what makes the power of a good editor is its automatic tests.
       So, if you are writing another Doom editor, you will probably want
       to do the same kind of tests in your program.  Fine, but if you use
       these ideas, don't forget to credit DEU...  Just a reminder... :-)
       */
    
    /* now check if all SideDefs are facing a SideDef with the same Sector number */
    for (n = 0; n < NumLineDefs; n++) {
		ObjectsNeeded( OBJ_LINEDEFS, 0);
		sd = LineDefs[ n].sidedef1;
		if (sd >= 0) {
			s = GetOppositeSector( n, TRUE);
			ObjectsNeeded( OBJ_SIDEDEFS, 0);
			if (s < 0 || SideDefs[ sd].sector != s) {
				if (s < 0) {
					sprintf( msg1, "Sector #%d Is Not Closed!", SideDefs[ sd].sector);
					sprintf( msg2, "Check LineDef #%d (First SideDef: #%d)", n, sd);
				}
				else {
					sprintf( msg1, "Sectors #%d And #%d Are Not Closed!", SideDefs[ sd].sector, s);
					sprintf( msg2, "Check LineDef #%d (First SideDef: #%d) And The One Facing It", n, sd);
				}
				if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_LINEDEFS, n);
					return;
				}
			}
		}
		ObjectsNeeded( OBJ_LINEDEFS, 0);
		sd = LineDefs[ n].sidedef2;
		if (sd >= 0) {
			s = GetOppositeSector( n, FALSE);
			ObjectsNeeded( OBJ_SIDEDEFS, 0);
			if (s < 0 || SideDefs[ sd].sector != s) {
				if (s < 0) {
					sprintf( msg1, "Sector #%d Is Not Closed!", SideDefs[ sd].sector);
					sprintf( msg2, "Check LineDef #%d (Second SideDef: #%d)", n, sd);
				}
				else {
					sprintf( msg1, "Sectors #%d And #%d Are Not Closed!", SideDefs[ sd].sector, s);
					sprintf( msg2, "Check LineDef #%d (Second SideDef: #%d) And The One Facing It", n, sd);
				}
				if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_LINEDEFS, n);
					return;
				}
			}
		}
    }
}



/*
   check cross-references and delete unused objects
   */	  

void CheckCrossReferences( void) /* SWAP! */
{
    char   msg[ 80];
    BCINT	n, m;
    SelPtr cur;
    
    CheckingObjects( -1, -1);
    LogMessage( "\nVerifying cross-references...\n");
    ObjectsNeeded( OBJ_LINEDEFS, 0);
    for (n = 0; n < NumLineDefs; n++) {
		/* check for missing first SideDefs */
		if (LineDefs[ n].sidedef1 < 0) {
			sprintf( msg, "ERROR: LineDef #%d Has No First SideDef!", n);
			CheckFailed( -1, -1, msg, NULL, TRUE);
			GoToObject( OBJ_LINEDEFS, n);
			return;
		}
		/* check for SideDefs used twice in the same LineDef */
		if (LineDefs[ n].sidedef1 == LineDefs[ n].sidedef2) {
			sprintf( msg, "ERROR: LineDef #%d Uses The Same SideDef Twice (#%d)", n, LineDefs[ n].sidedef1);
			CheckFailed( -1, -1, msg, NULL, TRUE);
			GoToObject( OBJ_LINEDEFS, n);
			return;
		}
		/* check for Vertices used twice in the same LineDef */
		if (LineDefs[ n].start == LineDefs[ n].end) {
			sprintf( msg, "ERROR: LineDef #%d Uses The Same Vertex Twice (#%d)", n, LineDefs[ n].start);
			CheckFailed( -1, -1, msg, NULL, TRUE);
			GoToObject( OBJ_LINEDEFS, n);
			return;
		}
    }
    
    /* check if there aren't two LineDefs between the same Vertices */
    cur = NULL;
    for (n = NumLineDefs - 1; n >= 1; n--) {
		for (m = n - 1; m >= 0; m--)
			if ((LineDefs[ n].start == LineDefs[ m].start && LineDefs[ n].end == LineDefs[ m].end)
				|| (LineDefs[ n].start == LineDefs[ m].end && LineDefs[ n].end == LineDefs[ m].start)) {
				SelectObject( &cur, n);
				break;
			}
    }
    if (cur && (Expert || Confirm(-1, -1, "There Are Multiple LineDefs Between The Same Vertices", "Do You Want To Delete The Redundant LineDefs?")))
		DeleteObjects( OBJ_LINEDEFS, &cur);
    else
		ForgetSelection( &cur);
    
    CheckingObjects( -1, -1);
    /* check for invalid flags in the LineDefs */
    for (n = 0; n < NumLineDefs; n++)
		if ((LineDefs[ n].flags & 0x01) == 0 && LineDefs[ n].sidedef2 < 0)
			SelectObject( &cur, n);
    if (cur && (Expert || Confirm(-1, -1, "Some LineDefs Have Only One Side But Their Im Bit Is Not Set", "Do You Want To Set The 'Impassible' Flag?"))) {
		while (cur) {
			LineDefs[ cur->objnum].flags |= 0x01;
			UnSelectObject( &cur, cur->objnum);
		}
    }
    else
		ForgetSelection( &cur);
    
    CheckingObjects( -1, -1);
    for (n = 0; n < NumLineDefs; n++)
		if ((LineDefs[ n].flags & 0x04) != 0 && LineDefs[ n].sidedef2 < 0)
			SelectObject( &cur, n);
    if (cur && (Expert || Confirm(-1, -1, "Some LineDefs Have Only One Side But Their 2S Bit Is Set", "Do You Want To Clear The 'Two-Sided' Flag?"))) {
		while (cur) {
			LineDefs[ cur->objnum].flags &= ~0x04;
			UnSelectObject( &cur, cur->objnum);
		}
    }
    else
		ForgetSelection( &cur);
    
    CheckingObjects( -1, -1);
    for (n = 0; n < NumLineDefs; n++)
		if ((LineDefs[ n].flags & 0x04) == 0 && LineDefs[ n].sidedef2 >= 0)
			SelectObject( &cur, n);
    if (cur && (Expert || Confirm(-1, -1, "Some LineDefs Have Two Sides But Their 2S Bit Is Not Set", "Do You Want To Set The 'Two-Sided' Flag?"))) {
		while (cur) {
			LineDefs[ cur->objnum].flags |= 0x04;
			UnSelectObject( &cur, cur->objnum);
		}
    }
    else
		ForgetSelection( &cur);
    
    CheckingObjects( -1, -1);
    /* select all Vertices */
    for (n = 0; n < NumVertexes; n++)
		SelectObject( &cur, n);
    /* unselect Vertices used in a LineDef */
    for (n = 0; n < NumLineDefs; n++) {
		m = LineDefs[ n].start;
		if (cur && m >= 0)
			UnSelectObject( &cur, m);
		m = LineDefs[ n].end;
		if (cur && m >= 0)
			UnSelectObject( &cur, m);
		continue;
    }
    /* check if there are any Vertices left */
    if (cur && (Expert || Confirm(-1, -1, "Some Vertices Are Not Bound To Any LineDef", "Do You Want To Delete These Unused Vertices?"))) {
		DeleteObjects( OBJ_VERTEXES, &cur);
		ObjectsNeeded( OBJ_LINEDEFS, 0);
    }
    else
		ForgetSelection( &cur);
    
    CheckingObjects( -1, -1);
    /* select all SideDefs */
    for (n = 0; n < NumSideDefs; n++)
		SelectObject( &cur, n);
    /* unselect SideDefs bound to a LineDef */
    for (n = 0; n < NumLineDefs; n++) {
		m = LineDefs[ n].sidedef1;
		if (cur && m >= 0)
			UnSelectObject( &cur, m);
		m = LineDefs[ n].sidedef2;
		if (cur && m >= 0)
			UnSelectObject( &cur, m);
		continue;
    }
    /* check if there are any SideDefs left */
    if (cur && (Expert || Confirm(-1, -1, "Some SideDefs Are Not Bound To Any LineDef", "Do You Want To Delete These Unused SideDefs?")))
		DeleteObjects( OBJ_SIDEDEFS, &cur);
    else
		ForgetSelection( &cur);
    
    CheckingObjects( -1, -1);
    /* select all Sectors */
    for (n = 0; n < NumSectors; n++)
		SelectObject( &cur, n);
    /* unselect Sectors bound to a SideDef */
    for (n = 0; n < NumLineDefs; n++) {
		ObjectsNeeded( OBJ_LINEDEFS, 0);
		m = LineDefs[ n].sidedef1;
		ObjectsNeeded( OBJ_SIDEDEFS, 0);
		if (cur && m >= 0 && SideDefs[ m].sector >= 0)
			UnSelectObject( &cur, SideDefs[ m].sector);
		ObjectsNeeded( OBJ_LINEDEFS, 0);
		m = LineDefs[ n].sidedef2;
		ObjectsNeeded( OBJ_SIDEDEFS, 0);
		if (cur && m >= 0 && SideDefs[ m].sector >= 0)
			UnSelectObject( &cur, SideDefs[ m].sector);
		continue;
    }
    /* check if there are any Sectors left */
    if (cur && (Expert || Confirm(-1, -1, "Some Sectors Are Not Bound To Any SideDef", "Do You Want To Delete These Unused Sectors?")))
		DeleteObjects( OBJ_SECTORS, &cur);
    else
		ForgetSelection( &cur);
}



/*
   check for missing textures
   */

void CheckTextures( void) /* SWAP! */
{
    BCINT  n;
    BCINT  sd1, sd2;
    BCINT  s1, s2;
    char msg1[ 80], msg2[ 80];
    
    CheckingObjects( -1, -1);
    LogMessage( "\nVerifying Textures...\n");
    ObjectsNeeded( OBJ_SECTORS, 0);
    for (n = 0; n < NumSectors; n++) {
		if (Sectors[ n].ceilt[ 0] == '-' && Sectors[ n].ceilt == '\0') {
			sprintf( msg1, "Error: Sector #%d Has No Ceiling Texture", n);
			sprintf( msg2, "You Probaly Used A Brain-Damaged Editor To Do That...");
			CheckFailed( -1, -1, msg1, msg2, TRUE);
			GoToObject( OBJ_SECTORS, n);
			return;
		}
		if (Sectors[ n].floort[ 0] == '-' && Sectors[ n].floort == '\0') {
			sprintf( msg1, "Error: Sector #%d Has No Floor Texture", n);
			sprintf( msg2, "You Probaly Used A Brain-Damaged Editor To Do That...");
			CheckFailed( -1, -1, msg1, msg2, TRUE);
			GoToObject( OBJ_SECTORS, n);
			return;
		}
		if (Sectors[ n].ceilh < Sectors[ n].floorh) {
			sprintf( msg1, "Error: Sector #%d Has Its Ceiling Lower Than Its Floor", n);
			sprintf( msg2, "The Textures Will Never Be Displayed If You Cannot Go There");
			CheckFailed( -1, -1, msg1, msg2, TRUE);
			GoToObject( OBJ_SECTORS, n);
			return;
		}
		if (Sectors[ n].ceilh - Sectors[ n].floorh > 1023) {
			sprintf( msg1, "Error: Sector #%d Has Its Ceiling Too High", n);
			sprintf( msg2, "The Maximum Difference Allowed Is 1023 (Ceiling - Floor)");
			CheckFailed( -1, -1, msg1, msg2, TRUE);
			GoToObject( OBJ_SECTORS, n);
			return;
		}
    }
    for (n = 0; n < NumLineDefs; n++) {
		ObjectsNeeded( OBJ_LINEDEFS, 0);
		sd1 = LineDefs[ n].sidedef1;
		sd2 = LineDefs[ n].sidedef2;
		ObjectsNeeded( OBJ_SIDEDEFS, OBJ_SECTORS, 0);
		if (sd1 >= 0)
			s1 = SideDefs[ sd1].sector;
		else
			s1 = -1;
		if (sd2 >= 0)
			s2 = SideDefs[ sd2].sector;
		else
			s2 = -1;
		if (s1 >= 0 && s2 < 0) {
			if (SideDefs[ sd1].tex3[ 0] == '-' && SideDefs[ sd1].tex3[ 1] == '\0') {
				sprintf( msg1, "Error In One-Sided Linedef #%d: SideDef #%d Has No Normal Texture", n, sd1);
				sprintf( msg2, "Do You Want To Set The Texture To \"%s\" And Continue?", DefaultWallTexture);
				if (CheckFailed( -1, -1, msg1, msg2, FALSE))
					{
						GoToObject( OBJ_LINEDEFS, n);
						return;
					}
				strncpy( SideDefs[ sd1].tex3, DefaultWallTexture, 8);
				CheckingObjects( -1, -1);
			}
		}
		if (s1 >= 0 && s2 >= 0 && Sectors[ s1].ceilh > Sectors[ s2].ceilh) {
			if (SideDefs[ sd1].tex1[ 0] == '-' && SideDefs[ sd1].tex1[ 1] == '\0'
				&& (strncmp( Sectors[ s1].ceilt, "F_SKY1", 8) || strncmp( Sectors[ s2].ceilt, "F_SKY1", 8))) {
				sprintf( msg1, "Error In First SideDef Of Linedef #%d: SideDef #%d Has No Upper Texture", n, sd1);
				sprintf( msg2, "Do You Want To Set The Texture To \"%s\" And Continue?", DefaultUpperTexture);
				if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_LINEDEFS, n);
					return;
				}
				strncpy( SideDefs[ sd1].tex1, DefaultUpperTexture, 8);
				CheckingObjects( -1, -1);
			}
		}
		if (s1 >= 0 && s2 >= 0 && Sectors[ s1].floorh < Sectors[ s2].floorh) {
			if (SideDefs[ sd1].tex2[ 0] == '-' && SideDefs[ sd1].tex2[ 1] == '\0') {
				sprintf( msg1, "Error In First SideDef Of Linedef #%d: SideDef #%d Has No Lower Texture", n, sd1);
				sprintf( msg2, "Do You Want To Set The Texture To \"%s\" And Continue?", DefaultLowerTexture);
				if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_LINEDEFS, n);
					return;
				}
				strncpy( SideDefs[ sd1].tex2, DefaultLowerTexture, 8);
				CheckingObjects( -1, -1);
			}
		}
		if (s1 >= 0 && s2 >= 0 && Sectors[ s2].ceilh > Sectors[ s1].ceilh) {
			if (SideDefs[ sd2].tex1[ 0] == '-' && SideDefs[ sd2].tex1[ 1] == '\0'
				&& (strncmp( Sectors[ s1].ceilt, "F_SKY1", 8) || strncmp( Sectors[ s2].ceilt, "F_SKY1", 8))) {
				sprintf( msg1, "Error In Second SideDef Of Linedef #%d: SideDef #%d Has No Upper Texture", n, sd2);
				sprintf( msg2, "Do You Want To Set The Texture To \"%s\" And Continue?", DefaultUpperTexture);
				if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_LINEDEFS, n);
					return;
				}
				strncpy( SideDefs[ sd2].tex1, DefaultUpperTexture, 8);
				CheckingObjects( -1, -1);
			}
		}
		if (s1 >= 0 && s2 >= 0 && Sectors[ s2].floorh < Sectors[ s1].floorh) {
			if (SideDefs[ sd2].tex2[ 0] == '-' && SideDefs[ sd2].tex2[ 1] == '\0') {
				sprintf( msg1, "Error In Second SideDef Of Linedef #%d: SideDef #%d Has No Lower Texture", n, sd2);
				
				sprintf( msg2, "Do You Want To Set The Texture To \"%s\" And Continue?", DefaultLowerTexture);
				if(CheckFailed( -1, -1, msg1, msg2, FALSE)) {
					GoToObject( OBJ_LINEDEFS, n);
					return;
				}
				strncpy( SideDefs[ sd2].tex2, DefaultUpperTexture, 8);
				CheckingObjects( -1, -1);
			}
		}
    }
}



/*
   check if a texture name matches one of the elements of a list
   */

Bool IsTextureNameInList( char *name, char **list, BCINT numelems)
{
    BCINT n;
    
    for (n = 0; n < numelems; n++)
		if (! strnicmp( name, list[ n], 8))
			return TRUE;
    return FALSE;
}



/*
   check for invalid texture names
   */

void CheckTextureNames( void) /* SWAP! */
{
    BCINT  n;
    char msg1[ 80], msg2[ 80];
    
    LogMessage( "\nVerifying texture names...\n");

	/* SO 9/4/95:
	   Removed another "break level if not registered" check */
    
    ObjectsNeeded( OBJ_SECTORS, 0);
    for (n = 0; n < NumSectors; n++) {
		if (! IsTextureNameInList( Sectors[ n].ceilt, FTexture, NumFTexture)) {
			sprintf( msg1, "Invalid Ceiling Texture In Sector #%d", n);
			sprintf( msg2, "The Name \"%s\" Is Not A Floor/Ceiling Texture", Sectors[ n].ceilt);
			if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
				GoToObject( OBJ_SECTORS, n);
				return;
			}						   
			CheckingObjects( -1, -1);
		}
		if (! IsTextureNameInList( Sectors[ n].floort, FTexture, NumFTexture)) {
			sprintf( msg1, "Invalid Floor Texture In Sector #%d", n);
			sprintf( msg2, "The Name \"%s\" Is Not A Floor/Ceiling Texture", Sectors[ n].floort);
			if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
				GoToObject( OBJ_SECTORS, n);
				return;
			}						
			CheckingObjects( -1, -1);
		}
    }
    ObjectsNeeded( OBJ_SIDEDEFS, 0);
    for (n = 0; n < NumSideDefs; n++) {
		if (! IsTextureNameInList( SideDefs[ n].tex1, WTexture, NumWTexture)) {
			sprintf( msg1, "Invalid Upper Texture In SideDef #%d", n);
			sprintf( msg2, "The Name \"%s\" Is Not A Wall Texture", SideDefs[ n].tex1);
			if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
				GoToObject( OBJ_SIDEDEFS, n);
				return;			   
			}
			CheckingObjects( -1, -1);
		}
		if (! IsTextureNameInList( SideDefs[ n].tex2, WTexture, NumWTexture)) {
			sprintf( msg1, "Invalid Lower Texture In SideDef #%d", n);
			sprintf( msg2, "The Name \"%s\" Is Not A Wall Texture", SideDefs[ n].tex2);
			if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
				GoToObject( OBJ_SIDEDEFS, n);
				return;
			}						
			CheckingObjects( -1, -1);
		}
		if (! IsTextureNameInList( SideDefs[ n].tex3, WTexture, NumWTexture)) {
			sprintf( msg1, "Invalid Normal Texture In SideDef #%d", n);
			sprintf( msg2, "The Name \"%s\" Is Not A Wall Texture", SideDefs[ n].tex3);
			if (CheckFailed( -1, -1, msg1, msg2, FALSE)) {
				GoToObject( OBJ_SIDEDEFS, n);
				return;
			}
			CheckingObjects( -1, -1);
		}
    }
}



/*
   check the level consistency
   */								

void CheckLevel( BCINT x0, BCINT y0) /* SWAP! */
{
    switch (DisplayMenu( -1, -1, ((x0 == -1) ? "Check Level Consistency" : NULL),
						"Number Of Objects",
						"Check If All Sectors Are Closed",
						"Check All Cross-references",
						"Check For Missing Textures",
						"Check Texture Names",
						NULL)) {
    case 1:
		Statistics( -1, -1);
		break;
    case 2:
		CheckSectors();
		break;
    case 3:
		CheckCrossReferences();
		break;
    case 4:
		CheckTextures();
		break;
    case 5:
		CheckTextureNames();
		break;
    }
}



/*
   check for players starting points
   */

Bool CheckStartingPos() /* SWAP! */
{
	return TRUE;
}



/*
   insert a standard object at given position
   */

void InsertStandardObject( BCINT x0, BCINT y0, BCINT xpos, BCINT ypos) /* SWAP! */
{
    BCINT sector;
    BCINT choice, n;
    BCINT a, b;
    
    /* show where the object will be inserted */
    if (UseMouse)
		HideMousePointer();
    DrawPointer( TRUE);
    if (UseMouse)
		ShowMousePointer();
    /* are we inside a Sector? */
    sector = GetCurObject( OBJ_SECTORS, xpos, ypos, xpos, ypos);
    if (sector >= 0)
		choice = DisplayMenu( -1, -1, ((x0 == -1) ? "Insert A Pre-Defined Object (Inside A Sector)" : NULL),
							 "Rectangle",
							 "Polygon (N sides)",
							 NULL);
    else
		choice = DisplayMenu( -1, -1, ((x0 == -1) ? "Insert A Pre-Defined Object (Outside)" : NULL),
							 "Rectangle",
							 "Polygon (N sides)",
							 NULL);
    /* !!!! Should also check for overlapping objects (Sectors) !!!! */
    switch (choice) {
    case 1:
		a = 256;
		b = 128;
		if (Input2Numbers( -1, -1, "Width", "Height", 2000, 2000, &a, &b)) {
			if (a < 8)
				a = 8;
			if (b < 8)
				b = 8;
			xpos = xpos - a / 2;
			ypos = ypos - b / 2;
			InsertObject( OBJ_VERTEXES, -1, xpos, ypos);
			InsertObject( OBJ_VERTEXES, -1, xpos + a, ypos);
			InsertObject( OBJ_VERTEXES, -1, xpos + a, ypos + b);
			InsertObject( OBJ_VERTEXES, -1, xpos, ypos + b);
			if (sector < 0)
				InsertObject( OBJ_SECTORS, -1, 0, 0);
			for (n = 0; n < 4; n++) {
				InsertObject( OBJ_LINEDEFS, -1, 0, 0);
				LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs;
				InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
				if (sector >= 0)
					SideDefs[ NumSideDefs - 1].sector = sector;
			}
			ObjectsNeeded( OBJ_LINEDEFS, 0);
			if (sector >= 0) {
				LineDefs[ NumLineDefs - 4].start = NumVertexes - 4;
				LineDefs[ NumLineDefs - 4].end = NumVertexes - 3;
				LineDefs[ NumLineDefs - 3].start = NumVertexes - 3;
				LineDefs[ NumLineDefs - 3].end = NumVertexes - 2;
				LineDefs[ NumLineDefs - 2].start = NumVertexes - 2;
				LineDefs[ NumLineDefs - 2].end = NumVertexes - 1;
				LineDefs[ NumLineDefs - 1].start = NumVertexes - 1;
				LineDefs[ NumLineDefs - 1].end = NumVertexes - 4;
			}
			else {
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
		if (Input2Numbers( -1, -1, "Number Of Sides", "Radius", 32, 2000, &a, &b)) {
			if (a < 3)
				a = 3;
			if (b < 8)
				b = 8;
			InsertPolygonVertices( xpos, ypos, a, b);
			if (sector < 0)
				InsertObject( OBJ_SECTORS, -1, 0, 0);
			for (n = 0; n < a; n++) {
				InsertObject( OBJ_LINEDEFS, -1, 0, 0);
				LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs;
				InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
				if (sector >= 0)
					SideDefs[ NumSideDefs - 1].sector = sector;
			}
			ObjectsNeeded( OBJ_LINEDEFS, 0);
			if (sector >= 0) {
				LineDefs[ NumLineDefs - 1].start = NumVertexes - 1;
				LineDefs[ NumLineDefs - 1].end = NumVertexes - a;
				for (n = 2; n <= a; n++) {
					LineDefs[ NumLineDefs - n].start = NumVertexes - n;
					LineDefs[ NumLineDefs - n].end = NumVertexes - n + 1;
				}
			}
			else {
				LineDefs[ NumLineDefs - 1].start = NumVertexes - a;
				LineDefs[ NumLineDefs - 1].end = NumVertexes - 1;
				for (n = 2; n <= a; n++) {
					LineDefs[ NumLineDefs - n].start = NumVertexes - n + 1;
					LineDefs[ NumLineDefs - n].end = NumVertexes - n;
				}
			}
		}
		break;
    case 3:
		/*
		   a = 6;
		   b = 16;
		   if (Input2Numbers( -1, -1, "Number Of Steps", "Step Height", 32, 48, &a, &b)) {
		   if (a < 2)
		   a = 2;
		   ObjectsNeeded( OBJ_SECTORS, 0);
		   n = Sectors[ sector].ceilh;
		   h = Sectors[ sector].floorh;
		   if (a * b < n - h) {
		   Beep();
		   Notify( -1, -1, "The Stairs Are Too High For This Sector", NULL);
		   return;
		   }
		   xpos = xpos - 32;
		   ypos = ypos - 32 * a;
		   for (n = 0; n < a; n++) {
		   InsertObject( OBJ_VERTEXES, -1, xpos, ypos);
		   InsertObject( OBJ_VERTEXES, -1, xpos + 64, ypos);
		   InsertObject( OBJ_VERTEXES, -1, xpos + 64, ypos + 64);
		   InsertObject( OBJ_VERTEXES, -1, xpos, ypos + 64);
		   ypos += 64;
		   InsertObject( OBJ_SECTORS, sector, 0, 0);
		   h += b;
		   Sectors[ NumSectors - 1].floorh = h;
		   
		   InsertObject( OBJ_LINEDEFS, -1, 0, 0);
		   LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs;
		   LineDefs[ NumLineDefs - 1].sidedef2 = NumSideDefs + 1;
		   InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		   SideDefs[ NumSideDefs - 1].sector = sector;
		   InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
		   
		   ObjectsNeeded( OBJ_LINEDEFS, 0);
		   LineDefs[ NumLineDefs - 4].start = NumVertexes - 4;
		   LineDefs[ NumLineDefs - 4].end = NumVertexes - 3;
		   LineDefs[ NumLineDefs - 3].start = NumVertexes - 3;
		   LineDefs[ NumLineDefs - 3].end = NumVertexes - 2;
		   LineDefs[ NumLineDefs - 2].start = NumVertexes - 2;
		   LineDefs[ NumLineDefs - 2].end = NumVertexes - 1;
		   LineDefs[ NumLineDefs - 1].start = NumVertexes - 1;
		   LineDefs[ NumLineDefs - 1].end = NumVertexes - 4;
		   }
		   }
		   break;
		   */
    case 4:
		NotImplemented();
		break;
    }
}



/*
   menu of miscellaneous operations
   */

void MiscOperations( BCINT x0, BCINT y0, BCINT objtype, SelPtr *list)
{
    char   msg[ 80];
    BCINT	val;
    BCINT	angle, scale;
    
    sprintf( msg, "Rotate and Scale %s", GetEditModeName( objtype));
    if (objtype == OBJ_VERTEXES) {
		val = DisplayMenu( -1, -1, ((x0 == -1) ? "Misc. Operations" : NULL),
						  "Find First Free Tag Number",
						  msg,
						  "Delete Vertex And Join LineDefs",
						  "Merge Several Vertices Into One",
						  "Add A LineDef And Split Sector",
						  NULL);
    }
    else if (objtype == OBJ_LINEDEFS) {
		val = DisplayMenu( -1, -1, ((x0 == -1) ? "Misc. Operations" : NULL),
						  "Find First Free Tag Number",
						  msg,
						  "Split LineDef And Add A Vertex",
						  "Split LineDefs And Add A Sector",
						  "Delete LineDefs And Join Sectors",
						  "Flip LineDef",
						  "Swap SideDefs",
						  "Align Textures Intelligently On Y",
						  "Align Textures Intelligently On X",
						  NULL);
    }
    else if (objtype == OBJ_SECTORS) {
		val = DisplayMenu( -1, -1, ((x0 == -1) ? "Misc. Operations" : NULL),
						  "Find First Free Tag Number",
						  msg,
						  "Make Door From Sector",
						  "Make Lift From Sector",
						  "Distribute Sector Floor Heights",
						  "Distribute Sector Ceiling Heights",
						  "Distribute Sector Light Levels",
						  NULL);
    }														 
    else {
		val = DisplayMenu( -1, -1, ((x0 == -1) ? "Misc. Operations" : NULL),
						  "Find First Free Tag Number",
						  msg,
						  NULL);
    }
    if (val > 1 && *list == NULL) {
		Beep();
		sprintf( msg, "You Must Select At Least One %s", GetObjectTypeName( objtype));
		Notify( -1, -1, msg, NULL);
		return;
    }
    switch (val) {
    case 1:
		sprintf( msg, "First Free Tag Number: %d", FindFreeTag());
		Notify( -1, -1, msg, NULL);
		break;
    case 2:
		if ((objtype == OBJ_THINGS || objtype == OBJ_VERTEXES) && (*list)->next == NULL) {
			Beep();
			sprintf( msg, "You Must Select More Than One %s", GetObjectTypeName( objtype));
			Notify( -1, -1, msg, NULL);
			return;
		}
		angle = 0;
		scale = 100;
		if (Input2Numbers( -1, -1, "Rotation Angle (ø)", "Scale (%)", 360, 1000, &angle, &scale))
			RotateAndScaleObjects( objtype, *list, (double) angle * 0.0174533, (double) scale * 0.01);
		break;
    case 3:
		if (objtype == OBJ_VERTEXES) {
			DeleteVerticesJoinLineDefs( *list);
			ForgetSelection( list);
		}
		else if (objtype == OBJ_LINEDEFS) {
			SplitLineDefs( *list);
		}
		else if (objtype == OBJ_SECTORS) {
			if ((*list)->next != NULL) {
				Beep();
				Notify( -1, -1, "You Must Select Exactly One Sector", NULL);
			}
			else {
				MakeDoorFromSector( (*list)->objnum);
			}
		}
		break;
    case 4:
		if (objtype == OBJ_VERTEXES) {
			MergeVertices( list);
		}
		else if (objtype == OBJ_LINEDEFS) {
			if ((*list)->next == NULL || (*list)->next->next != NULL) {
				Beep();
				Notify( -1, -1, "You Must Select Exactly Two LineDefs", NULL);
			}
			else {
				SplitLineDefsAndSector( (*list)->objnum, (*list)->next->objnum);
				ForgetSelection( list);
			}
		}
		else if (objtype == OBJ_SECTORS) {
			if ((*list)->next != NULL) {
				Beep();
				Notify( -1, -1, "You Must Select Exactly One Sector", NULL);
			}
			else {
				MakeLiftFromSector( (*list)->objnum);
			}
		}
		break;
    case 5:
		if (objtype == OBJ_VERTEXES) {
			if ((*list)->next == NULL || (*list)->next->next != NULL) {
				Beep();
				Notify( -1, -1, "You Must Select Exactly Two Vertices", NULL);
			}
			else {
				SplitSector( (*list)->objnum, (*list)->next->objnum);
				ForgetSelection( list);
			}
		}
		else if (objtype == OBJ_LINEDEFS) {
			DeleteLineDefsJoinSectors( list);
		}
		break;
    case 6:
		if (objtype == OBJ_LINEDEFS) {
			FlipLineDefs( *list, TRUE);
		}
		else if (objtype == OBJ_SECTORS) {
			if ((*list)->next == NULL || (*list)->next->next == NULL) {
				Beep();
				Notify( -1, -1, "You Must Select Three Or More Sectors", NULL);
			}
			else {
				DistributeSectorCeilings( *list);
			}
		}
		break;
    case 7:
		if (objtype == OBJ_LINEDEFS) {
			if (Expert || Confirm( -1, -1, "Warning: The Sector References Are Also Swapped", "You May Get Strange Results If You Don't Know What You Are Doing..."))
				FlipLineDefs( *list, FALSE);
		}
		break;
    case 8:
		if (objtype == OBJ_LINEDEFS) {
			char	*menustr[ 30];
			BCINT	dummy[ 30];
			BCINT	n, val;
			
			x0 = (ScrMaxX - 29 - 8 * 44) / 2;
			y0 = (ScrMaxY - 24 - 10 * 5) / 2;
			for (n = 0; n < 5; n++)
				menustr[ n] = (char*)GetMemory( 80);
			strcpy( menustr[ 4], "Y Texture Alignment");
			strcpy( menustr[ 0], "Align Y With Texture Checking");
			strcpy( menustr[ 1], "Align Y With Texture Checking And Initial Offset");
			strcpy( menustr[ 2], "Align Y Without Texture Checking");
			strcpy( menustr[ 3], "Align Y Without Texture Checking And Initial Offset");
			val = DisplayMenuArray( -1, -1, menustr[ 4], 4, NULL, menustr, dummy);
			for (n = 0; n < 5; n++)
				FreeMemory( menustr[ n]);
			
			if(val >= 1 && val <= 4) {
				val--;
				AlignTexturesY(list, /* list of linedefs */
							   (val & 2) ? TRUE : FALSE, 	/* NoChecking */
							   (val & 1) ? TRUE : FALSE);	/* Initial offset */
			}
		}
		break;
    case 9:
		if (objtype == OBJ_LINEDEFS) {
			char	*menustr[ 30];
			BCINT	dummy[ 30];
			BCINT	n, val;
			
			x0 = (ScrMaxX - 29 - 8 * 44) / 2;
			y0 = (ScrMaxY - 24 - 10 * 5) / 2;
			for (n = 0; n < 5; n++)
				menustr[ n] = (char*)GetMemory( 80);
			strcpy( menustr[ 4], "X Texture Alignment");
			strcpy( menustr[ 0], "Align X With Texture Checking");
			strcpy( menustr[ 1], "Align X With Texture Checking And Initial Offset");
			strcpy( menustr[ 2], "Align X Without Texture Checking");
			strcpy( menustr[ 3], "Align X Without Texture Checking And Initial Offset");
			val = DisplayMenuArray( -1, -1, menustr[ 4], 4, NULL, menustr, dummy);
			for (n = 0; n < 5; n++)
				FreeMemory( menustr[ n]);
			
			if(val >= 1 && val <= 4) {
				val--;
				AlignTexturesX(list, /* list of linedefs */
							   (val & 2) ? TRUE : FALSE, 	/* NoChecking */
							   (val & 1) ? TRUE : FALSE);	/* Initial offset */
			}
			
		}
		else if (objtype == OBJ_SECTORS) {
			if ((*list)->next == NULL || (*list)->next->next == NULL) {
				Beep();
				Notify( -1, -1, "You Must Select Three Or More Sectors", NULL);
			}
			else {
				DistributeLightLevels( *list);
			}
		}
		break;
    }
}



/*
   display a "Preferences" menu (change default textures, etc.)
   */

void Preferences( BCINT x0, BCINT y0)
{
    char   *menustr[ 30];
    BCINT	 dummy[ 30];
    BCINT	 n, val;
    char	texname[ 9];
    
    if (x0 < 0)
		x0 = (ScrMaxX - 50 * 8 - 19) / 2;
    if (y0 < 0)
		y0 = (ScrMaxY - 5 * 10 - 28) / 2;
    for (n = 0; n < 11; n++)
		menustr[ n] = (char*)GetMemory( 100);
    sprintf( menustr[ 10], "Preferences");
    sprintf( menustr[ 0], "Change Default Wall Texture    (Current: %s)", DefaultWallTexture);
    sprintf( menustr[ 1], "Change Default Upper Texture   (Current: %s)", DefaultUpperTexture);
    sprintf( menustr[ 2], "Change Default Lower Texture   (Current: %s)", DefaultLowerTexture);
    sprintf( menustr[ 3], "Change Default Floor Texture   (Current: %s)", DefaultFloorTexture);
    sprintf( menustr[ 4], "Change Default Ceiling Texture (Current: %s)", DefaultCeilingTexture);
    sprintf( menustr[ 5], "Change Default Floor Height    (Current: %d)", (int)DefaultFloorHeight & 0xffff);
    sprintf( menustr[ 6], "Change Default Ceiling Height  (Current: %d)", (int)DefaultCeilingHeight & 0xffff);
    sprintf( menustr[ 7], "Change Default Door Track      (Current: %s)", DefaultDoorTrack);
    sprintf( menustr[ 8], "Change Default Door Texture    (Current: %s)", DefaultDoorTexture);
    sprintf( menustr[ 9], "Change How Close To Line To Split (Currently %d)", (int)CloseToLine);
    val = DisplayMenuArray( -1, -1, menustr[ 10], 10, NULL, menustr, dummy);
    for (n = 0; n < 11; n++)
		FreeMemory( menustr[ n]);
    switch (val) {
    case 1:
		strcpy( texname, DefaultWallTexture);
		ChooseWallTexture( -1, -1, "Choose Normal Texture", NumWTexture, WTexture, texname);
		if (strlen( texname) > 0)
			strcpy( DefaultWallTexture, texname);
		break;
    case 2:
		strcpy( texname, DefaultUpperTexture);
		ChooseWallTexture( -1, -1, "Choose Upper Texture ", NumWTexture, WTexture, texname);
		if (strlen( texname) > 0)
			strcpy( DefaultUpperTexture, texname);
		break;
    case 3:
		strcpy( texname, DefaultLowerTexture);
		ChooseWallTexture( -1, -1, "Choose Lower Texture ", NumWTexture, WTexture, texname);
		if (strlen( texname) > 0)
			strcpy( DefaultLowerTexture, texname);
		break;
    case 4:
		strcpy( texname, DefaultFloorTexture);
		ChooseFloorTexture( -1, -1, "Choose Floor Texture", NumFTexture, FTexture, texname);
		if (strlen( texname) > 0)
			strcpy( DefaultFloorTexture, texname);
		break;
    case 5:
		strcpy( texname, DefaultCeilingTexture);
		ChooseFloorTexture( -1, -1, "Choose Ceiling Texture", NumFTexture, FTexture, texname);
		if (strlen( texname) > 0)
			strcpy( DefaultCeilingTexture, texname);
		break;
    case 6:
		val = InputIntegerValue( -1, -1, -512, 511, DefaultFloorHeight);
		if (val >= -512)
			DefaultFloorHeight = val;
		break;
    case 7:
		val = InputIntegerValue( -1, -1, -512, 511, DefaultCeilingHeight);
		if (val >= -512)
			DefaultCeilingHeight = val;
		break;
    case 8:
		strcpy( texname, DefaultDoorTrack);
		ChooseWallTexture( -1, -1, "Choose Door Track  ", NumWTexture, WTexture, texname);
		if (strlen( texname) > 0)
			strcpy( DefaultDoorTrack, texname);
		break;
    case 9:
		strcpy( texname, DefaultDoorTexture);
		ChooseWallTexture( -1, -1, "Choose Door Texture", NumWTexture, WTexture, texname);
		if (strlen( texname) > 0)
			strcpy( DefaultDoorTexture, texname);
		break;
    case 10:
		val = InputIntegerValue( -1, -1, 1, 16, CloseToLine);
		if (val >= 1)
			CloseToLine = val;
		break;
    }			   
}

BCINT SelectThingType()
{
	int i;
	thing_class *c;
	thing_type *t;
	char *menuarr[30];
	BCINT dummy[30];
	BCINT val;
	Bool input_integer = FALSE;

	c = Thing_classes;
	for(i = 0; (c && i < 30); i++) {
		menuarr[i] = c->name;
		c = c->next;
	}
	menuarr[i++] = "(Enter A Decimal Value)";

	val = DisplayMenuArray( -1, -1, "Choose A Thing Type:",
						   i, NULL, menuarr, dummy);

	if(val == 0)
		val = -1;

	if(val == i) {
		input_integer = TRUE;
		val = InputIntegerValue(-1, -1, 0, 32767, 0);
	}

	if(val > 0) {
		if(!input_integer) {
			c = Thing_classes;
			while(--val)
				c = c->next;
			/* now c points to the class we are
			   going to display a submenu for */
			
			t = c->types;
			for(i = 0; (t && i < 30); i++) {
				menuarr[i] = t->name;
				t = t->next;
			}
			
			if(i == 1) {
				val = c->types->type;
			}
			else {
				val = -1;
				val = DisplayMenuArray(-1, -1, "Choose A Thing Type:",
									   i, NULL, menuarr, dummy);
			}
		}
		
		if (val > 0) {
			if(!input_integer) {
				t = c->types;
				while(--val)
					t = t->next;
				val = t->type;
			}
		}
		else
			val = -1;
	}
	return val;
}

thing_class *SelectThingClass()
{
	int i;
	thing_class *c;
	char *menuarr[30];
	BCINT dummy[30];
	BCINT val;

	c = Thing_classes;
	for(i = 0; (c && i < 30); i++) {
		menuarr[i] = c->name;
		c = c->next;
	}

	val = DisplayMenuArray( -1, -1, "Choose A Thing Class:",
						   i, NULL, menuarr, dummy);

	if(val == 0)
		return(thing_class *)NULL;

	c = Thing_classes;
	while(--val)
		c = c->next;

	return c;
}

/* end of file */
