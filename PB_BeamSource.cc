//
// $Id: PB_BeamSource.cc,v 1.1 2000/08/29 21:24:04 burghart Exp $
//
// Copyright (C) 1999
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

# include <assert.h>
# include <stdio.h>
# include "PB_BeamSource.hh"
# include "PB_Beam.hh"

PB_BeamSource::PB_BeamSource( const Receiver* rcvr )
{
    LastDataTime = time( 0 );
    BufLen = 0;
    Rcvr = rcvr;
}


Beam*
PB_BeamSource::NextBeam( void )
//
// Return a pointer to the next beam if we can generate a good one, otherwise
// return 0.  A good beam must be deleted by the caller.
//
{
    PB_Beam *beam = new PB_Beam( Buffer, BufLen, Rcvr );

    if (beam->OK())
    {
	LastDataTime = time (0); // using *our* clock, not the source's!
	return beam;
    }
    else
    {
	delete beam;
	return 0;
    }
}


int
PB_BeamSource::AcceptData( const char *data, int len )
{
//
// We assume that we'll never get more than MaxBufLen at a time...
//
    assert( len < MaxBufLen );
//
// Make sure we have enough space to hold the incoming data
//
    if ((len + BufLen) > MaxBufLen)
    {
	int diff = len + BufLen - MaxBufLen;
	memmove( Buffer, Buffer + diff, diff );
	fprintf( stderr, "PB_BeamSource: dropped %d old bytes\n", diff );
	BufLen -= diff;
    }
//
// Add the new data to the end of our buffer
//
    memmove( Buffer + BufLen, data, len );
    BufLen += len;

    return 1;
}
