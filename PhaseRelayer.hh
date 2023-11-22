// PhaseRelayer.hh
// Class to relay per-pulse phases from the radar to receivers
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

# include "Receiver.hh"

class PhaseRelayer
{
private:
    const ReceiverList_t* Rcvrs;
    int InFd;
    int OutFd;
    int InPort;
    int OutPort;

public:
    PhaseRelayer( int in_port, int out_port, const ReceiverList_t* rcvrs );
    inline int FD( void ) const { return InFd; }
    void ReadAndRelay( void ) const;
    void Relay( const void* packet, int len ) const;
};
