/*
   Memory swapping by Rapha‰l Quinet <quinet@montefiore.ulg.ac.be>
		  and Christian Johannes Schladetsch <s924706@yallara.cs.rmit.OZ.AU>

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   SWAPMEM.C - When the memory is low....

   Note from RQ:
      Yuck!  I don't like this horrible thing.  The program should be
      able to swap almost anything to XMS or to disk, not only the
      five objects used here (Things, LineDefs, SideDefs, Vertexes and
      Sectors).  That was a quick and dirty hack...  I didn't have the
      time to write a cleaner code...

   Note2 from RQ:
      After having tested these routines, I see that they are not very
      useful...  I'm still getting "out of memory" errors while editing
      E2M7 and other huge levels.  I should rewrite all this for GCC,
      use a flat memory model and a DOS extender, then just delete all
      this code...  I will have to do that anyway if I want to port it
      to other systems (Unix, Linux), so why not?
      Moral of the story: never waste long hours writing high-level
      memory swapping routines on a deficient OS.  Use a real OS with
      a better memory management instead.

   Note for CJS:
      It should be easy to include your XMS code in this file.  Just
      add the necessary lines in InitSwap(), SwapIn() and SwapOut().
      You won't need to edit any other file.  Put all your routines
      in XMS.C, with the necessary includes in XMS.H.  Please keep it
      short and simple... :-)
      ... And delete this note once you're done.
*/

/* the includes and typedefs */
#include "deu.h"
#include "levels.h"
#ifdef SWAP_TO_XMS
#include "xms.h"
typedef XMSHandle SwapHandle;	/* XMS handle */
#define INVALID_HANDLE		-1
#else
typedef char * SwapHandle;	/* name of the temporary disk file */
#define INVALID_HANDLE		NULL
#endif /* SWAP_TO_XMS */

/* global variables */
Bool NeedThings = FALSE;
Bool NeedLineDefs = FALSE;
Bool NeedSideDefs = FALSE;
Bool NeedVertexes = FALSE;
Bool NeedSectors = FALSE;
SwapHandle ThingsH;
SwapHandle LineDefsH;
SwapHandle SideDefsH;
SwapHandle VertexesH;
SwapHandle SectorsH;


/*
   do the necessary initialisation for the secondary storage
*/

void InitSwap()
{
#ifdef SWAP_TO_XMS
   /* Init XMS */
   ...
#endif /* SWAP_TO_XMS */
   ThingsH = INVALID_HANDLE;
   LineDefsH = INVALID_HANDLE;
   SideDefsH = INVALID_HANDLE;
   VertexesH = INVALID_HANDLE;
   SectorsH = INVALID_HANDLE;
}



/*
   moves an object from secondary storage to lower RAM
*/

void huge *SwapIn( SwapHandle handle, unsigned long size)
{
   void huge *ptr;
#ifdef SWAP_TO_XMS
   /* allocate a new memory block (in lower RAM) */
   ptr = GetFarMemory( size);
   /* read the data from XMS */
   ...
   /* free the XMS memory block */
   ...
   /* delete the handle */
   ...
#else
   FILE      *file;
   char huge *data;

   if (handle == INVALID_HANDLE)
      ProgError( "BUG: SwapIn called with an invalid handle");
#ifdef DEBUG
   LogMessage( "swapping in %lu bytes from %s\n", size, handle);
#endif /* DEBUG */
   /* allocate a new memory block (in lower RAM) */
   ptr = GetFarMemory( size);
   /* read the data from the temporary file */
   data = ptr;
   file = fopen( handle, "rb");
   if (file == NULL)
      ProgError( "error opening temporary file \"%s\"", handle);
   while (size > 0x8000)
   {
      if (fread( data, 1, 0x8000, file) != 0x8000)
	 ProgError( "error reading from temporary file \"%s\"", handle);
      data = data + 0x8000;
      size -= 0x8000;
   }
   if (fread( data, 1, size, file) != size)
      ProgError( "error reading from temporary file \"%s\"", handle);
   fclose( file);
   /* delete the file */
   unlink( handle);
   /* free the handle (file name) */
   FreeMemory( handle);
#endif /* !SWAP_TO_XMS */
   return ptr;
}



/*
   moves an object from lower RAM to secondary storage
*/

SwapHandle SwapOut( void huge *ptr, unsigned long size)
{
   SwapHandle handle;
#ifdef SWAP_TO_XMS
   /* get a new XMS handle */
   ...
   /* write the data to XMS */
   ...
#else
   FILE      *file;
   char huge *data;

   /* get a new (unique) file name */
   handle = tempnam( NULL, "{DEU}");
   if (handle == NULL)
      ProgError( "cannot create a temporary file name (out of memory)");
#ifdef DEBUG
   LogMessage( "swapping out %lu bytes to %s\n", size, handle);
#endif /* DEBUG */
   /* write the data to the temporary file */
   data = ptr;
   file = fopen( handle, "wb");
   if (file == NULL)
      ProgError( "error creating temporary file \"%s\"", handle);
   while (size > 0x8000)
   {
      if (fwrite( data, 1, 0x8000, file) != 0x8000)
	 ProgError( "error writing to temporary file \"%s\"", handle);
      data = data + 0x8000;
      size -= 0x8000;
   }
   if (fwrite( data, 1, size, file) != size)
      ProgError( "error writing to temporary file \"%s\"", handle);
   fclose( file);
#endif /* !SWAP_TO_XMS */
   /* free the data block (in lower RAM) */
   FreeFarMemory( ptr);
   return handle;
}



