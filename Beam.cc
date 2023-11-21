// Beam.cc
// generic Beam class
//
// Copyright © 1999 Binet Incorporated
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

# include <cstdio>
# include "Beam.hh"

int
Beam::VelData( float* data, int maxlen, float badval ) const
{
    if (maxlen < Gates())
    {
	fprintf( stderr, "Beam::VelData: not enough data space\n" );
	return 0;
    }

    DataType type;
    int step;
    float scale, offset;
    const void *rawdata = VelRaw( &type, &step, &scale, &offset );

    if (rawdata)
	MakeFloatArray( rawdata, type, step, scale, offset, data, Gates() );
    else
	for (int i = 0; i < Gates(); i++)
	    data[i] = badval;
    
    return( 1 );
}



int
Beam::dBZData( float* data, int maxlen, float badval ) const
{
    if (maxlen < Gates())
    {
	fprintf( stderr, "Beam::dBZData: not enough data space\n" );
	return 0;
    }

    DataType type;
    int step;
    float scale, offset;
    const void *rawdata = dBZRaw( &type, &step, &scale, &offset );

    if (rawdata)
	MakeFloatArray( rawdata, type, step, scale, offset, data, Gates() );
    else
	for (int i = 0; i < Gates(); i++)
	    data[i] = badval;

    return( 1 );
}



int
Beam::dBmData( float* data, int maxlen, float badval ) const
{
    if (maxlen < Gates())
    {
	fprintf( stderr, "Beam::dBmData: not enough data space\n" );
	return 0;
    }

    DataType type;
    int step;
    float scale, offset;
    const void *rawdata = dBmRaw( &type, &step, &scale, &offset );

    if (rawdata)
	MakeFloatArray( rawdata, type, step, scale, offset, data, Gates() );
    else
	for (int i = 0; i < Gates(); i++)
	    data[i] = badval;

    return( 1 );
}



int
Beam::NCPData( float* data, int maxlen, float badval ) const
{
    if (maxlen < Gates())
    {
	fprintf( stderr, "Beam::NCPData: not enough data space\n" );
	return 0;
    }

    DataType type;
    int step;
    float scale, offset;
    const void *rawdata = NCPRaw( &type, &step, &scale, &offset );

    if (rawdata)
	MakeFloatArray( rawdata, type, step, scale, offset, data, Gates() );
    else
	for (int i = 0; i < Gates(); i++)
	    data[i] = badval;

    return( 1 );
}
