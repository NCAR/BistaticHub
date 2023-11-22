// vb_fakesource.cc
// Crude simulator for a VIRAQ-based bistatic receiver
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
# include <cmath>
# include <sys/time.h>
# include <csignal>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include "VIRAQ_Beam.hh"

//
// Make sure that PRT*HITS divides evenly into 60 * 8000000 (This forces
// a new beam to start at the top of each minute, and makes math
// bearable below.  Remember that this is just a testing program...)
//
const int PRT = 8000;
const int HITS = 60;
const int NGATES = 250;

VIRAQ_Hdr Hdr;
VIRAQ_RHdr RHdr;
int Socket;

void nextbeam( int junk );
void sendout( const char* data, int len );


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
    if ((Socket = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
    {
	fprintf( stderr, "Error %d creating or configuring command socket\n",
		 errno );
	exit (1);
    }

    int one = 1;
    if (setsockopt(Socket, SOL_SOCKET, SO_BROADCAST, (void*)&one, 
		     sizeof(one)) != 0)
	fprintf( stderr, "Cannot set SO_BROADCAST flag for socket\n" );
//
// Set up the recipient's address using the given host name
// and the VIRAQ data port (0x6006)
//
    struct sockaddr_in to;
    memset( (char*)&to, 0, sizeof( to ) );
    to.sin_family = AF_INET;
    to.sin_port = htons( 0x6006 );
    inet_pton( AF_INET, argv[1], &to.sin_addr );

    if (connect( Socket, (struct sockaddr*)&to, sizeof( to ) ) < 0)
    {
	fprintf( stderr, 
		 "Error %d associating address %s with the data socket\n",
		 errno, argv[1] );
	exit( 1 );
    }
//
// per-beam header stuff that will remain constant
//
    strncpy( Hdr.desc, "DWEL", 4 );
    Hdr.recordlen = (short)(VIRAQ_HdrLen + NGATES * 12);
    Hdr.gates = (short)NGATES;
    Hdr.hits = (short)HITS;
    Hdr.rcvr_pulsewidth = 2.0e-6;
    Hdr.prt = PRT / 8000000.0;
    Hdr.delay = 0.0;
    Hdr.clutterfilter = (char)0;
    Hdr.timeseries = (char)0;
    Hdr.tsgate = (short)0;
    Hdr.radar_longitude = 0.0; 
    Hdr.radar_latitude = 0.0;
    Hdr.radar_altitude = 0.0;
    Hdr.ew_velocity = 0.0;
    Hdr.ns_velocity = 0.0;
    Hdr.vert_velocity = 0.0;
    Hdr.dataformat = (unsigned char)0;	// DATA_SIMPLEPP
    Hdr.prt2 = 0.0;
    Hdr.scan_type = (unsigned char)8;	// SUR
    Hdr.transition = (char)0;
    Hdr.hxmit_power = 1.0e6;
    Hdr.vxmit_power = 1.0e6;
    memset( Hdr.spare, 0, 100 );
//
// "Radar" header stuff; also constant
//
    strncpy( RHdr.desc, "RHDR", 4 );
    RHdr.recordlen = (short)VIRAQ_RHdrLen;
    RHdr.rev = (short)0;
    RHdr.year = (short)1999;
    strncpy( RHdr.radar_name, "BOGUS   ", 8 );
    RHdr.polarization = 'V';
    RHdr.test_pulse_pwr = 0.0;
    RHdr.test_pulse_frq = 0.0;
    RHdr.frequency = 5.5e9;
    RHdr.peak_power = 1.0e6;
    RHdr.noise_figure = 0.0;
    RHdr.noise_power = 0.0;
    RHdr.receiver_gain = 60.0;
    RHdr.data_sys_sat = -2.5;
    RHdr.antenna_gain = 40.0;
    RHdr.horz_beam_width = 1.0;
    RHdr.vert_beam_width = 1.0;
    RHdr.xmit_pulsewidth = 1.0e-6;
    RHdr.rconst = 70.0;
    RHdr.phaseoffset = 0.0;
    RHdr.vreceiver_gain = 60.0;
    RHdr.test_pulse_pwr = 0.0;
    RHdr.vantenna_gain = 40.0;
    memset( RHdr.misc, 0, 24 );
    memset( RHdr.text, 0, 960 );
//
// Catch the ALRM signal, and set an interval timer
//
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
    static int lastvol = -1;
    static char* buf = 0;
    static LE_Float* data = 0;
    
    if (! buf)
    {
	buf = new char[sizeof( Hdr ) + 3 * sizeof( LE_Float ) * NGATES];
	LE_Float* data = (LE_Float*)(buf + sizeof( Hdr ));
    //
    // A, B, P in "simplepp" form
    //
	float p = 1.0e17;
	float a = 0.5 * p;
	float b = 0.0 * p;

	for (int g = 0; g < NGATES; g++)
	{
	    data[3*g] = a;
	    data[3*g+1] = b;
	    data[3*g+2] = p;
	}
    }
//
// Get current time and figure out the beam number within this minute.
// (The assertion in main guarantees that a new beam starts exactly at
// the top of each minute)
//
    struct timeval tv;
    gettimeofday( &tv, 0 );
    unsigned long sec_in_minute = tv.tv_sec % 60;
    unsigned long beamnum = (8000000 * sec_in_minute + 8 * tv.tv_usec) /
	(PRT * HITS);
//
// Get the time for this beam and put it in the header
//
    unsigned long minute = tv.tv_sec / 60;
    double beamtime = (double)(beamnum * PRT * HITS) / 8e6;
    unsigned long ibeamtime = (unsigned long) beamtime;

    Hdr.time = minute * 60 + ibeamtime;
    Hdr.subsec = (short)((beamtime - ibeamtime) * 10000);
//
// Two minute volumes with four sweeps per volume, 500 beams/sweep,
// and elevations: 0.5, 1.5, 2.5, and 3.5 degrees.
//
    int sweep = (tv.tv_sec / 30) % 4;
    Hdr.az = (beamnum % 500) / 500.0 * 360.0;
    Hdr.el = sweep + 0.5;
    Hdr.fxd_angle = Hdr.el;
    Hdr.scan_num = sweep;
    Hdr.vol_num = (unsigned char)((minute/2) & 0xff);
    Hdr.ray_count = beamnum & 0x7fffffff;
//
// Send out a RHdr before each volume
//
    if (Hdr.vol_num.Value() != lastvol)
    {
	lastvol = Hdr.vol_num.Value();
	sendout( (char*)&RHdr, VIRAQ_RHdrLen );
    }
//
// Send the data header and data
//
    memmove( buf, &Hdr, sizeof( Hdr ) );
    sendout( buf, sizeof( Hdr ) + 3 * sizeof( LE_Float ) * NGATES );

    signal( SIGALRM, nextbeam );
}



void
sendout( const char* data, int len )
{
    int nsent = 0;
    const int PKTSIZE = 1000;	// break data into packets of this size
    static int pktsequence = 0;
//
// per-packet header
//
    typedef struct _UDPHeader
    {
	short type;	// first or continue
	long sequence;
	short frames_per_radial;
	short frame_num;
    } UDPHeader;
    UDPHeader udphdr = { 0, 0, 0, 0 };

    char buf[PKTSIZE + sizeof( UDPHeader )];

    int npkts = (len + PKTSIZE - 1) / PKTSIZE;
    for (int pkt = 0; pkt < npkts; pkt++)
    {
	udphdr.sequence = pktsequence++;
	udphdr.frames_per_radial = npkts;
	udphdr.frame_num = pkt;
    
	int dstart = pkt * PKTSIZE;
	int dlen = (len - dstart) > 1000 ? 1000 : (len - dstart);

	memmove( buf, &udphdr, sizeof( udphdr ) );
	memmove( buf + sizeof( udphdr ), data + dstart, dlen );
	
	int n = send( Socket, buf, dlen + sizeof( udphdr ), 0 );
    //
    // Exit on errors, except "Connection refused"
    //
	if (n < 0 && errno != ECONNREFUSED)
	{
	    perror("data send");
	    exit( 1 );
	}
    }
}
