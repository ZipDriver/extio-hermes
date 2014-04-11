#if !defined __EXTIO_CONFIG_H__
#define		 __EXTIO_CONFIG_H__

//#include "ExtIO_hermes.h"

#include "hpsdr.h"

struct int24 {
	unsigned char v1;
	unsigned char v2;
	unsigned char v3;
};
struct int24_hpsdr {
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

typedef ExtIOtype < 5, int24>		ExtIO_int24;
typedef ExtIOtype < 6, int	>		ExtIO_int32;
typedef ExtIOtype < 7, float>		ExtIO_float32;
typedef ExtIOtype < 8, int24_hpsdr>	ExtIO_int_hpsdr;



/*
**	Extio base parametrs
**
**
**/

typedef ExtIO_int24 EXTIO_BASE_TYPE;			// type of sample exchanged on each callback invocation
const int EXTIO_DEFAULT_SAMPLE_RATE = DEFAULT_SAMPLE_RATE;	// starting sampling rate: 192000
const int EXTIO_NS = 1024;						// samples exchanged on each callback invocation

//typedef ExtioHpsdrRadio <EXTIO_BASE_TYPE> ExtioHpsdrRadioT;

#endif
