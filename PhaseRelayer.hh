//
// class to relay phases from the radar to receivers
//

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
