/** 
 * @file ExtIO_hermes.cpp
 * @brief ExtIO_hermes.cpp : Defines the exported functions for the DLL application.
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

/* Copyright (C) 
 * Andrea Montefusco IW0HDV
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <algorithm>    // for std::min std::max macros

#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

#if defined _MSC_VER
#include <strsafe.h>
#endif

#include "util.h"
#include "hpsdr.h"
#include "log.h"
#include "ExtIO_hermes.h"
#include "guiutil.h"
#include "gui.h"
#include "dllmain.h"

extern "C" EXTIO_API void __stdcall CloseHW();

#include "dllmain.h"

class ExtIODll : public Dll {
public:
	ExtIODll(HMODULE h) : Dll(h) {}

	void ProcessAttach()
	{
		// initialize Windows sockets
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		wVersionRequested = MAKEWORD(1, 1);
		err = WSAStartup(wVersionRequested, &wsaData);

	};
	void ProcessDetach() { CloseHW(); };
	void ThreadAttach() {};
	void ThreadDetach() {};
private:
};


template < typename ST >
Gui * ExtioMercuryRadio<ST>::CreateGui(int sr)
{
	return new MercuryGui(sr);
}

template < typename ST >
Gui * ExtioHermesRadio<ST>::CreateGui(int sr)
{
	return new HermesGui(sr);
}


void ExtioEthernet::FatalError(const char *pMsg) 
{ 
	LOGT("%s\n", "**************");

	if (pg) {
		pg->setHw(pMsg);
	} else {
		LOGT("%s\n", "NO GUI");
	}
};

void ExtioEthernet::TransmissionTmo(const char *pMsg) 
{ 
	LOGT("%s: [%s]\n", "TIMEOUT", pMsg);

	if (pg) {
		pg->setHw(pMsg);
	} else {
		LOGT("%s\n", "NO GUI");
	}
};

template <typename ST>
void ExtioHpsdrRadio<ST> :: setSampleRateHW(int new_sr)
{
	pR_->setSampleRate(new_sr);

	/* 100
		This status value indicates that a sampling frequency change has taken place,
		either by a hardware action, or by an interaction of the user with the DLL GUI.
		When the main program receives this status, it calls immediately after
		the GetHWSR() API to know the new sampling rate.
		We are calling the callback only for the first instance (otherwise Studio 1 is looping on Start/Stop cycle - TBI).
	*/
	if (*pExtioCallback && (::GetInstanceNumber()==1)) {
		LOGT("new sample rate: %d\n", new_sr);
		(*pExtioCallback) (-1, 100, 0., 0);
		pCr_->SendOtherInstancesNewSampleRate (new_sr);
	}
}

//
// Globals
//
DLL_CLASS(ExtIODll, hModule)

#pragma data_seg (".SS_EXTIO_HERMES")

// !!!! have to be initialized vars, due to shared segments rules constraints
unsigned char bufHR[ MAX(sizeof(ExtioMercuryRadio < EXTIO_BASE_TYPE>), sizeof(ExtioMercuryRadio < EXTIO_BASE_TYPE >)) ] SHARED = { 0 };
char bufHE [sizeof(ExtioEthernet)]	SHARED = { 0 };

#pragma data_seg ()


IdllComm < EXTIO_BASE_TYPE, EXTIO_NS > *rxIQ = 0;

EXTIO_RX_CALLBACK ExtioCallback = 0;


// pointers to abstract types
Radio  *pR = 0;
ExtioHpsdrRadio < EXTIO_BASE_TYPE > *pExr = 0;
Gui *pGui = 0;

HpsdrSplash *pSplash = 0;
ExtioEthernet *pExtioEth = 0;
CommandReceiver *pCmdRec = 0;

//
// Radio Factory Helper
//
template <
	typename EXTIO_BASE_TYPE
>
ExtioHpsdrRadio<EXTIO_BASE_TYPE> * CreateExtioHpsdrRadio (const char *board_id, CommandReceiver *pCr)
{
	RadioFactory<EXTIO_BASE_TYPE> rf;

	return rf.Create(board_id, bufHR, &ExtioCallback, pCr);
}

//
// discovery thread helper
//
pthread_t scan_thread_id;

void *scan_dev_thread(void *p)
{
	HpsdrSplash *pSplash = (HpsdrSplash *)p;

	LOGT("%s\n", "Starting radio scan");

	Ethernet::scan_devices (pSplash);

	return 0;
}



