/*
 * Generic byte order function, returns either LittleEndian or BigEndian.
 *
 * $Id: ByteOrder.hh,v 1.1 2001/08/28 16:03:55 burghart Exp $
 */

# include <stdlib.h>
# include <stdio.h>
# include <assert.h>

typedef enum _ByteOrder
{
    LittleEndian, BigEndian
} ByteOrder;


inline ByteOrder
GetByteOrder(void)
{
    assert (sizeof(long) == 4);
    
    char bytes[] = { 0, 1, 2, 3 };
    unsigned long x = *(long*)bytes;
    switch (x)
    {
      case 0x03020100:
	return LittleEndian;
      case 0x00010203:
	return BigEndian;
      default:
	fprintf( stderr, "Weird machine byte order 0x%08x\n", x );
	exit( 1 );
    }
}
