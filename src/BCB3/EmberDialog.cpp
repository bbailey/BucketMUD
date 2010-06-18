//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#define CPP

#include <time.h>
#include "EmberDialog.h"
#include "EmberForm.h"
#include "merc.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"

extern "C" void 		do_tell( CHAR_DATA *ch, char *argument );

extern CHAR_DATA *ConsoleChar;

TSendDialog *SendDialog;
//---------------------------------------------------------------------
__fastcall TSendDialog::TSendDialog(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TSendDialog::MessageKeyPress(TObject *Sender, char &Key)
{
	switch(Key)
    	{
        case 13:
        	if (!Message->Text.Length())
            	{
                Visible=false;
                break;
                }
            char buff[MAX_STRING_LENGTH];
            sprintf(buff, "%s %s", Form1->UserTree->Selected->Text.c_str(),
            		Message->Text.c_str());
        	do_tell(ConsoleChar, buff);
            Visible=false;
            Message->Clear();
            break;
        case 27:
        	Visible=false;
        	Message->Clear();
            break;
        default:
        	break;
        }
}
//---------------------------------------------------------------------------
