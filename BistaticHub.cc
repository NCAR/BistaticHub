//
// $Id: BistaticHub.cc,v 1.1 2000/08/29 21:23:58 burghart Exp $
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
# include <unistd.h>
# include <errno.h>
# include <ctype.h>
# include <signal.h>
# include <dirent.h>
# include <fcntl.h>
# include <getopt.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/vfs.h>

# include "NetReader.hh"
# include "CommandSender.hh"
# include "Beam.hh"
# include "PB_Beam.hh"
# include "MergedBeam.hh"
# include "Receiver.hh"
# include "NCWriter.hh"
# include "PhaseRelayer.hh"

# define GEN_XMTR_INFO 1	// Do we need to generate xmitter info packets?
# if GEN_XMTR_INFO
#	include "VIRAQ_Beam.hh"
# endif

# include "../display/datanotice.hh"

ReceiverList_t Rcvrs;
# define NumRcvrs Rcvrs.size()

const int PHASE_IN_PORT = 21100;
const int PHASE_OUT_PORT = 21000;
const int IN_DATA_PORT = 0x6006;
const int OUT_DATA_PORT = 2345;

//
// A linked list to hold the beams we haven't merged yet
//
typedef struct _BeamListEntry
{
    Beam	*p_beam;
    int		source;	// 0 = xmitting radar, then count up
    struct _BeamListEntry	*next;
    struct _BeamListEntry	*prev;
} BeamListEntry;

BeamListEntry	*BeamList = 0, *BeamListTail = 0;
int		BeamListLen = 0;
const int	MaxBeamListLen = 1000;

//
// How much debugging info do we want to spew?
//
int DebugLevel = 0;

//
// NCP threshold value for wind calculations
//
float NCPThresh = 0.0;

//
// Where do we find our receiver info?
//
char	RcvrInfoFile[128] = "RcvrInfo";

//
// Data writing
//
int	SaveData = 0;
char	BaseDataDir[128] = "/data";

//
// Globals
//
int	MatchCount = 0;

//
// Prototypes
//
static void HandleArgs( int argc, char** argv );
static void Usage( const char* progname );
static void SetupRcvrs( NetReader* nr, const char* fname );
static void sig_handler( int sig );
static void Watchdog( int sig );
static void Shutdown( void );
static void SendToRcvr( const char* cmd, int target );
static void HandleCommand( const char* cmd );
static void SetRcvrState( const char* params, int enabled );
static void ReportRcvrState( int rcvr, const char* extra = 0 );
static void SetRecordingState( int state );
static void ReportRecordingState( void );
static void ReportAll( void );
static void SetDebugLevel( const char* param );
static void ReportDebugLevel( void );
static void SetNCPThresh( const char* param );
static void ReportNCPThresh( void );
static void HandleBeam( Beam *newb, int source );
static void DeleteEntry( BeamListEntry *e );
static void LookForMatch( BeamListEntry *entry );
static void WriteBeam( const MergedBeam* mb );
static int CheckDataSpace( const char* dir, unsigned long threshold );
static void Barf( void );
static void NudgeClients( const char* filename, int index );





