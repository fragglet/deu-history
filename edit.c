/*
   Doom Editor Utility, by Brendon Wyber. Use and Abuse!

   EDIT.C - Editor routines.
*/

/* the includes */
#include "deu.h"

/* the global data */
MDirPtr Level = NULL;         /* master dictionary entry for the level */
int NumVertex = 0;            /* number of vertexes */
int *VertexX = NULL;          /* x position of vertex */
int *VertexY = NULL;          /* y position of vertex */
int MaxX = 0;                 /* maximum X value of map */
int MaxY = 0;                 /* maximum Y value of map */
int MinX = 0;                 /* minimum X value of map */
int MinY = 0;                 /* minimum Y value of map */
int NumLineDefs = 0;          /* number of line defs */
int *LineStart = NULL;        /* starting vertex of linedef */
int *LineEnd = NULL;          /* endvertex of linedef */
int NumThings = 0;            /* the number of things */
TPtr ThingList = NULL;        /* the actual things */
TPtr LastThing = NULL;        /* the last thing */
TPtr CurThing = NULL;         /* the current thing */
int DefType = THING_TROOPER;  /* the default thing type */
int DefAngle = 270;           /* the default thing angle */
int DefWhen = 0x07;           /* the default thing appears when */
int MoveSpeed = 20;           /* movement speed */
int SaveChanges = 0;          /* save changes */
int MadeChanges = 0;          /* made changes changes */



/*
   the driving program
*/

void EditLevel( int game, int level, char *dest)
{
   char *outfile = GetMemory( strlen( dest) + 2);
   strcpy( outfile, dest);
   ReadLevelData( game, level);
   InitGfx();
   EditorLoop();
   TermGfx();
   if (SaveChanges == 0)
      printf( "Editing aborted...\n");
   else if (MadeChanges == 0)
      printf( "No changes made, file \"%s\" not saved.\n", outfile);
   else
      SaveLevelData( outfile);
   ForgetLevelData();
   if (SaveChanges && MadeChanges)
      OpenPatchWad( outfile);
}



/*
   read in the level data
*/

void ReadLevelData( int game, int level)
{
   MDirPtr Things, Vertexes, LineDefs;
   char name[ 5];
   int n;
   int val;
   int junk[ 5];
   TPtr new;

   /* find the various level information from the master dictionary */
   name[ 0] = 'E';
   name[ 1] = '0' + game;
   name[ 2] = 'M';
   name[ 3] = '0' + level;
   name[ 4] = '\0';
   printf( "Reading data for level %s.\n", name);
   Level = FindMasterDir( MasterDir, name);
   if (!Level)
      ProgError( "level data not found");
   Things = FindMasterDir( Level, "THINGS");
   LineDefs = FindMasterDir( Level, "LINEDEFS");
   Vertexes = FindMasterDir( Level, "VERTEXES");

   /* read in the vertexes which are all the corners of the level */
   NumVertex = Vertexes->dir.size / 4;
   VertexX = GetMemory( NumVertex * 2);
   VertexY = GetMemory( NumVertex * 2);
   BasicWadSeek( Vertexes->wadfile, Vertexes->dir.start);
   for (n = 0; n < NumVertex; n++)
   {
      BasicWadRead( Vertexes->wadfile, &val, 2);
      if (val < MinX)
         MinX = val;
      if (val > MaxX)
         MaxX = val;
      VertexX[ n] = val;
      BasicWadRead( Vertexes->wadfile, &val, 2);
      if (val < MinY)
         MinY = val;
      if (val > MaxY)
         MaxY = val;
      VertexY[ n] = val;
   }

   /* read in the line def information */
   NumLineDefs = LineDefs->dir.size / 14;
   LineStart = GetMemory( NumLineDefs * 2);
   LineEnd = GetMemory( NumLineDefs * 2);
   BasicWadSeek( LineDefs->wadfile, LineDefs->dir.start);
   for (n = 0; n < NumLineDefs; n++)
   {
      BasicWadRead( Vertexes->wadfile, &(LineStart[ n]), 2);
      BasicWadRead( Vertexes->wadfile, &(LineEnd[ n]), 2);
      BasicWadRead( Vertexes->wadfile, &junk, 10);  /* skip this */
   }

   /* read in the things and make a thing list */
   NumThings = Things->dir.size / 10;
   BasicWadSeek( Things->wadfile, Things->dir.start);
   for (n = 0; n < NumThings; n++)
   {
      new = GetMemory( sizeof( struct Thing));
      new->next = NULL;
      if (ThingList)
         LastThing->next = new;
      else
         ThingList= new;
      LastThing = new;
      BasicWadRead( Things->wadfile, &(new->xpos), 2);
      BasicWadRead( Things->wadfile, &(new->ypos), 2);
      BasicWadRead( Things->wadfile, &(new->angle), 2);
      BasicWadRead( Things->wadfile, &(new->type), 2);
      BasicWadRead( Things->wadfile, &(new->when), 2);
   }
}



