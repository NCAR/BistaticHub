// V_BeamSource.hh
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

# ifndef _V_BEAMSOURCE_HH_
# define _V_BEAMSOURCE_HH_

# include "BeamSource.hh"

class Receiver;

class V_BeamSource : public BeamSource
{
public:
    V_BeamSource( const Receiver* rcvr );
    Beam *NextBeam( void );
    int AcceptData( const char *data, int len );
private:
    static const int MaxBufLen = 65536;	// max buffer len
    char Buffer[MaxBufLen];		// buffer for incoming data
    int BufLen;		// how much are we actually using?
    const Receiver* Rcvr;
};

# endif // _V_BEAMSOURCE_HH_
