/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   THINGS.C - Thing name and type routines.
*/

/* the includes */
#include "deu.h"
#include "things.h"


/*
   get the colour of a thing
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
   case THING_CACODEMON:
   case THING_SPIDERBOSS:
   case THING_CYBERDEMON:
      return LIGHTRED;
   case THING_BLUECARD:
   case THING_YELLOWCARD:
   case THING_REDCARD:
   case THING_BLUESKULLKEY:
   case THING_YELLOWSKULLKEY:
   case THING_REDSKULLKEY:
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
   case THING_CACODEMON:
      return "Cacodemon";
   case THING_SPIDERBOSS:
      return "Spider Boss";
   case THING_CYBERDEMON:
      return "Cyber Demon";

   /* enhancements */
   case THING_BLUECARD:
      return "Blue KeyCard";
   case THING_YELLOWCARD:
      return "Yellow KeyCard";
   case THING_REDCARD:
      return "Red KeyCard";
   case THING_BLUESKULLKEY:
      return "Blue Skull Key";
   case THING_YELLOWSKULLKEY:
      return "Yellow Skull Key";
   case THING_REDSKULLKEY:
      return "Red Skull Key";
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
   case THING_TECHCOLUMN:
      return "Technical Column";
   case THING_TGREENPILLAR:
      return "Tall Green Pillar";
   case THING_TREDPILLAR:
      return "Tall Red Pillar";
   case THING_SGREENPILLAR:
      return "Short Green Pillar";
   case THING_SREDPILLAR:
      return "Short Red Pillar";
   case THING_PILLARHEART:
      return "Pillar w/Heart";
   case THING_PILLARSKULL:
      return "Red Pillar w/Skull";
   case THING_EYEINSYMBOL:
      return "Eye in Symbol";
   case THING_GREYTREE:
      return "Grey Tree";
   case THING_BROWNSTUB:
      return "Brown Stub";
   case THING_BROWNTREE:
      return "Tall Brown Tree";

   case THING_LAMP:
      return "Lamp";
   case THING_CANDLE:
      return "Candle";
   case THING_CANDELABRA:
      return "Candelabra";
   case THING_TBLUETORCH:
      return "Tall Blue Torch";
   case THING_TGREENTORCH:
      return "Tall Green Torch";
   case THING_TREDTORCH:
      return "Tall Red Torch";
   case THING_SBLUETORCH:
      return "Short Blue Torch";
   case THING_SGREENTORCH:
      return "Short Green Torch";
   case THING_SREDTORCH:
      return "Short Red Torch";

   case THING_DEADPLAYER:
      return "Dead Player (Green)";
   case THING_DEADTROOPER:
      return "Dead Trooper";
   case THING_DEADSARGEANT:
      return "Dead Sargeant";
   case THING_DEADIMP:
      return "Dead Imp";
   case THING_DEADDEMON:
      return "Dead Demon";
   case THING_DEADCACODEMON:
      return "Dead CacoDemon";
   case THING_DEADLOSTSOUL:
      return "Dead Lost Soul";
   case THING_BONES:
      return "Guts and Bones";
   case THING_BONES2:
      return "Guts and Bones 2";
   case THING_POOLOFBLOOD:
      return "Pool of Blood";
   case THING_SKULLTOPPOLE:
      return "Pole with Skull";
   case THING_HEADSKEWER:
      return "Skewer with Heads";
   case THING_PILEOFSKULLS:
      return "Pile of Skulls";
   case THING_IMPALEDBODY:
      return "Impaled body";
   case THING_IMPALEDBODY2:
      return "Twitching Impaled Body";
   case THING_SKULLSINFLAMES:
      return "Skulls in Flames";

   case THING_HANGINGSWAYING:
      return "Swaying Body";
   case THING_HANGINGARMSOUT:
      return "Hanging Arms Out";
   case THING_HANGINGONELEG:
      return "One-legged Body";
   case THING_HANGINGTORSO:
      return "Hanging Torso";
   case THING_HANGINGLEG:
      return "Hanging Leg";
   case THING_HANGINGSWAYING2:
      return "Swaying Body 2";
   case THING_HANGINGARMSOUT2:
      return "Hanging Arms Out 2";
   case THING_HANGINGONELEG2:
      return "One-legged Body 2";
   case THING_HANGINGTORSO2:
      return "Hanging Torso 2";
   case THING_HANGINGLEG2:
      return "Hanging Leg 2";

   /* teleport */
   case THING_TELEPORT:
      return "Teleport exit";
   }

   /* unknown */
   sprintf( temp, "<UNKNOWN %04d>", type);
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
      return "East";
   case 45:
      return "NorthEast";
   case 90:
      return "North";
   case 135:
      return "NorthWest";
   case 180:
      return "West";
   case 225:
      return "SouthWest";
   case 270:
      return "South";
   case 315:
      return "SouthEast";
   }
   return "<ILLEGAL ANGLE>";
}



/*
   get string of when something will appear
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
      strcat( temp, "Deaf ");
   if (when & 0x10)
      strcat( temp, "Multi ");
   return temp;
}

/* end of file */
