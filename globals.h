// globals.h
// Assorted historic structures and values...
//
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

#include        "vdefine.h"

#define MAXNUM 10000
#define PRODS_ELEMENTS  16      /* number of elements in prods array */
#define NUMPARMS 16
#define PLANES  (NUMPARMS+2)

/* structure to completely define the operation of the VIRAQ */
/* there will be one of these structures for each VIRAQ in the system */
typedef struct
	{
	int     memhandle;  /* actual VIRAQ VME base address */
	int     intenable,intvector,intlevel,intautoclear;
	int     numfred,vfifo,tpflag,trigflag,tpdelay,tpwidth;
	int     prt,prt2,timingmode,delay,velsign,viraqdsp,boardnumber;
	char    dspfilename[80],outfilename[80],outpath[80];
	} VIRAQCFG;

/* structure to completely define the operation of the QUAD */
/* there will be one of these structures for each QUAD in the system */
typedef struct
	{
	int     memhandle;  /* actual VIRAQ VME base address */
	int     intenable,intvector,intlevel,intautoclear;
	int     boardnumber,velsign,fifointsel;
	char    dspfilename[80],outfilename[80],outpath[80];
	} QUADCFG;

/* structure to specify the data to be put in the DPRAM */
typedef struct
	{
	int     gates,hits,pulsewidth,gate0mode;
	int     pcorrect,clutterfilter,timeseries,tsgate,whoamibase;
	int     afcflag,xmitpulse,export_ts; /* xmitpulse used for discrim only */
	float   afcgain,dacval;
	float   dacmin,dacmax,locktime;
	int     cf_start,cf_gates,num_avg;
	} DPRAMCFG;

/* structure to define all parameters in any DPRAM */
typedef struct
	{
	int     *membase;  /* effective VME address for VIRAQ base */
	int     *gates,*hits,*pulsewidth,*timeseries,*gate0mode;
	int     *tsgate,*pcorrect,*clutterfilter;
	int     *afcflag,*afcgain,*locktime,*whoami;
	int     *eofstatus,*xmitpulse,*dacmax,*dacmin,*export_ts;
	volatile int *flag,*dacval,*gate0power,*s1,*s2,*s3,*s4,*status,*raycount,*cf_start,*cf_gates,*num_avg;   
	int     *dataformat,*tsptr[2],*bufptr[2];
	} DPRAM;
	
/* structure containing all information about a VIRAQ */
typedef struct
	{
	short           controlimg;
	short           *counter1,*counter2,*counter3;     
	unsigned char   *intcontrol,*intvector;
	short           *difcontrol,*intclear;
	VIRAQCFG        config;         /* desired viraq operation */
	DPRAMCFG        dpconfig;       /* desired DSP operation */
	DPRAM           dpram[2];       /* dpram mem pointers */
	} VIRAQ;

/* structure to define all registers in a QUAD */
typedef struct
	{
	unsigned char   *dspcontrol,*tclk0ctrl,*tclk1ctrl;     
	unsigned char   *intcontrol,*intvector;
	QUADCFG         config;         /* desired quad operation */
	DPRAMCFG        dpconfig;       /* desired DSP operation */
	DPRAM           dpram[4];
	} QUAD;

/* structure to completely define the operation of FRED */
typedef struct
	{
	VIRAQ   *viraq[20];     /* up to 20 viraqs */
	QUAD    *quad[20];      /* up to 20 quad boards */
	int     viraqnum,quadnum,gates,hits,prt,prt2,timingmode;
	int     delay,pulsewidth,xmitpulse,clutterfilter;
	int     timeseries,tsgate,velsign,num_avg;
	char    outpath[80],vfile[20][80],qfile[20][80];
	} FRED;

/* structure for data that transmits between a bistatic site and hub */
typedef enum _PktDataFmt
{
    PDF_VelOnly, PDF_dBZOnly, PDF_ABP
} PktDataFmt;

