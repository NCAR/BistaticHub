//
// $Id: VIRAQ_Beam.hh,v 1.1 2000/08/29 21:24:07 burghart Exp $
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
# ifndef _VIRAQ_BEAM_HH_
# define _VIRAQ_BEAM_HH_

# include "Beam.hh"
# include "LE_Unpack.hh"

//
// VIRAQ beam header 
// (derived from globals.h HEADER)
//
class VIRAQ_Hdr
{
public:
    VIRAQ_Hdr( void ) {}

    char	desc[4];	// == 'DWEL'
    LE_Short	recordlen;	// header + data
    LE_Short	gates;
    LE_Short	hits;
    LE_Float	rcvr_pulsewidth;
    LE_Float	prt;
    LE_Float	delay;		// delay to first gate
    LE_Char	clutterfilter;
    LE_Char	timeseries;
    LE_Short	tsgate;
    LE_Long	time;		// seconds since 1970
    LE_Short	subsec;		// fractional seconds (.1 mS)
    LE_Float	az;
    LE_Float	el;
    LE_Float	radar_longitude; 
    LE_Float	radar_latitude;
    LE_Float	radar_altitude;
    LE_Float	ew_velocity;
    LE_Float	ns_velocity;
    LE_Float	vert_velocity;
    LE_UChar	dataformat;	// see VIRAQFormats enum above
    LE_Float	prt2;
    LE_Float	fxd_angle;
    LE_UChar	scan_type; 
    LE_UChar	scan_num;	// bumped by one for each new scan
    LE_UChar	vol_num;	// bumped by one for each new vol
    LE_Long	ray_count;
    LE_Char	transition;
    LE_Float	hxmit_power;	// on the fly hor power
    LE_Float	vxmit_power;	// on the fly ver power
    char	spare[100];
};

const int VIRAQ_HdrLen = sizeof (VIRAQ_Hdr);

//
// Special once-per-volume or when-something-changes data structure
// (derived from globals.h RADAR)
//
class VIRAQ_RHdr
{
public:
    VIRAQ_RHdr( void ) {}

    char	desc[4];	// == 'RHDR'
    LE_Short	recordlen;
    LE_Short	rev;
    LE_Short	year;
    char	radar_name[8];
    LE_Char	polarization;	// H or V
    LE_Float	test_pulse_pwr;	// TP power (referred to antenna flange)
    LE_Float	test_pulse_frq;	// test pulse frequency
    LE_Float	frequency;	// transmit frequency
    LE_Float	peak_power;	// typical xmit power (at antenna flange)
    LE_Float	noise_figure;
    LE_Float	noise_power;	// for subtracting from data
    LE_Float	receiver_gain;	// gain from antenna flange to PIRAQ input
    LE_Float	data_sys_sat;	// PIRAQ input power required for full scale
    LE_Float	antenna_gain;
    LE_Float	horz_beam_width;
    LE_Float	vert_beam_width;
    LE_Float	xmit_pulsewidth;// transmitted pulse width
    LE_Float	rconst;		// radar constant
    LE_Float	phaseoffset;	// offset for phi dp
    LE_Float	vreceiver_gain;	// ver gain from antenna flange to VIRAQ
    LE_Float	vtest_pulse_pwr;// ver TP power referred to antenna flange
    LE_Float	vantenna_gain;  
    LE_Float	misc[6];        // 6 more misc floats
    char	text[960];
};

const int VIRAQ_RHdrLen = sizeof (VIRAQ_RHdr);

//
// Finally, the VIRAQ_Beam class itself
//

class VIRAQ_Beam : public Beam
{
public:
    VIRAQ_Beam( char *buf, int& len, const Receiver* rcvr );
    ~VIRAQ_Beam( void ) { delete[] data; delete[] productdata; }
    int OK( void ) const;
    double Time( void ) const;
    int Gates( void ) const;
    float RcvrPulseWidth( void ) const;
// this is decreed to always be 150 m
    inline float RangeToFirstGate( void ) const { return 150.0; }
    float Azimuth( void ) const;
    float Elevation( void ) const;
    float FixedAngle( void ) const;
    float Frequency( void ) const;
    float PRT( void ) const;
    float Delay( void ) const;
    float RadarLat( void ) const;
    float RadarLon( void ) const;
    float RadarAlt( void ) const;
    int VolNum( void ) const;
    int Hits( void ) const;
    int SweepNum( void ) const;
    const char* ScanType( void ) const;
    inline const Receiver* Rcvr( void ) const { return MyReceiver; }
    const void* VelRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
    const void* dBZRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
    const void* dBmRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
    const void* NCPRaw( DataType *type, int *step, float *scale, 
			 float *offset ) const;
    const void* XmtrInfoPacket( int* len );
private:
    VIRAQ_RHdr rhdr;
    VIRAQ_Hdr hdr;
    const Receiver* MyReceiver;
    void *data;
    float *productdata;

    void CalculateProducts( void );
};

# endif // _VIRAQ_BEAM_HH_