/*
   get the objects needed (if they aren't already in memory)
*/

void SwapInObjects()
{
   if (NeedThings && NumThings > 0 && Things == NULL)
      Things = SwapIn( ThingsH, (unsigned long) NumThings * sizeof (struct Thing));
   if (NeedLineDefs && NumLineDefs > 0 && LineDefs == NULL)
      LineDefs = SwapIn( LineDefsH, (unsigned long) NumLineDefs * sizeof (struct LineDef));
   if (NeedSideDefs && NumSideDefs > 0 && SideDefs == NULL)
      SideDefs = SwapIn( SideDefsH, (unsigned long) NumSideDefs * sizeof (struct SideDef));
   if (NeedVertexes && NumVertexes > 0 && Vertexes == NULL)
      Vertexes = SwapIn( VertexesH, (unsigned long) NumVertexes * sizeof (struct Vertex));
   if (NeedSectors && NumSectors > 0 && Sectors == NULL)
      Sectors = SwapIn( SectorsH, (unsigned long) NumSectors * sizeof (struct Sector));
}


/*
   mark the objects that should be in lower RAM
*/

void ObjectsNeeded( int objtype, ...)
{
   va_list args;

   /* get the list of objects */
   NeedThings = FALSE;
   NeedLineDefs = FALSE;
   NeedSideDefs = FALSE;
   NeedVertexes = FALSE;
   NeedSectors = FALSE;
   va_start( args, objtype);
   while (objtype > 0)
   {
      switch (objtype)
      {
      case OBJ_THINGS:
	 NeedThings = TRUE;
	 break;
      case OBJ_LINEDEFS:
	 NeedLineDefs = TRUE;
	 break;
      case OBJ_SIDEDEFS:
	 NeedSideDefs = TRUE;
	 break;
      case OBJ_VERTEXES:
	 NeedVertexes = TRUE;
	 break;
      case OBJ_SECTORS:
	 NeedSectors = TRUE;
	 break;
      }
      objtype = va_arg( args, int);
   }
   va_end( args);
   /* get the objects if they aren't already in memory */
   SwapInObjects();
}



/*
   free some memory by moving some objects out of lower RAM
*/

void FreeSomeMemory()
{
#ifdef OLD_FREEMEM
   if (!NeedThings && NumThings > 0 && Things != NULL)
   {
      ThingsH = SwapOut( Things, (unsigned long) NumThings * sizeof (struct Thing));
      Things = NULL;
   }
   if (!NeedLineDefs && NumLineDefs > 0 && LineDefs != NULL)
   {
      LineDefsH = SwapOut( LineDefs, (unsigned long) NumLineDefs * sizeof (struct LineDef));
      LineDefs = NULL;
   }
   if (!NeedSideDefs && NumSideDefs > 0 && SideDefs != NULL)
   {
      SideDefsH = SwapOut( SideDefs, (unsigned long) NumSideDefs * sizeof (struct SideDef));
      SideDefs = NULL;
   }
   if (!NeedVertexes && NumVertexes > 0 && Vertexes != NULL)
   {
      VertexesH = SwapOut( Vertexes, (unsigned long) NumVertexes * sizeof (struct Vertex));
      Vertexes = NULL;
   }
   if (!NeedSectors && NumSectors > 0 && Sectors != NULL)
   {
      SectorsH = SwapOut( Sectors, (unsigned long) NumSectors * sizeof (struct Sector));
      Sectors = NULL;
   }
#else
   /* move everything to secondary storage */
   if (NumThings > 0 && Things != NULL)
   {
      ThingsH = SwapOut( Things, (unsigned long) NumThings * sizeof (struct Thing));
      Things = NULL;
   }
   if (NumLineDefs > 0 && LineDefs != NULL)
   {
      LineDefsH = SwapOut( LineDefs, (unsigned long) NumLineDefs * sizeof (struct LineDef));
      LineDefs = NULL;
   }
   if (NumSideDefs > 0 && SideDefs != NULL)
   {
      SideDefsH = SwapOut( SideDefs, (unsigned long) NumSideDefs * sizeof (struct SideDef));
      SideDefs = NULL;
   }
   if (NumVertexes > 0 && Vertexes != NULL)
   {
      VertexesH = SwapOut( Vertexes, (unsigned long) NumVertexes * sizeof (struct Vertex));
      Vertexes = NULL;
   }
   if (NumSectors > 0 && Sectors != NULL)
   {
      SectorsH = SwapOut( Sectors, (unsigned long) NumSectors * sizeof (struct Sector));
      Sectors = NULL;
   }
   /* re-load the objects that are needed */
   SwapInObjects();
#endif /* OLD_FREEMEM */
}


/* end of file */