typedef struct  {
		char            cookie[3]; /* 'PRQ' */
		char            version;   /* version number */
		unsigned int    time;      /* seconds since 1970 */
		unsigned short  subsec;    /* fractional seconds (.1 mS) */
		unsigned short  az;        /* 65536 = 360 degrees */
		unsigned short  el;        /* 65536 = 360 degrees */
		unsigned short  flag;      /* status flag */
		unsigned int    sync;      /* gps-to-pulse gap (100nS cnts) */
		char            firstgate; /* first gate / 4 */
		char            format;    /* data format */
		short           ngates;    /* number of gates */
		unsigned int    power;                     
		unsigned int    velocity;
		unsigned char   pulsewidth;                
		unsigned char   hits;
		unsigned short  prt;
		unsigned short  delay;
		char            cmd;
		char            trailer;                
		} __attribute__ ((packed))  PACKET;

/* structure that defines communication parameters to the display */
typedef struct
	{
	int     displayparm,fakeangles,recording,type;
	double  threshold,rgpp,offset;
	double  dbzhi,dbzlo,powlo,powhi,zdrlo,zdrhi;
	double  phaselo,phasehi,ldrlo,ldrhi;
	int     mousex,mousey,mousebutton,mousegate;
	double  mouserange,mouseangle;
	int     xc,yc;  /* location of radar in pixel coordinates */
	char    title[40];
	double  lat,lon;
	void    *plane[PLANES]; /* display and overlay bitmaps (0 = overlay) */
	int     dispnum;        /* number of parameters calculated */
	} DISPLAY;

typedef struct
	{
	char    *label,*units;
	double  low,high;
	int     divs,c0;
	double  scale,offset;
	} PARAMDAT;


/* header for each dwell describing parameters which might change dwell by dwell */
/* this structure will appear on tape before each abp set */
typedef struct  {
		char            desc[4];
		unsigned short  recordlen;
		short           gates,hits;
		float           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		short           tsgate;
		unsigned int    time;      /* seconds since 1970 */
		unsigned short           subsec;    /* fractional seconds (.1 mS) */
		float           az,el;
		float           radar_longitude; 
		float           radar_lattitude;
		float           radar_altitude;
		float           ew_velocity;
		float           ns_velocity;
		float           vert_velocity;
		char            dataformat;     /* 0 = abp, 1 = abpab (poly), 2 = abpab (dual prt) */
		float           prt2;
		float           fxd_angle;
		unsigned char   scan_type;
		unsigned char   scan_num;
		unsigned char   vol_num;
		unsigned int    ray_count;
		char            transition;
		float           hxmit_power;    /* on the fly hor power */
		float           vxmit_power;    /* on the fly ver power */
		float           yaw;            /* platform heading in deg */
		float           pitch;          /* platform pitch in deg */
		float           roll;           /* platform roll in deg */
		float           gate0mag;       /* gate zero magnitude in rel dB */
		float           dacv;           /* dac voltage value or afc freq */
		char            spare[80];
		} __attribute__((packed)) HEADER;

/* this structure gets recorded for each dwell */
typedef struct  {                
		HEADER          header;
		short           abp[MAXNUM * 14]; /* a,b,p + time series */
		} __attribute__((packed)) DWELL;

/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		short   recordlen;
		short   rev;
		short   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		float   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		float   test_pulse_frq; /* test pulse frequency */
		float   frequency;      /* transmit frequency */
		float   peak_power;     /* typical xmit power (at antenna flange) read from config.rdr file */
		float   noise_figure;
		float   noise_power;    /* for subtracting from data */
		float   receiver_gain;  /* hor chan gain from antenna flange to VIRAQ input */
		float   data_sys_sat;   /* VIRAQ input power required for full scale */
		float   antenna_gain;
		float   horz_beam_width;
		float   vert_beam_width;
		float   xmit_pulsewidth; /* transmitted pulse width */
		float   rconst;         /* radar constant */
		float   phaseoffset;    /* offset for phi dp */
		float   vreceiver_gain; /* ver chan gain from antenna flange to VIRAQ */
		float   vtest_pulse_pwr; /* ver test pulse power refered to antenna flange */
		float   vantenna_gain;  /* vertical antenna gain */
		float   vnoise_power;   /* for subtracting from data */
		float   zdr_fudge_factor; /* what else? */
		float   missmatch_loss;   /* receiver missmatch loss (positive number) */
		float   misc[3];        /* 4 more misc floats */
		char    text[960];
		} __attribute__ ((packed)) RADAR;