main( int argc, char *argv[] )
{
//
// Force stdout to be line buffered.  This is the default in general, but we
// get a block buffered stdout if we're exec'ed from a Python script...
//
    setvbuf( stdout, 0, _IOLBF, 0 );
//
// Be ready to handle a couple of signals
//
    signal( SIGALRM, sig_handler );
    signal( SIGINT, sig_handler );
//
// Version
//
    printf( "BistaticHub version %s\n", VERSION );
//
// Handle args
//
    HandleArgs( argc, argv );
//
// Create our network data reader for the incoming data port
//
    NetReader netreader( IN_DATA_PORT );
//
// Build the receiver list
//
    SetupRcvrs( &netreader, RcvrInfoFile );
//
// Prepare to relay phases from the radar to the receivers
//
    PhaseRelayer p_relayer( PHASE_IN_PORT, PHASE_OUT_PORT, &Rcvrs );
//
// Report our state
//
    ReportAll();
//
// Send everybody "R", "K", and "J" to resync and get V and P+NCP data flowing
//
    SendToRcvr( "R", -1 );
    printf( "Sleeping for 5 seconds\n" );
    sleep( 5 );
    SendToRcvr( "K", -1 );
    SendToRcvr( "J", -1 );
//
// Set up a watchdog to do some status checking every 3 seconds
//
    signal( SIGALRM, Watchdog );
    alarm( 3 );
//
// We'll be watching for data from stdin and from our NetReader's incoming 
// file descriptor.
//
    fd_set watchfds;

    FD_ZERO( &watchfds );
    FD_SET( 0, &watchfds );	// stdin
    FD_SET( netreader.FD(), &watchfds );
    FD_SET( p_relayer.FD(), &watchfds );
//
// Go into our main loop
//
    fd_set fds;
    while (1)
    {
    //
    // Wait for something to happen on one of our input sources
    //
	fds = watchfds;
	if (select( FD_SETSIZE, &fds, 0, 0, 0 ) < 0)
	{
	    if (errno == EINTR)
       		continue;

	    fprintf( stderr, "Error %d from select!\n", errno );
	    exit( 1 );
	}
    //
    // If something's on stdin, read it as a command and dispatch it
    //
	if (FD_ISSET( 0, &fds ))
	{
	    char line[512], *cmdline;
	    int len;

	    if (! fgets( line, sizeof( line ), stdin ))
	    {
		fprintf( stderr, "Error reading stdin!\n" );
		exit( 1 );
	    }
	    
	    if ((len = strlen( line )) < 2)
		continue;
	//
	// Drop the trailing '\n', or complain about a too-long command
	//
	    if (line[len-1] == '\n')
		line[len-1] = '\0';
	    else
	    {
		fprintf( stderr, 
			 "Command line > %d bytes ignored: '%40s...'\n", 
			 line );
		continue;
	    }
	//
	// Skip leading whitespace
	//
	    cmdline = line;
	    while (isspace( cmdline[0] ))
		   cmdline++;
	//
	// Handle the command
	//
	    HandleCommand( cmdline );
	}
    //
    // If we have data ready from the net, tell the NetReader to read it,
    // getting back the Receiver associated with the new data.  Then check
    // the Receiver to see if it can deliver a complete beam yet.
    //
	if (FD_ISSET( netreader.FD(), &fds ))
	{
	    Receiver *rcvr = netreader.Read();
	    Beam *newbm;
	    if (rcvr && (newbm = rcvr->NextBeam()) != NULL)
	    {
	    //
	    // Find the index of the Receiver in our list
	    //
		int rnum;
		for (rnum = 0; rnum < NumRcvrs; rnum++)
		    if (rcvr == Rcvrs[rnum])
			break;

		if (rnum == NumRcvrs)
		{
		    fprintf( stderr, "Bad Receiver in main!\n" );
		    delete newbm;
		    continue;
		}
# if GEN_XMTR_INFO
	    //
	    // Generate and relay transmitter information, if the 
	    // transmitter itself is not doing this...
	    //
		if (rnum == 0)
		{
		    VIRAQ_Beam *vbm = (VIRAQ_Beam*)newbm;
		    int infopktlen;
		    const void* infopkt;
		    infopkt = vbm->XmtrInfoPacket( &infopktlen );
		    p_relayer.Relay( infopkt, infopktlen );
		}
# endif         
	    //
	    // Pass off the new beam to be processed
	    //
		HandleBeam( newbm, rnum );
	    }
	}
    //
    // Relay phases from the radar to the receivers
    //
	if (FD_ISSET( p_relayer.FD(), &fds ))
	{
	    p_relayer.ReadAndRelay();
	}
    }
    
    Shutdown();
}



