#if !defined	__EXTIO_HERMES_H__
#define			__EXTIO_HERMES_H__

#include "Extio_config.h"



#define EXTIO_API __declspec(dllexport)

// type for Extio callback, used for signallingof data and events to main program
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
		if ( (ni % 4096) == 0 ) { 
			LOGT("---------- receive: RX: %d  buflen: %d\n", channel, len); 
			ni++;
		}
		ni++;
		#endif
		if (len != (NS*2*sizeof(typename ST::sample_type))) {
			LOGX("ERROR Callback on channel %d: length: %d (%p): expected:%d received: %d\n", channel, len, *cback, (NS*2*sizeof(typename ST::sample_type)), len);
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



/*
 * class ExtioDataConversion
 *
 * helper class for buffering and conversion of HPSDR samples to Extio formats 
*/
template < typename ST >
class ExtioDataConversion {
public:
	ExtioDataConversion (int _ns):ns(_ns) { pb = new unsigned char[2 * ns * sizeof(typename ST::sample_type)]; }
	~ExtioDataConversion() { delete [] pb;  }
	int getNs() { return ns; }

	unsigned char *pb;
	int ns;

	int convert_iq_from_rx (HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int24)
	{
		int24 *p = (int24 *)pb;
		for (int n = 0; n < ns; ++n) {
			p->v1 = i->s3, p->v2 = i->s2, p->v3 = i->s1; ++p;
			p->v1 = q->s3, p->v2 = q->s2, p->v3 = q->s1; ++p;
			++i, ++q;
		}
		return 0;
	}

	int convert_iq_from_rx (HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int32)
	{
		int *p = (int *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->int_32(),
			*p++ = q->int_32(),
			++i,
			++q;
		return 0;
	}

	int convert_iq_from_rx (HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_float32)
	{
		float *p = (float *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->float_32(),
			*p++ = q->float_32(),
			++i,
			++q;
		return 0;
	}

	int convert_iq_from_rx(HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int_hpsdr)
	{
		int24_hpsdr *p = (int24_hpsdr *)pb;
		for (int n = 0; n < ns; ++n) {
			p->v1 = i->s3, p->v2 = i->s2, p->v3 = i->s1; ++p;
			p->v1 = q->s3, p->v2 = q->s2, p->v3 = q->s1; ++p;
			++i, ++q;
		}
		return 0;
	}

};

class Gui;

template <typename ST>
class ExtioHpsdrRadio : public ExtioDataConversion<ST>
{
public:
	ExtioHpsdrRadio(int ns, Radio *p, EXTIO_RX_CALLBACK *pCb) : ExtioDataConversion<ST>(ns), cnt(0), pidc(0), pR_(p), pExtioCallback(pCb)
	{  }

	virtual ~ExtioHpsdrRadio() {}


	void setIdllComm(IntraComm *pIdc)
	{ 
		pidc = pIdc; 
	}

	// called when the rx buffer is full
	int send_iq_from_rx_to_dsp (int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns)
	{
		// convert and copy raw I/Q data from native HPSDR format to ExtIO format
		this->convert_iq_from_rx(i, q, ST()); // Extio type selection is done at compile type !

		// internal buffers counter
		//if ((cnt % 1024) == 0) { LOGT("---------- RX: %d  buflen: %d\n", nrx, ns); cnt++; }

		// setup a buffer for ExtIO
		if (pidc) pidc->send(nrx, this->pb, 2 * this->getNs() * sizeof(typename ST::sample_type));
		cnt++;
		return 0;
	}

	void setSampleRateHW(int new_sr)
	{
		pR_->setSampleRate(new_sr);

		/* 100
		This status value indicates that a sampling frequency change has taken place,
		either by a hardware action, or by an interaction of the user with the DLL GUI.
		When the main program receives this status, it calls immediately after
		the GetHWSR() API to know the new sampling rate.
		*/
		if (*pExtioCallback) (*pExtioCallback) (-1, 100, 0., 0);
	}
	Radio * getRadio() { return pR_; }

	virtual Gui *CreateGui(int sr) = 0;

protected:
	IntraComm *pidc;
	int cnt;
	Radio *pR_;
	EXTIO_RX_CALLBACK *pExtioCallback;
};

template < typename ST >
class ExtioMercuryRadio : public Mercury, public ExtioHpsdrRadio<ST>
{
public:
	ExtioMercuryRadio(int ns, EXTIO_RX_CALLBACK *pCb) : Mercury(), ExtioHpsdrRadio<ST>(ns, this, pCb)
	{  }

	Gui *CreateGui(int sr);

	//
	// this is a virtual method inherited from Radio base class 
	// it is called when the rx buffer is full
	//
	int process_iq_from_rx(int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns)
	{
		return this->send_iq_from_rx_to_dsp(nrx, i, q, ns);
			//ExtioHpsdrRadio<ST>::send_iq_from_rx_to_dsp(nrx, i, q, ns);
	}
};


template < typename ST >
class ExtioHermesRadio : public Hermes, public ExtioHpsdrRadio<ST>
{
public:
	ExtioHermesRadio(int ns, EXTIO_RX_CALLBACK *pCb) : Hermes(), ExtioHpsdrRadio<ST>(ns, this, pCb)
	{  }

	Gui *CreateGui(int sr);

	//
	// this is a virtual method inherited from Radio base class
	// it is called when the rx buffer is full
	//
	int process_iq_from_rx(int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns)
	{
		return this->send_iq_from_rx_to_dsp(nrx, i, q, ns);
	}
};

class Gui;

//
// redirects to the proper GUI elements all the events coming from HPSDR Flow
//
class ExtioEthernet : public Ethernet {
public:
	ExtioEthernet(Gui *pg_, Flow *pF) : Ethernet(pF), pg(pg_)
	{}
	~ExtioEthernet() {}

	void FatalError(const char *pMsg);
	void TransmissionTmo(const char *pMsg);

private:
	Gui *pg;
};

#include "ExtIO_config.h"

template <
	typename EXTIO_BASE_TYPE
>
struct RadioFactory {

	ExtioHpsdrRadio<EXTIO_BASE_TYPE> *Create(const char *board_id, unsigned char *buf, EXTIO_RX_CALLBACK *pCb)
	{
		ExtioHpsdrRadio<EXTIO_BASE_TYPE> *pExr = 0;
		// decides at run time which HW we have 
		// placement new used, in order to share it among all instances
		if (strcmp(board_id, "Mercury") == 0 || strcmp(board_id, "Metis") == 0) {
			pExr = new (buf)ExtioMercuryRadio < EXTIO_BASE_TYPE >(EXTIO_NS, pCb);
		}
		else
		if (strcmp(board_id, "Hermes") == 0) {
			pExr = new (buf)ExtioHermesRadio < EXTIO_BASE_TYPE >(EXTIO_NS, pCb);
		}
		return pExr;
	}

	ExtioHpsdrRadio<EXTIO_BASE_TYPE> *Pointer(const char *board_id, unsigned char *buf)
	{
		ExtioHpsdrRadio<EXTIO_BASE_TYPE> *pExr = 0;
		// decides at run time which HW we have 
		// placement new used, in order to share it among all instances
		if (strcmp(board_id, "Mercury") == 0 || strcmp(board_id, "Metis") == 0) {
			pExr = (ExtioMercuryRadio < EXTIO_BASE_TYPE > *) buf;
		}
		else
		if (strcmp(board_id, "Hermes") == 0) {
			pExr = (ExtioHermesRadio < EXTIO_BASE_TYPE > *) buf;
		}
		return pExr;
	}

};

template <
	typename EXTIO_BASE_TYPE
>
ExtioHpsdrRadio<EXTIO_BASE_TYPE> * CreateExtioHpsdrRadio(const char *board_id);

#endif