/*
   forget the level data
*/

void ForgetLevelData()
{
   TPtr next, cur;

   /* forget the level pointer */
   Level = NULL;

   /* forget the vertexes */
   free( VertexX);
   free( VertexY);
   *VertexX = NULL;
   *VertexY = NULL;
   NumVertex = 0;
   MaxX = 0;
   MaxY = 0;
   MinX = 0;
   MinY = 0;

   /* forget the line defs */
   NumLineDefs = 0;
   free( LineStart);
   free( LineEnd);
   *LineStart = NULL;
   *LineEnd = NULL;

   /* forget the things */
   cur = ThingList;
   while (cur)
   {
      next = cur->next;
      free( cur);
      cur = next;
   }
   NumThings = 0;
   ThingList = NULL;
   LastThing = NULL;
}



/*
   save the level data to a PWAD file
*/

void SaveLevelData( char *outfile)
{
   FILE *file;
   MDirPtr dir;
   long counter = 11;
   int n;
   void *data;
   long size;
   long dirstart;
   TPtr thing;

   /* open the file */
   printf( "Saving data to \"%s\"...\n", outfile);
   if ((file = fopen( outfile, "wb")) == NULL)
      ProgError( "Unable to open file \"%s\"", outfile);
   WriteBytes( file, "PWAD", 4L);     /* PWAD file */
   WriteBytes( file, &counter, 4L);   /* 11 entries */
   WriteBytes( file, &counter, 4L);   /* fix this up later */
   counter = 12;

   /* output the things data */
   for (thing = ThingList; thing; thing = thing->next)
   {
      WriteBytes( file, &(thing->xpos), 2L);
      WriteBytes( file, &(thing->ypos), 2L);
      WriteBytes( file, &(thing->angle), 2L);
      WriteBytes( file, &(thing->type), 2L);
      WriteBytes( file, &(thing->when), 2L);
      counter += 10;
   }

   /* save the other 9 directory entries with data */
   data = GetMemory( 0x8000 + 2);
   for (n = 0, dir = Level->next->next; n < 9; n++, dir = dir->next)
   {
      size = dir->dir.size;
      counter += size;
      BasicWadSeek( dir->wadfile, dir->dir.start);
      while (size > 0x8000)
      {
         BasicWadRead( dir->wadfile, data, 0x8000);
         WriteBytes( file, data, 0x8000);
         size -= 0x8000;
      }
      BasicWadRead( dir->wadfile, data, size);
      WriteBytes( file, data, size);
   }

   /* output the actual directory */
   dirstart = counter;
   counter = 12;
   size = 0;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, &(Level->dir.name), 8L);
   size = NumThings * 10;
   WriteBytes( file, &counter, 4L);
   WriteBytes( file, &size, 4L);
   WriteBytes( file, "THINGS\0\0", 8L);
   counter += size;
   for (n = 0, dir = Level->next->next; n < 9; n++, dir = dir->next)
   {
      size = dir->dir.size;
      WriteBytes( file, &counter, 4L);
      WriteBytes( file, &size, 4L);
      WriteBytes( file, &(dir->dir.name), 8L);
      counter += size;
   }

   /* fix up the directory start information */
   if (fseek( file, 8L, 0))
      ProgError( "error writing to file");
   WriteBytes( file, &dirstart, 4L);

   /* close the file */
   fclose( file);
}



/*
   the editor main loop
*/

