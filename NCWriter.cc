//
// $Id: NCWriter.cc,v 1.2 2001/08/28 16:27:36 burghart Exp $
// netCDF file writer class
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
# include <stdlib.h>
# include <netcdf.h>
# include <string.h>
# include <sys/stat.h>
# include <unistd.h>
# include "NCWriter.hh"
# include "MergedBeam.hh"
# include "Receiver.hh"

static int CheckPath( const char* filename );

void
NCWriter::NCErrTest( int status )
{
    if (status != NC_NOERR)
    {
	fprintf( stderr, "NCWriter: netCDF error (%s)\n", 
		 nc_strerror( status ) );
	abort();
    }
}



NCWriter::NCWriter( const char* dir, int temponly )
{
    BaseDir = new char[strlen( dir ) + 1];
    TempOnly = temponly;
    strcpy( BaseDir, dir );
    NCid = -1;
    CurrentVolume = CurrentSweep = -1;

    ZVars = VelVars = NCPVars = UVars = VVars = 0;
}



NCWriter::~NCWriter( void )
{
    CloseFile();

    delete[] BaseDir;

    delete[] ZVars;
    delete[] VelVars;
    delete[] NCPVars;
    delete[] UVars;
    delete[] VVars;
}


int
NCWriter::Write( const MergedBeam* mb )
//
// Write out the given MergedBeam.  
//
{
//
// Start a new file if:
//	1) we have no file open
//	2) this beam starts a new volume
//	3) this beam starts a new sweep
//	4) the number of receivers has changed
//
    if (NCid < 0 || mb->VolNum() != CurrentVolume || 
	mb->SweepNum() != CurrentSweep || mb->NumMembers() != NumRcvrs)
    {
	if (! StartNewFile( mb ))
	    return 0;
    }

    CurrentVolume = mb->VolNum();
    CurrentSweep = mb->SweepNum();
//
// Finish the file if it's getting huge...
//
    const long MEGA = 1024 * 1024;
    const long maxsize = 100 * MEGA;
    
    if (Filesize > maxsize)
    {
	fprintf( stderr, 
		 "NCWriter: hit file size limit.  Closing file @ %d MB\n",
		 maxsize / MEGA );
	CloseFile();
    }
//
// If we're only writing a temporary file, we just keep overwriting time
// index 0
//
    if (TempOnly)
	TimeIndex = 0;
//
// Write the data
//
    size_t index[2] = { TimeIndex++, 0 };
    size_t count[2] = { 1, 0 };
    float fval;
    double dval;
//
// time, az, and el
//
    dval = mb->Time() - BaseTime;
    NCErrTest( nc_put_var1_double( NCid, TimeVar, index, &dval ) );

    fval = mb->Azimuth();
    NCErrTest( nc_put_var1_float( NCid, AzVar, index, &fval ) );

    fval = mb->Elevation();
    NCErrTest( nc_put_var1_float( NCid, ElVar, index, &fval ) );
//
// best u and best v
//
    float* fdata = new float[MaxCells];
    short* sdata = new short[MaxCells];
    int gate;
    int ngates = mb->Gates( 0 );

    count[1] = ngates;

    mb->UBestData( fdata, MaxCells, FBADVAL );
    for (int gate = 0; gate < ngates; gate++)
    {
	if (fdata[gate] != FBADVAL)
	    sdata[gate] = (short)((fdata[gate] - VELOFFSET) / VELSCALE);
	else
	    sdata[gate] = SBADVAL;
    }
    NCErrTest( nc_put_vara_short( NCid, UBestVar, index, count, sdata ) );
    if (! TempOnly)
	Filesize += 2 * MaxCells;

    mb->VBestData( fdata, MaxCells, FBADVAL );
    for (int gate = 0; gate < ngates; gate++)
    {
	if (fdata[gate] != FBADVAL)
	    sdata[gate] = (short)((fdata[gate] - VELOFFSET) / VELSCALE);
	else
	    sdata[gate] = SBADVAL;
    }
    
    NCErrTest( nc_put_vara_short( NCid, VBestVar, index, count, sdata ) );
    if (! TempOnly)
	Filesize += 2 * MaxCells;
//
// Per-receiver vars
//
    for (size_t r = 0; r < NumRcvrs; r++)
    {
	ngates = mb->Gates( r );
    //
    // Loop through the five per-receiver vars
    //
	for (int fld = 0; fld < 5; fld++)
	{
	    int varid;
	    float scale, offset;
	//
	// Set up for this field
	//
	    switch (fld)
	    {
	    // Z or power
	      case 0:
		varid = ZVars[r];
		if (r == 0)
		    mb->dBZData( fdata, MaxCells, FBADVAL, r );
		else
		    mb->dBmData( fdata, MaxCells, FBADVAL, r );
		scale = PWRSCALE;
		offset = PWROFFSET;
		break;
	    // radial velocity
	      case 1:
		varid = VelVars[r];
		mb->VelData( fdata, MaxCells, FBADVAL, r );
		scale = VELSCALE;
		offset = VELOFFSET;
		break;
	    // NCP
	      case 2:
		varid = NCPVars[r];
		mb->NCPData( fdata, MaxCells, FBADVAL, r );
		scale = NCPSCALE;
		offset = NCPOFFSET;
		break;
	    // U wind
	      case 3:
		if (r == 0)
		    continue;	// no xmitter/xmitter U

		varid = UVars[r];
		mb->UWindData( fdata, MaxCells, FBADVAL, r );
		ngates = mb->Gates( 0 );	// u is in xmitter gates
		scale = VELSCALE;
		offset = VELOFFSET;
		break;
	    // V wind
	      case 4:
		if (r == 0)
		    continue;	// no xmitter/xmitter V

		varid = VVars[r];
		mb->VWindData( fdata, MaxCells, FBADVAL, r );
		ngates = mb->Gates( 0 );	// v is in xmitter gates
		scale = VELSCALE;
		offset = VELOFFSET;
		break;
	      default:
		fprintf( stderr, 
			 "NCWriter::Write(): BUG: unexpected field %d!\n", 
			 fld );
		exit( 1 );
	    }
	//	
	// Scale the floating point data into shorts
	//
	    for (int gate = 0; gate < ngates; gate++)
	    {
		if (fdata[gate] != FBADVAL)
		    sdata[gate] = (short)((fdata[gate] - offset) / scale);
		else
		    sdata[gate] = SBADVAL;
	    }
	//
	// Write the short array
	//
	    count[1] = ngates;
	    NCErrTest( nc_put_vara_short( NCid, varid, index, count, sdata ) );
	    if (! TempOnly)
		Filesize += 2 * MaxCells;
	}
    }

    delete[] sdata;
    delete[] fdata;

    return 1;
}



