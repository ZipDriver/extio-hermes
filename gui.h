#if !defined __GUI_H__
#define __GUI_H__


struct Device;
class GuiImpl;
class Hermes;

class Gui {
public:
	Gui (int sample_rate);
	~Gui ();

	void Show ();
	void Hide ();

	void SetHw (const char *);
	void AppendMessage (const char *);
	int  getRecNumber (void);
	void HermesSetHwAddressGUI (const Ethernet::Device *);

	void setRadio (Hermes *pR);

	friend GuiImpl;
//private:
	GuiImpl *pi;
};

#endif
