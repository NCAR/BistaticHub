//
// $Id: Receiver.cc,v 1.1 2000/08/29 21:24:06 burghart Exp $
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

# include <stdio.h>
# include <string.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include "Receiver.hh"
# include "Beam.hh"
# include "PB_Beam.hh"
# include "NetReader.hh"
# include "PB_BeamSource.hh"
# include "V_BeamSource.hh"




Receiver::Receiver( const char *line, char id, NetReader* nr )
//
// This constructor is for a Receiver which delivers its data via a
// network socket, which is owned by the given NetReader.
//
{
    char site[80], fmtname[40], ipstring[24];
    float az, range;
//
// Receiver definition line should contain exactly five elements:
//	site name: quoted string
//	format: VIRAQ or PIRAQ
//	azimuth: degrees w.r.t. transmitting radar
//	range: distance from transmitting radar in km
//	IP address: in dot notation
//
    int nelems = sscanf( line, "%s%s%f%f%s", site, fmtname, &az, 
			 &range, ipstring );
    if (nelems != 5)
    {
	fprintf( stderr, "Bad receiver line '%s'\n", line );
	exit( 1 );
    }
//
// Site name
//
    Sitename = new char[strlen( site ) + 1];
    strcpy( Sitename, site );
//
// Translate the format name string to either FmtPB or FmtVIRAQ
//
    if (! strcmp( fmtname, "PIRAQ" ))
        Fmt = FmtPB;
    else if (! strcmp( fmtname, "VIRAQ" ))
	Fmt = FmtVIRAQ;
    else
    {
	fprintf( stderr, "Unknown receiver data format '%s' for %s", 
		 fmtname, site );
	exit( 1 );
    }
//
// Save the az and range
//
    Az = az;
    Range = range;
//
// Translate the IP address string to network order integer representation
//
    struct in_addr addr;
    if (! inet_aton( ipstring, &addr ))
    {
	fprintf( stderr, "Bad IP address %s for %s\n", addr, site );
	exit( 1 );
    }

    IPAddr = addr.s_addr;
//
// Register with our NetReader
//
    nr->Register( this );
//
// A few initializations
//
    Identifier = id;
    Count = 0;
    DebugLvl = 0;
//
// We're enabled to start with
//
    Enable();
}



void
Receiver::Enable( void )
//
// Enable - set up to accept incoming data
//
{
//
// If we're already enabled, just return
//
    if (Enabled)
	return;
//
// Create our BeamSource object
//
    switch (Fmt)
    {
      case FmtPB:
	Src = new PB_BeamSource( this );
	break;
      case FmtVIRAQ:
	Src = new V_BeamSource( this );
	break;
      default:
	fprintf( stderr, "Huh?  Bad format %d in Receiver::Enable\n" );
	exit( 1 );
    }

    Enabled = 1;
}



void
Receiver::Disable( void )
//
// Disable - drop any incoming data
//
{
//
// Delete our BeamSource, so we get rid of any currently existing data
//
    if (Src)
	delete Src;

    Src = 0;
    Enabled = 0;
}



int 
Receiver::AcceptData( const char* data, int len )
//
// Pass incoming data on to our BeamSource, unless we're disabled
//
{
    if (! Enabled)
	return 1;
//
// Debugging info
//
    if (DebugLvl > 1)
    {
    	fprintf( stderr, "%c", Identifier );
	fflush( stderr );
    }

    return Src->AcceptData( data, len );
}

    
Beam*
Receiver::NextBeam( void )
//
// Try to return the next beam from our source, incrementing our beam
// count if we get a good one.
//
{
    Beam* beam;
    
    if (! Enabled || !(beam = Src->NextBeam()))
	return 0;
//
// Update the receiver info once in a while
//
    Count++;
    if (! (Count % 10))
	ReportStatus( beam );
//
// Maybe print some information based on the debug level
//
    switch (DebugLvl)
    {
      case 0:
      case 1:
	break;
      case 2:
	fprintf( stderr, "(%c)", Identifier );
	fflush( stderr );
	break;
      default:
	fprintf( stderr, 
		 "(%c)  t: %.3lf  pwidth: %.3e  ng: %d  prt: %.4e\n", 
		 Identifier, beam->Time(), beam->RcvrPulseWidth(), 
		 beam->Gates(), beam->PRT() );
    }

    return beam;
}



void
Receiver::ReportStatus( Beam *beam )
//
// Print a status line as we want it to show up in a display
//
{
    char info[80];
//
// Info string
//
    time_t t = (time_t)(beam->Time());

    strftime( info, sizeof( info ), "%Y.%m.%d %H:%M:%S", gmtime( &t ) );
    if (Fmt == FmtPB)
	sprintf( info + strlen( info ), " %s", ((PB_Beam*)beam)->Info() );

    printf( "status: %s %s\n", Sitename, info );
//
// Sync info only for PB_Beam data right now
//
    if (Fmt != FmtPB)
	return;
//
// Print power and velocity one-byte data for the first eight gates
//
    int pwr[8], vel[8];
    ((PB_Beam*)beam)->SyncData( pwr, vel );

    printf( "syncinfo: %s ", Sitename );
    printf( "p %02d %02d %02d %02d %02d %02d %02d %02d ", pwr[0], pwr[1], 
	    pwr[2], pwr[3], pwr[4], pwr[5], pwr[6], pwr[7] );
    printf( "v %+d %+d %+d %+d %+d %+d %+d %+d\n", vel[0], vel[1], vel[2], 
	    vel[3], vel[4], vel[5], vel[6], vel[7] );
}



void
Receiver::SetDebugLevel( int level )
{
    DebugLvl = level;
    
    if (DebugLvl > 3)
	DebugLvl = 3;

    if (DebugLvl < 0)
	DebugLvl = 0;
}