/* this is what the top of either the radar or dwell struct looks like */
/* it is used for recording on disk and tape */
typedef struct
	{
	char    desc[4];
	short   recordlen;
	} TOP;

typedef struct
	{
	int     xmouse,ymouse,l,m,r;
	int     plane,zoom,xpan,ypan;
	int     xclick,yclick,pclick,cclick;
	} MOUSE;

#define LEFTBUTTON      0x01
#define RIGHTBUTTON     0x02
#define MIDDLEBUTTON    0x04
#define NOMOUSE         0x80

/* definition of several different data formats */
#define DATA_SIMPLEPP   0 /* simple pulse pair ABP */
#define DATA_POLYPP     1 /* poly pulse pair ABPAB */
#define DATA_DUALPP     2 /* dual prt pulse pair ABP,ABP */
#define DATA_POL1       3 /* dual polarization pulse pair ABP,ABP */
#define DATA_POL2       4 /* more complex dual polarization ??????? */
#define DATA_POL3       5 /* almost full dual polarization with log integers */
#define DATA_SIMPLE16   6 /* 16 bit magnitude/angle representation of ABP */
#define DATA_DOW        7 /* dow data format */
#define DATA_FULLPOL1   8 /* 1998 full dual polarization matrix for alternating H-V */
#define DATA_FULLPOLP   9 /* 1999 full dual polarization matrix plus for alternating H-V */
#define DATA_MAXPOL    10 /* 2000 full dual polarization matrix plus for alternating H-V */
#define DATA_HVSIMUL   11 /* 2000 copolar matrix for simultaneous H-V */
#define DATA_SHRTPUL   12 /* 2000 full dual pol matrix w/gate averaging no Clutter Filter */

/* QUAD defines */
#define QCSET(q,a)      (*q->dspcontrol = (a))
#define QUADDSPCONTROL  0x40001
#define QUADTCLK0CTRL   0x40003
#define QUADTCLK1CTRL   0x40005
#define QUADINTCONTROL  0x40011
#define QUADINTVECTOR   0x40013

/* VIRAQ defines */
#define VCBITSET(v,a)      (*v->difcontrol = v->controlimg |= (a))
#define VCBITCLR(v,a)      (*v->difcontrol = v->controlimg &= ~(a))
#define VRQCOUNTER1     0x40000
#define VRQCOUNTER2     0x40008
#define VRQCOUNTER3     0x40010
#define VRQINTCONTROL   0x40029
#define VRQINTVECTOR    0x4002B
#define VRQDIFCONTROL   0x40030
#define VRQINTCLEAR     0x40032

/* addresses for the VME parellel digital IO card */
//#define PP_BASE 0xf000
//#define PP_RESET 0xe
//#define PP_AZLO 0xe
//#define PP_AZHI 0xc
//#define PP_ELLO 0x6
//#define PP_ELHI 0x4
//#define PP_METERLO 0xa
//#define PP_METERHI 0x8
//#define PP_ATTEN 0x0


#define K2      0.93
#define C       2.99792458E8
#define TWOPI   6.283185307

typedef struct  {float x,y;}    complex;

/* fsubs.c */
FRED   *initfred(char *);
int    fredreginit(VIRAQ *viraq);
void   stop(FRED *);
int    start(FRED *);
int    wait(FRED *);
void   fredconfig(char *,FRED *);
int    vicenable(int handle);
int    get_vme_memory_handle(int address,int size);
int    fredsetup(FRED *f);
int    pci(int);

/* vsubs.c */
int     viraqreginit(VIRAQ *viraq);
VIRAQ   *initviraq(char *filename);
void    vstop(VIRAQ *viraq);
int     vstart(VIRAQ *v);
void    dac(VIRAQ *viraq,double v);
void    afclock(VIRAQ *viraq);
void    savev(VIRAQ *v);
void    readv(VIRAQ *v);

/* fft.c */
void fft(complex c[],int n);
void arcfft(complex c[], int n);
void hann(complex a[], int n);
void hamm(complex a[], int n);
void dolookup(int n);

