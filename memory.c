/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   MEMORY.C - Memory allocation routines.
*/

/* the includes */
#include "deu.h"

/*
   allocate memory with error checking
*/

void *GetMemory( size_t size)
{
   void *ret;

   ret = malloc( size);
   if (!ret)
   {
      /* retry after having freed some memory, if possible */
      FreeSomeMemory();
      ret = malloc( size);
   }
   if (!ret)
      ProgError( "out of memory (cannot allocate %u bytes)", size);
   return ret;
}



/*
   reallocate memory with error checking
*/

void *ResizeMemory( void *old, size_t size)
{
   void *ret;

   ret = realloc( old, size);
   if (!ret)
   {
      FreeSomeMemory();
      ret = realloc( old, size);
   }
   if (!ret)
      ProgError( "out of memory (cannot reallocate %u bytes)", size);
   return ret;
}



/*
   free memory
*/

void FreeMemory( void *ptr)
{
   /* just a wrapper around free(), but provide an entry point */
   /* for memory debugging routines... */
   free( ptr);
}


/*
   allocate memory from the far heap with error checking
*/

void huge *GetFarMemory( unsigned long size)
{
   void huge *ret;

   ret = farmalloc( size);
   if (!ret)
   {
      /* retry after having freed some memory, if possible */
      FreeSomeMemory();
      ret = farmalloc( size);
   }
   if (!ret)
      ProgError( "out of memory (cannot allocate %lu far bytes)", size);
   return ret;
}



/*
   reallocate memory from the far heap with error checking
*/

void huge *ResizeFarMemory( void huge *old, unsigned long size)
{
   void huge *ret;

   ret = farrealloc( old, size);
   if (!ret)
   {
      FreeSomeMemory();
      ret = farrealloc( old, size);
   }
   if (!ret)
      ProgError( "out of memory (cannot reallocate %lu far bytes)", size);
   return ret;
}



/*
   free memory from the far heap
*/

void FreeFarMemory( void huge *ptr)
{
   /* just a wrapper around farfree(), but provide an entry point */
   /* for memory debugging routines... */
   farfree( ptr);
}


/* end of file */
