/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   THING.C - Thing name and type routines.
*/

/* the includes */
#include "deu.h"
#include "things.h"


/*
   get the colour of a thing (green players, red enemies, else white)
*/

int GetThingColour( int type)
{
   switch( type)
   {
   case THING_PLAYER1:
   case THING_PLAYER2:
   case THING_PLAYER3:
   case THING_PLAYER4:
   case THING_DEATHMATCH:
      return GREEN;
   case THING_SARGEANT:
   case THING_TROOPER:
   case THING_IMP:
   case THING_DEMON:
   case THING_SPECTOR:
   case THING_BARON:
   case THING_LOSTSOUL:
   case THING_COCADEMON:
   case THING_SPIDERBOSS:
   case THING_CYBERDEMON:
      return LIGHTRED;
   case THING_BLUECARD:
   case THING_YELLOWCARD:
   case THING_REDCARD:
   case THING_ARMBONUS1:
   case THING_HLTBONUS1:
   case THING_GREENARMOR:
   case THING_BLUEARMOR:
   case THING_SOULSPHERE:
   case THING_MEDKIT:
   case THING_STIMPACK:
   case THING_RADSUIT:
   case THING_MAP:
   case THING_BLURSPHERE:
   case THING_BESERK:
   case THING_INVULN:
   case THING_LITEAMP:
      return LIGHTGREEN;
   case THING_SHOTGUN:
   case THING_CHAINSAW:
   case THING_CHAINGUN:
   case THING_LAUNCHER:
   case THING_PLASMAGUN:
   case THING_BFG9000:
   case THING_AMMOCLIP:
   case THING_AMMOBOX:
   case THING_SHELLS:
   case THING_SHELLBOX:
   case THING_ROCKET:
   case THING_ROCKETBOX:
   case THING_ENERGYCELL:
   case THING_ENERGYPACK:
   case THING_BACKPACK:
      return BROWN;
   }
   return WHITE;
}



/*
   get the name of a thing
*/

char *GetThingName( int type)
{
   static char temp[ 20];

   switch( type)
   {
   /* the players */
   case THING_PLAYER1:
      return "Player 1 Start";
   case THING_PLAYER2:
      return "Player 2 Start";
   case THING_PLAYER3:
      return "Player 3 Start";
   case THING_PLAYER4:
      return "Player 4 Start";
   case THING_DEATHMATCH:
      return "DEATHMATCH Start";

   /* enhancements */
   case THING_BLUECARD:
      return "Blue Card";
   case THING_YELLOWCARD:
      return "Yellow Card";
   case THING_REDCARD:
      return "Red Card";
   case THING_ARMBONUS1:
      return "Armour Helmet";
   case THING_HLTBONUS1:
      return "Health Potion";
   case THING_GREENARMOR:
      return "Green Armour";
   case THING_BLUEARMOR:
      return "Blue Armour";
   case THING_SOULSPHERE:
      return "Soul Sphere";
   case THING_MEDKIT:
      return "Medical Kit";
   case THING_STIMPACK:
      return "Stim Pack";
   case THING_RADSUIT:
      return "Radiation Suit";
   case THING_MAP:
      return "Computer Map";
   case THING_BLURSPHERE:
      return "Blur Sphere";
   case THING_BESERK:
      return "Beserk Sphere";
   case THING_INVULN:
      return "Invulnerability";
   case THING_LITEAMP:
      return "Lite Amp. Goggles";

   /* weapons */
   case THING_SHOTGUN:
      return "Shotgun";
   case THING_CHAINSAW:
      return "Chainsaw";
   case THING_CHAINGUN:
      return "Chaingun";
   case THING_LAUNCHER:
      return "Rocket Launcher";
   case THING_PLASMAGUN:
      return "Plasma Gun";
   case THING_BFG9000:
      return "BFG9000";
   case THING_AMMOCLIP:
      return "Ammo Clip";
   case THING_AMMOBOX:
      return "Box of Ammo";
   case THING_SHELLS:
      return "Shells";
   case THING_SHELLBOX:
      return "Box of Shells";
   case THING_ROCKET:
      return "Rocket";
   case THING_ROCKETBOX:
      return "Box of Rockets";
   case THING_ENERGYCELL:
      return "Energy Cell";
   case THING_ENERGYPACK:
      return "Energy Pack";
   case THING_BACKPACK:
      return "Backpack";

   /* decorations */
   case THING_BARREL:
      return "Barrel";
   case THING_LAMP:
      return "Lamp";
   case THING_CANDLE:
      return "Candle";
   case THING_CANDLESTICK:
      return "Candlestick";
   case THING_TORCH:
      return "Torch";
   case THING_TECHCOLUMN:
      return "Technical Column";
   case THING_BONES:
      return "Guts and Bones";
   case THING_BONES2:
      return "Guts and Bones 2";
   case THING_DEADBUDDY:
      return "Dead Guy (Green)";
   case THING_POOLOFBLOOD:
      return "Pool of Blood";

   /* enemies */
   case THING_SARGEANT:
      return "Sargeant";
   case THING_TROOPER:
      return "Trooper";
   case THING_IMP:
      return "Imp";
   case THING_DEMON:
      return "Demon";
   case THING_BARON:
      return "Baron";
   case THING_SPECTOR:
      return "Spector";
   case THING_LOSTSOUL:
      return "Lost Soul";
   case THING_COCADEMON:
      return "Cocademon";
   case THING_SPIDERBOSS:
      return "Spider Boss";
   case THING_CYBERDEMON:
      return "Cyber Demon";
   }

   /* unknown */
   sprintf( temp, "Unknown <%04d>", type);
   return temp;
}



/*
   get the name of the angle
*/

char *GetAngleName( int angle)
{
   switch (angle)
   {
   case 0:
      return "West";
   case 45:
      return "NorthWest";
   case 90:
      return "North";
   case 135:
      return "NorthEast";
   case 180:
      return "East";
   case 225:
      return "SouthEast";
   case 270:
      return "South";
   case 315:
      return "SouthWest";
   }
   return "<ILLEGAL ANGLE>";
}



/*
   get string of when something wil appear
*/

char *GetWhenName( int when)
{
   static char temp[ 40];
   temp[ 0] = '\0';
   if (when & 0x01)
      strcat( temp, "D12 ");
   if (when & 0x02)
      strcat( temp, "D3 ");
   if (when & 0x04)
      strcat( temp, "D4 ");
   if (when & 0x08)
      strcat( temp, "Net? ");
   if (when & 0x10)
      strcat( temp, "Reg? ");
   return temp;
}

/* end of file */
