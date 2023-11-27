// datanotice.hh
// Just the port number for sending data notices to xbistat
//
// The notices themselves are just ASCII strings of the form:
//	<integer_data_index>
//		or
//	filename: <filename>
//
static const int XBDATANOTICE_PORT = 0x0bda; // "b"istatic "d"ata "a"vailable

