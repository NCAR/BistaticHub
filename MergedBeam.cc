//
// $Id: MergedBeam.cc,v 1.1 2000/08/29 21:24:01 burghart Exp $
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

# include <stdio.h>
# include <string.h>
# include <math.h>
# include "MergedBeam.hh"
# include "Receiver.hh"

inline float RAD_TO_DEG( float x ) { return( 180.0 * x / M_PI ); }
inline float DEG_TO_RAD( float x ) { return( M_PI * x / 180.0 ); }
const float _C_ = 2.998e8;



MergedBeam::MergedBeam( const Beam* beams[], int nbeams, float ncp_thresh )
//
// Instantiate a "merged" beam from the given beams.  The first beam in
// the list is assumed to be that from the transmitting site, and is used
// for values that must be common among the beams, e.g., time, azimuth,
// etc.
//
{
    NMembers = nbeams;
//
// stash our member beams
//
    Members = new Beam*[NMembers];
    memmove( Members, beams, NMembers * sizeof( Members[0] ) );
//
// save the NCP threshold value
//
    NCPThresh = ncp_thresh;
//
// Derive winds
//
    DeriveWinds();
}



MergedBeam::~MergedBeam( void )
{
    delete[] Members;
    for (int m = 0; m < NMembers; m++)
    {
	delete[] U[m];
	delete[] V[m];
    }
	
    delete[] U;
    delete[] V;
    delete[] UBest;
    delete[] VBest;
}



int
MergedBeam::OK( void ) const
{
//
// Just make sure all our members are OK
//
    for (int m = 0; m < NMembers; m++)
    {
	if (! Members[m]->OK())
	    return 0;
    }
    return 1;
}



int 
MergedBeam::UWindData( float* data, int maxlen, float badval, int rcvr ) const
{
//
// Make sure we're given enough space
//
    if (maxlen < Gates())
    {
	fprintf( stderr, "MergedBeam::UWindData: not enough data space\n" );
	return 0;
    }
//
// Copy the data into the user's array and substitute in the user's bad
// value flag
//
    memmove( data, U[rcvr], Gates() * sizeof(float) );
    for (int g = 0; g < Gates(); g++)
	if (U[rcvr][g] == FBADVAL)
	    data[g] = badval;

    return 1;
}



int 
MergedBeam::VWindData( float* data, int maxlen, float badval, int rcvr ) const
{
//
// Make sure we're given enough space
//
    if (maxlen < Gates())
    {
	fprintf( stderr, "MergedBeam::VWindData: not enough data space\n" );
	return 0;
    }
//
// Copy the data into the user's array and substitute in the user's bad
// value flag
//
    memmove( data, V[rcvr], Gates() * sizeof(float) );
    for (int g = 0; g < Gates(); g++)
	if (V[rcvr][g] == FBADVAL)
	    data[g] = badval;

    return 1;
}



int 
MergedBeam::UBestData( float* data, int maxlen, float badval ) const
{
//
// Make sure we're given enough space
//
    if (maxlen < Gates())
    {
	fprintf( stderr, "MergedBeam::UBestData: not enough data space\n" );
	return 0;
    }
//
// Copy the data into the user's array and substitute in the user's bad
// value flag
//
    memmove( data, UBest, Gates() * sizeof(float) );
    for (int g = 0; g < Gates(); g++)
	if (UBest[g] == FBADVAL)
	    data[g] = badval;

    return 1;
}



int 
MergedBeam::VBestData( float* data, int maxlen, float badval ) const
{
//
// Make sure we're given enough space
//
    if (maxlen < Gates())
    {
	fprintf( stderr, "MergedBeam::VBestData: not enough data space\n" );
	return 0;
    }
//
// Copy the data into the user's array and substitute in the user's bad
// value flag
//
    memmove( data, VBest, Gates() * sizeof(float) );
    for (int g = 0; g < Gates(); g++)
	if (VBest[g] == FBADVAL)
	    data[g] = badval;

    return 1;
}



void
MergedBeam::RcvrGateAndAz( int x_gate, int rcvr, int *r_gate, float *r_az,
			   float phi, float sin_phi, float cos_phi )
