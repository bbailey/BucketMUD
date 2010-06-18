//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("EmberForm.cpp", Form1);
USEUNIT("EmberThread.cpp");
USEFORM("EmberDialog.cpp", SendDialog);
USEUNIT("..\update.c");
USEUNIT("..\act_info.c");
USEUNIT("..\act_move.c");
USEUNIT("..\act_obj.c");
USEUNIT("..\act_wiz.c");
USEUNIT("..\bit.c");
USEUNIT("..\board.c");
USEUNIT("..\clan.c");
USEUNIT("..\comm.c");
USEUNIT("..\const.c");
USEUNIT("..\db.c");
USEUNIT("..\fight.c");
USEUNIT("..\handler.c");
USEUNIT("..\interp.c");
USEUNIT("..\magic.c");
USEUNIT("..\mem.c");
USEUNIT("..\mob_commands.c");
USEUNIT("..\mob_prog.c");
USEUNIT("..\olc.c");
USEUNIT("..\olc_act.c");
USEUNIT("..\olc_save.c");
USEUNIT("..\random.c");
USEUNIT("..\save.c");
USEUNIT("..\skills.c");
USEUNIT("..\special.c");
USEUNIT("..\string.c");
USEUNIT("..\act_comm.c");
USEUNIT("..\Win32Common\win32util.c");
USEUNIT("..\Win32Common\crypt_util.c");
USERES("EmberGUI.res");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TForm1), &Form1);
        Application->CreateForm(__classid(TSendDialog), &SendDialog);
        Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
