// NetReader.hh
// Network reader for monostatic radar data
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


# ifndef _NETREADER_HH_
# define _NETREADER_HH_

# include <cstdint>
# include <map>
# include "Receiver.hh"

//
// type to map from IP addresses to the associated Receiver
//
typedef std::map<uint32_t, Receiver*> ClientMap_t;



class NetReader
{
public:
    NetReader( uint16_t socknum );
// register a client who's expecting data from the net
    int Register( Receiver *client );
// which file descriptor have we opened?
    inline int FD( void ) const { return Fd; }
// read incoming data
    Receiver* Read( void );
private:
    int Socknum;
    int Fd;
    ClientMap_t ClientMap;
};

# endif // _NETREADER_HH_
