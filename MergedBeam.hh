//
// $Id: MergedBeam.hh,v 1.1 2000/08/29 21:24:02 burghart Exp $
//
// This is the form of a merged bistatic beam
//
// Copyright (C) 2000
// Binet Incorporated 
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

# ifndef _MERGEDBEAM_HH_
# define _MERGEDBEAM_HH_

# include <values.h>
# include "Beam.hh"


class MergedBeam : public Beam
{
public:
    MergedBeam( const Beam* beams[], int nbeams, float ncp_thresh = 0.0 );
    ~MergedBeam( void );
    inline int NumMembers( void ) const { return NMembers; }
// usable beam?
    int OK( void ) const;
// beam time, seconds since 1-Jan-1970,00:00 UTC
    double Time( void ) const;
// number of gates
    int Gates( void ) const;
    int Gates( int rcvr ) const;
// receiver pulsewidth, seconds
    float RcvrPulseWidth( void ) const;
    float RcvrPulseWidth( int rcvr ) const;
// range to center of first gate, meters
    float RangeToFirstGate( void ) const;
    float RangeToFirstGate( int rcvr ) const;
// beam azimuth, degrees clockwise from true north
    float Azimuth( void ) const;
// beam elevation, degrees above horizontal
    float Elevation( void ) const;
// fixed angle, degrees
    float FixedAngle( void ) const;
// transmitter frequency, Hz
    float Frequency( void ) const;
// xmit pulse repetition time, seconds
    float PRT( void ) const;
// number of hits
    int Hits( void ) const;
    int Hits( int rcvr ) const;
// receiver
    const Receiver* Rcvr( void ) const;
    const Receiver* Rcvr( int rcvr ) const;
// lat
    float RadarLat( void ) const;
    float RadarLat( int rcvr ) const;
// lon
    float RadarLon( void ) const;
    float RadarLon( int rcvr ) const;
// alt
    float RadarAlt( void ) const;
    float RadarAlt( int rcvr ) const;
// delay between xmit pulse and sample start, seconds
    float Delay( void ) const;
    float Delay( int rcvr ) const;
// volume number
    int VolNum( void ) const;
// sweep number
    int SweepNum( void ) const;
// scan type
    const char* ScanType( void ) const;
// get a format-specific information string
    const char* Info( void ) const;
// raw velocity data
    const void* VelRaw( DataType *type, int *step, float *scale,
			float *offset ) const;
    const void* VelRaw( DataType *type, int *step, float *scale, 
			float *offset, int rcvr ) const;
// raw reflectivity data
    const void* dBZRaw( DataType *type, int *step, float *scale,
			float *offset ) const;
    const void* dBZRaw( DataType *type, int *step, float *scale, 
			float *offset, int rcvr ) const;
// raw power data
    const void* dBmRaw( DataType *type, int *step, float *scale,
			float *offset ) const;
    const void* dBmRaw( DataType *type, int *step, float *scale, 
			float *offset, int rcvr ) const;
// raw NCP data
    const void* NCPRaw( DataType *type, int *step, float *scale,
			float *offset ) const;
    const void* NCPRaw( DataType *type, int *step, float *scale, 
			float *offset, int rcvr ) const;
// non-raw data methods
    int VelData( float* data, int maxlen, float badval, int rcvr ) const;
    int dBZData( float* data, int maxlen, float badval, int rcvr ) const;
    int dBmData( float* data, int maxlen, float badval, int rcvr ) const;
    int NCPData( float* data, int maxlen, float badval, int rcvr ) const;
// u and v wind data (calculated using receiver 0 and the receiver specified)
    int UWindData( float* data, int maxlen, float badval, int rcvr ) const;
    int VWindData( float* data, int maxlen, float badval, int rcvr ) const;
// best u and v
    int UBestData( float* data, int maxlen, float badval ) const;
    int VBestData( float* data, int maxlen, float badval ) const;
private:
    Beam** Members;
    int NMembers;
    float NCPThresh;	// NCP threshold for acceptable winds
    float** U;		// U[NMembers][ngates]
    float** V;		// V[NMembers][ngates]
    float* UBest;	// UBest[ngates]
    float* VBest;	// VBest[ngates]

    static const float VelMax = 200.0;
    static const float VelScale = VelMax / 32767.0; // fits +-VelMax into short

    static const float FBADVAL = -MAXFLOAT;
    static const short SBADVAL = MINSHORT;

    void RcvrGateAndAz( int x_gate, int rcvr, int *r_gate, float *r_az, 
			float phi, float sin_phi, float cos_phi );
    void DeriveWinds( void );
};


//
// A bunch of stuff is inlined
//
inline double 
MergedBeam::Time( void ) const
{
    return Members[0]->Time();
}

inline int 
MergedBeam::Gates( void ) const
{
    return Gates( 0 );
}

inline int 
MergedBeam::Gates( int rcvr ) const
{
    return Members[rcvr]->Gates();
}

inline float 
MergedBeam::RcvrPulseWidth( void ) const
{
    return RcvrPulseWidth( 0 );
}

inline float
MergedBeam::RcvrPulseWidth( int rcvr ) const
{ 
    return Members[rcvr]->RcvrPulseWidth(); 
}

