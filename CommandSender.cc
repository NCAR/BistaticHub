//
// $Id: CommandSender.cc,v 1.1 2000/08/29 21:23:59 burghart Exp $
//
// Copyright (C) 1998
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

# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include "CommandSender.hh"


CommandSender::CommandSender( void )
{
//
// Open a UDP socket for sending commands to bistatic receivers
//
    if ((Socket = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
	fprintf( stderr, "Error %d creating or configuring command socket\n",
		 errno );
	exit (1);
    }
}


void
CommandSender::Send( const char *cmd, unsigned long ip_addr )
{
    struct sockaddr_in to;
//
// Set up the recipient's address using the given IP address (must be in 
// network order) and the bistatic command port 21001
//
    memset( (char*)&to, 0, sizeof( to ) );
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = ip_addr;
    to.sin_port = htons (21001);
//
// Send the command
//
    sendto( Socket, (void*)cmd, strlen( cmd ), 0, (struct sockaddr*)&to, 
	    sizeof( to ) );
}
