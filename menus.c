/*
   Menu utilities by Rapha‰l Quinet <quinet@montefiore.ulg.ac.be>

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...
   Put a credit notice somewhere with my name on it.  Thanks!  ;-)

   MENUS.C - Simple menus for DEU (or other programs).
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
      DrawScreenText( x0 + 10, y0 + 8 + line * 10, "%d - %s", line + 1, text);
   else
      DrawScreenText( x0 + 10, y0 + 8 + line * 10, "%c - %s", line + 56, text);
   if (UseMouse)
      ShowMousePointer();
}



/*
   display and execute a menu
*/

int DisplayMenuArray( int x0, int y0, char *menutitle, int numitems, char *menustr[ 30])
{
   va_list args;
   int     maxlen, line, oldline;
   Bool    ok;
   int     key, buttons, oldbuttons;

   /* compute maxlen */
   if (menutitle)
      maxlen = strlen( menutitle) - 4;
   else
      maxlen = 1;
   for (line = 0; line < numitems; line++)
      if (strlen( menustr[ line]) > maxlen)
	 maxlen = strlen( menustr[ line]);

   /* display the menu */
   if (UseMouse)
      HideMousePointer();
   if (x0 < 0)
      x0 = (getmaxx() - maxlen * 8 - 53) / 2;
   if (y0 < 0)
      y0 = (getmaxy() - numitems * 10 - (menutitle ? 28 : 12)) / 2;
   DrawScreenBox3D( x0, y0, x0 + maxlen * 8 + 53, y0 + numitems * 10 + (menutitle ? 28 : 12));
   SetColor( YELLOW);
   if (menutitle)
      DrawScreenText( x0 + 10, y0 + 8, menutitle);
   if (UseMouse)
      ShowMousePointer();
   SetColor( BLACK);
   for (line = 0; line < numitems; line++)
      DisplayMenuText( x0, y0 + (menutitle ? 16 : 0), line, menustr[ line]);

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
	    line = (PointerY - y0 - (menutitle ? 24 : 8)) / 10;
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
	 /* up arrow = select previous line */
	 else if ((key & 0xFF00) == 0x4800)
	 {
	    if (line > 0)
	       line--;
	    else
	       line = numitems - 1;
	 }
	 /* down arrow = select next line */
	 else if ((key & 0xFF00) == 0x5000)
	 {
	    if (line < numitems - 1)
	       line++;
	    else
	       line = 0;
	 }
	 /* other key */
	 else
	    Beep();
      }
      if (line != oldline)
      {
	 if (oldline >= 0 && oldline < numitems)
	 {
	    SetColor( BLACK);
	    DisplayMenuText( x0, y0 + (menutitle ? 16 : 0), oldline, menustr[oldline]);
	 }
	 if (line >= 0 && line < numitems)
	 {
	    SetColor( WHITE);
	    DisplayMenuText( x0, y0 + (menutitle ? 16 : 0), line, menustr[line]);
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
   int     num;
   char   *menustr[ 30];

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
   display the integer input box
*/

int InputInteger( int x0, int y0, int *valp, int minv, int maxv)
{
   int  key, val;
   Bool neg, ok, firstkey;

   DrawScreenBoxHollow( x0, y0, x0 + 61, y0 + 13);
   neg = (*valp < 0);
   val = neg ? -(*valp) : *valp;
   firstkey = TRUE;
   for (;;)
   {
      ok = (neg ? -val : val) >= minv && (neg ? -val : val) <= maxv;
      SetColor( BLACK);
      DrawScreenBox( x0 + 1, y0 + 1, x0 + 60, y0 + 12);
      if (ok)
	 SetColor( WHITE);
      else
	 SetColor( LIGHTGRAY);
      if (neg)
	 DrawScreenText( x0 + 3, y0 + 3, "-%d", val);
      else
	 DrawScreenText( x0 + 3, y0 + 3, "%d", val);
      key = bioskey( 0);
      if (firstkey && (key & 0x00FF) > ' ')
      {
	 val = 0;
	 neg = FALSE;
      }
      firstkey = FALSE;
      if (val < 3275 && (key & 0x00FF) >= '0' && (key & 0x00FF) <= '9')
	 val = val * 10 + (key & 0x00FF) - '0';
      else if (val > 0 && (key & 0x00FF) == 0x0008)
	 val = val / 10;
      else if (neg && (key & 0x00FF) == 0x0008)
	 neg = FALSE;
      else if ((key & 0x00FF) == '-')
	 neg = !neg;
      else if (ok && (key & 0x00FF) == 0x000D)
	 break; /* return "val" */
      else if ((key & 0xFF00) == 0x4800 || (key & 0xFF00) == 0x5000 ||
	       (key & 0xFF00) == 0x4B00 || (key & 0xFF00) == 0x4D00 ||
	       (key & 0x00FF) == 0x0009 || (key & 0xFF00) == 0x0F00)
	 break; /* return "val", even if not valid */
      else if ((key & 0x00FF) == 0x001B)
      {
	 val = -32768; /* return a value out of range */
	 break;
      }
      else
	 Beep();
   }
   if (neg)
      *valp = -val;
   else
      *valp = val;
   return key;
}



/*
   ask for an integer value and check for minimum and maximum
*/

int InputIntegerValue( int x0, int y0, int minv, int maxv, int defv)
{
   int  val, key;
   char prompt[ 80];

   if (UseMouse)
      HideMousePointer();
   sprintf( prompt, "Enter a decimal number between %d and %d:", minv, maxv);
   if (x0 < 0)
      x0 = (getmaxx() - 25 - 8 * strlen( prompt)) / 2;
   if (y0 < 0)
      y0 = (getmaxy() - 55) / 2;
   DrawScreenBox3D( x0, y0, x0 + 25 + 8 * strlen( prompt), y0 + 55);
   SetColor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, prompt);
   val = defv;
   while (((key = InputInteger( x0 + 10, y0 + 28, &val, minv, maxv)) & 0x00FF) != 0x000D && (key & 0x00FF) != 0x001B)
      Beep();
   if (UseMouse)
      ShowMousePointer();
   return val;
}



/*
   ask for a name in a given list and call a function (for displaying objects, etc.)
*/

void InputNameFromListWithFunc( int x0, int y0, char *prompt, int listsize, char **list, int listdisp, char *name, int width, int height, void (*hookfunc)(int x1, int y1, char *name))
{
   int  key, n, l;
   int  x1, y1, x2, y2;
   int  maxlen;
   Bool ok, firstkey;

   /* compute maxlen */
   maxlen = 1;
   for (n = 0; n < listsize; n++)
      if (strlen( list[ n]) > maxlen)
	 maxlen = strlen( list[ n]);
   for (n = strlen(name) + 1; n <= maxlen; n++)
      name[ n] = '\0';
   /* compute the minimum width of the dialog box */
   l = maxlen;
   if (strlen( prompt) > l + 12)
      l = strlen( prompt) - 12;
   l = l * 8 + 110;
   x1 = l + 3;
   y1 = 10 + 1;
   if (width > 0)
      l += 10 + width;
   if (height > 65)
      n = height + 20;
   else
      n = 85;
   if (x0 < 0)
      x0 = (getmaxx() - l) / 2;
   if (y0 < 0)
      y0 = (getmaxy() - n) / 2;
   x1 += x0;
   y1 += y0;
   if (x1 + width - 1 < getmaxx())
      x2 = x1 + width - 1;
   else
      x2 = getmaxx();
   if (y1 + height - 1 < getmaxy())
      y2 = y1 + height - 1;
   else
      y2 = getmaxy();
   /* draw the dialog box */
   DrawScreenBox3D( x0, y0, x0 + l, y0 + n);
   DrawScreenBoxHollow( x0 + 10, y0 + 28, x0 + 101, y0 + 41);
   DrawScreenText( x0 + 10, y0 + 8, prompt);
   if (width > 0)
   {
      DrawScreenBox( x1, y1, x2 + 1, y2 + 1);
      SetColor( DARKGRAY);
      DrawScreenBox( x1 - 1, y1 - 1, x2, y2);
   }
   firstkey = TRUE;
   for (;;)
   {
      /* test if "name" is in the list */
      for (n = 0; n < listsize; n++)
	 if (strcmp( name, list[ n]) <= 0)
	    break;
      ok = n < listsize ? !strcmp( name, list[ n]) : FALSE;
      if (n > listsize - 1)
	 n = listsize - 1;
      /* display the "listdisp" next items in the list */
      SetColor( LIGHTGRAY);
      DrawScreenBox( x0 + 110, y0 + 30, x0 + 110 + 8 * maxlen, y0 + 30 + 10 * listdisp);
      SetColor( BLACK);
      for (l = 0; l < listdisp && n + l < listsize; l++)
	 DrawScreenText( x0 + 110, y0 + 30 + l * 10, list[ n + l]);
      l = strlen( name);
      DrawScreenBox( x0 + 11, y0 + 29, x0 + 100, y0 + 40);
      if (ok)
	 SetColor( WHITE);
      else
	 SetColor( LIGHTGRAY);
      DrawScreenText( x0 + 13, y0 + 31, name);
      if (hookfunc)
      {
	 setviewport( x1, y1, x2, y2, TRUE);
	 clearviewport();
	 hookfunc( 0, 0, name);
	 setviewport( 0, 0, getmaxx(), getmaxy(), TRUE);
      }
      key = bioskey( 0);
      if (firstkey && (key & 0x00FF) > ' ')
      {
	 for (l = 0; l <= maxlen; l++)
	    name[ l] = '\0';
	 l = 0;
      }
      firstkey = FALSE;
      if (l < maxlen && (key & 0x00FF) >= 'a' && (key & 0x00FF) <= 'z')
      {
	 name[ l] = key & 0x00FF + 'A' - 'a';
	 name[ l + 1] = '\0';
      }
      else if (l < maxlen && (key & 0x00FF) > ' ')
      {
	 name[ l] = key & 0x00FF;
	 name[ l + 1] = '\0';
      }
      else if (l > 0 && (key & 0x00FF) == 0x0008)
	 name[ l - 1] = '\0';
      else if (n < listsize - 1 && (key & 0xFF00) == 0x5000)
	 strcpy(name, list[ n + 1]);
      else if (n > 0 && (key & 0xFF00) == 0x4800)
	 strcpy(name, list[ n - 1]);
      else if (n < listsize - listdisp && (key & 0xFF00) == 0x5100)
	 strcpy(name, list[ n + listdisp]);
      else if (n > 0 && (key & 0xFF00) == 0x4900)
      {
	 if (n > listdisp)
	    strcpy(name, list[ n - listdisp]);
	 else
	    strcpy(name, list[ 0]);
      }
      else if ((key & 0xFF00) == 0x4F00)
	 strcpy(name, list[ listsize - 1]);
      else if ((key & 0xFF00) == 0x4700)
	 strcpy(name, list[ 0]);
      else if ((key & 0x00FF) == 0x0009)
	 strcpy(name, list[ n]);
      else if (ok && (key & 0x00FF) == 0x000D)
	 break; /* return "name" */
      else if ((key & 0x00FF) == 0x001B)
      {
	 name[ 0] = '\0'; /* return an empty string */
	 break;
      }
      else
	 Beep();
   }
}



/*
   ask for a name in a given list
*/

void InputNameFromList( int x0, int y0, char *prompt, int listsize, char **list, char *name)
{
   if (UseMouse)
      HideMousePointer();
   InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 5, name, 0, 0, NULL);
   if (UseMouse)
      ShowMousePointer();
}



