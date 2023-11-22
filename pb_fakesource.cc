// pb_fakesource.cc
// Crude simulator for a PIRAQ-based bistatic receiver
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
# include <cstring>
# include <cstdlib>
# include <unistd.h>
# include <cerrno>
# include <cassert>
# include <sys/time.h>
# include <csignal>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include "PB_Beam.hh"

//
// Make sure that PRT*HITS divides evenly into 60 * 8000000 (This forces
// a new beam to start at the top of each minute, and makes math
// bearable below.  Remember this is just a testing program anyway...)
//
const int PRT = 8000;	// * 1/8 us
const int HITS = 60;
const int NGATES = 150;

PB_Hdr Hdr;
int Socket;
struct sockaddr_in To;

void nextbeam( int junk );


int main( int argc, char *argv[] )
{
//
// Sanity checks
//
    if (argc != 2)
    {
	fprintf( stderr, "Usage: %s <ip_addr>\n", argv[0] );
	exit( 1 );
    }
    
    assert ((60 * 8000000) % (PRT * HITS) == 0);
//
// Open our outgoing socket
//
    if ((Socket = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
	fprintf( stderr, "Error %d creating or configuring command socket\n",
		 errno );
	exit (1);
    }
//
// Set up the recipient's address using the given IP address (must be in 
// network order) and the bistatic data port 0x6006
//
    memset( (char*)&To, 0, sizeof( To ) );
    To.sin_family = AF_INET;
    To.sin_port = htons( 0x6006 );
    inet_aton( argv[1], &To.sin_addr );
//
// Header stuff that will remain constant
//
    strncpy( Hdr.cookie, "PRQ", 3 );
    Hdr.version = (unsigned char)1;
    Hdr.azimuth = (unsigned short)0;
    Hdr.elevation = (unsigned short)0;
    Hdr.flag = (unsigned short)(0x20);
    Hdr.sync = (unsigned long)0;
    Hdr.qtr_firstgate = (unsigned char)0;
    Hdr.format = 1 << PBFld_V | 1 << PBFld_P_NCP;
    Hdr.ngates = NGATES;
    Hdr.hits = HITS - 1;
    Hdr.pulsewidth = 16;	// 2 us
    Hdr.prt = PRT;
    Hdr.delay = (unsigned short)0;
    Hdr.cmd = (unsigned char)0;
    Hdr.trailer = 'X';
    
    signal( SIGALRM, nextbeam );
    
    struct itimerval itv;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = (PRT * HITS) / 8;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 1;

    setitimer( ITIMER_REAL, &itv, 0 );

    while (1)
    {
	pause();
    }
}

    
void
nextbeam( int junk )
{
    static char* vel_data = 0;
    static char* p_ncp_data = 0;

    if (! vel_data)
    {
	char val;

	vel_data = new char[NGATES];
	val = 32;		// -128 to 127 => -nyquist to +nyquist
	memset( vel_data, val, NGATES );


	p_ncp_data = new char[NGATES];
	val = 36;		// 5-bit power + 3-bit NCP
	memset( p_ncp_data, val, NGATES );
    }
//
// Get current time and figure out the beam number within this minute.
// (The assertion in main guarantees that a new beam starts exactly at
// the top of each minute)
//
    struct timeval tv;
    gettimeofday( &tv, 0 );
    unsigned long minute = tv.tv_sec / 60;
    unsigned long minutesec = tv.tv_sec % 60;

    double d_beamnum = (8e6 * minutesec + 8 * tv.tv_usec) / (PRT * HITS);
    unsigned long beamnum = (unsigned long)d_beamnum;
//    printf( "%ld\n", beamnum );
    double beamtime = (double)(beamnum * PRT * HITS) / 8e6;
    unsigned long ibeamtime = (unsigned long) beamtime;
//
// Convert to the time needed in the header
//
    Hdr.time = minute * 60 + ibeamtime;
    Hdr.subsec = (short)((beamtime - ibeamtime) * 10000);
//
// 8 4-bit gates of power (first four zero)
//
    Hdr.power = random() & 0xffff;
//
// 8 4-bit gates of velocity
//
    Hdr.velocity = random();
//
// Send the header, then the data
//
    if (sendto( Socket, (void*)&Hdr, PB_HdrLen, 0, (struct sockaddr*)&To, 
		sizeof( To ) ) < 0 ||
	sendto( Socket, (void*)vel_data, NGATES, 0, (struct sockaddr*)&To, 
		sizeof( To ) ) < 0 ||
	sendto( Socket, (void*)p_ncp_data, NGATES, 0, (struct sockaddr*)&To, 
		sizeof( To ) ) < 0)
	if (errno != ECONNREFUSED)
	    fprintf( stderr, "Error %d sending data\n", errno );

    signal( SIGALRM, nextbeam );
}