void EditorLoop()
{
   int key;

   OrigX = (MinX + MaxX) / 2;
   OrigY = (MinY + MaxY) / 2;
   Scale = 10;
   SaveChanges = MadeChanges = 0;

   for (;;)
   {
      /* draw the map and get user input */
      DrawMap();
      key = bioskey( 0);

      /* user wants to exit */
      if ((key & 0x00FF) == 'E' || (key & 0x00FF) == 'e')
      {
         SaveChanges = 1;
         break;
      }
      else if ((key & 0x00FF) == 'Q' || (key & 0x00FF) == 'q')
      {
         setcolor( YELLOW);
         DrawScreenBox( 0, 0, 639, 11);
         setcolor( BLACK);
         DrawScreenText( 5, 2, "Really abandon edit? (Y to confirm, any other key aborts)");
         key = bioskey( 0);
         if ((key & 0x00FF) == 'Y' || (key & 0x00FF) == 'y')
            break;
      }

      /* edit the current or default thing */
      else if ((key & 0x00FF) == 'T' || (key & 0x00FF) == 't')
      {
         if (CurThing)
         {
            int type  = CurThing->type;
            int angle = CurThing->angle;
            int when  = CurThing->when;
            EditThingInfo( &type, &angle, &when);
            if (type != CurThing->type)
            {
               CurThing->type = type;
               MadeChanges = 1;
            }
            if (when != CurThing->when)
            {
               CurThing->when = when;
               MadeChanges = 1;
            }
            if (angle != CurThing->angle)
            {
               CurThing->angle = angle;
               MadeChanges = 1;
            }
         }
         else
            EditThingInfo( &DefType, &DefAngle, &DefWhen);
      }

      /* user wants to delete the select object */
      else if ((key & 0xFF00) == 0x5300 && CurThing)
      {
         TPtr cur, list;
         NumThings--;
         MadeChanges = 1;
         list = ThingList;
         ThingList = LastThing = NULL;
         while( list)                     /* rebuild the thing list */
         {
            cur = list;
            list = list->next;
            if (cur == CurThing)
               continue;                  /* skipping the current object */
            cur->next = NULL;
            if (ThingList)
               LastThing->next = cur;
            else
               ThingList= cur;
            LastThing = cur;
         }
         free( CurThing);
      }

      /* user wants to insert a default object */
      else if ((key & 0xFF00) == 0x5200 && CurThing == NULL)
      {
         TPtr cur = GetMemory( sizeof( struct Thing));
         NumThings++;
         MadeChanges = 1;
         cur->xpos  = OrigX;
         cur->ypos  = OrigY;
         cur->type  = DefType;
         cur->angle = DefAngle;
         cur->when  = DefWhen;
         cur->next = NULL;
         if (ThingList)
            LastThing->next = cur;
         else
            ThingList= cur;
         LastThing = cur;
      }

      /* user wants to change the scale */
      else if (((key & 0x00FF) == '+' || (key & 0x00FF) == '=') && Scale > 1)
         Scale--;
      else if (((key & 0x00FF) == '-' || (key & 0x00FF) == '_') && Scale < 20)
         Scale++;

      /* user wants to move */
      else if ((key & 0xFF00) == 0x5000 && OrigY > MinY)
         OrigY -= MoveSpeed * Scale;
      else if ((key & 0xFF00) == 0x4800 && OrigY < MaxY)
         OrigY += MoveSpeed * Scale;
      else if ((key & 0xFF00) == 0x4b00 && OrigX > MinX)
         OrigX -= MoveSpeed * Scale;
      else if ((key & 0xFF00) == 0x4d00 && OrigX < MaxX)
         OrigX += MoveSpeed * Scale;

      /* user wants to change the movement speed */
      else if ((key & 0x00FF) == ' ')
         MoveSpeed = MoveSpeed == 1 ? 20 : MoveSpeed == 20 ? 100 : 1;

      /* user likes music */
      else
      {
         sound( 640);
         delay( 100);
         nosound();
      }
   }
}



/*
   draw the actual game map
*/

