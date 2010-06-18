//---------------------------------------------------------------------------
#include <vcl.h>

#include <vcl\registry.hpp>
#include <windows.h>
#pragma hdrstop

#define CPP
#define CONSOLE 4
#define MAX_STRING      5242880

#include "EmberForm.h"
#include <time.h>
#include <shellapi.h>
#include <io.h>
#include <process.h>
extern "C" {
#include "merc.h"
}
#include "EmberThread.h"
#include "EmberDialog.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"

extern "C" {
void 	do_shutdown	( CHAR_DATA *ch, char *argument );
void 	do_reboot	( CHAR_DATA *ch, char *argument );
char *	remove_color( const char *str );
void    	logf_string	( const char *str, ... );
void		log_string	( const char *str );
void 	insert_sort (CHAR_DATA *who_list[300], CHAR_DATA *ch, int length);
void		AddUser		(CHAR_DATA *ch);
void		RemoveUser	(CHAR_DATA *ch);
void		ChannelMessage (char *mesg, CHAR_DATA *ch);
void 	do_disconnect( CHAR_DATA *ch, char *argument );
void 	do_freeze	( CHAR_DATA *ch, char *argument );
void		do_ban		( CHAR_DATA *ch, char *argument );
void		do_restore	( CHAR_DATA *ch, char *argument );
void 	do_shout	( CHAR_DATA *ch, char *argument );
void 	do_immtalk	( CHAR_DATA *ch, char *argument );
void 	do_question	( CHAR_DATA *ch, char *argument );
void		do_gossip	( CHAR_DATA *ch, char *argument );
void		do_answer	( CHAR_DATA *ch, char *argument );
void		do_auction	( CHAR_DATA *ch, char *argument );
void		do_sendinfo	( CHAR_DATA *ch, char *argument );
void		do_echo		( CHAR_DATA *ch, char *argument );
};

void 				SetStatus	(char *Message);
bool				LogSave		(void);
void				UpdateStats (void);
void				MudStart	(void);
void				LoadRegistry(void);
void				SaveRegistry(void);

extern time_t 		current_time;
extern bool			merc_down;
extern int			social_count;
extern int			mobile_count;
extern int			nAllocString;
extern int			sAllocString;
extern int			nAllocPerm;
extern int			sAllocPerm;

TForm1 *Form1;
Ember *EmberThread;
NOTIFYICONDATA *IconData;
TIcon *TrayIcon;
int LastLog=-1;
bool LogFull=false;
bool NightlySave=true;
tm *TempTime;
bool MudDown=true;
bool ShuttingDown=false;
bool SaveLogfile=false;
bool Reboot=false;
bool LogChanged=false;
CHAR_DATA *ConsoleChar;
int UserCount=0;
char tbuff[MAX_STRING_LENGTH];
int RestartTime=0;
bool DownFromGui=false;
char tname[MAX_STRING_LENGTH];

#define LOG_DIR 		"..\\log\\"	// Log Directory

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	char buff[200];
    int TempLength;

	IconData=new NOTIFYICONDATA;
    TrayIcon=new TIcon;
    Application->OnMinimize=AppMinimize;
    Application->OnRestore=AppRestore;
    _wtzset();
    ConsoleChar=new CHAR_DATA;
    ConsoleChar->Class=CONSOLE;
    ConsoleChar->level=MAX_LEVEL;
    ConsoleChar->desc=NULL;
    ConsoleChar->act=PLR_CONSOLE;
    ConsoleChar->next=NULL;
    ConsoleChar->next_in_room=NULL;
    ConsoleChar->master=NULL;
    ConsoleChar->fighting=NULL;
    ConsoleChar->reply=NULL;
    ConsoleChar->pet=NULL;
    ConsoleChar->memory=NULL;