//
// Translate the xmitter gate and azimuth into gate and azimuth for 
// receiver 'rcvr'.  Phi is the angle from the xmitter-to-receiver baseline
// to the beam (range -180 to 180).
//
{
//
// Get the baseline length, then calculate the range from the xmitter and
// from the receiver to the gate.
//
    float base = Members[rcvr]->Rcvr()->RangeFromRadar() * 1000.0; // km -> m

    float x_range = (x_gate + 0.5) * (0.5 * _C_ * RcvrPulseWidth( 0 ));
    float r_range = sqrt( x_range * x_range + base * base -
			  2 * x_range * base * cos_phi );
//
// We should get the real delay at some point, but for now assume the 
// bistatic receivers start sampling 4 gates before the first pulse arrives
// from the transmitter.
//
    float delay = (base / _C_) - (4.0 * RcvrPulseWidth( rcvr ));
//
// Get the floating point bistatic receiver distance to the illuminated 
// volume, in units of bistatic gates.
//
    float r_gate_f = 
	(x_range + r_range) / (_C_ * RcvrPulseWidth( rcvr )) - 
	delay / RcvrPulseWidth( rcvr );

    if (r_gate_f < 0.0 )
	r_gate_f = 0.0;

    *r_gate = (int) r_gate_f;
//
// Finally, the azimuth from the receiver to the gate
//
    if (phi < 0.0)
	*r_az = Members[rcvr]->Rcvr()->AzFromRadar() + 180.0 +
	    RAD_TO_DEG( acos( (r_range*r_range + base*base - x_range*x_range) /
			      (2.0 * r_range * base) ) );
    else
	*r_az = Members[rcvr]->Rcvr()->AzFromRadar() + 180.0 -
	    RAD_TO_DEG( acos( (r_range*r_range + base*base - x_range*x_range) /
			      (2.0 * r_range * base) ) );

    if (*r_az < 0.0)
	*r_az += 360.0;
    else if (*r_az > 360.0)
	*r_az -= 360.0;
}



