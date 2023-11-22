// NCWriter.hh
// BistaticHub netCDF writer class
//
// Copyright © 2000 Binet Incorporated
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
    int Write( const MergedBeam* mbeam );
    inline int LastIndexOfFile( void ) const { return TimeIndex - 1; }
private:
    char* BaseDir;
    char FName[256];
    int TempOnly;
    int NumRcvrs;
    int NCid;
    int BaseTime;
    size_t TimeIndex;
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
    static constexpr float FBADVAL = -MAXFLOAT;
    static constexpr int16_t SBADVAL = MINSHORT;
    static constexpr float _C_ = 2.998e8;
// constants for scaling float values to int16_ts
    static constexpr float PWRSCALE = 0.01;
    static constexpr float PWROFFSET = 0.0;
    static constexpr float VELSCALE = 0.01;
    static constexpr float VELOFFSET = 0.0;
    static constexpr float NCPSCALE = 0.01;
    static constexpr float NCPOFFSET = 0.0;
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
		     const char* units, const int16_t validrange[2],
		     int16_t missingval, int16_t fillval, int system_index,
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