static void
sig_handler( int sig )
//
// Deal with various bad signals
//
{
    fprintf( stderr, "Shutting down on signal %d!\n", sig );
    Shutdown ();
}


static void
Watchdog( int sig )
//
// Regularly scheduled watchdog to do some status checking
//
{
//
// Make sure we're getting data from the enabled receivers
//
    for (int i = 0; i < NumRcvrs; i++)
    {
	if (Rcvrs[i]->IsEnabled())
	{
	    int t;
	    if ((t = Rcvrs[i]->TimeSinceLastData()) > 5)
	    {
		char extra[80];
		sprintf( extra, "No data in %d:%02d", t / 60, t % 60 );
		ReportRcvrState( i, extra );
	    }
	}
    }
//
// Set handler again on non-BSD systems
//
    signal (SIGALRM, Watchdog);
//
// And reset the alarm
//
    alarm (3);
}



static void
HandleArgs( int argc, char** argv )
{
    int	option_index;
    char c;
    static struct option long_options[] =
    {
	{"base_data_dir", 1, 0, 0},	// deprecated in favor of data_dir
	{"data_dir", 1, 0, 0},
	{"rcvr_info", 1, 0, 0},
	{"help", 0, 0, 0},
	{0, 0, 0, 0}
    };
//
// Get our args
//
    while ((c = getopt_long (argc, argv, "b:d:r:h", long_options, 
			     &option_index)) != EOF)
    {
    //
    // Just turn a long option into the equivalent short option by using the
    // first letter of the long option name.
    //
	if (c == 0)
	    c = long_options[option_index].name[0];
    //
    // Deal with the short options
    //
	switch (c)
	{
	  case 'b':	// base_data_dir is deprecated in favor of data_dir
	  case 'd':
	    strcpy( BaseDataDir, optarg );
	    break;
	  case 'r':
	    strcpy( RcvrInfoFile, optarg );
	    break;
	  case 'h':
	    Usage( argv[0] );
	    exit( 0 );
	    break;
	  case '?':
	  default:
	    Usage( argv[0] );
	    exit( 1 );
	}
    }
//
// There should be nothing left
//
    if (optind != argc)
    {
	Usage( argv[0] );
	exit( 1 );
    }
}



static void 
Usage( const char* progname )
{
    fprintf (stderr, "Usage: %s [options]\n", progname);
    fprintf (stderr, "  Options:\n");
    fprintf (stderr, "	-d, --data_dir <dir>\n");
    fprintf (stderr, "	-r, --rcvr_info <receiver_info_file>\n");
    fprintf (stderr, "	-h, --help\n");
}



static void
SetupRcvrs( NetReader* netreader, const char* fname )
//
// Read the receiver info file and build our Receiver list
//
{
    FILE *rcvrfile;
    char string[80];

    if (!(rcvrfile = fopen( fname, "r" )))
    {
	fprintf( stderr, "Error %d opening RcvrInfo file '%s'\n", errno,
		 fname );
	exit( 1 );
    }
//
// Get the info for each receiver
//
    int rcvrnum = 0;
    
    while (fgets( string, sizeof( string ), rcvrfile ))
    {
    //
    // Skip comments
    //
	if (string[0] == '#')
	    continue;
    //
    // The rest of the lines we pass on to create Receivers and add them
    // to our list
    //
	char id = '0' + rcvrnum++;
	Rcvrs.push_back( new Receiver( string, id, netreader ) );
    }
}



