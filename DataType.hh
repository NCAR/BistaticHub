// DataType.hh
// Tools for bistatic data typing
//
// Copyright © 1998 Binet Incorporated
// Copyright © 1998 University Corporation for Atmospheric Research
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

# ifndef _DATATYPE_HH_
# define _DATATYPE_HH_

#include <cstdint>
#include <map>

// Data types we might return
enum DataType {
    DT_Double, DT_Float, DT_Long, DT_UnsignedShort, DT_Short,
    DT_UnsignedChar, DT_Char
};

// map from each of the data types above to its associated size in bytes
static const std::map<DataType, std::size_t> DataTypeLen {
    { DT_Double, sizeof(double) },
    { DT_Float, sizeof(float) },
    { DT_Long, sizeof(long) },
    { DT_UnsignedShort, sizeof(uint16_t) },
    { DT_Short, sizeof(int16_t) },
    { DT_UnsignedChar, sizeof(uint8_t) },
    { DT_Char, sizeof(int8_t) }
};

// Convert an array of one of our data types to an array of floats.
void MakeFloatArray( const void *rawdata, DataType type, int step,
		     float scale, float offset, float *fdata, int nvals );

# endif // _DATATYPE_HH_
