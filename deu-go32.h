/*
   Doom Editor Utility, by Brendon Wyber and Rapha‰l Quinet.

   You are allowed to use any parts of this code in another program, as
   long as you give credits to the authors in the documentation and in
   the program itself.  Read the file README.1ST for more information.

   This program comes with absolutely no warranty.

   DEU-GO32.H - Some DJGPP/GO32 specific things.

   Originally written by Per Allansson (c91peral@und.ida.liu.se)

   Changes:

           940505: (from Per Kofod)

                   - farcoreleft() now returns the size of the free
                     memory (not virtual, but 'real' memory...)
                   - changed handling of gfx-modes to match that of
                     deu/16. (and removed -x switch)
                   - also changed handling of colors to remove
                     the annoying white spots.


           940508: (from Per Kofod)

                   - changed BCINT ---> int in SelectLevel(...)

                 Have looked for bug with flags f. linedef. Couldn't
                 find any... Try harder...


           940509: (several people (Tom Neff, Barry ...) have reported
                   that there were problems when editing LineDefs.)

                   - editobj.c , when edit. linedefs, use of uninitialized
                     ptr 'cur'  (line 86x)

                    (the funny thing was that the error ONLY occured if
                     the source was compiled with '-O2' ... )


           940511: (Per Allansson)

                   - removed swapmem.c completely and defined some
                     nice macros instead.

           940512: - updated to BETA 2

           940513: - fixed 'drag'-bug
                   - fixed 'sector'-bug
                   - moved memory.c to deu-go32.h (sort of)
                   - removed ';' in objects.  (oops!)

           940516: - updated to BETA 3 (did NOT include Cirrus patches!!!)

           940518: - updated to final DEU 5.2 release (by Per Kofod)

           940520: - updated to final final DEU 5.2 release (by Per Allansson)

                     - fixed "crash-when-no-doom.wad-file" bug
                     - fixed "you-have-always-unclosed-sectors" bug

                       (learn: be careful with copy/paste ....)

*/




#ifndef DEU_GO32_H
#define DEU_GO32_H
#if defined(__GNUC__)

#include <pc.h>       /* For sound,nosound */
#include <libbcc.h>   /* For all borl**d-gfx */
#include <dpmi.h>     /* for _go32....memory() */

#define huge          /* No huge things in gcc */
                      /* Should be done in another way, but ... */


#ifdef __cplusplus
extern "C" {
#endif
extern int getch(void);
extern int bioskey(int);
extern void delay(unsigned);
#ifdef __cplusplus
}
#endif


#ifndef max
#define max(x,y)   (((x)>(y))?(x):(y))
#define min(x,y)   (((x)<(y))?(x):(y))
#endif

#define farcoreleft() ((long)_go32_dpmi_remaining_physical_memory())


#define FreeSomeMemory()
#define ObjectsNeeded(x,y...)   /* GNU CPP ONLY !!! :-) */
#define InitSwap()

#define GetMemory(x)         malloc(x)
#define FreeMemory(x)        free(x)
#define ResizeMemory(x,y)    realloc(x,y)
#define GetFarMemory(x)      malloc(x)
#define FreeFarMemory(x)     free(x)
#define ResizeFarMemory(x,y) realloc(x,y)

#endif  /* __GNUC__   */
#endif  /* DEU_GO32_H */