int
NCWriter::StartNewFile( const MergedBeam *mb )
{
//
// Close out our old file, if any
//
    CloseFile();
//
// Get a name for the new file
//
    MakeFileName( mb );
//
// Try to create the file
//
    if (! CheckPath( FName ) )
	return 0;

    NCErrTest( nc_create( FName, NC_NOCLOBBER | NC_SHARE | NC_FILL, &NCid ) );
    printf( "Starting new %sfile:\n '%s'\n", TempOnly ? "(temporary) " : "",
	    FName );

    Filesize = 0;
    TimeIndex = 0;
    NumRcvrs = mb->NumMembers();
    BaseTime = (int)mb->Time();
//
// Allocate space for per-receiver vars
// 
    delete[] ZVars;
    ZVars = new int[NumRcvrs];
    delete[] VelVars;
    VelVars = new int[NumRcvrs];
    delete[] NCPVars;
    NCPVars = new int[NumRcvrs];
    delete[] UVars;
    UVars = new int[NumRcvrs];
    delete[] VVars;
    VVars = new int[NumRcvrs];
//
// Put in the definitions
//
    return DoDefinitions( mb );
}



void
NCWriter::CloseFile( void )
{
    if (NCid < 0)
	return;
    
    nc_close( NCid );

    if (TempOnly)
	unlink( FName );

    printf( "NCWriter: closed %sfile:\n '%s'\n", 
	    TempOnly ? "and removed " : "", FName );	

    FName[0] = '\0';
    NCid = -1;
}

    
int
NCWriter::DoDefinitions( const MergedBeam* mb )
{
    int r, status;
    int irange[2], imissing, ifill;
    short srange[2], smissing, sfill;
    double drange[2], dmissing, dfill;
    float frange[2], fmissing, ffill;
    
    MaxCells = 0;
    for (r = 0; r < NumRcvrs; r++)
    {
	if (mb->Gates( r ) > MaxCells)
	    MaxCells = mb->Gates( r );
    }
//
// Global attributes
//
    GlobalAttrs( mb );
//
// Dimensions
//
    int namelendim;
    
    NCErrTest( nc_def_dim( NCid, "time", NC_UNLIMITED, &TimeDim ) );
    NCErrTest( nc_def_dim( NCid, "maxCells", MaxCells, &GateDim ) );
    NCErrTest( nc_def_dim( NCid, "numSystems", NumRcvrs, &RcvrDim ) );
    NCErrTest( nc_def_dim( NCid, "namelen", 64, &namelendim ) );
//
// Metadata variables
//
    int dims[2];
// base_time
    ifill = 0;
    int base_var = 
	DefIntVar( "base_time", 0, 0, "Unix Date/Time value for first record",
		   0, "seconds since 1970-01-01 00:00 UTC", 0, 0, ifill );
// Fixed_Angle
    frange[0] = -360.0;
    frange[1] = 360.0;
    fmissing = FBADVAL;
    ffill = 0.0;

    int fa_var = 
	DefFloatVar( "Fixed_Angle", 0, 0, "Targeted fixed angle",
		     "Used if one axis of motion is nominally constant",
		     "degrees", frange, fmissing, ffill );
// TransmitFrequency
    frange[0] = 0.0;
    frange[1] = 1000.0;
    int freq_var = 
	DefFloatVar( "TransmitFrequency", 0, 0, "transmitter frequency",
		     0, "GHz", frange, 0, 0 );
// per-receiver name
    dims[0] = RcvrDim;
    dims[1] = namelendim;
    int name_var = DefVar( "system_name", NC_CHAR, 2, dims, 
			   "Receiver system name", "null-terminated string", 
			   0, 0, 0, 0 );
// per-receiver Latitude
    dims[0] = RcvrDim;
    drange[0] = -90.0;
    drange[1] = 90.0;
    dmissing = FBADVAL;
    dfill = FBADVAL;
    int lat_var = 
	DefDoubleVar( "Latitude", 1, dims, "Latitude of instrument",
		      0, "degrees_north", drange, dmissing, dfill );
// per-receiver Longitude
    dims[0] = RcvrDim;
    drange[0] = -180.0;
    drange[1] = 180.0;
    dmissing = FBADVAL;
    dfill = FBADVAL;
    int lon_var = 
	DefDoubleVar( "Longitude", 1, dims, "Longitude of instrument",
		      0, "degrees_east", drange, dmissing, dfill );
// per-receiver Altitude
    dims[0] = RcvrDim;
    drange[0] = -10000.0;
    drange[1] = 90000.0;
    dmissing = FBADVAL;
    dfill = FBADVAL;
    int alt_var = 
	DefDoubleVar( "Altitude", 1, dims, "Altitude ASL of instrument",
		      0, "m", drange, dmissing, dfill );
// per-receiver Range_to_First_Cell
    dims[0] = RcvrDim;
    frange[0] = -10000.0;
    frange[1] = 300000.0;
    fmissing = FBADVAL;
    ffill = 0.0;
    int rfc_var = 
	DefFloatVar( "Range_to_First_Cell", 1, dims, 
		     "Range to the center of the first cell", 
		     "Instrument effects on range should be removed", "m",
		     frange, fmissing, ffill );
// per-receiver Cell_Spacing
    dims[0] = RcvrDim;
    frange[0] = 0.0;
    frange[1] = 10000.0;
    fmissing = FBADVAL;
    ffill = 0.0;
    int cs_var =
	DefFloatVar( "Cell_Spacing", 1, dims, "Distance between cells",
		     0, "m", frange, fmissing, ffill );
// per-receiver NumCells
    dims[0] = RcvrDim;
    irange[0] = 0;
    irange[1] = 32768;
    imissing = -1;
    ifill = -1;
    int nc_var = 
	DefIntVar( "NumCells", 1, dims, "number of sample cells", 0, 0,
		   irange, imissing, ifill );
// per-receiver AzFromRadar
    dims[0] = RcvrDim;
    frange[0] = 0.0;
    frange[1] = 360.0;
    fmissing = FBADVAL;
    ffill = FBADVAL;
    int afr_var =
	DefFloatVar( "AzFromRadar", 1, dims, "azimuth from radar to receiver",
		     0, "degrees", frange, fmissing, ffill );
// per-receiver RangeFromRadar
    dims[0] = RcvrDim;
    frange[0] = 0.0;
    frange[1] = 300000.0;
    fmissing = FBADVAL;
    ffill = FBADVAL;
    int rfr_var =
	DefFloatVar( "RangeFromRadar", 1, dims, "range from radar to receiver",
		     0, "m", frange, fmissing, ffill );
// per-receiver first sample delay time
    dims[0] = RcvrDim;
    frange[0] = -0.01;
    frange[1] = 0.01;
    fmissing = FBADVAL;
    ffill = FBADVAL;
    int delay_var =
	DefFloatVar( "SampleDelay", 1, dims,
		     "delay between transmit time and initial sampling",
		     0, "s", frange, fmissing, ffill );
// time_offset
    dims[0] = TimeDim;
    dfill = 0.0;
    TimeVar = 
	DefDoubleVar( "time_offset", 1, dims, "time offset from base_time", 
		      0, "s", 0, 0, dfill );
// Azimuth
    dims[0] = TimeDim;
    frange[0] = -360.0;
    frange[1] = 360.0;
    fmissing = FBADVAL;
    ffill = FBADVAL;
    AzVar = DefFloatVar( "Azimuth", 1, dims, "Pointing azimuth", 0,
			 "degrees clockwise from true north", frange,
			 fmissing, ffill );
// Elevation
    dims[0] = TimeDim;
    frange[0] = -360.0;
    frange[1] = 360.0;
    fmissing = FBADVAL;
    ffill = FBADVAL;
    ElVar = DefFloatVar( "Elevation", 1, dims, "Pointing elevation", 0,
			 "degrees from earth tangent towards zenith", frange,
			 fmissing, ffill );
//
// Now the "real" variables
//
    char name[8], longname[64];
    
    dims[0] = TimeDim;
    dims[1] = GateDim;
    smissing = MINSHORT;
    sfill = MINSHORT;
// reflectivity or power for each receiver
    for (r = 0; r < NumRcvrs; r++)
    {
	const char *units;
	
	sprintf( name, "%s%d", r ? "DM" : "DZ", r );
	sprintf( longname, "%s (%s)", r ? "received power" : "reflectivity",
		 mb->Rcvr(r)->Site() );
	units = r ? "dBm" : "dBZ";

	ZVars[r] = DefShortVar( name, 2, dims, longname, 0, units, 0, 
				smissing, sfill, r, PWRSCALE, PWROFFSET );
    }
// velocity for each receiver
    for (r = 0; r < NumRcvrs; r++)
    {
	sprintf( name, "VE%d", r );
	sprintf( longname, "%sradial velocity (%s)", r ? "apparent " : "",
		 mb->Rcvr(r)->Site() );
	VelVars[r] = DefShortVar( name, 2, dims, longname, 0, "m/s", 0, 
				  smissing, sfill, r, VELSCALE, VELOFFSET );
    }
// NCP for each receiver
    for (r = 0; r < NumRcvrs; r++)
    {
	sprintf( name, "NCP%d", r );
	sprintf( longname, "normalized coherent power (%s)", 
		 mb->Rcvr(r)->Site() );
	NCPVars[r] = DefShortVar( name, 2, dims, longname, 0, 0, 0, 
				  smissing, sfill, r, NCPSCALE, NCPOFFSET );
    }
// u and v for each receiver except the transmitting radar
    for (r = 1; r < NumRcvrs; r++)
    {
	sprintf( name, "u%d", r );
	sprintf( longname, "u wind component (%s and %s)", 
		 mb->Rcvr(0)->Site(), mb->Rcvr(r)->Site() );
	UVars[r] = DefShortVar( name, 2, dims, longname, 0, "m/s", 0, 
				smissing, sfill, 0, VELSCALE, VELOFFSET );

	sprintf( name, "v%d", r );
	sprintf( longname, "v wind component (%s and %s)", 
		 mb->Rcvr(0)->Site(), mb->Rcvr(r)->Site() );
	VVars[r] = DefShortVar( name, 2, dims, longname, 0, "m/s", 0, 
				smissing, sfill, 0, VELSCALE, VELOFFSET );
    }
// best u
    UBestVar = DefShortVar( "u_best", 2, dims, 
			    "u wind component (best estimate)", 0, "m/s", 0, 
			    smissing, sfill, 0, VELSCALE, VELOFFSET );
// best v
    VBestVar = DefShortVar( "v_best", 2, dims, 
			    "v wind component (best estimate)", 0, "m/s", 0, 
			    smissing, sfill, 0, VELSCALE, VELOFFSET );
//
// Done with definitions
//
    nc_enddef( NCid );
//
// Now write the values that are fixed for this file
//
    int ival;
    float fval;
    double dval;
    
    NCErrTest( nc_put_var_int( NCid, base_var, &BaseTime ) );

    fval = mb->FixedAngle();
    NCErrTest( nc_put_var_float( NCid, fa_var, &fval ) );

    fval = mb->Frequency() * 1.0e-9;	// Hz -> GHz
    NCErrTest( nc_put_var_float( NCid, freq_var, &fval ) );
//
// per-receiver vars
//
    for (r = 0; r < NumRcvrs; r++)
    {
	size_t index[2] = { r, 0 };
    // (null-terminated) system name
	const char *sitename = mb->Rcvr( r )->Site();
	size_t count[2] = { 1, strlen( sitename ) + 1 };
	NCErrTest( nc_put_vara_text( NCid, name_var, index, count, 
				     sitename ) );
    // latitude
	dval = mb->RadarLat( r );
	NCErrTest( nc_put_var1_double( NCid, lat_var, index, &dval ) );
    // longitude
	dval = mb->RadarLon( r );
	NCErrTest( nc_put_var1_double( NCid, lon_var, index, &dval ) );
    // altitude
	dval = mb->RadarAlt( r );
	NCErrTest( nc_put_var1_double( NCid, alt_var, index, &dval ) );
    // range to first cell
	fval = mb->RangeToFirstGate( r );
	NCErrTest( nc_put_var1_float( NCid, rfc_var, index, &fval ) );
    // cell spacing
	fval = 0.5 * _C_ * mb->RcvrPulseWidth( r );
	NCErrTest( nc_put_var1_float( NCid, cs_var, index, &fval ) );
    // number of cells
	ival = mb->Gates();
	NCErrTest( nc_put_var1_int( NCid, nc_var, index, &ival ) );
    // azimuth from radar
	fval = mb->Rcvr( r )->AzFromRadar();
	NCErrTest( nc_put_var1_float( NCid, afr_var, index, &fval ) );
    // range from radar
	fval = mb->Rcvr( r )->RangeFromRadar() * 1000.0; // km -> m
	NCErrTest( nc_put_var1_float( NCid, rfr_var, index, &fval ) );
    // sampling delay time (from xmit pulse time)
	fval = mb->Delay( r );
	NCErrTest( nc_put_var1_float( NCid, delay_var, index, &fval ) );
    }

    return 1;
}



