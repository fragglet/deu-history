#include "deu.h"

#include <assert.h>
#include <stdio.h>

#ifdef NULL
#undef NULL
#define NULL ((char *)0x0L)
#endif


BCINT getcol(char *str);
void readcfg(char *file);
void add_thing(char *line);
void add_thing_class(char *line);
void add_linedef(char *line);
void add_linedef_class(char *line);
void add_sector_type(char *line);
void add_sector_class(char *line);
void add_levels(char *line);
void add_texture_sections(char *line);
void add_ftexture_sections(char *line);
void get_palette_colour(char *line);
int read_line(FILE *f, char *buf, int buflen);

/* stuff for manipulating SList s */
SList SList_append(SList l, char *str);
SList SList_find(SList l, char *str);

extern BCINT Palette[];

/* what all the various sections in the cfg file are */
typedef enum {
	NONE = 0,	/* haven't got a section header yet */
	WAD,
	THINGS,
	LINEDEFS,
	LEVELS,
	TEXTURES,
	FTEXTURES,
	REGTEST,
	SECTORS,
	COLORS,
	NUM_SECTIONS
} cfg_section;

char *header[NUM_SECTIONS] = {
    /* these go hand in hand with the numbers above */
	"",
	"[wad]",
	"[things]",
	"[linedefs]",
	"[levels]",
	"[textures]",
	"[ftextures]",
	"[regtest]",
	"[sectors]",
	"[colors]"
};

char *colours[23] = {
	"BLACK", "BLUE", "GREEN", "CYAN", "RED", "MAGENTA", "BROWN", "LIGHTGRAY",
	"DARKGRAY", "LIGHTBLUE", "LIGHTGREEN", "LIGHTCYAN", "LIGHTRED",
	"LIGHTMAGENTA", "YELLOW", "WHITE", "DARKBLUE", "DARKGREEN", "DARKRED",
	"DARKMAGENTA", "GRAY", "DARKERGRAY", "ORANGE"
};

ld_class *linedef_class = (ld_class *)NULL;
/* what was the last linedef class we had? */

thing_class *current_thing_class = (thing_class *)NULL;
/* ditto for thing class */

sector_class *current_sector_class = (sector_class *)NULL;
/* ditto for sectors */

BCINT getcol(char *str)
{
	BCINT i = -1;

	/* first, do the easy bit -- is the colour specified as a number? */
	if(sscanf(str, "%d", &i))
		return i;

	/* check it against the colour names */
	for(i = 0; i < 23; i++)
		if(!stricmp(str, colours[i]))
			return i;

	return -1;
}
	

void readcfg(char *file)
{
	static cfg_section what = NONE;
	/* what section are we currently reading? */

	FILE *thefile;
	char buf[256], *tok;

	int i;

	if(!(thefile = fopen(file, "rb")))
		return;

	/* OK, we opened the file */
	while(read_line(thefile, buf, 255)) {
		/* first, see whether it's just a comment */
		if(*buf == '#')
			goto nextline;

		if(!strnicmp("include", buf, 7)) {
			tok = strtok(buf + 7, "\t ");
			readcfg(tok);
			goto nextline;
		}

		for(i = 0; i < NUM_SECTIONS; i++) {
			if(!stricmp(buf, header[i])) {
				what = i;
				goto nextline;
			}
		}

		switch(what) {
		case NONE:
		case NUM_SECTIONS:
			break;

		case WAD:
			MainWad = strdup(buf);
			break;

		case THINGS:
			if(buf[0] == '"' && buf[strlen(buf) - 1] ==  '"')
				add_thing_class(buf);
			else
				add_thing(buf);
			break;

		case LINEDEFS:
			if(buf[0] == '"' && buf[strlen(buf) - 1] == '"')
				add_linedef_class(buf);
			else	
				add_linedef(buf);
			break;

		case SECTORS:
			if(buf[0] == '"' && buf[strlen(buf) - 1] == '"')
				add_sector_class(buf);
			else
				add_sector_type(buf);
			break;

		case REGTEST:
			RegTest = strdup(buf);
			break;

		case LEVELS:
			add_levels(buf);
			break;

		case TEXTURES:
			add_texture_sections(buf);
			break;

		case FTEXTURES:
			add_ftexture_sections(buf);
			break;

		case COLORS:
			get_palette_colour(buf);
			break;
		}

	nextline:
	}

	fclose(thefile);
}