static void
HandleCommand( const char* cmdline )
//
// We deal with certain commands...
//
{
    char cmd[32];
    const char* params;
    sscanf( cmdline, "%s", cmd );

    params = cmdline + strlen( cmd );
    while (isspace( params[0] ))
	   params++;
//
// disable <rcvr_num>
//
    if (! strcmp( cmd, "disable" ))
	SetRcvrState( params, 0 );
//
// enable <rcvr_num>
//
    else if (! strcmp( cmd, "enable" ))
	SetRcvrState( params, 1 );
//
// writeon (enable data recording)
//
    else if (! strcmp( cmd, "writeon" ))
	SetRecordingState( 1 );
//
// writeoff (disable data recording)
//
    else if (! strcmp( cmd, "writeoff" ))
	SetRecordingState( 0 );
//
// set debugging level
//
    else if (! strcmp( cmd, "debuglevel" ))
	SetDebugLevel( params );
//
// NCP threshold for wind calculations
//
    else if (! strcmp( cmd, "ncpthresh" ))
	SetNCPThresh( params );
//
// send a command to a receiver
//
    else if (! strcmp( cmd, "rcvrcommand" ))
    {
	char rcmd[24];
	int which;
	sscanf( params, "%s %d", rcmd, &which );
	SendToRcvr( rcmd, which );
    }
//
// exit
//
    else if (! strcmp( cmd, "exit" ))
	Shutdown();
//
// huh?
//
    else
	fprintf( stderr, "Unknown command '%s'!\n", cmd );

    return;
}


    

static void
SetRcvrState( const char* params, int enabled )
{
    int rnum, nparams;
    char extra[4];
//
// Get the receiver number and verify that we got nothing else
//
    nparams = sscanf( params, "%d%2s\n", &rnum, extra );
    if (nparams != 1 || rnum < 0 || rnum > (NumRcvrs - 1) )
    {
	fprintf( stderr, "Cannot %s receiver '%s'\n", 
		 enabled ? "enable" : "disable", params );
	return;
    }
//
// No disabling the transmitting radar...
//
    if (rnum == 0 && !enabled)
    {
	fprintf( stderr, "The transmitting radar can't be disabled!\n" );
	return;
    }
//
// Set the receiver's state as requested
//
    if (enabled)
	Rcvrs[rnum]->Enable();
    else
	Rcvrs[rnum]->Disable();
//
// Report the current state
//
    ReportRcvrState( rnum );
}

    
static void
ReportRcvrState( int rnum, const char* extra )
{
    printf( "receiver: %d %s %d %s\n", rnum, Rcvrs[rnum]->Site(), 
	    Rcvrs[rnum]->IsEnabled(), extra ? extra : "" );
}



static void
SetRecordingState( int state )
{
    WriteBeam( 0 );	// close the current file, if any
    SaveData = state;
    ReportRecordingState();
}



static void
ReportRecordingState( void )
{
    printf( "recording: %d\n", SaveData );
}
    


static void
ReportAll( void )
//
// Give a complete report of our state
//
{
    printf( "Base data directory is: %s\n", BaseDataDir );
    for (int r = 0; r < NumRcvrs; r++)
	ReportRcvrState( r );

    ReportRecordingState();
    ReportDebugLevel();
    ReportNCPThresh();
}


    
static void
SendToRcvr( const char* cmd, int target )
//
// Send a command string to one of the receivers (or all if "target" is -1)
//
{
    static CommandSender sender;
    
    int first, last;
//
// Set up to send to everybody if target < 0
//    
    if (target < 0)
    {
	first = 0;
	last = NumRcvrs - 1;
	printf( "Sending '%s' to all\n", cmd );
    }
    else
    {
	first = last = target;
	printf( "Sending '%s' to %s\n", cmd, Rcvrs[target]->Site() );
    }
//
// Send the command to the target receiver(s)
//
    for (int r = first; r <= last; r++)
	sender.Send( cmd, Rcvrs[r]->IP_Address() );
}

    

static void
SetDebugLevel( const char *param )
{
    DebugLevel = atoi( param );
    
    if (DebugLevel < 0)
	DebugLevel = 0;
    if (DebugLevel > 3)
	DebugLevel = 3;
//
// For now, force our Receivers to the same debug level
//
    for (int r = 0; r < NumRcvrs; r++)
	Rcvrs[r]->SetDebugLevel( DebugLevel );

    ReportDebugLevel();
}

    

