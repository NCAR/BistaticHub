// PB_BeamSource.cc
// BeamSource implementation for PIRAQ-based systems
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
