//
// $Id: NetReader.cc,v 1.1 2000/08/29 21:24:03 burghart Exp $
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

# include <map>
# include <errno.h>
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

    
    