void add_thing(char *line)
{
	BCINT c1, c2, radius, type;
	char *tok;
	thing_type *p, *new = (thing_type *)NULL;

	if(!current_thing_class)
		return;

	tok = strtok(line, "\t ,");
	if(tok && (type = atoi(tok))) {
		tok = strtok(NULL, "\t ,");
		if(tok && (c1 = getcol(tok)) != -1) {
			tok = strtok(NULL, "\t ,");
			if(tok && (c2 = getcol(tok)) != -1) {
				tok = strtok(NULL, "\t ,");
				if(tok && (radius = atoi(tok))) {
					tok = strtok(NULL, "\t\",");
					/* bodging so that we can get a string delimited
					   by quotes which may contain spaces */
					while(tok && (tok[0] == ' '))
						tok = strtok(NULL, "\t\",");
					/* do the add here */
					for(p = current_thing_class->types;
						(p && p->next); p = p->next);

					new = (thing_type *)malloc(sizeof(thing_type));
					assert(new);

					new->type = type;
					new->col1 = c1;
					new->col2 = c2;
					new->radius = radius;
					new->name = strdup(tok);
					new->next = (thing_type *)NULL;

					if(p)
						p->next = new;
					else
						current_thing_class->types = new;
				}
			}
		}
	}
}
						   
void add_thing_class(char *line)
{
	thing_class *p, *new = (thing_class *)NULL;

	line[strlen(line) - 1] = '\0';
	line++;
	/* add it here */

	for(p = Thing_classes; (p && p->next); p = p->next)
		if(!strcmp(line, p->name)) {
			current_thing_class = p;
			return;
		}

	new = (thing_class *)malloc(sizeof(thing_class));
	assert(new);

	new->name = strdup(line);
	new->next = (thing_class *)NULL;

	if(p)
		p->next = new;
	else
		Thing_classes = new;

	current_thing_class = new; 
}

void add_linedef(char *line)
{
	char *short_desc, *long_desc, *tok;
	BCINT type = -1;
	ld_type *p, *new = (ld_type *)NULL;

	if(!linedef_class)
		return;
	/* can't add a linedef if we don't know what class
	   to put it in */

	tok = strtok(line, "\t ,");
	if(tok && (type = atoi(tok)) != -1) {
		tok = strtok(NULL, "\t,\"");
		while(tok && ((tok[0] == ' ') || (tok[0] == ',')))
			tok = strtok(NULL, "\t\"");
		/* should by now have the short description */
		short_desc = strdup(tok);

		tok = strtok(NULL, "\t,\"");
		while(tok && ((tok[0] == ' ') || (tok[0] == ',')))
			tok = strtok(NULL, "\t\"");
		/* should by now have the long description */
		long_desc = strdup(tok);

		/* now add it */
		for(p = linedef_class->types; (p && p->next); p = p->next);
		/* make p point to the last item in the list, or be NULL */

		new = (ld_type *)malloc(sizeof(ld_type));
		assert(new);

		new->longname = long_desc;
		new->shortname = short_desc;
		new->type = type;
		new->next = (ld_type *)NULL;

		if(p)
			p->next = new;
		else
			linedef_class->types = new;
	}
}

void add_linedef_class(char *line)
{
	ld_class *p, *new = (ld_class *)NULL;

	line++;
	line[strlen(line) - 1] = '\0';
	/* lose the quotes */

	for(p = Linedef_classes; (p && p->next); p = p->next)
		if(!strcmp(p->name, line)) {
			linedef_class = p;
			return;
			/* we already had it */
		}

	/* p now points to the last item in the list, or is NULL
	   if there is no list. */

	new = (ld_class *)malloc(sizeof(ld_class));
	assert(new);
	/* get memory */

	new->name = strdup(line);
	new->types = (ld_type *)NULL;
	new->next = (ld_class *)NULL;
	/* fill in the fields */

	if(p)
		p->next = new;
	else
		Linedef_classes = new;

	linedef_class = new;
}