static void
ReportDebugLevel( void )
{
    printf( "debuglevel: %d\n", DebugLevel );
}
    



static void
SetNCPThresh( const char *param )
{
    NCPThresh = strtod( param, 0 );
    
    if (NCPThresh < 0.0)
	NCPThresh = 0.0;
    if (NCPThresh > 1.0)
	NCPThresh = 1.0;

    ReportNCPThresh();
}

    

static void
ReportNCPThresh( void )
{
    printf( "ncpthresh: %.2f\n", NCPThresh );
}
    



//
// Stuff below here needs to be objectified...
//



void
HandleBeam( Beam *newb, int source )
// 
// Insert the given beam into our beam list, which is sorted by time.  It
// is our responsibility to delete the Beam when it gets removed from the
// list.
//
{
    BeamListEntry *new_e;
//
// Make a new beam list entry
//
    new_e = new BeamListEntry;
    new_e->p_beam = newb;
    new_e->source = source;
    new_e->prev = new_e->next = 0;
//
// Empty list is easy
//
    if (! BeamList)
	BeamList = new_e;
//
// Also easy if this beam is later than anything in the list
//
    else if (new_e->p_beam->Time() >= BeamList->p_beam->Time())
    {
	new_e->next = BeamList;
	BeamList->prev = new_e;
	BeamList = new_e;
    }
//
// Nope, we have to insert somewhere deeper in the list
//
    else
    {
	BeamListEntry *e = BeamList;

	while (e && (new_e->p_beam->Time() < e->p_beam->Time()))
	{
	    new_e->prev = e;
	    e = e->next;
	}

	if ((new_e->next = new_e->prev->next) != 0)
	    new_e->next->prev = new_e;

	new_e->prev->next = new_e;
    }
//
// Set the tail if necessary
//
    if (! (new_e->next))
	BeamListTail = new_e;
//
// Increment the list length, and remove the oldest beam if the list is too
// long.
//
    BeamListLen++;
    if (BeamListLen > MaxBeamListLen)
    {
	if (new_e == BeamListTail)
	    new_e = NULL;

	DeleteEntry( BeamListTail );
    }

    if (new_e)
	LookForMatch( new_e );
}