void
MergedBeam::DeriveWinds( void )
//
// Calculate dual-Doppler u and v given the current beam data.
// If ncp_threshold is > 0.0, only calculate winds for gates where the
// NCP from the bistatic receiver is greater than the given threshold.
// If ncp_threshold is <= 0.0, no thresholding is applied.
//
{
    int gate;
//
// allocate per-member u and v data arrays
//
    U = new float*[NMembers];
    V = new float*[NMembers];
    for (int m = 0; m < NMembers; m++)
    {
	if (m)
	{
	    U[m] = new float[Gates()];
	    V[m] = new float[Gates()];
	}
	else
	{
	// no u and v for the transmitter paired with itself...
	    U[0] = V[0] = 0;
	}
    }
//
// space for best u and v arrays
//    
    UBest = new float[Gates()];
    VBest = new float[Gates()];
//
// Transmitter gate length in meters and number of gates
//
    float gatelen = 0.5 * _C_ * RcvrPulseWidth( 0 );
    int n_xmtr_gates = Gates( 0 );
//
// Radial velocity from the transmitter
//
    float *xmtr_vel = new float[n_xmtr_gates];
    VelData( xmtr_vel, n_xmtr_gates, FBADVAL, 0 );
//
// Arrays to keep weighted best winds estimates
//
    float* weight = new float[n_xmtr_gates];
//
// Geometry terms that are constant for the beam
//
    float az = Azimuth();
    float cosat = cos( DEG_TO_RAD( az ) );
    float sinat = sin( DEG_TO_RAD( az ) );
//
// Initialize for best winds calculations
//
    for (gate = 0; gate < n_xmtr_gates; gate++)
    {
	UBest[gate] = VBest[gate] = 0.0;
	weight[gate] = 0.0;
    }
//
// Do the calculations for each bistatic receiver
//
    for (int b = 1; b < NMembers; b++)
    {
	DataType dt;
	int step;
	float scale, offset;
	int n_rcvr_gates = Gates( b );
    //
    // Angle from the radar->receiver baseline to the beam
    //
	float	phi = az - Members[b]->Rcvr()->AzFromRadar();
	if (phi < -180.0)
	    phi += 360.0;
	else if (phi > 180.0)
	    phi -= 360.0;
	
	float sin_phi = sin( DEG_TO_RAD( phi ) );
	float cos_phi = cos( DEG_TO_RAD( phi ) );
    //
    // Get the (floating-point) velocity and NCP data for this receiver
    //
	float *rcvr_vel = new float[n_rcvr_gates];
	float *rcvr_ncp = new float[n_rcvr_gates];
	VelData( rcvr_vel, n_rcvr_gates, FBADVAL, b );
	NCPData( rcvr_ncp, n_rcvr_gates, FBADVAL, b );
    //
    // For each xmitter gate..
    //
	for (gate = 1; gate < n_xmtr_gates; gate++)
	{
	    U[b][gate] = FBADVAL;
	    V[b][gate] = FBADVAL;
	//
	// get bistatic receiver gate and az that corresponds to xmitter gate
	//
	    float b_az;

	    int b_gate;

	    RcvrGateAndAz( gate, b, &b_gate, &b_az, phi, sin_phi, cos_phi );
	//
	// Make sure we're in the valid range of gates for this receiver
	// (Ignore receiver gates less than 4, since the receivers begin
	// sampling 4 gates before the arrival of the transmit pulse.)
	//
	    if (b_gate < 5)
		continue;
	    
	    if (b_gate > n_rcvr_gates)
	    {
	    //
	    // Fill the rest of the gates with bad values, and we're done
	    // with this receiver
	    //
		for (; gate < n_xmtr_gates; gate++)
		{
		    U[b][gate] = FBADVAL;
		    V[b][gate] = FBADVAL;
		}
		
		break;
	    }
	//
	// Threshold on NCP if we were given a threshold value > 0.0
	//
	    if (NCPThresh > 0.0 && rcvr_ncp[b_gate] < NCPThresh)
		continue;
	//
	// For each gate, calculate u,v from geometry and
	// V(bistatic) and V(xmtr) This formulation follows from
	// Wurman, et al., Journal of Applied Meteor.  #32,
	// December, 1993, p1802-1814.  Invert equation #3
	// omitting 2nd receiver terms and terms that contain the
	// elevation angle of the transmitter
	//

	//
	// calculate geometry terms, ignoring elevation angle
	//
	    float cosab = cos( DEG_TO_RAD( b_az ) );
	    float sinab = sin( DEG_TO_RAD( b_az ) );
	    float sumsin = (sinab + sinat);
	    float sumcos = (cosab + cosat);
	    float denom = 1.0 / sin( DEG_TO_RAD( az - b_az ) );
	//
	// Set vt to transmitter radial velocity at the current
	// gate, and vb to the bistatic receiver elliptic-normal velocity
	// at the overlapping bistatic gate.
	//
	// The elliptical correction for bistatic receiver velocities is 
	// applied here:
	//     v_actual = v_uncorrected / cos (scattering_angle / 2)
	//
	    float scat_ang = b_az - az;
	    if (scat_ang < -180.0)
		scat_ang += 360.0;
	    else if (scat_ang > 180.0)
		scat_ang -= 360.0;
	    
	    float vt = xmtr_vel[gate];
	    float vb = rcvr_vel[b_gate] / cos( 0.5 * DEG_TO_RAD( scat_ang ) );
	//
	// calculate cartesian dual-Doppler (u,v) velocities
	//
	    float uspd = (-2 * cosat * vb + sumcos * vt) * denom;
	    float vspd = (2 * sinat * vb - sumsin * vt) * denom;
	//
	// If the total wind speed is unreasonably large (> VelMax m/s),
	// then skip gate
	//
	    if ((uspd * uspd + vspd * vspd) > (VelMax * VelMax))
	    {
		U[b][gate] = FBADVAL;
		V[b][gate] = FBADVAL;
	    }
	//
	// Otherwise, stash u and v and calculate weighting factor for use
	// in finding best wind
	//
	    else
	    {
		U[b][gate] = uspd;
		V[b][gate] = vspd;
	    //
	    // Weight this wind (currently using just the sine of the
	    // scattering angle), and add it into our weighted best wind
	    // estimate for this gate.
	    //
		float bweight = sin( DEG_TO_RAD( scat_ang ));

		UBest[gate] += uspd * bweight;
		VBest[gate] += vspd * bweight;
		weight[gate] += bweight;
	    }

	}

	delete[] rcvr_vel;
	delete[] rcvr_ncp;
    }
//
// Divide out the weight from our weighted best wind estimates
//
    for (gate = 1; gate < n_xmtr_gates; gate++)
    {
	if (weight[gate])
	{
	    UBest[gate] /= weight[gate];
	    VBest[gate] /= weight[gate];
	}
	else
	{
	    UBest[gate] = VBest[gate] = FBADVAL;
	}
    }

    delete[] weight;
    delete[] xmtr_vel;
}
