/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.
   
   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.
   
   This program comes with absolutely no warranty.
   
   THINGS.C - Thing name and type routines.
   */

/* the includes */
#include "deu.h"

#define _WHITE	15

thing_class *Thing_classes;

/*
   get the colour of a thing
   */

BCINT GetThingColour( BCINT type)
{
    thing_class *c;
    thing_type *t;
	BCINT col;

    for(c = Thing_classes; c; c = c->next)
    	for(t = c->types; t; t = t->next)
    		if(t->type == type) {
    			col = t->col1;
    			goto found_it;
    		}
    
    /* fall through */
    col = _WHITE;
    
 found_it:
    return((col == _WHITE) ? WHITE : col);
}



/*
   get the name of a thing
   */

char *GetThingName( BCINT type)
{
    thing_class *c;
    thing_type *t;

    for(c = Thing_classes; c; c = c->next)
    	for(t = c->types; t; t = t->next)
    		if(t->type == type)
    			return t->name;
    
    /* fall through */
    return("Unknown Thing Type");
}



/*
   get the size of a thing
   */

BCINT GetThingRadius( BCINT type)
{
    thing_class *c;
    thing_type *t;

    for(c = Thing_classes; c; c = c->next)
    	for(t = c->types; t; t = t->next)
    		if(t->type == type)
    			return t->radius;
    
    /* fall through */
    return(16);
}


/*
   get the name of the angle
   */

char *GetAngleName( BCINT angle)
{
    switch (angle) {
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

char *GetWhenName( BCINT when)
{
    static char temp[ 40];
    
    temp[ 0] = '\0';
    if (when & 0x01)
	strcat( temp, "Easy ");
    if (when & 0x02)
	strcat( temp, "Medium ");
    if (when & 0x04)
	strcat( temp, "Hard ");
    if (when & 0x08)
	strcat( temp, "Deaf ");
    if (when & 0x10)
	strcat( temp, "Multi ");
    return temp;
}



/* end of file */