static void
LookForMatch( BeamListEntry *entry )
//
// Look through our beam list, around the given entry, searching for a set 
// of beams that:
//	1) includes one beam from each source
//	2) has all beam times within our allowed tolerance from the time of 
//	   the transmitting radar's beam
// If we find such a set, clear out earlier beams from the list (possibly
// sending out incomplete merged sets), then merge and send out the set
// of beams we found.  Otherwise do nothing.
//
{
    const int MAX_SOURCES = 8;
    BeamListEntry	*e, *src_entries[MAX_SOURCES];
    double	entrytime, diffs[MAX_SOURCES];
    int	i, listlen;
    static long	lastmatch = 0;
    static float tolerance = 1.0;
    
    assert( NumRcvrs < MAX_SOURCES );
//
// Initialize
//
    for (i = 0; i < NumRcvrs; i++)
    {
	diffs[i] = 999.0;
	src_entries[i] = 0;
    }

    entrytime = entry->p_beam->Time();
    src_entries[entry->source] = entry;
    diffs[entry->source] = 0.0;
//
// Set the tolerance (to half a dwell period) each time we get a beam 
// from the transmitter
//
    if (entry->source == 0)
	tolerance = entry->p_beam->PRT() * entry->p_beam->Hits() * 0.5;
//
// Move toward the newer beams in the list until we hit a beam more than
// "tolerance" seconds newer than the entry we're trying to match.
//
    for (e = entry; e->prev; e = e->prev)
    {
	double diff = e->prev->p_beam->Time() - entrytime;
	if (diff > tolerance)
	    break;
    }
//
// Now move backward in time through the list, keeping track of the closest
// matches from each source to entry's time.  Stop when we get more than
// "tolerance" earlier than the entry we're trying to match.
//
    listlen = 0;
	
    for (; e; e = e->next)
    {
	if (e == entry)
	    continue;

	double diff = fabs (e->p_beam->Time() - entrytime);

	if (diff > tolerance)
	    break;
	else if (diff < diffs[e->source])
	{
	    src_entries[e->source] = e;
	    diffs[e->source] = diff;
	}
    }
//
// Return now if we don't have an entry for every enabled source
//
    for (i = 0; i < NumRcvrs; i++)
    {
	if (! Rcvrs[i]->IsEnabled())
	    continue;
	
	if (! src_entries[i])
	{
	    if (! lastmatch)
		lastmatch = time (0);

	    if ((time (0) - lastmatch) > 60)
	    {
		Barf();
		lastmatch = time (0);
	    }
	    return;
	}
    }
//
// Keep track of the time of our last match
//
    lastmatch = time( 0 );
    MatchCount++;
//
// We need to tell PIRAQ PB_Beam sources what the xmitter frequency is
//
    PB_Beam::SetFrequency( src_entries[0]->p_beam->Frequency() );
//
// We have a successful match, so print a message if requested.
//
    if (DebugLevel == 1 || DebugLevel == 2)
    {
	fprintf( stderr, "+" );
	fflush( stderr );
    }
    else if (DebugLevel > 2)
	fprintf( stderr, "Matched beam @ %.3f\n", entry->p_beam->Time() );
//
// The list of matching beams is contained in src_entries.  Now we create a
// MergedBeam from them.
//
    int nmerged = 0;
    const Beam** mbeams = new const Beam*[NumRcvrs];

    for (i = 0; i < NumRcvrs; i++)
    {
	if (! src_entries[i])
	    continue;
    //
    // Add this beam to the merged list
    //
	mbeams[nmerged++] = src_entries[i]->p_beam;
    }

    MergedBeam mb( mbeams, nmerged, NCPThresh );
    delete[] mbeams;
//
// Write it to a file
//
    WriteBeam( &mb );
//
// Get rid of the beam list entries we just used
//
    for (i = 0; i < NumRcvrs; i++)
	if (src_entries[i])
	    DeleteEntry( src_entries[i] );
//
// Clear out stuff in the list earlier than the last entry we looked at
//
    while (e && e->next)
	DeleteEntry( e->next );

    return;
}



static void
WriteBeam( const MergedBeam* mb )
//
// Write the given MergedBeam, or close our current file if the pointer is zero
//
{
    static NCWriter* ncw = 0;
    static int check = 0;
//
// Close our file and return on null input pointer
//
    if (! mb)
    {
	if (ncw)
	{
	    delete ncw;
	    ncw = 0;
	}
	return;
    }
//
// Create a new NCWriter if necessary, forcing temporary files if writing
// is disabled.
//
    if (! ncw)
    {
	if (SaveData)
	    ncw = new NCWriter( BaseDataDir, 0 );
	else
	    ncw = new NCWriter( "/tmp", 1 );
    }
//
// Check our free space once in a while
//
    const long minspace = 100;	// MB
    
    if (SaveData && ! (check++ % 100) && 
	! CheckDataSpace( BaseDataDir, minspace ))
    {
	fprintf( stderr, "Free disk space < %d MB in %s\n", minspace,
		 BaseDataDir );
	fprintf( stderr, "Disabling data writing\n" );
	SetRecordingState( 0 );
	return;
    }
//
// Don't record if the antenna's not moving.  (This should be selectable
// at some point).
//
    static float prev_az = 0.0;
    static float prev_el = 0.0;
    float diff = fabs( (mb->Azimuth() - prev_az) + 
		       (mb->Elevation() - prev_el) );
	
    if (diff <= 0.05)
    {
	if (DebugLevel > 1)
	    fprintf( stderr, "Antenna stationary; beam not written.\n" );
	return;
    }
//
// Finally, try to write the beam.  If writing is disabled, we actually still
// write, but we just overwrite the same record over and over; this allows
// for data to still be available for display.  If the write fails, 
// disable writing.
//
    if (! ncw->Write( mb ))
    {
	delete ncw;
	ncw = 0;
	fprintf( stderr, "Write failed, disabling data writing\n" );
	SetRecordingState( 0 );
    }
    else
//
// On success, tell clients that something new is available
//
	NudgeClients( ncw->FileName(), ncw->LastIndexOfFile() );
}



