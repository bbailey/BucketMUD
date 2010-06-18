//----------------------------------------------------------------------------
#ifndef EmberDialogH
#define EmberDialogH
//----------------------------------------------------------------------------
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
//----------------------------------------------------------------------------
class TSendDialog : public TForm
{
__published:        
	TEdit *Message;
	void __fastcall MessageKeyPress(TObject *Sender, char &Key);
private:
public:
	virtual __fastcall TSendDialog(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern TSendDialog *SendDialog;
//----------------------------------------------------------------------------
#endif    
