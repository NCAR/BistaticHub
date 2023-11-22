// BeamSource.hh
// BeamSource is used to construct and provide Beam-s of radar data from raw
// radar data streams.
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


// BeamSource Interface:
//
//	AcceptData( const char* data, int len )
//		accept 'len' bytes of data, appending it to the data we 
//		already have
//
//	NextBeam( void )
//		return a good beam if we have enough data to make one,
//		clearing any data used to build it from our buffer.
//		NextBeam() should set the LastDataTime member to time(0) 
//		when a good beam is returned.
//
//
# ifndef _BEAMSOURCE_HH_
# define _BEAMSOURCE_HH_

# include <time.h>

class Beam;

class BeamSource
{
protected:
    time_t LastDataTime;
public:
    virtual Beam *NextBeam( void ) = 0;
    virtual int AcceptData( const char *data, int len ) = 0;
    time_t TimeSinceLastData( void ) const 
	{ return( time( 0 ) - LastDataTime ); }
};

# endif // _BEAMSOURCE_HH_
