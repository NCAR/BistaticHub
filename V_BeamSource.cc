// V_BeamSource.cc
// BeamSource implementation for a VIRAQ-based bistatic receiver
//
// Copyright © 1999 Binet Incorporated
// Copyright © 1999 University Corporation for Atmospheric Research
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

# include <cassert>
# include <cstdio>
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