int
NCWriter::DefVar( const char* name, nc_type type, int ndims, const int* dims, 
		  const char* longname, const char* comment, const char* units,
		  const void* validrange, const void* missingval, 
		  const void* fillval )
//
// Define a variable with the given parameters, and return the varid.
// Use the Def<type>Var functions instead of calling this directly!
//
{
    int var;
//
// create variable    
    NCErrTest( nc_def_var( NCid, name, type, ndims, dims, &var ) );
//
// Add the attributes
//
    if (longname)
	NCErrTest( nc_put_att_text( NCid, var, "long_name", strlen( longname ),
				    longname ) );

    if (comment)
	NCErrTest( nc_put_att_text( NCid, var, "Comment", strlen( comment ), 
				    comment ) );

    if (units)
	NCErrTest( nc_put_att_text( NCid, var, "units", strlen( units ), 
				    units ) );

    if (validrange)
    {
	switch( type )
	{
	  case NC_SHORT:
	    NCErrTest( nc_put_att_short( NCid, var, "valid_range", NC_SHORT, 
					 2, (short*)validrange ) );
	    break;
	  case NC_INT:
	    NCErrTest( nc_put_att_int( NCid, var, "valid_range", NC_INT, 
					 2, (int*)validrange ) );
	    break;
	  case NC_FLOAT:
	    NCErrTest( nc_put_att_float( NCid, var, "valid_range", NC_FLOAT, 
					 2, (float*)validrange ) );
	    break;
	  case NC_DOUBLE:
	    NCErrTest( nc_put_att_double( NCid, var, "valid_range", NC_DOUBLE, 
					 2, (double*)validrange ) );
	    break;
	  default:
	    fprintf( stderr, 
		     "NCWriter: Can't handle type %d for valid_range!\n",
		     type );
	}
    }

    if (missingval)
    {
	switch( type )
	{
	  case NC_SHORT:
	    NCErrTest( nc_put_att_short( NCid, var, "missing_value", NC_SHORT,
					 1, (short*)missingval ) );
	    break;
	  case NC_INT:
	    NCErrTest( nc_put_att_int( NCid, var, "missing_value", NC_INT,
					 1, (int*)missingval ) );
	    break;
	  case NC_FLOAT:
	    NCErrTest( nc_put_att_float( NCid, var, "missing_value", NC_FLOAT,
					 1, (float*)missingval ) );
	    break;
	  case NC_DOUBLE:
	    NCErrTest( nc_put_att_double( NCid, var, "missing_value", 
					  NC_DOUBLE, 1, 
					  (double*)missingval ) );
	    break;
	  default:
	    fprintf( stderr, 
		     "NCWriter: Can't handle type %d for missing_value!\n",
		     type );
	}
    }

    if (fillval)
    {
	switch( type )
	{
	  case NC_SHORT:
	    NCErrTest( nc_put_att_short( NCid, var, "_FillValue", NC_SHORT, 1, 
					 (short*)fillval ) );
	    break;
	  case NC_INT:
	    NCErrTest( nc_put_att_int( NCid, var, "_FillValue", NC_INT, 1, 
					 (int*)fillval ) );
	    break;
	  case NC_FLOAT:
	    NCErrTest( nc_put_att_float( NCid, var, "_FillValue", NC_FLOAT, 1, 
					 (float*)fillval ) );
	    break;
	  case NC_DOUBLE:
	    NCErrTest( nc_put_att_double( NCid, var, "_FillValue", NC_DOUBLE, 
					  1, (double*)fillval ) );
	    break;
	  default:
	    fprintf( stderr, 
		     "NCWriter: Can't handle type %d for _FillValue!\n",
		     type );
	}
    }

    return var;
}



