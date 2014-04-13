/**
* @file gui.h
* @brief Header for Extio DLL GUI
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

#if !defined __GUI_H__
#define		 __GUI_H__

class GuiImpl;
struct GuiEvent;
struct GuiEventHScroll;

class Gui : public MsgAllocator {
public:
	Gui();
	Gui(int resource_id);

	virtual ~Gui();

	virtual void Show();
	virtual void Hide();

	void setRadio(ExtioHpsdrRadio < EXTIO_BASE_TYPE > *pExr);
	ExtioHpsdrRadio < EXTIO_BASE_TYPE > * getRadio();

	void setHw(const char *);
	void appendMessage(const char *);
	int  getRecNumber(void);
	void setHwAddressGUI(const Ethernet::Device *);

	virtual void EnableControls();
	virtual void DisableControls();

protected:
	void EnableAll(const GuiEvent& ev1, const GuiEvent& ev2);
	// events managers
	virtual bool OnInit(const GuiEvent& ev) = 0;
	virtual bool ButtonClick(const GuiEvent &ev) { return false; }
	virtual bool ButtonDoubleClick(const GuiEvent &ev) { return false; }
	virtual bool ListBoxDoubleClick(const GuiEvent &ev) { return false; }
	virtual bool OnHScroll(const GuiEventHScroll& ev) { return false; }
	virtual bool OnWmUser(int n, const GuiEvent& ev) { return false; }

	friend GuiImpl;
	GuiImpl *pi;
};



class HermesGui: public Gui {
public:
	HermesGui (int sample_rate);
	~HermesGui () {}

	void EnableControls();
	void DisableControls();

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool ButtonClick(const GuiEvent &ev);
	virtual bool OnHScroll(const GuiEventHScroll& ev);
	virtual bool OnWmUser(int n, const GuiEvent& ev);

private:
	int sr;
};

class MercuryGui: public Gui {
public:
	MercuryGui (int sample_rate);
	~MercuryGui () {}

	void EnableControls();
	void DisableControls();

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool ButtonClick(const GuiEvent &ev);
	virtual bool OnHScroll(const GuiEventHScroll& ev);
	virtual bool OnWmUser(int n, const GuiEvent& ev);

private:
	int sr;
};

class CommandReceiver;

class HpsdrSplash: public Gui, public ScanWatcher {
public:
	HpsdrSplash(Gui **pG, CommandReceiver **pCr);
	~HpsdrSplash() {}
	void Show();

	void SetHw(const char *);
	template <typename... ARGS>
	void SetStatus (const char *, ARGS... args);
	void AppendMessage(const char *); 

	virtual int ScanStarted();
	virtual int ScanStopped(int nh);
	virtual int	InterfaceFound(Ethernet::NetInterface *pni);
	virtual int	DeviceFound(Ethernet::Device *pd);
	int GetSel() { return sel; }

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool OnWmUser(int n, const GuiEvent& ev);
	virtual bool ListBoxDoubleClick(const GuiEvent &ev);

private:
	int sel;
	Gui **ppGui_;
	CommandReceiver **ppCr_;
	Ethernet::Device *pDev;
};

class CommandReceiver: public Gui {
public:
	CommandReceiver ();
	~CommandReceiver ();
	virtual bool OnInit(const GuiEvent& ev);
	
	void SendOtherInstancesNewSampleRate (unsigned int nsr);
	void SendOtherInstancesStart ();
	void SendOtherInstancesStop ();
	//void SendOtherInstancesHWLO(long freq);
	void SendOtherInstancesClose ();

	virtual bool OnWmUser(int n, const GuiEvent& ev);

protected:
	
};
#endif
