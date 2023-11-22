// PhaseRelayer.cc
// Class to relay per-pulse phases from the radar to receivers
//
// Copyright © 2000 Binet Incorporated
// Copyright © 2000 University Corporation for Atmospheric Research
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
# include <cstdlib>
# include <cerrno>
# include <cstring>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include "PhaseRelayer.hh"

PhaseRelayer::PhaseRelayer( int in_port, int out_port, 
			    const ReceiverList_t* rcvrs )
{
    struct sockaddr_in saddr;

    InPort = htons( in_port );		// convert to network order
    OutPort = htons( out_port );

    memset ((char*)&saddr, 0, sizeof( saddr ));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = InPort;

    if ((InFd = socket (AF_INET, SOCK_DGRAM, 0)) < 0 ||
	bind (InFd, (struct sockaddr*)&saddr, sizeof (saddr)))
    {
        fprintf( stderr, "Error creating socket for incoming phases\n" );
        exit( 1 );
    }

    if ((OutFd = socket (AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
        fprintf( stderr, "Error creating socket for outgoing phases\n" );
        exit( 1 );
    }
//
// Keep hold of the receiver list
//
    Rcvrs = rcvrs;
}



void 
PhaseRelayer::ReadAndRelay( void ) const
//
// Read a phase packet from our incoming socket and relay it to our
// receivers.
//
{
    const int buflen = 1024;
    char buf[buflen];
//
// Get the incoming phase data which should be waiting
//
    struct sockaddr_in from;
    unsigned int fromlen = sizeof( struct sockaddr_in );

    int nread = recvfrom( InFd, buf, buflen, 0, (struct sockaddr*)&from, 
			  &fromlen );
    if (nread < 0)
    {
	fprintf( stderr, "Error %d reading phases from radar!\n", errno );
	return;
    }
//
// Relay it
//
    Relay( buf, nread );
}



void
PhaseRelayer::Relay( const void* packet, int len ) const
{
//
// Now send the info to all but the transmitting radar (Rcvrs[0])
//
    for (int i = 1; i < Rcvrs->size(); i++)
    {
    	if (! (*Rcvrs)[i]->IsEnabled())
    	    continue;
	struct sockaddr_in to;

	memset( (char*)&to, 0, sizeof( to ) );
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = (*Rcvrs)[i]->IP_Address();
	to.sin_port = OutPort;

	int nsent = sendto( OutFd, packet, len, 0, (struct sockaddr*)&to, 
			    sizeof( to ) );

	if (nsent < 0)
	{
	    if (errno != ECONNREFUSED)
		fprintf( stderr, "Error %d sending phases to %x\n", errno, 
			 (*Rcvrs)[i]->IP_Address() );
	}
	else if (nsent < len)
	{
	    fprintf( stderr, "Short phase write to %x (%d < %d)\n", 
		     (*Rcvrs)[i]->IP_Address(), nsent, len );
	}
	
    }
}