//    ConsoleChar->spec_fun=NULL;
    ConsoleChar->pIndexData=NULL;
    ConsoleChar->desc=NULL;
    ConsoleChar->affected=NULL;
    ConsoleChar->carrying=NULL;
    ConsoleChar->in_room=NULL;
    ConsoleChar->was_in_room=NULL;
    ConsoleChar->pcdata=NULL;
    ConsoleChar->gen_data=NULL;
    ConsoleChar->TNode=NULL;
    Pages->ActivePage=LogSheet;
    Form1->ChannelBox->ItemIndex=0;
    strcpy(buff, remove_color(EMBER_MUD_VERSION));
	TempLength=strlen(buff);
    buff[TempLength-1]=0;
    buff[TempLength-2]=0;
    Caption=buff;
    LoadRegistry();
    if (CheckVisible->State == cbChecked)
	    ConsoleChar->comm=VIS_CONSOLE;
    else
    	ConsoleChar->comm=0;
    strcpy(tname, Form1->ConsoleName->Text.c_str());
    ConsoleChar->name=tname;
    RestartTime=RestartEdit->Text.ToInt() * 1000;
    if (CheckStartOnline->State == cbChecked)
    	{
       	EmberThread=new Ember(true);
		EmberThread->Port=Form1->PortEdit->Text;
        EmberThread->Resume();
        }
}
//---------------------------------------------------------------------------
void log_string( const char *str )
{
	if (LogFull)
    	return;

	char Output[4096];
    char *strtime;
    int LogResult;

    LogChanged=true;
    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    sprintf( Output, "%s :: %s", strtime, str );
	LogResult=Form1->LogMemo->Lines->Add(Output);
    if (LastLog==LogResult)
    	{
        char Temp[4096];

		if (LogSave())
		    sprintf( Temp, "%s :: Log file saved.", strtime);
        else
		    sprintf( Temp, "%s :: Error saving log file!", strtime);
		Form1->LogMemo->Lines->Clear();
    	Form1->LogMemo->Lines->Add(Temp);
        LogResult=Form1->LogMemo->Lines->Add(Output);
        }
    LastLog=LogResult;
    return;
}
//---------------------------------------------------------------------------
void logf_string( const char *str, ... )
{
	if (LogFull)
    	return;

	char Output[4096];
    char *strtime;
    char list[ 4096*4 ];
    int LogResult;
    {
        va_list param;

        va_start(param, str);
        vsprintf( list, str, param );
        va_end(param);
    }

	LogChanged=true;
    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    sprintf( Output, "%s :: %s", strtime, list );
	LogResult=Form1->LogMemo->Lines->Add(Output);
    if (LastLog==LogResult)
    	{
        char Temp[4096];

		if (LogSave())
		    sprintf( Temp, "%s :: Log file saved.", strtime);
        else
		    sprintf( Temp, "%s :: Error saving log file!", strtime);
		Form1->LogMemo->Lines->Clear();
    	Form1->LogMemo->Lines->Add(Temp);
        LogResult=Form1->LogMemo->Lines->Add(Output);
        }
    LastLog=LogResult;
    return;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormShow(TObject *Sender)
{
    Form1->Images->GetIcon(0,TrayIcon);

    IconData->cbSize=sizeof(NOTIFYICONDATA);
    IconData->hWnd=Form1->Handle;
    IconData->uID=9000;
    IconData->uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
    IconData->uCallbackMessage=9000;
    IconData->hIcon=TrayIcon->Handle;
    strcpy(IconData->szTip,Form1->Caption.c_str());
}
//---------------------------------------------------------------------------
void SetStatus(char *Message)
{
	Form1->MudStatus->SimpleText=Message;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AppMinimize(TObject *Sender)
{
	ShowWindow(Application->Handle, SW_HIDE);
    Shell_NotifyIcon(NIM_ADD, IconData);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AppRestore(TObject *Sender)
{
    Shell_NotifyIcon(NIM_DELETE, IconData);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::WndProc(Messages::TMessage &Message)
{
  	if(  (Message.Msg    == 9000)  && (Message.LParam == 515) )
		{
		ShowWindow(Application->Handle, SW_SHOW);
        Application->Restore();
		Form1->LogMemo->Perform(EM_LINESCROLL,0,Form1->LogMemo->Lines->Count-1);
		}
	else
    	TForm::WndProc(Message);
}
//---------------------------------------------------------------------------
bool LogSave(void)
{
    LogFull=true;

	char LogFile[4096];
	char TimeString[4096];
	tm *CurrTm;
    time_t t;
    int x=0;
    long Error;

    t=time(NULL);
    CurrTm=gmtime(&t);
    sprintf(TimeString,"%02d%02d%02d",CurrTm->tm_mon+1,CurrTm->tm_mday,CurrTm->tm_year);
	sprintf(LogFile, "%sEmber-%s.log",LOG_DIR,TimeString);
    if (!access(LogFile,0))
    	do {
        	x++;
        	sprintf(LogFile,"%sEmber-%s-%d.log",LOG_DIR,TimeString,x);
        } while (!access(LogFile,0));
    try
	{
	    Form1->LogMemo->Lines->SaveToFile(LogFile);
	}
	catch (Exception &exception)
    {
        return false;
    }
    LastLog=-1;
    LogFull=false;
    LogChanged=false;
    return true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TimeyTimer(TObject *Sender)
{
	TempTime=gmtime(&current_time);
    if ((TempTime->tm_min==59)&&(TempTime->tm_hour==23)&&(NightlySave))
    	{
        char *strtime;

	    strtime = ctime( &current_time );
	    strtime[strlen(strtime)-1] = '\0';
        if (LogSave())
        	{
    		Form1->LogMemo->Lines->Clear();
			logf_string("%s :: Nightly log file saved successfully.", strtime);
            }
        else
        	{
            Form1->LogMemo->Lines->Clear();
            logf_string("%s :: Unable to save Nightly log!", strtime);
            }
        NightlySave=false;
        }
    if ((!NightlySave)&&(TempTime->tm_hour!=23))
    	NightlySave=true;
    if (!merc_down && Form1->Pages->ActivePage==Form1->StatSheet)
    	UpdateStats();
}
//---------------------------------------------------------------------------
void UpdateStats(void)
{
    char buf[MAX_STRING_LENGTH];
	sprintf( buf, "Affects %5d\n\r", top_affect    );
    Form1->Affects->Caption=buf;
    sprintf( buf, "Areas   %5d\n\r", top_area      );
    Form1->Areas->Caption=buf;
    sprintf( buf, "ExDes   %5d\n\r", top_ed        );
    Form1->ExDes->Caption=buf;
    sprintf( buf, "Exits   %5d\n\r", top_exit      );
    Form1->Exits->Caption=buf;
    sprintf( buf, "Helps   %5d\n\r", top_help      );
	Form1->Helps->Caption=buf;
    sprintf( buf, "Socials %5d\n\r", social_count  );
    Form1->Socials->Caption=buf;
    sprintf( buf, "Mobs    %5d\n\r", top_mob_index );
    Form1->Mobs->Caption=buf;
    sprintf( buf, "(in use)%5d\n\r", mobile_count  );
    Form1->InUse->Caption=buf;
    sprintf( buf, "Objs    %5d\n\r", top_obj_index );
    Form1->Objects->Caption=buf;
    sprintf( buf, "Resets  %5d\n\r", top_reset     );
    Form1->Resets->Caption=buf;
    sprintf( buf, "Rooms   %5d\n\r", top_room      );
    Form1->Rooms->Caption=buf;
    sprintf( buf, "Shops   %5d\n\r", top_shop      );
    Form1->Shops->Caption=buf;

    sprintf( buf, "Strings %5d strings of %7d bytes (max %d).\n\r",
	nAllocString, sAllocString, MAX_STRING );
    Form1->Strings->Caption=buf;
    sprintf( buf, "Perms   %5d blocks  of %7d bytes.\n\r",
	nAllocPerm, sAllocPerm );
    Form1->Perms->Caption=buf;

    UserCount=0;

    for ( DESCRIPTOR_DATA * d = descriptor_list ; d ; d = d->next )
		if ( (d->connected == CON_PLAYING) || (d->connected == CON_NOTE_TO) ||
		(d->connected == CON_NOTE_SUBJECT) || (d->connected == CON_NOTE_EXPIRE)
		|| (d->connected == CON_NOTE_TEXT) || (d->connected == CON_NOTE_FINISH))
		    UserCount++;

    sprintf(buf, "Users   %5d", UserCount);
    Form1->Users->Caption=buf;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Exit1Click(TObject *Sender)
{
    Form1->Close();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Shutdown1Click(TObject *Sender)
{
	if (merc_down)
    	{
        ShowMessage("EmberMUD isn't currently running.");
        return;
        }
	do_shutdown(ConsoleChar, "");
   	SaveLogfile=true;
    DownFromGui=true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	if (!merc_down)
		do_shutdown(ConsoleChar, "");
    ShuttingDown=true;
    if (MudDown)
    	CanClose=true;
    else
    	CanClose=false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    Shell_NotifyIcon(NIM_DELETE, IconData);
	if (SaveLogfile)
    	LogSave();
	SaveRegistry();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DownTimerTimer(TObject *Sender)
{
	if (ShuttingDown)
    	{
        SaveLogfile=true;
        Form1->Close();
        return;
        }
    if ((SaveLogfile) && (MudDown))
    	{
        LogSave();
        SaveLogfile=false;
        }
    if (Reboot)
        MudStart();
    if (!MudDown)
    	{
   		RestartTime-=DownTimer->Interval;
	    if (RestartTime<=0)
    		MudStart();
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Startup1Click(TObject *Sender)
{
	if (!MudDown)
    	{
    	ShowMessage("EmberMUD is already running.");
        return;
        }
	if (EmberThread)
    	MudStart();
    else
    	{
       	EmberThread=new Ember(true);
		EmberThread->Port=Form1->PortEdit->Text;
        EmberThread->Resume();
        }
}
//---------------------------------------------------------------------------
void MudStart(void)
{
	SaveRegistry();
    execl("..\\src\\embergui.exe", "embergui.exe", NULL);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Reboot1Click(TObject *Sender)
{
	do_reboot(ConsoleChar, "");
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ClearLog1Click(TObject *Sender)
{
	if (LogChanged)
    	LogSave();
    Form1->LogMemo->Lines->Clear();
}
//---------------------------------------------------------------------------
void StatusUp(void)
{
	Form1->MudStatus->SimpleText="EmberMUD Running.";
}
//---------------------------------------------------------------------------
void StatusDown(void)
{
	Form1->MudStatus->SimpleText="EmberMUD Down.";
}
//---------------------------------------------------------------------------
void AddUser(CHAR_DATA *ch)
{
    TTreeNode *TempNode;

	ch->TNode=(void *)Form1->UserTree->Items->Add(Form1->UserTree->Items->GetFirstNode(), ch->name);
    TempNode=(TTreeNode *)ch->TNode;
    TempNode->Data=(void *)ch;
}
//---------------------------------------------------------------------------
void RemoveUser(CHAR_DATA *ch)
{
	if (!ch)
    	{
        log_string("Error: NULL pointer passed to RemoveUser(CHAR_DATA *ch)");
        return;
        }
	Form1->UserTree->Items->Delete((TTreeNode *)ch->TNode);
}
//---------------------------------------------------------------------------
void ChannelMessage(char *mesg, CHAR_DATA *ch)
{
	int length;

    if (ch)
        sprintf(tbuff, "%s%s", ch->name, remove_color(mesg));
    else
    	sprintf(tbuff, "%s", remove_color(mesg));
	length=strlen(tbuff);
    tbuff[length-1]=0;
    tbuff[length-2]=0;
	EmberThread->Synchronize(EmberThread->AddMessage);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UserTreeMouseDown(TObject *Sender, TMouseButton Button,	TShiftState Shift, int x, int y)
{
	if (Button=mbRight)
    	{
        TTreeNode *TempNode;

        TempNode=Form1->UserTree->GetNodeAt(x, y);
        if(TempNode)
        	{
        	Form1->UserTree->Selected=TempNode;
            Form1->Kick1->Visible=true;
            Form1->Freeze1->Visible=true;
            Form1->Ban1->Visible=true;
            Form1->N3->Visible=true;
            Form1->SendMessage1->Visible=true;
            Form1->Restore1->Visible=true;
            }
        else
        	{
            Form1->Kick1->Visible=false;
            Form1->Freeze1->Visible=false;
            Form1->Ban1->Visible=false;
            Form1->N3->Visible=false;
            Form1->SendMessage1->Visible=false;
            Form1->Restore1->Visible=false;
            }
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Kick1Click(TObject *Sender)
{
	CHAR_DATA *ch;

    ch=(CHAR_DATA *) Form1->UserTree->Selected->Data;
	do_disconnect(ConsoleChar,ch->name);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Freeze1Click(TObject *Sender)
{
	CHAR_DATA *ch;

    ch=(CHAR_DATA *) Form1->UserTree->Selected->Data;
	do_freeze(ConsoleChar,ch->name);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Ban1Click(TObject *Sender)
{
	CHAR_DATA *ch;

    ch=(CHAR_DATA *) Form1->UserTree->Selected->Data;
	do_ban(ConsoleChar,ch->desc->host);
    Kick1Click(this);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Restore1Click(TObject *Sender)
{
	CHAR_DATA *ch;

    ch=(CHAR_DATA *) Form1->UserTree->Selected->Data;
	do_restore(ConsoleChar,ch->name);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RestoreAll1Click(TObject *Sender)
{
	do_restore(ConsoleChar, "all");
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SendMessage1Click(TObject *Sender)
{
	SendDialog->Visible=true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ChanMessageKeyPress(TObject *Sender, char &Key)
{
	if (Key == 13)
    	{
		if (merc_down || MudDown)
    		{
        	ShowMessage("Sorry, EmberMUD is currently down.");
	    	return;
    	    }
        if (Form1->ChanMessage->Text == "")
        	return;
        switch (Form1->ChannelBox->ItemIndex)
        	{
            	case 0:
                	do_gossip(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
					break;
                case 1:
                	do_shout(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
                	break;
                case 2:
                	do_auction(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
                	break;
                case 3:
                	do_question(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
                	break;
                case 4:
                	do_answer(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
                	break;
                case 5:
                	do_immtalk(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
                	break;
                case 6:
                	do_sendinfo(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
					break;
                case 7:
                	do_echo(ConsoleChar, Form1->ChanMessage->Text.c_str());
                    Form1->ChanMessage->Clear();
                	break;
                default:
                	ShowMessage("Unhandled Channel selected.");
                    return;
                    break;
            }
        }
}
//---------------------------------------------------------------------------
void LoadRegistry(void)
{
	TRegistry *MyReg = new TRegistry;

    MyReg->RootKey = HKEY_LOCAL_MACHINE;

	if (MyReg->OpenKey("\\Software\\EmberMUD", FALSE))
   	{
    	try
      	{
            Form1->ConsoleName->Text=MyReg->ReadString("ConsoleName");
        }
        catch (...)
       	{
            ;
        }
    	try
      	{
            Form1->PortEdit->Text=MyReg->ReadString("Port");
        }
        catch (...)
       	{
            ;
        }
    	try
      	{
            Form1->RestartEdit->Text=MyReg->ReadString("RestartInterval");
        }
        catch (...)
       	{
            ;
        }
    	try
       	{
            Form1->CheckVisible->State=MyReg->ReadInteger("Visible");
        }
        catch (...)
       	{
            ;
        }
    	try
      	{
            Form1->CheckMinimized->State=MyReg->ReadInteger("StartMinimized");
        }
        catch (...)
        {
    	    ;
	    }
    	try
      	{
            Form1->CheckStartOnline->State=MyReg->ReadInteger("StartOnline");
        }
        catch (...)
      	{
            ;
	    }
    }
}
//---------------------------------------------------------------------------
void SaveRegistry(void)
{
	TRegistry *MyReg = new TRegistry;

    MyReg->RootKey = HKEY_LOCAL_MACHINE;

	if (MyReg->OpenKey("\\Software\\EmberMUD", TRUE))
   	{
    	try
        {
        	MyReg->WriteString("ConsoleName", Form1->ConsoleName->Text.c_str());
        }
        catch (...)
        {
        	;
        }
    	try
        {
        	MyReg->WriteString("Port", Form1->PortEdit->Text.c_str());
        }
        catch (...)
        {
        	;
        }
    	try
        {
        	MyReg->WriteString("RestartInterval", Form1->RestartEdit->Text.c_str());
        }
        catch (...)
        {
        	;
        }
    	try
        {
        	MyReg->WriteInteger("Visible", Form1->CheckVisible->State);
        }
        catch (...)
        {
        	;
        }
    	try
        {
        	MyReg->WriteInteger("StartMinimized", Form1->CheckMinimized->State);
        }
        catch (...)
        {
        	;
        }
    	try
        {
        	MyReg->WriteInteger("StartOnline", Form1->CheckStartOnline->State);
        }
        catch (...)
        {
        	;
        }
   	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::PortEditKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_SPACE || (isalpha(Key) && !isdigit(Key)))
    	{
    	ShowMessage("Numbers only please.");
        Key=0;
        }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
	if (CheckMinimized->State == cbChecked)
    	Application->Minimize();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ConsoleNameChange(TObject *Sender)
{
    strcpy(tname, Form1->ConsoleName->Text.c_str());
}
//---------------------------------------------------------------------------
