//---------------------------------------------------------------------------
#ifndef EmberThreadH
#define EmberThreadH


//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
//---------------------------------------------------------------------------
extern "C" void		ChannelMessage (char *mesg, CHAR_DATA *ch);

class Ember : public TThread
{
friend void		ChannelMessage (char *mesg, CHAR_DATA *ch);
private:
protected:
public:
	AnsiString Port;
//    bool HotBoot; No HotBoot yet for Win32 - Zane
	void __fastcall Execute();
	__fastcall Ember(bool CreateSuspended);
	void __fastcall AddMessage();
};
//---------------------------------------------------------------------------
#endif
