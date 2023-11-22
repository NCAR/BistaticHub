//
// NetReader.cc
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

# include <map>
# include <cerrno>
# include <cstring>
# include <cstdlib>
# include <stdio.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include "NetReader.hh"
# include "Receiver.hh"


NetReader::NetReader( unsigned short snum )
//
// Open a port for reading network data, using the given socket number
// in local representation.
//
{
    Socknum = snum;
//
// Open a UDP socket for the given socket number
//
    struct sockaddr_in saddr;

    memset ((char*)&saddr, 0, sizeof( saddr ));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons( Socknum );

    if ((Fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0 ||
	bind (Fd, (struct sockaddr*)&saddr, sizeof (saddr)))
    {
	fprintf( stderr, 
		 "NetReader %d: Error creating or configuring socket\n",
		 Socknum );
	exit( 1 );	// fatal (for now?)
    }
}



int
NetReader::Register( Receiver* client )
//
// Register a client to receive data from its Receiver's IP address
//
{
    unsigned int ip_addr = client->IP_Address();
//
// Make sure we don't already have a client for this IP address
//
    if (ClientMap[ip_addr])
    {
	struct in_addr addr;
	addr.s_addr = ip_addr;
	fprintf( stderr, "NetReader %d: I already have a client for %s\n",
		 Socknum, inet_ntoa( addr ) );
	return 0;
    }
//
// Stash the client in our map
//
    ClientMap[ip_addr] = client;
    return 1;
}



Receiver*
NetReader::Read( void )
//
// Read anything waiting on our socket and pass it along to the appropriate
// client Receiver (if any), returning a pointer to the Receiver or NULL
//
{
    char buf[2048];
    struct sockaddr_in from;
    unsigned int fromlen = sizeof( from );
    
    int nread = recvfrom( Fd, buf, sizeof( buf ), 0, (struct sockaddr*) &from,
			  &fromlen );
    if (nread < 0)
    {
	fprintf( stderr, "NetReader %d: Error %d on data read\n", 
		 Socknum, errno );
	return 0;
    }
//
// If we have a client associated with the source IP address, pass along
// the new data.  Otherwise, complain and return.
//
    Receiver *client = ClientMap[from.sin_addr.s_addr];

    if (! client)
    {
	fprintf( stderr, "NetReader %d: Unexpected data received from %s\n",
		 Socknum, inet_ntoa( from.sin_addr ) );
	return 0;
    }
//
// Pass the data on to our client
//
    if (client->AcceptData( buf, nread ))
	return client;
    else
	return 0;
}

    
    
