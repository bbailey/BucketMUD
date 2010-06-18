//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#define CPP 1

#include <time.h>
#include "EmberForm.h"
#include "merc.h"
#include "EmberThread.h"

extern "C" int 		embermain	( int argc, char **argv );

//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Ember::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

extern bool MudDown;
extern char tbuff[MAX_STRING_LENGTH];

__fastcall Ember::Ember(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	FreeOnTerminate=false;
}
//---------------------------------------------------------------------------
void __fastcall Ember::Execute()
{
	char *argv[2];

	MudDown=false;
    StatusUp();
    argv[0]="embergui.exe";
    argv[1]=Port.c_str();
	embermain( 2, argv );
    MudDown=true;
    StatusDown();
}
//---------------------------------------------------------------------------
void __fastcall Ember::AddMessage()
{
	Form1->Channels->Lines->Add(tbuff);
}
//---------------------------------------------------------------------------