extern "C"
EXTIO_API bool __stdcall InitHW(char *name, char *model, int & extio_type)
{
	LOGT ("Instance #%d\n", GetInstanceNumber() ) ;

	static bool first = true;
	EXTIO_BASE_TYPE extio_type_;

	extio_type = extio_type_.value;

	if (first) {
		if (GetInstanceNumber() == 1) {
			first = false;
			ExtioCallback = NULL;

			if (pCmdRec == 0) pCmdRec = new CommandReceiver();

			pSplash = new HpsdrSplash(&pGui, &pCmdRec);

			int rc = ::pthread_create(&scan_thread_id, NULL, scan_dev_thread, (void *)pSplash);
			if (rc != 0) {
				LOGT("pthread_create failed on scan device thread: rc=%d\n", rc);
				return 0;
			}
			else {
				LOGT("%s\n", "scan device thread: thread succcessfully started");
			}
		}
	}
	strcpy(name, "HPSDR");
	strcpy(model, "unknown" );   // at this point scan is undergoing, so no answers available
	return true;
}

extern "C"
EXTIO_API bool __stdcall OpenHW()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	if (GetInstanceNumber() == 1) {
		if (pSplash) pSplash->Show();
		if (pGui) pGui->Show();
	} else {
		// setup command receiver for instances > 1
		if (pCmdRec == 0) pCmdRec = new CommandReceiver();
	}
	return true;
}

extern "C"
EXTIO_API int __stdcall StartHW(long LOfrequency)
{
	LOGT("Instance #%d LOfreq: %d\n", GetInstanceNumber(), LOfrequency);
	Ethernet::Device *pDev = 0;

	if (GetInstanceNumber() == 1) {

		if (!Ethernet::found()) {
			// signals to user that no proper hardware has been found
			GuiError x("No hardware found, unable to start receiver !");
			if (pSplash) pSplash->Hide();
			x.show();
			if (pSplash) pSplash->SetStatus("%s", (const char*)x), pSplash->Show();
			// return 0 is an error to the main program, hence the DSP processing is not started at all
			return 0;
		} else {
			pSplash->Hide();
		}

		if (pDev = Ethernet::found (pSplash->GetSel()) ) {

			if (pR == 0) {

				if (pGui) {
					// Gui and Radio already created in the Splash screen
					// only globals to be initialized
					pExr = pGui->getRadio();
					pR = pExr->getRadio();
				} else {
					// Create radio according to type discovered
					pExr = CreateExtioHpsdrRadio<EXTIO_BASE_TYPE>(pDev->board_id, pCmdRec);

					if (!pExr) {
						GuiError("Hardware unsupported, unable to start receiver !").show();
						return 0;
					} else {
						pR = pExr->getRadio(); // global pointer to Radio

						// create Gui and setup it
						pGui = pExr->CreateGui(EXTIO_DEFAULT_SAMPLE_RATE);
						pGui->setRadio(pExr);
					}
				}

				// The following call is really needed, in order to setup the samplerate
				{
					int new_sr;
					pR->getSampleRate( new_sr );
					LOGT("Instance #%d radio sample rate: %d\n", GetInstanceNumber(), new_sr);
					pExr->setSampleRateHW ( new_sr );
				}
				// create an Hpsdr flow object and assign to the Ethernet object
				Flow *pFlow = new Flow(pR);
				pExtioEth = new ExtioEthernet(pGui, pFlow);
			}

			pGui->setHwAddressGUI(pDev); // set the device pointer to gui
			pGui->Show();				 // and shows it
		}
		// here we have the receiver object created (pR != 0) 
		int act_sr;
		pR->getSampleRate(act_sr);
		LOGT("StartHW() before starting receive: SAMPLE RATE; %d #rx: %d\n", act_sr, pGui->getRecNumber());

		pR->setNumberOfRx(pGui->getRecNumber());// before the real receivers are started, 
												// we have to select how many receivers are to be used

		pExtioEth->startReceive(pExtioEth->found()); // finally, start the receiver(s)

		pR->setFrequency(LOfrequency); // establish the frequency

	} else { // for instances > 1
		pExtioEth = (ExtioEthernet *)bufHE;
		pDev = pExtioEth->found();
		
		// point local hermes radio object pointer to shared buffer
		RadioFactory<EXTIO_BASE_TYPE> rf;
		pExr = rf.Pointer (pDev->board_id, bufHR, pCmdRec);
		if (pExr) pR = pExr->getRadio();
	}

	//
	// all instances, even the first one, have to prepare to receive data from the main one
	//
	rxIQ = new IdllComm < EXTIO_BASE_TYPE, EXTIO_NS >(GetInstanceNumber() - 1, &ExtioCallback);

	if (rxIQ) {
		rxIQ->startReceive();
		LOGT("%s\n", "OK: rxIQ created !");
	} else {
		LOGT("%s\n", "FATAL: rxIQ not created !");
	}

	if (GetInstanceNumber() == 1) {
		// in first instance setup the internal data sender
		pExr->setIdllComm(new IntraComm());
		pGui->EnableControls();
	} else {
		LOGT("XXXXXXXXXXXXXXXX%s\n", "-");
		//
		// needed in order to set ExtIO callback !!!
		// 
		//pExr->setSampleRateHW(EXTIO_DEFAULT_SAMPLE_RATE);
		LOGT("2222222222222222%s\n", "-");

		// sanity checks
		//
		// first instance is:  #1
		// second instance is: #2
		// third instance is:  #3
		//....
		LOGT("33333333333333333%s\n", "-");
		if (pR) {
			if (GetInstanceNumber() > pR->getNumberOfRx()) {
				GuiError("Too many instances started, unable to start receiver !").show();
				return 0;
			}
			LOGT("44444444444444444%s\n", "-");
		} else {
			GuiError("Fatal error, no radio object instantiated !").show();
			return 0;
		}
	}
	if (pCmdRec && (GetInstanceNumber() == 1)) {
		LOGT("Sending start to other instances......%s\n", "-");
		pCmdRec->SendOtherInstancesStart();
	}

	return EXTIO_NS; // # of samples returned by callback
}

