unit MainUnit;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, StdCtrls,
  BTReceiver, BTManager;

type

  { TMainForm }

  TMainForm = class(TForm)
    LineFeedCB: TCheckBox;
    CarriageReturnCB: TCheckBox;
    SendBtn: TButton;
    DisconnectBtn: TButton;
    ConnectBtn: TButton;
    DevicesCB: TComboBox;
    TerminalEdit: TEdit;
    TerminalMemo: TMemo;
    ScanBtn: TButton;
    procedure ConnectBtnClick(Sender: TObject);
    procedure DisconnectBtnClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure LineFeedCBChange(Sender: TObject);
    procedure ScanBtnClick(Sender: TObject);
    procedure SendBtnClick(Sender: TObject);
    procedure ApplicationIdle(Sender: TObject; var Done: Boolean);
  private
    FBluetooth: TBTManager;
    FReceiver : TBTReceiver;
    FEndStr   : String;
    function EscapeStr(const AStr; ASize: Integer): string;
  end;

var
  MainForm: TMainForm;

implementation

{$R *.lfm}

{ TMainForm }

procedure TMainForm.ScanBtnClick(Sender: TObject);
var
  I    : Integer;
  AName: String;
begin
  DevicesCB.Items.Clear;
  FBluetooth.ScanDevices;
  for I:=0 to FBluetooth.DeviceCount-1
    do DevicesCB.Items.Add(FBluetooth.DeviceNames[I]);
end;

procedure TMainForm.SendBtnClick(Sender: TObject);
var
  AStr: String;
  AMsg: array [0..1023] of Char;
  I, L: Integer;
begin
  AStr:=TerminalEdit.Text + FEndStr;
  L:=Length(AStr);
  for I:=1 to 1024 do begin
    if I > L then begin
      break;
    end;
    AMsg[I-1]:=AStr[I];
  end;

  AStr:=TerminalEdit.Text;
  if L <= 1024 then begin
    TerminalEdit.Text:='';
  end else begin
    Delete(AStr, 1, 1024);
    TerminalEdit.Text:=AStr;
    L:=1024;
  end;
  FBluetooth.Send(AMsg, L);

  TerminalMemo.Text:=TerminalMemo.Text + #$D#$A + '>> ' + AStr + #$D#$A;
end;

procedure TMainForm.ConnectBtnClick(Sender: TObject);
var
  Status   : Integer;
  AStr     : array [0..255] of Char;
begin
  if DevicesCB.ItemIndex < 0
    then exit;
  ConnectBtn.Enabled:=false;
  DisconnectBtn.Enabled:=true;

  if not FBluetooth.Connect(FBluetooth.DeviceAddresses[DevicesCB.ItemIndex])
    then ShowMessage('Connection failed');

  FReceiver:=TBTReceiver.Create(FBluetooth);
  FReceiver.Start;

  Application.OnIdle:=@ApplicationIdle;
end;

procedure TMainForm.DisconnectBtnClick(Sender: TObject);
begin
  Application.OnIdle:=nil;

  ConnectBtn.Enabled:=true;
  DisconnectBtn.Enabled:=false;
  FReceiver.Stop;
end;

procedure TMainForm.FormCreate(Sender: TObject);
begin
  FEndStr:='';
  FBluetooth:=TBTManager.Create;
end;

procedure TMainForm.FormDestroy(Sender: TObject);
begin
  FBluetooth.Destroy;
end;

procedure TMainForm.LineFeedCBChange(Sender: TObject);
begin
  if CarriageReturnCB.Checked
    then FEndStr:=#$D
    else FEndStr:='';
  if LineFeedCB.Checked
    then FEndStr += #$A;
end;

function TMainForm.EscapeStr(const AStr; ASize: Integer): string;
var
  I    : Integer;
  AChar: Char;
  APos : PChar;
begin
  Result:='';
  APos:=@AStr;
  for I:=0 to ASize-1 do begin
    AChar:=APos^;
    if (AChar < ' ') or (AChar > '~') then begin
      Result += '\' + IntToStr(Ord(AChar));
    end else Result += AChar;
    Inc(APos);
  end;
end;

procedure TMainForm.ApplicationIdle(Sender: TObject; var Done: Boolean);
var
  ASize: Integer;
  AStr : array [0..BTReceiver.BUFFERSIZE] of Char;
begin
  ASize:=FReceiver.Receive(AStr);
  TerminalMemo.Text:=TerminalMemo.Text + EscapeStr(AStr, ASize);
  Done:=true;
end;

end.

