/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.
   
   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...
   
   NAMES.C - Object name and type routines.
   */

/* the includes */
#include "deu.h"

sector_class *Sector_classes = (sector_class *)NULL;
ld_class *Linedef_classes = (ld_class *)NULL;

/*
   get the name of an object type
   */

char *GetObjectTypeName( BCINT objtype)
{
    switch (objtype) {
	
    case OBJ_THINGS:
	return "Thing";
    case OBJ_LINEDEFS:
	return "LineDef";
    case OBJ_SIDEDEFS:
	return "SideDef";
    case OBJ_VERTEXES:
	return "Vertex";
    case OBJ_SEGS:
	return "Segment";
    case OBJ_SSECTORS:
	return "SSector";
    case OBJ_NODES:
	return "Node";
    case OBJ_SECTORS:
	return "Sector";
    case OBJ_REJECT:
	return "Reject";
    case OBJ_BLOCKMAP:
	return "Blockmap";
    }
    return "< Bug! >";
}



/*
   what are we editing?
   */

char *GetEditModeName( BCINT objtype)
{
    switch (objtype) {
	
    case OBJ_THINGS:
	return "Things";
    case OBJ_LINEDEFS:
    case OBJ_SIDEDEFS:
	return "LineDefs & SideDefs";
    case OBJ_VERTEXES:
	return "Vertices";
    case OBJ_SEGS:
	return "Segments";
    case OBJ_SSECTORS:
	return "Seg-Sectors";
    case OBJ_NODES:
	return "Nodes";
    case OBJ_SECTORS:
	return "Sectors";
    }
    return "< Bug! >";
}


BCINT ldt_index[141];

void index_ld_types()
{
/*    BCINT i;
    for(i = 0; ld_types[i].type != -1; i++)
	ldt_index[ ld_types[i].type ] = i; */
}

/*
   get a short (2 + 13 char.) description of the type of a linedef
   */

char *GetLineDefTypeName( BCINT type)
{
	ld_class *c;
	ld_type *t;

	for(c = Linedef_classes; c; c = c->next)
		for(t = c->types; t; t = t->next)
			if(t->type == type)
				return t->shortname;

	return("Unknown LineDef");
}



/*
   get a long description of the type of a linedef
   */

char *GetLineDefTypeLongName( BCINT type)
{
	ld_class *c;
	ld_type *t;

	for(c = Linedef_classes; c; c = c->next)
		for(t = c->types; t; t = t->next)
			if(t->type == type)
				return t->longname;

	return("Unknown LineDef");
}


/* is a linedef type only available in Doom 2 ? */
Bool LinedefIsDoom2Only( BCINT type )
{
    return 0;
}

/*
   get a short description of the flags of a linedef
   */

char *GetLineDefFlagsName( BCINT flags)
{
    static char temp[ 20];
    
    if (flags & 0x0100)
	strcpy( temp, "Ma"); /* Already on the map */
    else
	strcpy( temp, "-");
    if (flags & 0x80)
	strcat( temp, "In"); /* Invisible on the map */
    else
	strcat( temp, "-");
    if (flags & 0x40)
	strcat( temp, "So"); /* Blocks sound */
    else
	strcat( temp, "-");
    if (flags & 0x20)
	strcat( temp, "Se"); /* Secret (normal on the map) */
    else
	strcat( temp, "-");
    if (flags & 0x10)
	strcat( temp, "Lo"); /* Lower texture offset changed */
    else
	strcat( temp, "-");
    if (flags & 0x08)
	strcat( temp, "Up"); /* Upper texture offset changed */
    else
	strcat( temp, "-");
    if (flags & 0x04)
	strcat( temp, "2S"); /* Two-sided */
    else
	strcat( temp, "-");
    if (flags & 0x02)
	strcat( temp, "Mo"); /* Monsters can't cross this line */
    else
	strcat( temp, "-");
    if (flags & 0x01)
	strcat( temp, "Im"); /* Impassible */
    else
	strcat( temp, "-");
    if (strlen( temp) > 13) {
	temp[13] = '|';
	temp[14] = '\0';
    }
    return temp;
}



/*
   get a long description of one linedef flag
   */

char *GetLineDefFlagsLongName( BCINT flags)
{
    if (flags & 0x0100)
	return "Already on the map at startup";
    if (flags & 0x80)
	return "Invisible on the map";
    if (flags & 0x40)
	return "Blocks sound";
    if (flags & 0x20)
	return "Secret (shown as normal on the map)";
    if (flags & 0x10)
	return "Lower texture is \"unpegged\"";
    if (flags & 0x08)
	return "Upper texture is \"unpegged\"";
    if (flags & 0x04)
	return "Two-sided (may be transparent)";
    if (flags & 0x02)
	return "Monsters cannot cross this line";
    if (flags & 0x01)
	return "Impassible";
    return "UNKNOWN";
}



/*
   get a short (19 char.) description of the type of a sector
   */

char *GetSectorTypeName( BCINT type)
{
	sector_class *c;
	sector_type *t;

	for(c = Sector_classes; c; c = c->next)
		for(t = c->types; t; t = t->next)
			if(type == t->type)
				return t->shortname;

    return "DO NOT USE!";
}



/*
   get a long description of the type of a sector
   */

char *GetSectorTypeLongName( BCINT type)
{
	sector_class *c;
	sector_type *t;

	for(c = Sector_classes; c; c = c->next)
		for(t = c->types; t; t = t->next)
			if(type == t->type)
				return t->longname;

    return "DO NOT USE!";
}

/* end of file */

