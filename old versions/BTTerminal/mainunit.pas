unit MainUnit;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, StdCtrls,
  Bluetooth, ctypes, Sockets, FPSocketReceiver;

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
    procedure LineFeedCBChange(Sender: TObject);
    procedure ScanBtnClick(Sender: TObject);
    procedure SendBtnClick(Sender: TObject);
    procedure ApplicationIdle(Sender: TObject; var Done: Boolean);
  private
    FDevices     : array of bdaddr_t;
    FSocket      : cint;
    FSocketThread: TFPSocketReceiver;
    FEndStr      : String;
  end;

var
  MainForm: TMainForm;

implementation

{$R *.lfm}

{ TMainForm }

procedure TMainForm.ScanBtnClick(Sender: TObject);
var
  device_id, device_sock: cint;
  scan_info: array[0..127] of inquiry_info;
  scan_info_ptr: Pinquiry_info;
  found_devices: cint;
  DevName: array[0..255] of Char;
  PDevName: PCChar;
  RemoteName: array[0..255] of Char;
  PRemoteName: PCChar;
  timeout1: Integer = 5;
  timeout2: Integer = 5000;

  I    : Integer;
  AName: String;
begin
  DevicesCB.Items.Clear;

  // get the id of the first bluetooth device.
  device_id := hci_get_route(nil);
  if (device_id < 0)
   then raise Exception.Create('No Bluetooth Hardware found');
  //else writeln('device_id = ',device_id);

  // create a socket to the device
  device_sock := hci_open_dev(device_id);
  if (device_sock < 0)
   then raise Exception.Create('Could not open Bluetooth Device');
  //else writeln('device_sock = ',device_sock);

  // scan for bluetooth devices for 'timeout1' seconds
  scan_info_ptr:=@scan_info[0];
  FillByte(scan_info[0],SizeOf(inquiry_info)*128,0);
  found_devices := hci_inquiry_1(device_id, timeout1, 128, nil, @scan_info_ptr, IREQ_CACHE_FLUSH);

  //showMessage('found_devices (if any) = ' + IntToStr(found_devices));

  if (found_devices < 0) then
    raise Exception.Create('FindBlueTooth: hci_inquiry')
  else begin
    SetLength(FDevices, found_devices);
    for I:=0 to found_devices-1 do begin
      PDevName:=@DevName[0];
      ba2str(@scan_info[I].bdaddr, PDevName);

      FDevices[I]:=scan_info[I].bdaddr;
      AName:=DevName;
      //ShowMessage('Bluetooth Device Address (bdaddr) DevName = ' + PChar(PDevName));

      PRemoteName:=@RemoteName[0];
      // Read the remote name for 'timeout2' milliseconds
      if (hci_read_remote_name(device_sock,@scan_info[I].bdaddr,255,PRemoteName,timeout2) >= 0)
        then AName:=RemoteName;

      DevicesCB.Items.Add(AName);
    end;
  end;

  CloseSocket(device_sock);
  //c_close(device_sock);
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
  fpsend(FSocket, @AMsg, L, 0);

  TerminalMemo.Text:=TerminalMemo.Text + #$D#$A + '>> ' + AStr + #$D#$A;
end;

procedure TMainForm.ConnectBtnClick(Sender: TObject);
var
  Addr     : sockaddr_rc;
  Status   : Integer;
  AStr     : array [0..255] of Char;
begin
  if DevicesCB.ItemIndex < 0
    then exit;
  ConnectBtn.Enabled:=false;
  DisconnectBtn.Enabled:=true;

  addr.rc_bdaddr:=FDevices[DevicesCB.ItemIndex];

  // allocate a socket
  FSocket:=fpsocket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // set the connection parameters (who to connect to)
  addr.rc_family:=AF_BLUETOOTH;
  addr.rc_channel:=1;

  // connect to server
  status:=fpconnect(FSocket, @addr, sizeof(addr));


  // send a message
  if status <> 0 then{ begin
    AStr:='hello!';
    status:=fpsend(FSocket, @AStr, 6, 0);
  end else }begin
    ShowMessage('Socket Error: ' + IntToStr(Sockets.socketerror));
  end;

  if status < 0
    then ShowMessage('uh oh');

  FSocketThread:=TFPSocketReceiver.Create(FSocket);
  FSocketThread.Start;

  Application.OnIdle:=@ApplicationIdle;
end;

procedure TMainForm.DisconnectBtnClick(Sender: TObject);
begin
  Application.OnIdle:=nil;

  ConnectBtn.Enabled:=true;
  DisconnectBtn.Enabled:=false;
  FSocketThread.Terminate;
end;

procedure TMainForm.FormCreate(Sender: TObject);
begin
  FEndStr:='';
end;

procedure TMainForm.LineFeedCBChange(Sender: TObject);
begin
  if CarriageReturnCB.Checked
    then FEndStr:=#$D
    else FEndStr:='';
  if LineFeedCB.Checked
    then FEndStr += #$A;
end;

procedure TMainForm.ApplicationIdle(Sender: TObject; var Done: Boolean);
var
  ASize: Integer;
  AStr : array [0..FPSocketReceiver.BUFFERSIZE] of Char;
begin
  ASize:=FSocketThread.Receive(AStr);
  AStr[ASize]:=#0;
  TerminalMemo.Text:=TerminalMemo.Text + AStr;
  Done:=true;
end;

end.

