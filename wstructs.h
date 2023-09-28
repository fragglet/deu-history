/*
   Doom Editor Utility, by Brendon Wyber and Rapha�l Quinet.

   If you use any part of this code in one of your programs,
   please make it clear that you borrowed it from here...

   WSTRUCTS.H - WAD files data structures.
*/


/*
   this data structure contains the information about the THINGS
*/

struct Thing
{
   int xpos;      /* x position */
   int ypos;      /* y position */
   int angle;     /* facing angle */
   int type;      /* thing type */
   int when;      /* appears when? */
};
typedef struct Thing huge *TPtr;



/*
   this data structure contains the information about the LINEDEFS
*/
struct LineDef
{
   int start;     /* from this vertex ... */
   int end;       /* ... to this vertex */
   int flags;     /* see NAMES.C for more info */
   int type;      /* see NAMES.C for more info */
   int tag;       /* crossing this linedef activates the sector with the same tag */
   int sidedef1;  /* sidedef */
   int sidedef2;  /* only if this line adjoins 2 sectors */
};
typedef struct LineDef huge *LDPtr;



/*
   this data structure contains the information about the SIDEDEFS
*/
struct SideDef
{
   int xoff;      /* X offset for texture */
   int yoff;      /* Y offset for texture */
   char tex1[8];  /* texture name for the part above */
   char tex2[8];  /* texture name for the part below */
   char tex3[8];  /* texture name for the regular part */
   int sector;    /* adjacent sector */
};
typedef struct SideDef huge *SDPtr;



/*
   this data structure contains the information about the VERTEXES
*/
struct Vertex
{
   int x;         /* X coordinate */
   int y;         /* Y coordinate */
};
typedef struct Vertex huge *VPtr;



/*
   this data structure contains the information about the SEGS
*/
typedef struct Seg huge *SEPtr;
struct Seg
{
   SEPtr next;    /* next Seg in list */
   int start;     /* from this vertex ... */
   int end;       /* ... to this vertex */
   unsigned angle;/* angle (0 = east, 16384 = north, ...) */
   int linedef;   /* linedef that this seg goes along*/
   int flip;      /* true if not the same direction as linedef */
   unsigned dist; /* distance from starting point */
};



/*
   this data structure contains the information about the SSECTORS
*/
typedef struct SSector huge *SSPtr;
struct SSector
{
   SSPtr next;	  /* next Sub-Sector in list */
   int num;       /* number of Segs in this Sub-Sector */
   int first;     /* first Seg */
};



/*
   this data structure contains the information about the NODES
*/
typedef struct Node *NPtr;
struct Node
{
   int x, y;                         /* starting point */
   int dx, dy;                       /* offset to ending point */
   int miny1, maxy1, minx1, maxx1;   /* bounding rectangle 1 */
   int miny2, maxy2, minx2, maxx2;   /* bounding rectangle 2 */
   int child1, child2;               /* Node or SSector (if high bit is set) */
   NPtr node1, node2;                /* pointer if the child is a Node */
   int num;                          /* number given to this Node */
};



/*
   this data structure contains the information about the SECTORS
*/
struct Sector
{
   int floorh;    /* floor height */
   int ceilh;     /* ceiling height */
   char floort[8];/* floor texture */
   char ceilt[8]; /* ceiling texture */
   int light;     /* light level (0-255) */
   int special;   /* special behaviour (0 = normal, 9 = secret, ...) */
   int tag;       /* sector activated by a linedef with the same tag */
};
typedef struct Sector huge *SPtr;



/* end of file */
