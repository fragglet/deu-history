/*
   Texture display by Rapha�l Quinet <quinet@montefiore.ulg.ac.be>,
   Trevor Phillips <rphillip@cc.curtin.edu.au>,
   and Christian Johannes Schladetsch <s924706@yallara.cs.rmit.OZ.AU>
   
   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.
   
   This program comes with absolutely no warranty.
   
   TEXTURES.C - Textures in 256 colors.
   */

/* the includes */
#include "deu.h"
#include "wstructs.h"
#include <assert.h>

typedef struct _resource {
	char *name;
	void *data;
	struct _resource *next;
} resource;

resource *ResourceList;

Bool ForgetAfter = TRUE;
/* whether DisplayPic should de-allocate the picture resource
   after displaying it */

/*
   display a floor or ceiling texture at coords x0, y0 and not beyond x1, y1
   */

void DisplayFloorTexture( BCINT x0, BCINT y0, BCINT x1, BCINT y1, char *texname)
{
	MDirPtr			 dir;
	unsigned char huge *pixels;
	
	dir = FindMasterDir( MasterDir, texname);
	if (dir == NULL) {
		SetColor( DARKGRAY);
		DrawScreenLine( x0, y0, x1, y1);
		DrawScreenLine( x0, y1, x1, y0);
		return;
	}
	BasicWadSeek( dir->wadfile, dir->dir.start);
	
#if defined(__TURBOC__)
	
	pixels = (unsigned char huge*) GetFarMemory( 4100 * sizeof( char));
	BasicWadRead( dir->wadfile, &(pixels[ 4]), 4096L);
	if (GfxMode < -1) {
		/* Probably a bug in the VESA driver...	*/
		/* It requires "size-1" instead of "size"! */
		((UBCINT huge *)pixels)[ 0] = 63;
		((UBCINT huge *)pixels)[ 1] = 63;
	}
	else {
		((UBCINT huge *)pixels)[ 0] = 64;
		((UBCINT huge *)pixels)[ 1] = 64;
	}
	
#elif defined(__GNUC__)
	
	/* bcc2grx's getimage-bitmap has a lot more info in it's header
	   than borland's */
	
	pixels = (unsigned char*) GetFarMemory( 4096+sizeof(GrContext));
	getimage( x0, y0, x0+63, y0+63, pixels);   /* Hack! */
	BasicWadRead( dir->wadfile, pixels+sizeof(GrContext), 4096L);
	
#endif
	
	putimage( x0, y0, pixels, COPY_PUT);
	FreeFarMemory( pixels);
}

/*
   display a picture "picname" at coords x0, y0 and not beyond x1, y1
   */


/* SO 2/4/95:
   DisplayPic now uses GetResource to get the sprite/patch to display.
   It checks the variable ForgetAfter to see whether it should
   call ForgetResource after displaying; this should be set by
   the caller.
   ChooseSprite sets this to TRUE; DisplayWallTexture sets it to FALSE.
   DisplayWallTexture then takes the responsibility for calling
   ForgetResource for all the patches in the texture being displayed.
   This enables us to buffer all the patches in case they are used
   more than once in a texture (e.g. LITE5 in Doom (extreme case!)) */

void DisplayPic( BCINT x0, BCINT y0, BCINT x1, BCINT y1, char *picname)
{	
	PHeader *h;
	PColumn **columns, *c;
	BCINT x, y;
	BYTE *pixels;
	long p_resource;
	
	if(bioskey(1))
		return;
	
	p_resource = (long)GetResource(picname);
	if(!p_resource)
		return;
	
	h = (PHeader *)p_resource;
	columns = (PColumn **)(p_resource + sizeof(PHeader));
	
	for(x = 0; x < h->width; x++) {
		if(x > (x1 - x0))
			continue;
		
		c = (PColumn *)((long)columns[x] + p_resource);
		while(c->yofs != 255) {
			pixels = (BYTE *)((long)c + (long)sizeof(PColumn) +
							  (long)c->npixels + 1L);
			
			for(y = c->npixels; y >= 0; y--) {
				if((y + c->yofs) > (y1 - y0))
					pixels--;
				else
					putpixel(x + x0, y + y0 + c->yofs, *pixels--);
			}
			c = (PColumn *)((long)c + (long)(c->npixels) + 4L);
		} 
	}
	
	if(ForgetAfter)
		ForgetResource(picname);
}