int
NCWriter::DefIntVar( const char* name, int ndims, const int* dims, 
		     const char* longname, const char* comment, 
		     const char* units, const int validrange[2],
		     int missingval, int fillval )
//
// Define an int variable with the given parameters, and return the varid
//
{
    return DefVar( name, NC_INT, ndims, dims, longname, comment, units, 
		   (const void*)validrange, (const void*)&missingval, 
		   (const void*)&fillval );
}



int
NCWriter::DefShortVar( const char* name, int ndims, const int* dims, 
		       const char* longname, const char* comment, 
		       const char* units, const short validrange[2],
		       short missingval, short fillval, 
		       int system_index, float scale,
		       float offset )
//
// Define a short variable with the given parameters, and return the varid
//
{
    int svar = DefVar( name, NC_SHORT, ndims, dims, longname, comment, units, 
		       (const void*)validrange, (const void*)&missingval, 
		       (const void*)&fillval );
    NCErrTest( nc_put_att_int( NCid, svar, "system_index", NC_INT, 1, 
			       &system_index ) );
    NCErrTest( nc_put_att_float( NCid, svar, "scale_factor", NC_FLOAT, 1, 
				 &scale ) );
    NCErrTest( nc_put_att_float( NCid, svar, "add_offset", NC_FLOAT, 1, 
				 &offset ) );
    return svar;
}



