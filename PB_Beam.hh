//
// $Id: PB_Beam.hh,v 1.1 2000/08/29 21:24:04 burghart Exp $
//
// This is the form of the PIRAQ bistatic beam
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

# ifndef _PB_BEAM_HH_
# define _PB_BEAM_HH_

# include <values.h>
# include "Beam.hh"
# include "LE_Unpack.hh"	// little-endian types

class Receiver;

//
// The header structure
//
typedef struct _PB_Hdr
{
    char	cookie[3];	// == 'PRQ'
    LE_UChar	version;	// 1 or 2
    LE_ULong	time;		// Unix time stamp
    LE_UShort	subsec;		// 100 microsecond units
    LE_UShort	azimuth;	// radar azimuth, deg*(65536/360.0)
    LE_UShort	elevation;	// radar elev., deg*(65536/360.0)
    LE_UShort	flag;
    LE_ULong	sync;		// 100 nanosecond units
    LE_UChar	qtr_firstgate;	// first gate / 4
    LE_UChar	format;		// data format
    LE_UShort	ngates;
    LE_ULong	power;	    	// 1st 8 power gates, 1/2 byte each
    LE_ULong	velocity;	// 1st 8 vel gates, 1/2 byte each
// For format version 1, 1 clock count = 1/8 microsecond
// For format version 2, 1 clock count = 1/10 microsecond
    LE_UChar	pulsewidth;	// clock counts
    LE_UChar	hits;		// hits
    LE_UShort	prt;		// clock counts
    LE_UShort	delay;		// clock counts
    LE_UChar	cmd;		// command character
    LE_UChar	trailer;	// == 'X'
} PB_Hdr;

const int PB_HdrLen = sizeof (PB_Hdr);

//
// Bits in the format code tell us which fields are being sent.  Here's
// the table...
//
enum PBF_BitID	// Bit ID's so that we can refer to each by a name
{
    PBFld_V, PBFld_P, PBFld_NCP, PBFld_3, PBFld_4, PBFld_5, PBFld_6, 
    PBFld_P_NCP
};

static const struct _PBFormatInfo
{
    char* name;
    int bytes_per_gate;
} PBFormatInfo[8] =
{
    { "V", 1 },		/* 0 - 8 bit velocity		*/
    { "P", 1 },		/* 1 - 8 bit power		*/
    { "?", 0 },		/* 2 - 8 bit NCP		*/
    { "?", 0 },		/* 3 - unused			*/
    { "?", 0 },		/* 4 - unused			*/
    { "?", 0 },		/* 5 - unused			*/
    { "?", 0 },		/* 6 - unused			*/
    { "P+NCP", 1 }	/* 7 - 5 bit power + 3 bit NCP	*/
};



class PB_Beam : public Beam
{
public:
    PB_Beam( char* inbuf, int& inbuflen, const Receiver* rcvr );
    ~PB_Beam( void );
// usable beam?
    int OK( void ) const;
// beam time, seconds since 1-Jan-1970,00:00 UTC
    double Time( void ) const;
// number of gates
    int Gates( void ) const;
// receiver pulsewidth, seconds
    float RcvrPulseWidth( void ) const;
// range to center of first gate, meters
    float RangeToFirstGate( void ) const;
// beam azimuth, degrees clockwise from true north
    float Azimuth( void ) const;
// beam elevation, degrees above horizontal
    float Elevation( void ) const;
// beam fixed angle, degrees
    inline float FixedAngle( void ) const { return 0.0; }
// transmitter frequency, Hz
    inline float Frequency( void ) const { return PB_Frequency; }
// set transmitter frequency, Hz
// (this applies to *all* PB_Beams)
    static inline void SetFrequency( float freq ) { PB_Frequency = freq; }
// xmit pulse repetition time, seconds
    float PRT( void ) const;
// number of hits
    int Hits( void ) const;
// delay between xmit pulse and sample start, seconds
    float Delay( void ) const;
// lat, long, & alt
    float RadarLat( void ) const { return -MAXFLOAT; }
    float RadarLon( void ) const { return -MAXFLOAT; }
    float RadarAlt( void ) const { return -MAXFLOAT; }
// volume number
    inline int VolNum( void ) const { return 0; }
// sweep number
    inline int SweepNum( void ) const { return 0; }
// scan type (always unknown)
    inline const char* ScanType( void ) const { return "UNK"; }
// get a format-specific information string
    const char *Info( void ) const;
// power and velocity from the first eight gates, for sync verification	
    void SyncData( int *pwr, int *vel );
// raw velocity data
    const void *VelRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
// raw reflectivity data
    const void *dBZRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
// raw power data
    const void *dBmRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
// NCP data
    const void *NCPRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
// Receiver
    inline const Receiver* Rcvr( void ) const { return MyReceiver; }

private:
//
// transmitter frequency, Hz; this must be set before returned velocities
// will be correct
//
    static float PB_Frequency;
//
// simple beam structure: header + data
//
    PB_Hdr Hdr;
    void* Data;
//
// pointer to our Receiver info
//
    const Receiver *MyReceiver;
//
// Clock count time: 1/8 microsecond for format version 1 (old receivers), 
// and 1/10 microsecond for version 2 (newer receivers)
//
    float ClockCountTime;
//
// If we unpack some packed data, we keep it around
//
    char* UnpackedPwr;
    char* UnpackedNCP;
    
    int OffsetToField( PBF_BitID id ) const;
    void UnpackP_NCP();
};

# endif // _PB_BEAM_HH_
