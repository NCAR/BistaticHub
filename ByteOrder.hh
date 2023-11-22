// ByteOrder.hh,v 1.1 2001/08/28 16:03:55 burghart Exp $
// Generic byte order function, returns either LittleEndian or BigEndian.
//
// Copyright © 2001 University Corporation for Atmospheric Research
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

# include <cstdlib>
# include <cstdio>
# include <cassert>

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