/* subs.c */
void    timer(int timernum,int timermode,int count,int handle,short *addr);
int     load(char *fname,int base, int num);
void    viraqconfig(char *fname, VIRAQ *viraq);
void    initdpramptr(int membase, DPRAM *d);
void    cleardpram(int addr, int data);
void    setdpram(int membase, DPRAMCFG *c, DPRAM *d, int num);
float   toieee(int c);
int     frieee(float c);

/* radar.c */
RADAR *initradar(char *);
void readradar(char *,RADAR *);

/* products.c */
void products(DWELL *,RADAR *,float *);
void simplepp(DWELL *,RADAR *,float *);
void polypp(DWELL *,RADAR *,float *);
void dualprt(DWELL *,RADAR *,float *);
void dualpol1(DWELL *,RADAR *,float *);
void dualpol3(DWELL *,RADAR *,float *);
void dualpolfull1(DWELL *,RADAR *,float *);
void fullpolplus(DWELL *,RADAR *,float *);
void hvsimul(DWELL *,RADAR *,float *);

/* filesub.c */
void getkeyval(char *,char *,char *);
void set(char *, char *, void *, char *, int, char *);
int parse(char *, char *list[]);
#ifdef FILE
FILE    *openoutputfile(char *,char *);
#endif
void makefilename(char *pfilename, char *filename, char *path);

/* disk.c */
int    init_indisk(DWELL *,RADAR *,char *);
int    read_disk(DWELL *,RADAR *);
void   write_disk(int,DWELL *,RADAR *,char *);
void   rewind_disk(void);
void   close_disk();

/* acquire.c */
void filldwell(DWELL *,FRED *);
void fillheader(DWELL *,FRED *);
void readmisc(DWELL *,FRED *);
void readtime(DWELL *);
void readangles(DWELL *);
void readposition(DWELL *);
void readvelocity(DWELL *);
void readdata(DWELL *,FRED *);
void readother(DWELL *,FRED *,int size);
void gpstime(unsigned int *,unsigned short *);

/* coffload.c */
/* int     coffload(char *filename,int membase); */
int     coffload(char *filename,char *buf);

/* qsubs.c */
QUAD    *initquad(char *);
void    quadconfig(char *,QUAD *);
int     quadreginit(QUAD *q);
void    qstop(QUAD *q);
int     qstart(QUAD *q);

/* tape.c */
int    init_intape(DWELL *,RADAR *);
int    read_tape(DWELL *,RADAR *);
void   write_tape(int,DWELL *,RADAR *);
void   rewind_tape(void);
int    open_tape(void);
void   close_tape(void);
void   weof(void);
int    checkstatus(char *buf,int level);

/* vcardsubs.c */
void    xfillrect(int x1,int y1,int x2,int y2,int plane,int color);
void    xmove(int x,int y,int plane);
void    xdraw(int x,int y,int plane,int color);
void    xvector(int x1,int y1,int x2,int y2,int plane,int color);
void    xputcolormap(int start,int num,char *rgb);
void    xgetcolormap(int start,int num,char *rgb);
void    xputline(int x,int y,int num,int plane,char *color);
void    xgetline(int x,int y,int num,int plane,char *color);
void    xputstring(int x,int y,int plane,int color,char *str);
void    xreadmouse(MOUSE *m);
int     xreadkey(void);
void    xdefscale(int snum,SCALE *scale);
void    xray(double az, double el, int ngates, int nplanes, double pprg, char *c);
void    xrangerings(int spacing,int plane,int color);
void    xthreshold(int threshold,int plane);
void    xinitdisplay(void);
void    xshowscale(int scale);
void    xinitvcard(char *fname,int base);
void    xdisplaytype(int dtype);

/* getaz.c */
void    powermeter(double *,double *,double *);
void    setatten(int, int);
void    getazel(int *az, int *el);
int     bcdswitch(int offset);
void    settestpulse(VIRAQ *viraq, int delay, int width);
void    readswitch(VIRAQ *hviraq,VIRAQ *vviraq);

