/*
   Menu utilities by Rapha‰l Quinet <quinet@montefiore.ulg.ac.be>

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...
   Put a credit notice somewhere with my name on it.  Thanks!  ;-)

   MENUS.C - Simple menus for DEU.
*/

/* the includes */
#include "deu.h"


/*
   draws a line of text in a menu
*/
void DisplayMenuText( int x0, int y0, int line, char *text)
{
   if (UseMouse)
      HideMousePointer();
   if (line < 9)
     DrawScreenText( x0 + 10, y0 + 24 + line * 10, "%d - %s", line + 1, text);
   else
     DrawScreenText( x0 + 10, y0 + 24 + line * 10, "%c - %s", line + 56, text);
   if (UseMouse)
      ShowMousePointer();
}



/*
   display and execute a menu
*/
int DisplayMenuArray( int x0, int y0, char *menutitle, int numitems, char *menustr[ 30])
{
   va_list args;
   int maxlen, line, oldline, ok;
   int key, buttons, oldbuttons;

   /* compute maxlen */
   maxlen = strlen( menutitle) - 4;
   for (line = 0; line < numitems; line++)
      if (strlen( menustr[ line]) > maxlen)
	 maxlen = strlen( menustr[ line]);

   /* display the menu */
   if (UseMouse)
      HideMousePointer();
   DrawScreenBox3D( x0, y0, x0 + maxlen * 8 + 53, y0 + numitems * 10 + 28);
   setcolor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, menutitle);
   if (UseMouse)
      ShowMousePointer();
   setcolor( BLACK);
   for (line = 0; line < numitems; line++)
      DisplayMenuText( x0, y0, line, menustr[ line]);

   oldline = -1;
   line = 0;
   oldbuttons = 0x0000;
   ok = FALSE;

   while (! ok)
   {
      if (UseMouse)
      {
	 GetMouseCoords( &PointerX, &PointerY, &buttons);
	 /* right button = cancel */
	 if ((buttons & 0x0002) == 0x0002)
	 {
	   line = -1;
	   ok = TRUE;
	 }
	 /* left button = select */
	 else if (buttons == 0x0001 && PointerX >= x0 && PointerX <= x0 + maxlen * 8 + 53)
	   line = (PointerY - y0 - 24) / 10;
	 /* release left button = accept selection */
	 else if (buttons == 0x0000 && oldbuttons == 0x0001)
	   ok = TRUE;
	 oldbuttons = buttons;
      }
      if (bioskey( 1))
      {
	 key = bioskey( 0);

	 /* enter key = accept selection */
	 if ((key & 0x00FF) == 0x000D)
	    ok = TRUE;
	 /* escape key = cancel */
	 else if ((key & 0x00FF) == 0x001B)
	 {
	   line = -1;
	   ok = TRUE;
	 }
	 /* number or alphabetic key = enter selection */
	 else if ((key & 0x00FF) >= '1' && (key & 0x00FF) <= '9' && ((key & 0x00FF) - '1') < numitems)
	 {
	   line = (key & 0x00FF) - '1';
	   ok = TRUE;
	 }
	 else if ((key & 0x00FF) >= 'A' && (key & 0x00FF) <= 'Z' && ((key & 0x00FF) - 'A' + 9) < numitems)
	 {
	   line = (key & 0x00FF) - 'A' + 9;
	   ok = TRUE;
	 }
	 else if ((key & 0x00FF) >= 'a' && (key & 0x00FF) <= 'z' && ((key & 0x00FF) - 'a' + 9) < numitems)
	 {
	   line = (key & 0x00FF) - 'a' + 9;
	   ok = TRUE;
	 }
	 /* down arrow = select next line */
	 else if ((key & 0xFF00) == 0x4800 && (line > 0))
	    line--;
	 /* up arrow = select previous line */
	 else if ((key & 0xFF00) == 0x5000 && (line < numitems))
	    line++;
	 /* other key */
	 else
	    Beep();
      }
      if (line != oldline)
      {
	 if (oldline >= 0 && oldline < numitems)
	 {
	    setcolor( BLACK);
	    DisplayMenuText( x0, y0, oldline, menustr[oldline]);
	 }
	 if (line >= 0 && line < numitems)
	 {
	    setcolor( YELLOW);
	    DisplayMenuText( x0, y0, line, menustr[line]);
	 }
	 oldline = line;
      }
   }
   if (line >= 0 && line < numitems)
      return (line + 1);
   else
      return 0;
}



/*
   display and execute a menu defined with variable arguments
*/
int DisplayMenu( int x0, int y0, char *menutitle, ...)
{
   va_list args;
   int num;
   char *menustr[ 30];

   /* put the va_args in the menustr table */
   num = 0;
   va_start( args, menutitle);
   while ((num < 30) && ((menustr[ num] = va_arg( args, char *)) != NULL))
      num++;
   va_end( args);

   /* display the menu */
   return DisplayMenuArray( x0, y0, menutitle, num, menustr);
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
   while ((num < 30) && ((thingid[ num] = va_arg( args, int)) != 0))
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
   ask for an integer value and check for minimum and maximum
*/
int InputIntegerValue( int x0, int y0, int minv, int maxv, int defv)
{
   int key, val, neg;

   if (UseMouse)
      HideMousePointer();
   DrawScreenBox3D( x0, y0, x0 + 380, y0 + 55);
   setcolor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, "Enter a decimal number between %d and %d:", minv, maxv);
   DrawScreenBox( x0 + 11, y0 + 29, x0 + 71, y0 + 41);
   setcolor( DARKGRAY);
   DrawScreenBox( x0 + 10, y0 + 28, x0 + 70, y0 + 40);
   neg = (defv < 0);
   val = neg ? -defv : defv;
   for (;;)
   {
     setcolor( BLACK);
     DrawScreenBox( x0 + 11, y0 + 29, x0 + 70, y0 + 40);
     setcolor( WHITE);
     if (neg)
	DrawScreenText( x0 + 13, y0 + 31, "-%d", val);
     else
	DrawScreenText( x0 + 13, y0 + 31, "%d", val);
     key = bioskey( 0);
     if (val < 3275 && (key & 0x00FF) >= '0' && (key & 0x00FF) <= '9')
	val = val * 10 + (key & 0x00FF) - '0';
     else if (val > 0 && (key & 0x00FF) == 0x0008)
	val = val / 10;
     else if ((key & 0x00FF) == '-')
	neg = !neg;
     else if ((neg ? -val : val) >= minv && (neg ? -val : val) <= maxv && (key & 0x00FF) == 0x000D)
	break;
     else if ((key & 0x00FF) == 0x001B)
     {
	val = -32768;
	break;
     }
     else
	Beep();
   }
   if (UseMouse)
      ShowMousePointer();
   return neg ? -val : val;
}



/*
   let's make the user angry...
*/
void NotImplemented()
{
   if (UseMouse)
      HideMousePointer();
   DrawScreenBox3D( 140, 220, 500, 260);
   setcolor( RED);
   DrawScreenText( 150, 230, "This function is not implemented... Yet!");
   setcolor( YELLOW);
   DrawScreenText( 150, 245, "Press any key to return to the editor...");
   Beep();
   bioskey( 0);
   if (UseMouse)
      ShowMousePointer();
}


/* end of file */