void *GetResource(char *name)
{
	void *t = NULL;
	resource *p, *new = (resource *)NULL;
	MDirPtr r;
	
	for(p = ResourceList; p; p = p->next)
		if(!strcmp(name, p->name))
			return p->data;
	
	r = FindMasterDir(MasterDir, name);
	if(r && (t = GetMemory(r->dir.size))) {
		BasicWadSeek(r->wadfile, r->dir.start);
		BasicWadRead(r->wadfile, t, r->dir.size);
		
		new = (resource *)GetMemory(sizeof(resource));
		assert(new);
		new->data = t;
		new->name = strdup(name);
		new->next = ResourceList;
		ResourceList = new;
		return t;
	}
	return NULL;
}

void ForgetResource(char *name)
{
	resource *p, *q = NULL;
	
	for(p = ResourceList; p->next; q = p, p = p->next) {
		if(!strcmp(p->name, name)) {
			if(p == ResourceList)
				ResourceList = p->next;
			else
				q->next = p->next;
			
			FreeMemory(p->data);
			FreeMemory(p->name);
			FreeMemory(p);
			
			return;
		}
	}
}

void ForgetAllResources()
{
	resource *p, *q;
	for(p = ResourceList; p; p = q) {
		q = p->next;
		FreeMemory(p->data);
		FreeMemory(p->name);
		FreeMemory(p);
	}
	ResourceList = (resource *)NULL;
}


Texture *FindTexture(char *name)
{
	Texture *t;
	SList p;
	
	for(p = Texture_sections; p; p = p->next) {
		t = FindTextureIn(name, GetResource(p->string));
		if(t)
			return t;
	}
	return NULL;
}

Texture *FindTextureIn(char *name, void *tblock)
{
	long i, *offsets = tblock;
	Texture *t;
	
	if(tblock)
		for(i = 1; i <= offsets[0]; i++) {
			t = (Texture *)((long)offsets[i] + (long)tblock);
			if(!strncmp(t->name, name, 8))
				return t;
		}
	
	return NULL;
}

/*
   display a wall texture at coords x0, y0
   */

void DisplayWallTexture( BCINT x0, BCINT y0, BCINT x1, BCINT y1, char *texname) {
	BCINT	n, xsize, ysize;
	Texture  *tinfo;
	PatchDesc *pinfo;
	char	 picname[9];
	long patchname;
	void *Pnames;
	
	if (bioskey( 1) != 0)
		return; /* speedup */
	
	Pnames = GetResource("PNAMES");
	
	/* clear the box where the texture size will be drawn - see below */
	SetColor(DARKGRAY);
	DrawScreenBox( x0 - 171, y0 + 40, x0 - 82, y0 + 70);
	
	if (!(tinfo = FindTexture(texname)))
		return;
	
	/* read the info for this texture */
	xsize = tinfo->width;
	ysize = tinfo->height;
	
	SetColor( BLACK);
	DrawScreenText( x0 - 171, y0 + 40, "%dx%d", xsize, ysize);
	
	if (tinfo->nPatches == 1)
		DrawScreenText( x0 - 171, y0 + 60, "Transparent");
	else
		DrawScreenText( x0 - 171, y0 + 60, "%d Patches", tinfo->nPatches);
	
	if (bioskey( 1) != 0)
		return; /* speedup */
	
	if (x1 - x0 > xsize)
		x1 = x0 + xsize;
	if (y1 - y0 > ysize)
		y1 = y0 + ysize;
	
	/* not really necessary, except when xofs or yofs < 0 */
	setviewport( x0, y0, x1, y1, TRUE);
	
	/* display the texture */
	pinfo = (PatchDesc *)((long)tinfo + sizeof(Texture));
	ForgetAfter = FALSE;
	for (n = 0; n < tinfo->nPatches; n++, pinfo++) {
		
		/* OK, now look up the pic's name in Pnames. */
		patchname = (long)Pnames + 4L + ((long)(pinfo->PatchNumber) * 8);
		
		strncpy(picname, (char *)patchname, 8);
		picname[ 8] = '\0';
		/* coords changed because of the "setviewport" */
		DisplayPic( pinfo->xOffset, pinfo->yOffset,
				   x1 - x0, y1 - y0, strupr( picname));
	}
	
	/* de-allocate all patches */
	pinfo = (PatchDesc *)((long)tinfo + sizeof(Texture));
	for (n = 0; n < tinfo->nPatches; n++, pinfo++) {
		patchname = (long)Pnames + 4L + ((long)(pinfo->PatchNumber) * 8);
		strncpy(picname, (char *)patchname, 8);
		picname[ 8] = '\0';
		ForgetResource(picname);
	}
	/* restore the normal viewport */
	setviewport( 0, 0, ScrMaxX, ScrMaxY, TRUE);
}