void DrawMap()
{
   int n;
   TPtr cur;

   /* clear the screen */
   ClearScreen();
   /* draw the linedefs to form the map */
   setcolor( WHITE);
   for (n = 0; n < NumLineDefs; n++)
      DrawMapLine( VertexX[ LineStart[ n]], VertexY[ LineStart[ n]], VertexX[ LineEnd[ n]], VertexY[ LineEnd[ n]]);

   /* draw in the things */
   CurThing = NULL;
   for (cur = ThingList; cur; cur = cur->next)
   {
      /* draw on the thing */
      setcolor( GetThingColour( cur->type));
      DrawMapLine( cur->xpos - 5, cur->ypos, cur->xpos + 5, cur->ypos);
      DrawMapLine( cur->xpos, cur->ypos - 5, cur->xpos, cur->ypos + 5);

      /* check if thing is on cross hairs */
      if (CurThing == NULL && cur->xpos > OrigX - 5 && cur->xpos < OrigX + 5 && cur->ypos > OrigY - 5 && cur->ypos < OrigY + 5)
         CurThing = cur;
   }

   /* draw in the cross hairs */
   setcolor( YELLOW);
   DrawScreenLine( 303, 228, 335, 252);
   DrawScreenLine( 303, 252, 335, 228);

   /* draw in the title bar */
   setcolor( LIGHTRED);
   DrawScreenBox( 0, 0, 639, 12);
   setcolor( BLACK);
   DrawScreenText( 5, 2, "Editing Things on %s", Level->dir.name);

   /* put in the instructions */
   setcolor( YELLOW);
   DrawScreenBox( 0, 408, 340, 479);
   setcolor( BLACK);
   DrawScreenText( 5, 410, "Cursor keys to move around");
   DrawScreenText( 5, 420, "Space to change move speed (%s->%s)", MoveSpeed == 1 ? "Slow" : MoveSpeed == 20 ? "Medium" : "Fast", MoveSpeed == 100 ? "Slow" : MoveSpeed == 1 ? "Medium" : "Fast");
   DrawScreenText( 5, 430, "Q to Quit without saving changes");
   DrawScreenText( 5, 440, "E to Exit, saving changes");
   DrawScreenText( 5, 450, "+/- to change the map scale");
   if (CurThing)
   {
      DrawScreenText( 5, 460, "Del to delete the %s", GetThingName( CurThing->type));
      DrawScreenText( 5, 470, "T to edit the %s data", GetThingName( CurThing->type));
   }
   else
   {
      DrawScreenText( 5, 460, "Ins to insert a %s", GetThingName( DefType));
      DrawScreenText( 5, 470, "T to edit the default thing", GetThingName( DefType));
   }

   /* display the thing information */
   setcolor( CYAN);
   DrawScreenBox( 450, 438, 639, 479);
   setcolor( BLACK);
   if (CurThing)
   {
      DrawScreenText( 453, 440, "THING at (%d, %d)", CurThing->xpos, CurThing->ypos);
      DrawScreenText( 453, 450, "Type:  %s", GetThingName( CurThing->type));
      DrawScreenText( 453, 460, "Angle: %s", GetAngleName( CurThing->angle));
      DrawScreenText( 453, 470, "When:  %s", GetWhenName( CurThing->when));
   }
   else
   {
      DrawScreenText( 453, 440, "DEFAULT THING Info");
      DrawScreenText( 453, 450, "Type:  %s", GetThingName( DefType));
      DrawScreenText( 453, 460, "Angle: %s", GetAngleName( DefAngle));
      DrawScreenText( 453, 470, "When:  %s", GetWhenName(  DefWhen));
   }
}



/*
   edit a thing info
*/