static int
CheckDataSpace( const char* fname, unsigned long threshold )
//
// Assure that we have at least 'threshold' free MB in the filesystem
// containing 'fname'.
//
{
//
// We want just the directory information from the filename, so strip 
// everything from the last slash onward.  If there is no slash, set the
// directory to "."
//
    char dir[256];
    strcpy( dir, fname );

    char *lastslash = strrchr( dir, '/' );
    if (lastslash)
	lastslash[0] = '\0';
    else
	strcpy( dir, "." );
//
// Get the filesystem stats for the directory
//
    struct statfs fsinfo;
    statfs( dir, &fsinfo );
//
// If more than threshold MB are available, we're golden
//
    return( fsinfo.f_bavail > (threshold * 1024 * 1024 / fsinfo.f_bsize) );
}



static void
DeleteEntry( BeamListEntry *e )
//
// Delete the given entry from our beam list
//
{
    if (! e)
	return;

    if (e->next)
	e->next->prev = e->prev;

    if (e->prev)
	e->prev->next = e->next;

    if (e == BeamList)
	BeamList = e->next;

    if (e == BeamListTail)
	BeamListTail = e->prev;

    delete( e->p_beam );
    delete( e );

    BeamListLen--;
}


    

void
Barf( void )
//
// Forget all the beams in our list
//
{
    fprintf( stderr, "BARF!  Flushing beam list and input queues...\n" );

    while (BeamList)
	DeleteEntry( BeamListTail );
}




static void
Shutdown( void )
//
// We're done
//
{
//
// Send a "W" to everybody so that everyone stops sending data
// (so the ISDN routers will time out and hang up...)
//
    SendToRcvr( "W", -1 );
//
// Close our current file, if any
//
    WriteBeam( 0 );
//
// Now just exit
//
    exit (0);
}



static void 
NudgeClients( const char* filename, int index )
//
// Tell display clients that a new record is available
//
{	
    static FILE* outsock = 0;
//
// Open the outgoing socket if we haven't yet
//
    if (! outsock)
    {
	int fdsock;

	if ((fdsock = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
	{
	    fprintf( stderr, 
		     "Error %d creating or configuring notification socket\n",
		     errno );
	    exit( 1 );
	}
    //
    // Send notifications to XBDATANOTICE_PORT on localhost
    //
	struct sockaddr_in to;
	memset( (char*)&to, 0, sizeof( to ) );
	to.sin_family = AF_INET;
	to.sin_port = htons( XBDATANOTICE_PORT );
    	inet_aton( "localhost", &to.sin_addr );

	if (connect( fdsock, (struct sockaddr*)&to, sizeof( to ) ) < 0)
	{
	    fprintf( stderr, 
		     "Error %d connecting to DATANOTICE socket on localhost\n",
		     errno );
	    exit( 1 );
	}
    //
    // Keep a FILE* stream pointer to the socket
    //
	outsock = fdopen( fdsock, "w" );
    }
//
// Send the notification.  Two types of notification are possible:
//
//	Filename: <filename>
//		or
//	<new_time_index>
//
// We send the index for every new beam, and the filename only every
// tenth beam.
//
    if (! (index % 10))
	fprintf( outsock, "Filename: %s", filename );
    fflush( outsock );

    fprintf( outsock, "%d", index );
    fflush( outsock );
}