inline float 
MergedBeam::RangeToFirstGate( void ) const 
{
    return RangeToFirstGate( 0 ); 
}

inline float 
MergedBeam::RangeToFirstGate( int rcvr ) const
{ 
    return Members[rcvr]->RangeToFirstGate(); 
}

inline float 
MergedBeam::Azimuth( void ) const 
{ 
    return Members[0]->Azimuth(); 
}

inline float 
MergedBeam::Elevation( void ) const 
{ 
    return Members[0]->Elevation(); 
}

inline float 
MergedBeam::FixedAngle( void ) const 
{ 
    return Members[0]->FixedAngle(); 
}

inline float 
MergedBeam::Frequency( void ) const 
{ 
    return Members[0]->Frequency(); 
}

inline float 
MergedBeam::PRT( void ) const 
{ 
    return Members[0]->PRT(); 
}

inline int 
MergedBeam::Hits( void ) const 
{ 
    return Hits( 0 ); 
}

inline int 
MergedBeam::Hits( int rcvr ) const 
{	
    return Members[rcvr]->Hits(); 
}

inline const Receiver* 
MergedBeam::Rcvr( void ) const 
{ 
    return Rcvr( 0 ); 
}

inline const Receiver*
MergedBeam::Rcvr( int rcvr ) const 
{ 
    return Members[rcvr]->Rcvr(); 
}

inline float 
MergedBeam::RadarLat( void ) const 
{ 
    return RadarLat( 0 ); 
}

inline float 
MergedBeam::RadarLat( int rcvr ) const 
{ 
    return Members[rcvr]->RadarLat(); 
}

inline float 
MergedBeam::RadarLon( void ) const 
{ 
    return RadarLon( 0 ); 
}

inline float 
MergedBeam::RadarLon( int rcvr ) const 

{ 
    return Members[rcvr]->RadarLon(); 
}

inline float 
MergedBeam::RadarAlt( void ) const 
{ 
    return RadarAlt( 0 ); 
}

inline float 
MergedBeam::RadarAlt( int rcvr ) const 
{ 
    return Members[rcvr]->RadarAlt(); 
}

inline float 
MergedBeam::Delay( void ) const 
{ 
    return Delay( 0 ); 
}

inline float 
MergedBeam::Delay( int rcvr ) const 
{ 
    return Members[rcvr]->Delay(); 
}

inline int 
MergedBeam::VolNum( void ) const 
{ 
    return Members[0]->VolNum(); 
}

inline int 
MergedBeam::SweepNum( void ) const 
{ 
    return Members[0]->SweepNum(); 
}

inline const char* 
MergedBeam::ScanType( void ) const 
{
     return Members[0]->ScanType();
}

inline const char* 
MergedBeam::Info( void ) const 
{ 
    return "merged beam"; 
}

inline const void* 
MergedBeam::VelRaw( DataType *type, int *step, float *scale, 
		    float *offset ) const
{ 
    return VelRaw( type, step, scale, offset, 0 ); 
}

inline const void* 
MergedBeam::VelRaw( DataType *type, int *step, float *scale, float *offset, 
		    int rcvr ) const
{
    return Members[rcvr]->VelRaw( type, step, scale, offset );
}

inline const void* 
MergedBeam::dBZRaw( DataType *type, int *step, float *scale,
		    float *offset ) const
{ 
    return dBZRaw( type, step, scale, offset, 0 ); 
}

inline const void* 
MergedBeam::dBZRaw( DataType *type, int *step, float *scale, 
		    float *offset, int rcvr ) const
{
    return Members[rcvr]->dBZRaw( type, step, scale, offset );
}

inline const void* 
MergedBeam::dBmRaw( DataType *type, int *step, float *scale,
		    float *offset ) const
{ 
    return dBmRaw( type, step, scale, offset, 0 ); 
}

inline const void* 
MergedBeam::dBmRaw( DataType *type, int *step, float *scale, 
		    float *offset, int rcvr ) const
{
    return Members[rcvr]->dBmRaw( type, step, scale, offset );
}

inline const void* 
MergedBeam::NCPRaw( DataType *type, int *step, float *scale,
		    float *offset ) const
{ 
    return NCPRaw( type, step, scale, offset, 0 ); 
}

inline const void* 
MergedBeam::NCPRaw( DataType *type, int *step, float *scale, 
		    float *offset, int rcvr ) const
{
    return Members[rcvr]->NCPRaw( type, step, scale, offset );
}

inline int 
MergedBeam::VelData( float* data, int maxlen, float badval, int rcvr ) const
{
    return Members[rcvr]->VelData( data, maxlen, badval );
}

inline int 
MergedBeam::dBZData( float* data, int maxlen, float badval, int rcvr ) const
{
    return Members[rcvr]->dBZData( data, maxlen, badval );
}

inline int 
MergedBeam::dBmData( float* data, int maxlen, float badval, int rcvr ) const
{
    return Members[rcvr]->dBmData( data, maxlen, badval );
}

inline int 
MergedBeam::NCPData( float* data, int maxlen, float badval, int rcvr ) const
{

    return Members[rcvr]->NCPData( data, maxlen, badval );
}

# endif // _MERGEDBEAM_HH_
