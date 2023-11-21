// DataType.hh
// Enumerated type for bistatic data typing
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
enum class DataType {
    DOUBLE, FLOAT, INT32, UINT16, INT16, UINT8, INT8
};

// map from each of the data types above to its associated size in bytes
static const std::map<DataType, std::size_t> DataTypeLen {
    { DataType::DOUBLE, sizeof(double) },
    { DataType::FLOAT, sizeof(float) },
    { DataType::INT32, sizeof(long) },
    { DataType::UINT16, sizeof(uint16_t) },
    { DataType::INT16, sizeof(int16_t) },
    { DataType::UINT8, sizeof(uint8_t) },
    { DataType::INT8, sizeof(int8_t) }
};

// Convert an array of one of our data types to an array of floats.
void MakeFloatArray( const void *rawdata, DataType type, int step,
		     float scale, float offset, float *fdata, int nvals );

# endif // _DATATYPE_HH_