/*
   Function to get the size of a wall texture
   It's a lot smaller now that we have functions to get a pointer
   to the texture structure automagically :-)
   */

void GetWallTextureSize( BCINT *xsize_r, BCINT *ysize_r, char *texname)
{
	Texture *t = FindTexture(texname);
	
	if(t) {
		*xsize_r = t->width;
		*ysize_r = t->height;
	}
	else {
		/* texture data not found */
		*xsize_r = -1;
		*ysize_r = -1;
	}
}




/*
   choose a floor or ceiling texture
   */

void ChooseFloorTexture( BCINT x0, BCINT y0, char *prompt, BCINT listsize, char **list, char *name)
{
	if (UseMouse)
		HideMousePointer(); 
	SwitchToVGA256();
	/* if we only have a 320x200x256 VGA driver, we must change x0 and y0.  Yuck! */
	if (GfxMode > -2) {
		x0 = -1;
		y0 = -1;
	}
	InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 5, name, 64, 64, DisplayFloorTexture);
	SwitchToVGA16();
	if (UseMouse)
		ShowMousePointer();
}



/*
   choose a wall texture
   */

void ChooseWallTexture( BCINT x0, BCINT y0, char *prompt, BCINT listsize, char **list, char *name)
{
	if (UseMouse)
		HideMousePointer();
	SwitchToVGA256();
	/* if we only have a 320x200x256 VGA driver, we must change x0 and y0.  Yuck! */
	if (GfxMode > -2) {
		x0 = 0;
		y0 = -1;
	}
	InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 11, name, 256, 128, DisplayWallTexture);
	SwitchToVGA16();
	if (UseMouse)
		ShowMousePointer();
}



/*
   function used by qsort to sort the sprite names
   */
int SortSprites( const void *a, const void *b)
{
	return strcmp( *((char **)a), *((char **)b));
}



/*
   choose a "sprite"
   */

void ChooseSprite( BCINT x0, BCINT y0, char *prompt, char *sname)
{
	MDirPtr dir;
	BCINT n, listsize;
	char **list;
	char name[ 9];
	
	/* count the names */
	dir = FindMasterDir( MasterDir, "S_START");
	dir = dir->next;
	for (n = 0; dir && strcmp(dir->dir.name, "S_END"); n++)
		dir = dir->next;
	listsize = n;
	/* get the actual names from master dir. */
	dir = FindMasterDir( MasterDir, "S_START");
	dir = dir->next;
	list = (char**) GetMemory( listsize * sizeof( char *));
	for (n = 0; n < listsize; n++) {
		list[ n] = (char*) GetMemory( 9 * sizeof( char));
		strncpy( list[ n], dir->dir.name, 8);
		list[ n][ 8] = '\0';
		dir = dir->next;
	}
	qsort( list, listsize, sizeof( char *), SortSprites);
	if (sname != NULL)
		strncpy( name, sname, 8);
	else
		strcpy( name, list[ 0]);
	/*   if (UseMouse)
		 HideMousePointer(); */
	SwitchToVGA256();
	/* if we only have a 320x200x256 VGA driver, we must change x0 and y0.  Yuck! */
	if (GfxMode > -2) {
		x0 = 0;
		y0 = -1;
	}
	ForgetAfter = TRUE;
	InputNameFromListWithFunc( x0, y0, prompt, listsize, list, 11, name, 256, 128, DisplayPic);
	/*   SwitchToVGA16();
		 if (UseMouse)
		 ShowMousePointer(); */
	for (n = 0; n < listsize; n++)
		FreeMemory( list[ n]);
	FreeMemory( list);
}