void EditThingInfo( int *type, int *angle, int *when)
{
   int key;
   setcolor( YELLOW);
   DrawScreenBox( 0, 0, 639, 31);
   setcolor( BLACK);
   DrawScreenText( 5,  2, "1 To Change Type          (Current: %s)", GetThingName( *type));
   DrawScreenText( 5, 12, "2 To Change Angle         (Current: %s)", GetAngleName( *angle));
   DrawScreenText( 5, 22, "3 To Change When Appears  (Current: %s)", GetWhenName( *when));
   key = bioskey( 0);
   if ((key & 0x00FF) == '1')
   {
      setcolor( YELLOW);
      DrawScreenBox( 0, 0, 639, 31);
      setcolor( BLACK);
      DrawScreenText( 5,  2, "Select Class:  1-Player  2-Enemy  3-Weapon  4-Bonus  5-Decoration");
      DrawScreenText( 5, 12, "               6-Enter a decimal value");
      key = bioskey( 0);
      if ((key & 0x00FF) == '1')
      {
         setcolor( YELLOW);
         DrawScreenBox( 0, 0, 639, 31);
         setcolor( BLACK);
         DrawScreenText( 5,  2, "Select Start Position Type:  1-Player1  2-Player2  3-Player3  4-Player4");
         key = bioskey( 0);
         if ((key & 0x00FF) == '1')
            *type = THING_PLAYER1;
         else if ((key & 0x00FF) == '2')
            *type = THING_PLAYER2;
         else if ((key & 0x00FF) == '3')
            *type = THING_PLAYER3;
         else if ((key & 0x00FF) == '4')
            *type = THING_PLAYER4;
         else
            goto Beep;
      }
      else if ((key & 0x00FF) == '2')
      {
         setcolor( YELLOW);
         DrawScreenBox( 0, 0, 639, 31);
         setcolor( BLACK);
         DrawScreenText( 5,  2, "Select Enemy:  1-Trooper  2-Sargeant  3-Imp        4-Demon        5-Spector");
         DrawScreenText( 5, 12, "               6-Baron    7-LostSoul  8-Cacodemon  9-SpiderDemon  A-CyberDemon");
         key = bioskey( 0);
         if ((key & 0x00FF) == '1')
            *type = THING_TROOPER;
         else if ((key & 0x00FF) == '2')
            *type = THING_SARGEANT;
         else if ((key & 0x00FF) == '3')
            *type = THING_IMP;
         else if ((key & 0x00FF) == '4')
            *type = THING_DEMON;
         else if ((key & 0x00FF) == '5')
            *type = THING_SPECTOR;
         else if ((key & 0x00FF) == '6')
            *type = THING_BARON;
         else if ((key & 0x00FF) == '7')
            *type = THING_LOSTSOUL;
         else if ((key & 0x00FF) == '8')
            *type = THING_COCADEMON;
         else if ((key & 0x00FF) == '9')
            *type = THING_SPIDERBOSS;
         else if ((key & 0x00FF) == 'A' || (key & 0x00FF) == 'a')
            *type = THING_CYBERDEMON;
         else
            goto Beep;
      }
      else if ((key & 0x00FF) == '3')
      {
         setcolor( YELLOW);
         DrawScreenBox( 0, 0, 639, 31);
         setcolor( BLACK);
         DrawScreenText( 5,  2, "Select Weapon: 1-ShotGun   2-ChainGun  3-Launcher   4-PlasmaGun   5-ChainSaw");
         DrawScreenText( 5, 12, "               6-Shells    7-AmmoClip  8-Rocket     9-EnergyCell  A-BFG9000");
         DrawScreenText( 5, 22, "               B-ShellBox  C-AmmoBox   D-RocketBox  E-EnergyPack  F-BackPack");
         key = bioskey( 0);
         if ((key & 0x00FF) == '1')
            *type = THING_SHOTGUN;
         else if ((key & 0x00FF) == '2')
            *type = THING_CHAINGUN;
         else if ((key & 0x00FF) == '3')
            *type = THING_LAUNCHER;
         else if ((key & 0x00FF) == '4')
            *type = THING_PLASMAGUN;
         else if ((key & 0x00FF) == '5')
            *type = THING_CHAINSAW;
         else if ((key & 0x00FF) == '6')
            *type = THING_SHELLS;
         else if ((key & 0x00FF) == '7')
            *type = THING_AMMOCLIP;
         else if ((key & 0x00FF) == '8')
            *type = THING_ROCKET;
         else if ((key & 0x00FF) == '9')
            *type = THING_ENERGYCELL;
         else if ((key & 0x00FF) == 'A' || (key & 0x00FF) == 'a')
            *type = THING_BFG9000;
         else if ((key & 0x00FF) == 'B' || (key & 0x00FF) == 'b')
            *type = THING_SHELLBOX;
         else if ((key & 0x00FF) == 'C' || (key & 0x00FF) == 'c')
            *type = THING_AMMOBOX;
         else if ((key & 0x00FF) == 'D' || (key & 0x00FF) == 'd')
            *type = THING_ROCKETBOX;
         else if ((key & 0x00FF) == 'E' || (key & 0x00FF) == 'e')
            *type = THING_ENERGYPACK;
         else if ((key & 0x00FF) == 'F' || (key & 0x00FF) == 'f')
            *type = THING_BACKPACK;
         else
            goto Beep;
      }
      else if ((key & 0x00FF) == '4')
      {
         setcolor( YELLOW);
         DrawScreenBox( 0, 0, 639, 41);
         setcolor( BLACK);
         DrawScreenText( 5,  2, "Select Bonus: 1-RedCard     2-Armour+1  3-GreenArmour  4-StimPack  5-BlurSphere");
         DrawScreenText( 5, 12, "              6-YellowCard  7-Health+1  8-BlueArmour   9-MedKit    A-SoulSphere");
         DrawScreenText( 5, 22, "              B-BlueCard    C-Map       D-RadSuit      E-Beserk    F-Invuln.");
         DrawScreenText( 5, 32, "              G-LiteAmp");
         key = bioskey( 0);
         if ((key & 0x00FF) == '1')
            *type = THING_REDCARD;
         else if ((key & 0x00FF) == '2')
            *type = THING_ARMBONUS1;
         else if ((key & 0x00FF) == '3')
            *type = THING_GREENARMOR;
         else if ((key & 0x00FF) == '4')
            *type = THING_STIMPACK;
         else if ((key & 0x00FF) == '5')
            *type = THING_BLURSPHERE;
         else if ((key & 0x00FF) == '6')
            *type = THING_YELLOWCARD;
         else if ((key & 0x00FF) == '7')
            *type = THING_HLTBONUS1;
         else if ((key & 0x00FF) == '8')
            *type = THING_BLUEARMOR;
         else if ((key & 0x00FF) == '9')
            *type = THING_MEDKIT;
         else if ((key & 0x00FF) == 'A' || (key & 0x00FF) == 'a')
            *type = THING_SOULSPHERE;
         else if ((key & 0x00FF) == 'B' || (key & 0x00FF) == 'b')
            *type = THING_BLUECARD;
         else if ((key & 0x00FF) == 'C' || (key & 0x00FF) == 'c')
            *type = THING_MAP;
         else if ((key & 0x00FF) == 'D' || (key & 0x00FF) == 'd')
            *type = THING_RADSUIT;
         else if ((key & 0x00FF) == 'E' || (key & 0x00FF) == 'e')
            *type = THING_BESERK;
         else if ((key & 0x00FF) == 'F' || (key & 0x00FF) == 'f')
            *type = THING_INVULN;
         else if ((key & 0x00FF) == 'G' || (key & 0x00FF) == 'g')
            *type = THING_LITEAMP;
         else
            goto Beep;
      }
      else if ((key & 0x00FF) == '5')
      {
         setcolor( YELLOW);
         DrawScreenBox( 0, 0, 639, 31);
         setcolor( BLACK);
         DrawScreenText( 5,  2, "Select Decoration:  1-Barrel  2-TechColumn");
         key = bioskey( 0);
         if ((key & 0x00FF) == '1')
            *type = THING_BARREL;
         else if ((key & 0x00FF) == '2')
            *type = THING_TECHCOLUMN;
         else
            goto Beep;
      }
      else if ((key & 0x00FF) == '6')
      {
         int n, val = 0;
         setcolor( YELLOW);
         DrawScreenBox( 0, 0, 639, 31);
         setcolor( BLACK);
         DrawScreenText( 5,  2, "Enter a 4 digit decimal value: ");
         for (n = 0; n < 4; n++)
         {
            key = bioskey( 0);
            DrawScreenText( 250 + (n * 8),  2, "%c", key & 0x00FF);
            if ((key & 0x00FF) < '0' || (key & 0x00FF) > '9')
               goto Beep;
            val = (val * 10) + ((key & 0x00FF) - '0');
         }
         DefType = val;
      }
      else
         goto Beep;
   }
   else if ((key & 0x00FF) == '2')
   {
      setcolor( YELLOW);
      DrawScreenBox( 0, 0, 639, 31);
      setcolor( BLACK);
      DrawScreenText( 5,  2,  "Press:  1-North  2-NorthEast  3-East  4-Southeast");
      DrawScreenText( 5,  12, "        5-South  6-SouthWest  7-West  8-NorthWest");
      key = bioskey( 0);
      if ((key & 0x00FF) == '1')
         *angle = 90;
      else if ((key & 0x00FF) == '2')
         *angle = 135;
      else if ((key & 0x00FF) == '3')
         *angle = 180;
      else if ((key & 0x00FF) == '4')
         *angle = 225;
      else if ((key & 0x00FF) == '5')
         *angle = 270;
      else if ((key & 0x00FF) == '6')
         *angle = 315;
      else if ((key & 0x00FF) == '7')
         *angle = 0;
      else if ((key & 0x00FF) == '8')
         *angle = 45;
      else
         goto Beep;
   }
   else if ((key & 0x00FF) == '3')
   {
      setcolor( YELLOW);
      DrawScreenBox( 0, 0, 639, 31);
      setcolor( BLACK);
      DrawScreenText( 5,  2,  "Press choice of difficulty levels: 1-EasyMedHard  2-MediumHard  3-HardOnly");
      key = bioskey( 0);
      if ((key & 0x00FF) == '1')
         *when = 0x07;
      else if ((key & 0x00FF) == '2')
         *when = 0x06;
      else if ((key & 0x00FF) == '3')
         *when = 0x04;
      else
         goto Beep;
   }
   else
      goto Beep;
   return;
Beep:
   sound( 640);
   delay( 100);
   nosound();
   return;
}

/* end of file */
