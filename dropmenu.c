/*

 *** THIS FILE IS UNDER DEVELOPMENT ***

 I hacked at the code written by Dewi Morgan (D.Morgan@bradford.ac.uk)
 and I have not yet finished it.  Congrats for such a great idea should
 go to Dewi.  Complaints because it's not working should go to me...

 This file is for all the dropdowns, and any menus that may be
 called from them.
*/

#include "deu.h"
#include "wstructs.h"
#include "things.h"


/*
Future plans:
   - Use only one function for all menus, and move it into MENUS.C.
     This file will be deleted.
   - Function declaration: int PullDownMenu( int x0, int y0, ...);
   - The arguments will be the coordinates of the point where the menu
     should be displayed, followed by a list of strings (in pairs),
     followed by a NULL pointer.  Each pair of string will contain the
     message that should be displayed in the menu and the key (int) that
     should be returned.
   - (optional) an empty string in the list should be displayed as a break
     in the menu (horiz. line).
   - the function wil return the key, instead of having to use a pointer.
I don't have the time to write that right now...

-Rapha‰l
*/


void FileMenu( int *key)
{
      char *menustr[3];
      int n;

      for (n = 0; n < 3; n++)
	 menustr[ n] = GetMemory( 20);
      sprintf( menustr[ 0], "  save As...");
      sprintf( menustr[ 1], "X Print...");
      sprintf( menustr[ 2], "  eXit");
      switch (DisplayMenuArray( 18, 19, NULL, 3, /*"APX",*/ menustr))
      {
      case 1:
      case 2:
	 *key='0';
	 break;
      case 3:
	 *key=0x3c00;
	 break;
      case 4:
	 *key='0';
	 break;
      case 5:
	 *key='Q';
	 break;
      }
}

void EditMenu(int *key)
{
/*
 Ideally will also include:
  Drag mode,
  mark/unmark,
  mark/unmark all,
*/
      char *menustr[7];
      int n;

      for (n = 0; n < 7; n++)
	 menustr[ n] = GetMemory( 20);
      sprintf( menustr[ 0], "X cuT    Shift+Del");
      sprintf( menustr[ 1], "X Copy   Ctrl+Ins");
      sprintf( menustr[ 2], "X Paste  Shift+Ins");
      sprintf( menustr[ 3], "X clEar  Del");
      sprintf( menustr[ 4], "  Next object");
      sprintf( menustr[ 5], "  Prev object");
      sprintf( menustr[ 6], "  Jump to...");
      switch (DisplayMenuArray( 66, 19, NULL, 7, /*"TCPENPJ",*/ menustr))
      {
      case 1:
      case 2:
      case 3:
      case 4:
	 *key = '0';
	 break;
      case 5:
	 *key = 'N';
	 break;
      case 6:
	 *key = 'P';
	 break;
      case 7:
	 *key = 'J';
	 break;
      }
}

void SearchMenu(int *key)
{
      char *menustr[2];
      int n;

      for (n = 0; n < 2; n++)
	 menustr[ n] = GetMemory( 20);
      sprintf( menustr[ 0], "X Find/change   F3");
      sprintf( menustr[ 1], "X Repeat last find");
      switch (DisplayMenuArray( 114, 19, NULL, 2, /*"FR",*/ menustr))
      {
      case 1:
      case 2:
	 *key='0';
	 break;
      }
}

void OptionMenu(int *key)
{
/*
 Ideally this would have all the editmodes on a separate submenu.
 It will also contain things like
 scale,
 gridsize, grid on/off,
 move speed,
 mouse off/on,

*/
      switch (DisplayMenu( 178, 19, NULL,
	"Things",
	"Linedefs+Sidedefs",
	"Vertexes",
	"Sectors",
	"Next Mode",
	"Last Mode",
	NULL))
      {
      case 1:
	 *key='T';
	 break;
      case 2:
	 *key='L';
	 break;
      case 3:
	 *key='V';
	 break;
      case 4:
	 *key='S';
	 break;
      case 5:
	 *key=0x0009;
	 break;
      case 6:
	 *key=0x0f00;
	 break;
      }
}


void HelpMenu(int *key)
{
      char *menustr[3];
      int n;

      for (n = 0; n < 3; n++)
	 menustr[ n] = GetMemory( 20);
      sprintf( menustr[ 0], "X Getting started");
      sprintf( menustr[ 1], "  Keyboard");
      if (InfoShown == TRUE)
	 sprintf( menustr[ 2], "û Info Box");
      else
	 sprintf( menustr[ 2], "  Info Box");
      sprintf( menustr[ 3], "X About...");
      switch (DisplayMenuArray( getmaxx() - 180, 19, NULL, 4, /*"GKA",*/ menustr))
      {
      case 1:
	 *key='0';
	 break;
      case 2:
	 *key=0x3b00;
	 break;
      case 3:
	 if (InfoShown==TRUE)
	    InfoShown = FALSE;
	 else
	    InfoShown = TRUE;
	 break;
      case 4:
	 *key='0';
	 break;
      }
}
