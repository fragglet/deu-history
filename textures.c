#pragma warn -par
/*
   *** THIS PART OF THE CODE IS UNDER DEVELOPMENT ***

   Texture display by Rapha‰l Quinet <quinet@montefiore.ulg.ac.be>

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...
   Put a credit notice somewhere with my name on it.  Thanks!  ;-)

   TEXTURES.C - Textures in 256 colors.
*/

/* the includes */
#include "deu.h"


/*
   display a floor or ceiling texture
*/

void DisplayFloorTexture( int x0, int y0, char *texname)
{
/*
   look for texname in the master directory
   read the picture data
   display the picture (64x64)
*/
}



/*
   display a wall texture ("texture1" or "texture2" object)
*/

void DisplayWallTexture( int x0, int y0, char *texname)
{
/*
   same as above, but the picture is made up of several parts
*/
}



/*
   choose a floor or ceiling texture
*/

void ChooseFloorTexture( char *prompt, int numtextures, char **textures, char *texname)
{
   NotImplemented();
   texname[ 0] = '\0';
/*
   change video mode - need a VGA256 driver!
   get the color palette from the WAD file
   display the prompt
   while (! selected)
   {
      display a part of the list of texture names
      highlight the current texture name
      display the current texture
      parse keys
      (scroll up/down in the list)
   }
   restore normal video mode
*/
}



/*
   choose a wall texture ("texture1" or "texture2" objects)
*/

void ChooseWallTexture( char *prompt, int numtextures, char **textures, char *texname)
{
   NotImplemented();
   texname[ 0] = '\0';
/*
   same as above
*/
}
