// CommandSender.cc
// Class to handle sending commands to bistatic receivers
//
// Copyright © 1998 Binet Incorporated
// Copyright © 1998 University Corporation for Atmospheric Research
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

# include <stdio.h>
# include <stdlib.h>
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
