//
// $Id: NCWriter.hh,v 1.1 2000/08/29 21:24:02 burghart Exp $
// netCDF writer class
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
# ifndef _NCWRITER_HH_
# define _NCWRITER_HH_

# include <time.h>
# include <values.h>
# include <netcdf.h>

class MergedBeam;

class NCWriter
{
public:
    NCWriter( const char* dir, int temponly = 0 );
    ~NCWriter( void );
    inline const char* FileName( void ) const { return FName; }
    Write( const MergedBeam* mbeam );
    inline int LastIndexOfFile( void ) const { return TimeIndex - 1; }
private:
    char* BaseDir;
    char FName[256];
    int TempOnly;
    int NumRcvrs;
    int NCid;
    int BaseTime;
    int TimeIndex;
    int MaxCells;
// dimensions
    int	TimeDim;
    int	GateDim;
    int RcvrDim;
// variables
    int AzVar;
    int ElVar;
    int TimeVar;
    int *ZVars;
    int *VelVars;
    int *NCPVars;
    int *UVars;
    int *VVars;
    int UBestVar;
    int VBestVar;
// bookkeeping    
    int CurrentVolume;
    int CurrentSweep;
    long Filesize;
// general constants
    static const float FBADVAL = -MAXFLOAT;
    static const short SBADVAL = MINSHORT;
    static const float _C_ = 2.998e8;
// constants for scaling float values to shorts
    static const float PWRSCALE = 0.01;
    static const float PWROFFSET = 0.0;
    static const float VELSCALE = 0.01;
    static const float VELOFFSET = 0.0;
    static const float NCPSCALE = 0.01;
    static const float NCPOFFSET = 0.0;
//
// Private methods
//
    int StartNewFile( const MergedBeam *mb );
    void CloseFile( void );
    int DoDefinitions( const MergedBeam* mb );
    void MakeFileName( const MergedBeam* mb );
    void NCErrTest( int status );
    int DefVar( const char* name, nc_type type, int ndims, const int* dims, 
		const char* longname, const char* comment, const char* units,
		const void* validrange, const void* missingval, 
		const void* fillval );
    int DefShortVar( const char* name, int ndims, const int* dims, 
		     const char* longname, const char* comment, 
		     const char* units, const short validrange[2], 
		     short missingval, short fillval, int system_index, 
		     float scale, float offset );
    int DefIntVar( const char* name, int ndims, const int* dims, 
		   const char* longname, const char* comment, 
		   const char* units, const int validrange[2], 
		   int missingval, int fillval );
    int DefFloatVar( const char* name, int ndims, const int* dims, 
		     const char* longname, const char* comment, 
		     const char* units, const float validrange[2], 
		     float missingval, float fillval );
    int DefDoubleVar( const char* name, int ndims, const int* dims, 
		      const char* longname, const char* comment, 
		      const char* units, const double validrange[2], 
		      double missingval, double fillval );
    void GlobalAttrs( const MergedBeam* mb );
};
    
# endif // _NCWRITER_HH_

