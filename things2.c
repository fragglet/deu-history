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
#include <assert.h>

#define _WHITE	15

thing_class *Thing_classes;

typedef struct {
	BCINT type;
	thing_type *thing;
} thing_arr;

thing_arr *Thing_type_array;
int maxThing;

int BCINT_lessp(const void *x, const void *y)
{
	return (int)(*((BCINT *)x) - *((BCINT *)y));
}

void index_things()
{
	thing_class *c;
	thing_type *t;
	thing_arr *p;
	int nThingTypes = 0;
	
	for(c = Thing_classes; c; c = c->next)
		for(t = c->types; t; t = t->next)
			nThingTypes++;
	
	p = Thing_type_array = (thing_arr *)malloc(nThingTypes * sizeof(thing_arr));
	assert(Thing_type_array);
	
	for(c = Thing_classes; c; c = c->next)
		for(t = c->types; t; t = t->next) {
			p->type = t->type;
			p->thing = t;
			p++;
		}
	
	qsort(Thing_type_array, nThingTypes, sizeof(thing_arr),
		  BCINT_lessp);
	maxThing = nThingTypes - 1;
}


/*
   get the colour of a thing
   */

BCINT GetThingColour( BCINT type)
{
	BCINT col = _WHITE, t;
	int lowbound = 0, highbound = maxThing;
	int middle;
	
	do {
		middle = (lowbound + highbound) / 2;
		t = Thing_type_array[middle].type;
		if(t == type) {
			if(Colour2)
				col = (Thing_type_array[middle].thing)->col2;
			else
				col = (Thing_type_array[middle].thing)->col1;
			
			goto found_it;
		}
		
		if(t > type)
			highbound = middle - 1;
		else
			lowbound = middle + 1;
	} while(lowbound <= highbound);
    
    
 found_it:
    return((col == _WHITE) ? WHITE : col);
}



/*
   get the name of a thing
   */

char *GetThingName( BCINT type)
{
   	int lowbound = 0, highbound = maxThing, middle;
	BCINT t;
	
	do {
		middle = (lowbound + highbound) / 2;
		t = Thing_type_array[middle].type;
		
		if(t == type)
			return Thing_type_array[middle].thing->name;
		
		if(t > type)
			highbound = middle - 1;
		else
			lowbound = middle + 1;
	} while(lowbound <= highbound);
    
    /* fall through */
    return("Unknown thing type");
}



/*
   get the size of a thing
   */

BCINT GetThingRadius( BCINT type)
{
   	int lowbound = 0, highbound = maxThing, middle;
	BCINT t;
	
	do {
		middle = (lowbound + highbound) / 2;
		t = Thing_type_array[middle].type;
		
		if(t == type)
			return Thing_type_array[middle].thing->radius;
		
		if(t > type)
			highbound = middle - 1;
		else
			lowbound = middle + 1;
	} while(lowbound <= highbound);
    
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