int
NCWriter::DefFloatVar( const char* name, int ndims, const int* dims, 
		       const char* longname, const char* comment, 
		       const char* units, const float validrange[2],
		       float missingval, float fillval )
//
// Define a float variable with the given parameters, and return the varid
//
{
    return DefVar( name, NC_FLOAT, ndims, dims, longname, comment, units, 
		     (const void*)validrange, (const void*)&missingval, 
		     (const void*)&fillval );
}



int
NCWriter::DefDoubleVar( const char* name, int ndims, const int* dims, 
			const char* longname, const char* comment, 
			const char* units, const double validrange[2],
			const double missingval, const double fillval )
//
// Define a double variable with the given parameters, and return the varid
//
{
    return DefVar( name, NC_DOUBLE, ndims, dims, longname, comment, units, 
		   (const void*)validrange, (const void*)&missingval, 
		   (const void*)&fillval );
}



void
NCWriter::MakeFileName( const MergedBeam* mb )
//
// Set our data file name using our base directory and data time.
//
{
    sprintf( FName, "%s", BaseDir );
//
// Unfortunately, file naming was wrapped up into the content convention,
// with *lots* of redundant stuff included in the name:
//
//  ncswp_SPOL_19990217_193728.565_v3_s19_334.0_RHI
//  |    |     |         |        |   |   |    scan mode
//  |    |     |         |        |   |   Fixed angle
//  |    |     |         |        |   Scan num.
//  |    |     |         |        Vol. num
//  |    |     |         Time including milliseconds    
//  |    |     Date
//  |    Instrument name
//  Generic prefix identifying the file as a netCDF sweepfile
//
    double beamtime = mb->Time();
    time_t itime = (time_t)beamtime;
    struct tm* gmt = gmtime( &itime );

    int len = strlen( FName );
    strftime( FName + len, sizeof( FName ) - len, 
	      "/%Y%m%d/ncswp__%Y%m%d_%H%M%S", gmt );

    int msec = (int)((beamtime - (long)beamtime) * 1000 + 0.5); // milliseconds

    len = strlen( FName );
    sprintf( FName + len, ".%03d_v%d_s%d_%.1f_%s.nc", msec, mb->VolNum(),
	     mb->SweepNum(), mb->FixedAngle(), mb->ScanType() );
}