/* vtlib.c */
void readdisplay(char *,DISPLAY *);
int init_display(RADAR *,DWELL *,DISPLAY *);
void init_disp_scales(RADAR *,DWELL *,DISPLAY *);
int load_lookup(char *);
void display(DWELL *,DISPLAY *,float *);
void initgraphscales(int plane,int color);
void selections(DISPLAY *);
void colorarray(DWELL *,DISPLAY *,float *,unsigned char *);
void getpalette(int,char *,char *,char *);
void disp_update(DWELL *,DISPLAY *,float *);
void asprinttime(DWELL *,char *);
void drawkey(char *,double,double,int,int);
void init_palette(void);
void initparmdat(PARAMDAT *p,RADAR *radar,DWELL *dwell,DISPLAY *disp);
void close_display(void);
void initgraphscale(int plane, int color);

/* ray.c */
void vertray(DISPLAY *disp, char *color, int gates, int parmnum);
void drawspray(DISPLAY *disp, int plane, int dots, double beamwidth, double azlow, double azhigh, double ellow, double elhigh, double oldaz, double newaz, double oldel, double newel, int *color);

/* ethernet.c */
int open_enet_server(int handle,unsigned int address);
void close_enet_server(int descriptor);
int open_enet_client(int handle, unsigned int address);
void close_enet_client(int descriptor);
int ethernetout(char *buffer,int bytecount);
int sendethernetpacket(char *buffer,int bytecount,int totalpages,int pagenumber);
void    enetout(int descriptor, void *buf, int size);
int enetin(int descriptor, void *buf,int size);
void open_ethernet(void);
void close_ethernet(void);

/* main */
void closeall(void);
void math_handler(int);

/* physmap.c */
unsigned long     allocate_physical_memory(unsigned long physaddr,int size);

/* savescr */
int savescr(unsigned int time);
static void putword(int w, FILE *fp);
static void compress(int init_bits, FILE *outfile, int len);
static void output(int code);
static void cl_block (void);
static void cl_hash(long int hsize);
static void char_init(void);
static void char_out(int c);
static void flush_char(void);
char    mgetpixel(int x,int y);
void resetbyte(void);
int getbyte(FILE *fp);
void resetcode(void);
int     getcode(FILE *fp);
void gif2raster(char *file);
int imageseparator(FILE *fp);
int gifsignature(FILE *fp);
void screendescriptor(FILE *fp,int *x,int *y,int *bg);
void imagedescriptor(FILE *fp,int *xstart,int *ystart,int *width,int *height,int *m,int *i,int *p);
void setpalette(int n,char r,char g,char b);
void resetcompress(void);
int decompress(FILE *fp);
void resetparms(int size);
void filllifo(FILE *fp);


/* acquire.c */
void    readdualpol(DWELL *,FRED *);
void    readfullpol(DWELL *,FRED *);
void    readfullpolplus(DWELL *,FRED *);
void    readmaxpol(DWELL *,FRED *);
void    readhvsimul(DWELL *,FRED *);
void    readshrtpul(DWELL *,FRED *);
void    makelookup(void);

/* ctimer.c */
void init_timer(void);
void init_irig(void);
void xmit_enable(void);
void ptrigoff(void);
void ptrigon(void);
void trigger(void);
void timerstart(int pri);
void timerstop(void);

/* slib.c COM serial handler */
int     carrier(int port);
void    output_serial(int port,char c);
void    outsblock(int port,char *c,int size);
int     serialhit(int port);
int     input_serial(int port);
void    rtsoff(int port);
void    rtson(int port);
void    open_serial(int port);
void    close_serial(int port);
void    baud(int port,int baud,int parity,int bits,int stop);

/* glib.c */
void    gmode(void);
void    tmode(void);
void    plot(int x,int y,int c);
void    gclear(void);
void    setcolor(int color);
int     getcolor(void);
void    glibinit(void);
void    undisp(void);
void disp(float data[], int n);
void pdisp(float data[], int n);
void    gcursor(double xfract, double data);
void    uncursor(double xfract, double data);
void    display_axis(void);
void    drawgraphscale(char *xbuf,double xlo, double xhi, char *ybuf, double ylo, double yhi);
void    move(int x,int y);
void    draw(int x,int y);
void    grputs(int x,int y,char *string);
void    avedisp(float array[],int n,double filter);
void    polardisp(float array[],int n,double filter);

/* maplib.c county line map background */
void maplib(double ppm, double lat, double lon, int xc, int yc, int plane, int color);

void delay(unsigned msec);

void closeall(void);