/*
   ask for a filename
*/

void InputFileName( int x0, int y0, char *prompt, int maxlen, char *filename)
{
   int   key, l, boxlen;
   Bool  ok, firstkey;
   char *p;

   if (UseMouse)
      HideMousePointer();
   for (l = strlen(filename) + 1; l <= maxlen; l++)
      filename[ l] = '\0';
   /* compute the width of the input box */
   if (maxlen > 20)
      boxlen = 20;
   else
      boxlen = maxlen;
   /* compute the width of the dialog box */
   if (strlen( prompt) > boxlen)
      l = strlen( prompt);
   else
      l = boxlen;
   if (x0 < 0)
      x0 = (getmaxx() - 26 - 8 * l) / 2;
   if (y0 < 0)
      y0 = (getmaxy() - 50) / 2;
   /* draw the dialog box */
   DrawScreenBox3D( x0, y0, x0 + 26 + 8 * l, y0 + 50);
   DrawScreenBoxHollow( x0 + 10, y0 + 28, x0 + 15 + 8 * boxlen, y0 + 41);
   DrawScreenText( x0 + 10, y0 + 8, prompt);
   firstkey = TRUE;
   for (;;)
   {
      /* check that "filename" looks like a valid file name */
      ok = TRUE;
      if (filename[ 1] == ':')
	 p = filename + 2;
      else
	 p = filename;
      for (l = 8; *p; p++)
      {
	 if (*p == '.')
	    l = 3;
	 else if (*p == '\\')
	    l = 8;
	 else
	    l--;
	 if (l < 0)
	 {
	    ok = FALSE;
	    break;
	 }
      }

      l = strlen( filename);
      SetColor( BLACK);
      DrawScreenBox( x0 + 11, y0 + 29, x0 + 14 + 8 * boxlen, y0 + 40);
      if (ok)
	 SetColor( WHITE);
      else
	 SetColor( LIGHTGRAY);
      if (l > boxlen)
      {
	 DrawScreenText( x0 + 11, y0 + 31, "<");
	 DrawScreenText( x0 + 13, y0 + 31, "<%s", filename + (l - boxlen + 1));
      }
      else
	 DrawScreenText( x0 + 13, y0 + 31, filename);
      key = bioskey( 0);
      if (firstkey && (key & 0x00FF) > ' ')
      {
	 for (l = 0; l <= maxlen; l++)
	    filename[ l] = '\0';
	 l = 0;
      }
      firstkey = FALSE;
      if (l < maxlen && (key & 0x00FF) >= 'a' && (key & 0x00FF) <= 'z')
      {
	 filename[ l] = key & 0x00FF + 'A' - 'a';
	 filename[ l + 1] = '\0';
      }
      else if (l < maxlen && (key & 0x00FF) > ' ')
      {
	 filename[ l] = key & 0x00FF;
	 filename[ l + 1] = '\0';
      }
      else if (l > 0 && (key & 0x00FF) == 0x0008)
	 filename[ l - 1] = '\0';
      else if (ok && (key & 0x00FF) == 0x000D)
	 break; /* return "filename" */
      else if ((key & 0x00FF) == 0x001B)
      {
	 filename[ 0] = '\0'; /* return an empty string */
	 break;
      }
      else
	 Beep();
   }
   if (UseMouse)
      ShowMousePointer();
}



