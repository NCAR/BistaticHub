// DataType.cc
// Enumerated type for bistatic data typing
//
// Copyright © 1999 Binet Incorporated
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

# include "DataType.hh"
# include "LE_Unpack.hh"

void
MakeFloatArray( const void *rawdata, DataType type, int step, float scale, 
		float offset, float *fdata, int nvals )
/*
 * Unpack raw data of the given type into a floating point array.  The step
 * tells the spacing in bytes between consecutive values to use from the
 * raw array.  Conversion takes the form: floatval = rawval * scale + offset.
 */
{
    int i;
    
    for (i = 0; i < nvals; i++)
    {
	const char *c = (const char*) rawdata + i * step;
	
	switch (type)
	{
	  case DataType::FLOAT:
	    fdata[i] = LE_Float( c ).Value();
	    break;
	  case DataType::INT32:
	    fdata[i] = LE_Long( c ).Value();
	    break;
	  case DataType::INT16:
	    fdata[i] = LE_Short( c ).Value();
	    break;
	  case DataType::UINT8:
	    fdata[i] = LE_UChar( c ).Value();
	    break;
	  case DataType::INT8:
	    fdata[i] = LE_Char( c ).Value();
	    break;
	}
	fdata[i] *= scale;
	fdata[i] += offset;
    }
}