void
NCWriter::GlobalAttrs( const MergedBeam *mb )
{
    char *string;

    string = "This file contains one scan of remotely sensed data";
    NCErrTest( nc_put_att_text( NCid, NC_GLOBAL, "Content", strlen( string ),
				string ) );

    string = "NCAR/ATD-NOAA/ETL Scanning Remote Sensor";
    NCErrTest( nc_put_att_text( NCid, NC_GLOBAL, "Convention", 
				strlen( string ), string ) );

    NCErrTest( nc_put_att_text( NCid, NC_GLOBAL, "Scan_Mode", 
				strlen( mb->ScanType() ), mb->ScanType() ) );

    int vol = mb->VolNum();
    NCErrTest( nc_put_att_int( NCid, NC_GLOBAL, "Volume_Number", NC_INT, 
			       1, &vol ) );

    int swp = mb->SweepNum();
    NCErrTest( nc_put_att_int( NCid, NC_GLOBAL, "Scan_Number", NC_INT,
			       1, &swp ) );
}



static int
CheckPath( const char* filename)
//
// Verify that the directory path for our file exists, creating
// necessary directories if needed (and possible).  Return non-zero if
// successful, zero otherwise.
//
{
    const char* begin = filename;
    const char* end;
//
// Traverse the directory path, finding or creating each directory along
// the way.
//
    while (end = strchr( begin, '/' ))
    {
	if (end != begin)
	{
	    char dir[256];
	    struct stat stbuf;

	    strncpy( dir, filename, end - filename );
	    dir[end - filename] = '\0';

	    if (stat( dir, &stbuf ) < 0 && mkdir( dir, 0777 ) < 0)
	    {
		fprintf( stderr, "NCWriter: Error creating directory %s: ", 
			 dir );
		perror( "" );
		return 0;
	    }
	}
	begin = end + 1;
    }

    return 1;
}