/*
   ask for confirmation (prompt2 may be NULL)
*/
Bool Confirm( int x0, int y0, char *prompt1, char *prompt2)
{
   int key;
   int maxlen = 46;

   if (UseMouse)
      HideMousePointer();
   if (strlen( prompt1) > maxlen)
      maxlen = strlen( prompt1);
   if (prompt2 != NULL && strlen( prompt2) > maxlen)
      maxlen = strlen( prompt2);
   if (x0 < 0)
      x0 = (getmaxx() - 22 - 8 * maxlen) / 2;
   if (y0 < 0)
      y0 = (getmaxy() - (prompt2 ? 53 : 43)) / 2;
   DrawScreenBox3D( x0, y0, x0 + 22 + 8 * maxlen, y0 + (prompt2 ? 53 : 43));
   SetColor( WHITE);
   DrawScreenText( x0 + 10, y0 + 8, prompt1);
   if (prompt2 != NULL)
      DrawScreenText( x0 + 10, y0 + 18, prompt2);
   SetColor( YELLOW);
   DrawScreenText( x0 + 10, y0 + (prompt2 ? 38 : 28), "Press Y to confirm, or any other key to cancel");
   key = bioskey( 0);
   if (UseMouse)
      ShowMousePointer();
   return ((key & 0x00FF) == 'Y' || (key & 0x00FF) == 'y');
}



/*
   clear the screen and display a message
*/
void DisplayMessage( int x0, int y0, char *msg, ...)
{
   char prompt[ 120];
   va_list args;

   va_start( args, msg);
   vsprintf( prompt, msg, args);
   va_end( args);

   if (UseMouse)
      HideMousePointer();
   ClearScreen();
   if (x0 < 0)
      x0 = (getmaxx() - 40 - 8 * strlen( prompt)) / 2;
   if (y0 < 0)
      y0 = (getmaxy() - 40) / 2;
   DrawScreenBox3D( x0, y0, x0 + 40 + 8 * strlen( prompt), y0 + 40);
   DrawScreenText( x0 + 20, y0 + 17, prompt);
   if (UseMouse)
      ShowMousePointer();
}



/*
   let's make the user angry...
*/
void NotImplemented()
{
   if (UseMouse)
      HideMousePointer();
   DrawScreenBox3D( 140, 220, 500, 260);
   SetColor( RED);
   DrawScreenText( 150, 230, "This function is not implemented... Yet!");
   SetColor( YELLOW);
   DrawScreenText( 150, 245, "Press any key to return to the editor...");
   Beep();
   bioskey( 0);
   if (UseMouse)
      ShowMousePointer();
}


/* end of file */