void add_sector_class(char *line)
{
	sector_class *p, *new = (sector_class *)NULL;

	line[strlen(line) - 1] = '\0';
	line++;

	for(p = Sector_classes; (p && p->next); p = p->next)
		if(!strcmp(line, p->name)) {
			current_sector_class = p;
			return;
		}

	new = (sector_class *)malloc(sizeof(sector_class));
	assert(new);

	new->name = strdup(line);
	new->next = (sector_class *)NULL;

	if(p)
		p->next = new;
	else
		Sector_classes = new;

	current_sector_class = new; 
}

void add_sector_type(char *line)
{
	char *short_desc;
	char *long_desc;
	BCINT type = -1;
	char *tok;
	sector_type *p, *new = (sector_type *)NULL;

	tok = strtok(line, "\t ,");
	if(tok && (type = atoi(tok)) != -1) {
		tok = strtok(NULL, "\t\"");
		while(tok && ((tok[0] == ' ') || (tok[0] == ',')))
			tok = strtok(NULL, "\t\"");
		/* should by now have the short description */
		short_desc = strdup(tok);

		tok = strtok(NULL, "\t\"");
		while(tok && ((tok[0] == ' ') || (tok[0] == ',')))
			tok = strtok(NULL, "\t\"");
		/* should by now have the long description */
		long_desc = strdup(tok);

		/* add it now */
		new = (sector_type *)malloc(sizeof(sector_type));
		assert(new);
		new->shortname = short_desc;
		new->longname = long_desc;
		new->type = type;
		new->next = (sector_type *)NULL;

		for(p = current_sector_class->types; (p && p->next); p = p->next);

		if(p)
			p->next = new;
		else
			current_sector_class->types = new;
	}
}

void add_levels(char *line)
{
	char *tok;

	tok = strtok(line, "\t ,");
	while(tok) {
		/* add a level */
		LevelNames = SList_append(LevelNames, tok);

		tok = strtok(NULL, "\t ,");
	}
}

void add_texture_sections(char *line)
{
	char *tok;

	tok = strtok(line, "\t ,");
	while(tok) {
		/* add it to list */
		Texture_sections = SList_append(Texture_sections, tok);

		tok = strtok(NULL, "\t ,");
	}
}

void add_ftexture_sections(char *line)
{
	char *tok;

	tok = strtok(line, "\t ,");
	while(tok) {
		/* add it to list */
		Ftexture_sections = SList_append(Ftexture_sections, tok);

		tok = strtok(NULL, "\t ,");
	}
}

void get_palette_colour(char *line)
{
	char *tok;
	BCINT color = -1;
	BCINT palette_index;
	
	tok = strtok(line, "\t ,=");
	if(tok) {
		color = getcol(tok);
		if(color == -1)
			return;

		tok = strtok(NULL, "\t ,=");
		palette_index = atoi(tok);

		Palette[color] = palette_index;
	}
}

		

/* read a line in from the current file
   return zero if we hit the end of the file */
int read_line(FILE *f, char *buf, int buflen)
{
	int eof = 0, eol = 0;
	int c;

	while(((c = fgetc(f)) == '\n') ||
		  (c == '\r'));

	while(!(eof || eol) && (buflen > 0)) {
		if(c == EOF)
			eof = 1;
		else if((c == '\n') || (c == '\r'))
			eol = 1;
		else {
			*buf++ = (char)c;
			buflen--;
            c = fgetc(f);
		}
	}
	*buf = 0; /* bung a terminator on it */

	return(1 - eof);
}

SList SList_append(SList l, char *str)
{
	SList new, head = l;

	while(l->next)
		l = l->next;

	new = (SList)malloc(sizeof(struct _SList));
	if(head)
		l->next = new;
	else
		head = new;

	new->string = strdup(str);

	return head;
}

SList SList_find(SList l, char *str)
{
	while(l)
		if(!strcmp(l->string, str))
			return l;
		else
			l = l->next;

	return (SList)NULL;
}

