//
// $Id: V_BeamSource.cc,v 1.1 2000/08/29 21:24:07 burghart Exp $
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
# include "V_BeamSource.hh"
# include "VIRAQ_Beam.hh"

V_BeamSource::V_BeamSource( const Receiver* rcvr )
{
    LastDataTime = time( 0 );
    BufLen = 0;
    Rcvr = rcvr;
}


Beam*
V_BeamSource::NextBeam( void )
//
// Return a pointer to the next beam if we can generate a good one, otherwise
// return 0.  A good beam must be deleted by the caller.
//
{
    VIRAQ_Beam *beam = new VIRAQ_Beam( Buffer, BufLen, Rcvr );

    if (beam->OK())
    {
	LastDataTime = time( 0 ); // *our* time, not the source's
	return beam;
    }
    else
    {
	delete beam;
	return 0;
    }
}


int
V_BeamSource::AcceptData( const char *data, int len )
{
    static long lastone = -1;
    long thisone;
//
// Strip off the packet header and verify that this packet is sequential
//
    typedef struct _enHeader
    {
	short type;	// first or continue
	long sequence;
	short frames_per_radial;
	short frame_num;
    } enHeader;

    const enHeader* hdr = (enHeader*) data;
    data += sizeof( enHeader );
    len -= sizeof( enHeader );

    thisone = hdr->sequence;
    if (lastone >= 0 && thisone != lastone + 1)
    {
	fprintf( stderr, 
		 "V_BeamSource: out-of-sequence packet (%d != %d+1)\n", 
		 thisone, lastone );
	BufLen = 0;
    }
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
	fprintf( stderr, "V_BeamSource: dropped %d old bytes\n", diff );
	BufLen -= diff;
    }
//
// Add the new data to the end of our buffer
//
    memmove( Buffer + BufLen, data, len );
    BufLen += len;

    return 1;
}
