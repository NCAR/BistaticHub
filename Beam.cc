//
// Generic Beam class
//
// $Id: Beam.cc,v 1.1 2000/08/29 21:23:58 burghart Exp $
//
// Copyright (C) 1998
// Binet Incorporated 
//       and 
// University Corporation for Atmospheric Research
// 
// All rights reserved
//
// No part of this work covered by the copyrights herein may be reproduced
// or used in any form or by any means -- graphic, electronic, or mechanical,
// including photocopying, recording, taping, or information storage and
// retrieval systems -- without permission of the copyright owners.
//
// This software and any accompanying written materials are provided "as is"
// without warranty of any kind.
//

# include <stdio.h>
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
