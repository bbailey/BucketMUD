//---------------------------------------------------------------------------
#ifndef EmberFormH
#define EmberFormH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Menus.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Mask.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TPopupMenu *PopupMenu1;
	TMenuItem *ClearLog;
	TMainMenu *MainMenu1;
	TMenuItem *File1;
	TMenuItem *Help1;
	TMenuItem *Restart;
	TMenuItem *Shutdown;
	TMenuItem *ClearLog1;
	TMenuItem *HotBoot1;
	TMenuItem *Reboot1;
	TMenuItem *Shutdown1;
	TMenuItem *NoHelpforYou1;
	TMenuItem *N1;
	TPageControl *Pages;
	TStatusBar *MudStatus;
	TTabSheet *LogSheet;
	TTabSheet *StatSheet;
	TTabSheet *UserSheet;
	TPanel *Panel2;
	TImageList *Images;
	TTimer *Timey;
	TMenuItem *N2;
	TMenuItem *Exit1;
	TTimer *DownTimer;
	TMenuItem *Startup1;
	TMenuItem *Startup2;
	TLabel *Affects;
	TLabel *Areas;
	TLabel *ExDes;
	TLabel *Exits;
	TLabel *Helps;
	TLabel *Socials;
	TLabel *Mobs;
	TLabel *InUse;
	TLabel *Objects;
	TLabel *Resets;
	TLabel *Rooms;
	TLabel *Shops;
	TLabel *Strings;
	TLabel *Perms;
	TLabel *Users;
	TTabSheet *ChannelSheet;
	TRichEdit *Channels;
	TPanel *Panel3;
	TRichEdit *LogMemo;
	TTreeView *UserTree;
	TPanel *Panel4;
	TPopupMenu *PopupMenu2;
	TMenuItem *Kick1;
	TMenuItem *Ban1;
	TMenuItem *N3;
	TMenuItem *SendMessage1;
	TMenuItem *Freeze1;
	TMenuItem *Restore1;
	TMenuItem *RestoreAll1;
	TEdit *ChanMessage;
	TComboBox *ChannelBox;
	TTabSheet *OptionSheet;
	TLabel *Label1;
	TEdit *ConsoleName;
	TCheckBox *CheckVisible;
	TLabel *Label2;
	TCheckBox *CheckMinimized;
	TLabel *Label3;
	TMaskEdit *RestartEdit;
	TCheckBox *CheckStartOnline;
	TLabel *Label4;
	TBevel *Bevel1;
	TEdit *PortEdit;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall TimeyTimer(TObject *Sender);
	void __fastcall Exit1Click(TObject *Sender);
	void __fastcall Shutdown1Click(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall DownTimerTimer(TObject *Sender);

	void __fastcall Startup1Click(TObject *Sender);
	void __fastcall Reboot1Click(TObject *Sender);
	void __fastcall ClearLog1Click(TObject *Sender);
	void __fastcall UserTreeMouseDown(TObject *Sender, TMouseButton Button,	TShiftState Shift, int x, int y);
	void __fastcall Kick1Click(TObject *Sender);
	void __fastcall Freeze1Click(TObject *Sender);
	void __fastcall Ban1Click(TObject *Sender);
	void __fastcall Restore1Click(TObject *Sender);
	void __fastcall RestoreAll1Click(TObject *Sender);
	void __fastcall SendMessage1Click(TObject *Sender);
	void __fastcall ChanMessageKeyPress(TObject *Sender, char &Key);
	
	void __fastcall PortEditKeyPress(TObject *Sender, char &Key);
	void __fastcall FormCreate(TObject *Sender);
	
	void __fastcall ConsoleNameChange(TObject *Sender);
private:	// User declarations
	void __fastcall WndProc(Messages::TMessage &Message);
	void __fastcall AppMinimize(TObject *Sender);
    void __fastcall AppRestore(TObject *Sender);
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TForm1 *Form1;
//---------------------------------------------------------------------------
void	StatusDown(void);
void	StatusUp(void);
//---------------------------------------------------------------------------
#endif
