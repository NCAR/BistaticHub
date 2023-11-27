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
# include <cstdint>
# include <iostream>
# include <iomanip>

typedef enum _ByteOrder
{
    LittleEndian, BigEndian
} ByteOrder;


inline ByteOrder
GetByteOrder(void)
{
    char bytes[] = { 0, 1, 2, 3 };
    uint32_t x = *(uint32_t*)bytes;

    switch (x)
    {
      case 0x03020100:
          return LittleEndian;
      case 0x00010203:
          return BigEndian;
      default:
          std::cerr << "Weird machine byte order 0x" << std::hex << std::setw(8)
                    << std::setfill('0') << x << std::endl;
          exit( 1 );
    }
}
