#if !defined	__EXTIO_HERMES_H__
#define			__EXTIO_HERMES_H__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EXTIO_HERMES_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EXTIO_HERMES_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EXTIO_HERMES_EXPORTS
#define EXTIO_HERMES_API __declspec(dllexport)
#else
#define EXTIO_HERMES_API __declspec(dllimport)
#endif


typedef  void (* EXTIO_RX_CALLBACK) (int, int, float, int *) ;



#include "intradllcomm.h"
#include "log.h"

template < typename ST, int NS >
class IdllComm: public IntraComm 
{
public:
	IdllComm (int channel, EXTIO_RX_CALLBACK *pc):IntraComm(channel),cback(pc)
	#if !defined NDEBUG
	,ni(0) 
	#endif
	{}

	int receive (unsigned channel, unsigned char *buf, int len) {
		#if !defined NDEBUG
		if ( (ni % 256) == 0 ) {LOG (("---------- receive: RX: %d  buflen: %d\n", channel, len)); ni++;}
		ni++;
		#endif
		if (len != (NS*2*sizeof(ST::sample_type))) {
			LOG(("ERROR Callback on channel %d: length: %d (%p): expected:%d received: %d\n", channel, len, *cback, (NS*2*sizeof(ST::sample_type)), len));
		} else
			(*cback) (NS, 0, 0., (int *) buf );
		return 0;
	}
private:
	EXTIO_RX_CALLBACK *cback;
	#if !defined NDEBUG
	int ni;
	#endif
};



#include "hpsdr.h"

struct int24 {
	unsigned char v1;
	unsigned char v2;
	unsigned char v3;
};

template <int v, typename T>
struct ExtIOtype
{
	enum { value = v };
	typedef T sample_type;
};

typedef ExtIOtype < 5, int24> ExtIO_int24;
typedef ExtIOtype < 6, int	> ExtIO_int32;
typedef ExtIOtype < 7, float> ExtIO_float32;


template < typename ST >
class ExtioHermesRadio : public Hermes
{
public:
	ExtioHermesRadio (int ns): Hermes(), ns_(ns), cnt(0), pidc(0)
	{ pb = new unsigned char [2 * ns * sizeof(ST::sample_type)]; }

	void setIdllComm (IntraComm *pIdc) { pidc = pIdc; }

	void setSampleRate (int new_sr) 
	{
		Hermes::setSampleRate (new_sr);
		if (ExtioCallback) ExtioCallback (-1, 100, 0., 0);
	}

	// called when the rx buffer is full
	int process_iq_from_rx (int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns)
	{
		process_iq_from_rx ( ns, i, q, ST() ); // Extio type selection is done at compile type !
		// send to callback supplier
		return process_iq_from_rx ( nrx, pb, 2 * ns * sizeof(ST::sample_type));
	} 

protected:
	IntraComm *pidc; 
	int cnt;
	int ns_;
	unsigned char *pb;

private:	

	int process_iq_from_rx (int ns, HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int24 )
	{
		int24 *p = (int *)pb;
		for (int n = 0; n < ns; ++n) 
			*p++ = i->s1, *p++ = i->s2, *p++ = i->s3, 
			*p++ = q->s1, *p++ = q->s2, *p++ = q->s3, 
			++i, ++q
		; 
		return 0;
	} 


	int process_iq_from_rx (int ns, HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int32 )
	{
		int *p = (int *)pb;
		for (int n = 0; n < ns; ++n) 
			*p++ = i->int_32(), 
			*p++ = q->int_32(), 
			++i, 
			++q; 
		return 0;
	} 

	int process_iq_from_rx (int ns, HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_float32 )
	{
		float *p = (float *)pb;
		for (int n = 0; n < ns; ++n) 
			*p++ = i->float_32(), 
			*p++ = q->float_32(), 
			++i, 
			++q; 
		return 0;
	} 

	virtual int process_iq_from_rx (int nrx, unsigned char * b, int nbytes) { 
		if ( (cnt % 256) == 0 ) {LOG (("---------- RX: %d  buflen: %d\n", nrx, nbytes)); cnt++;}

		// setup a buffer for ExtIO
		if (pidc) pidc->send (nrx, b, nbytes);
		cnt++;
		return 0; 
	} // called when the rx buffer is full
};



#include "log.h"
#include "gui.h"

class HermesEthernet: public Ethernet {
public:
	HermesEthernet (Gui *p, Flow *pF): Ethernet(pF), pg(p)
	{}

	void FatalError     (const char *pMsg) { pg->SetHw (pMsg); };
	void TrasmissionTmo (const char *pMsg) { pg->AppendMessage (pMsg); };

private:
	Gui *pg;
};

extern "C" EXTIO_HERMES_API void __stdcall CloseHW();

#include "dllmain.h"

class ExtIODll: public Dll {
public:
	ExtIODll (HMODULE h): Dll(h) {}
	void ProcessAttach ()
	{
		// initialize Windows sockets
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		wVersionRequested = MAKEWORD(1, 1);
		err = WSAStartup(wVersionRequested, &wsaData);
	};
	void ProcessDetach () { CloseHW(); };
	void ThreadAttach () {};
	void ThreadDetach () {};
private:
};

DLL_CLASS(ExtIODll,hModule)

typedef ExtIO_int32 EXTIO_BASE_TYPE;

const int EXTIO_DEFAULT_SAMPLE_RATE	= 192000;

const int EXTIO_NS				= 1024;

#endif