extern "C"
EXTIO_API int __stdcall GetStatus()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	return 0;
}

extern "C"
EXTIO_API void __stdcall StopHW()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	if (GetInstanceNumber() == 1) {
		pGui->appendMessage ("Hardware stopped by user request.\n");
		pExtioEth->stopReceive ();
		pGui->setHwAddressGUI(Ethernet::found(pSplash->GetSel()));
		if (pCmdRec && (GetInstanceNumber() == 1)) pCmdRec->SendOtherInstancesStop();
	}
	return;
}

extern "C"
EXTIO_API void __stdcall CloseHW()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	if ( GetInstanceNumber() == 1 ) delete pGui, delete pSplash;
	if (pCmdRec) delete pCmdRec;
	LOG_CLOSE;
	return;
}

extern "C"
EXTIO_API int __stdcall SetHWLO(long freq)
{
	LOGT("Instance #%d freq: %d (Radio *: 0x%p)\n", GetInstanceNumber(), freq, pR);
	
	if (freq < 10000) return -10000;
	if (freq > 60000000) return 60000000;
	if (pR) pR->setFrequency ( freq, GetInstanceNumber() - 1 ) ;
	if (pCmdRec && (GetInstanceNumber() == 1)) pCmdRec->SendOtherInstancesHWLO(freq);
	return 0;
}

extern "C"
EXTIO_API long __stdcall GetHWLO()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	long LOfreq;
	if (pR) pR->getFrequency(LOfreq, GetInstanceNumber() - 1);
	LOGT("   return LOfreq: %d\n", LOfreq);
	return LOfreq;
}

extern "C"
EXTIO_API long __stdcall GetHWSR()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	int sr = EXTIO_DEFAULT_SAMPLE_RATE;
	if (pR) pR->getSampleRate(sr);
	LOGT("   return: %d\n", sr);
	return sr;
}

extern "C"
EXTIO_API void __stdcall SetCallback (EXTIO_RX_CALLBACK parentCallBack)
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	ExtioCallback = parentCallBack;
	return;
}

extern "C"
EXTIO_API void __stdcall ShowGUI()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	if ( GetInstanceNumber() == 1 ) {
		if (pSplash) pSplash->Show();
		if (pGui) pGui->Show ();
	}
	return;
}

extern "C"
EXTIO_API void __stdcall HideGUI()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	if ( GetInstanceNumber() == 1 ) {
		if (pGui) pGui->Hide ();
		if (pSplash) pSplash->Hide();
	}
	return;
}

#if 0 // not currently used not needed with HPSDR hardware
extern "C"
EXTIO_HERMES_API void __stdcall IFLimitsChanged (long low, long high)
{
	LOG (("IFLimitsChanged() called\n"));
	
	return;
}

extern "C"
EXTIO_HERMES_API void __stdcall TuneChanged (long freq)
{
	LOG (("TuneChanged() called with freq: %d\n", freq));
	
    return;
}


extern "C"
EXTIO_HERMES_API void __stdcall RawDataReady(long samprate, int *Ldata, int *Rdata, int numsamples)
{
	return;
}

#endif


