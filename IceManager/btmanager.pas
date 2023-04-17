unit BTManager;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Bluetooth, ctypes, Sockets;

type
  TBTAddress = bdaddr_t;

  TBTManager = class
  private
    FDeviceCount : Integer;
    FDeviceNames : array of string;
    FSocket      : cint;
    FScanInfo    : array[0..127] of inquiry_info;
    function GetDeviceName(AIndex: Integer): string;
    function GetDeviceAddress(AIndex: Integer): TBTAddress;
    function isConnected: Boolean;
  public
    constructor Create;
    destructor Destroy; override;
    procedure ScanDevices;
    function Connect(ADevice: TBTAddress): Boolean;
    procedure Disconnect;
    function Send(var AValue; ASize: Integer): Integer;
    function Receive(out AValue; ASize: Integer): Integer;
    function ReceiveComplete(out AValue; ASize: Integer): Boolean;

    property Connected: Boolean read IsConnected;
    property DeviceCount: Integer read FDeviceCount;
    property DeviceNames[AIndex: Integer]: string read GetDeviceName;
    property DeviceAddresses[AIndex: Integer]: TBTAddress read GetDeviceAddress;
  end;


implementation

{%REGION TBTManager}

constructor TBTManager.Create;
begin
  inherited Create;
  FDeviceCount:=0;
  FSocket:=0;
end;

destructor TBTManager.Destroy;
begin
  if FSocket <> 0
    then Disconnect;
  inherited Destroy;
end;

procedure TBTManager.ScanDevices;
var
  device_id, device_sock: cint;
  scan_info_ptr         : Pinquiry_info;
  DevName               : array[0..255] of Char;
  PDevName              : PCChar;
  I                     : Integer;
const
  timeout1              : Integer = 5;
  timeout2              : Integer = 5000;
begin
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
  scan_info_ptr:=@FScanInfo[0];
  FillByte(FScanInfo[0],SizeOf(inquiry_info)*128,0);
  FDeviceCount:=hci_inquiry_1(device_id, timeout1, 128, nil, @scan_info_ptr, IREQ_CACHE_FLUSH);

  if (FDeviceCount < 0) then
    raise Exception.Create('FindBlueTooth: hci_inquiry');

  SetLength(FDeviceNames, FDeviceCount);
  for I:=0 to FDeviceCount-1 do begin
    PDevName:=@DevName[0];
    // Read the remote name for 'timeout2' milliseconds
    if (hci_read_remote_name(device_sock,@FScanInfo[I].bdaddr,255,PDevName,timeout2) >= 0) then begin
      FDeviceNames[I]:=DevName;
    end else begin
      PDevName:=@DevName[0];
      ba2str(@FScanInfo[I].bdaddr, PDevName);
      FDeviceNames[I]:=DevName;
    end;
  end;
  CloseSocket(device_sock);
end;

function TBTManager.Connect(ADevice: TBTAddress): Boolean;
var
  Addr     : sockaddr_rc;
  Status   : Integer;
  AStr     : array [0..255] of Char;
begin
  addr.rc_bdaddr:=ADevice;

  // allocate a socket
  FSocket:=fpsocket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // set the connection parameters (who to connect to)
  addr.rc_family:=AF_BLUETOOTH;
  addr.rc_channel:=1;

  // connect to server
  status:=fpconnect(FSocket, @addr, sizeof(addr));
  Result:=status = 0;
end;

procedure TBTManager.Disconnect;
begin
  CloseSocket(FSocket);
  FSocket:=0;
end;

function TBTManager.Send(var AValue; ASize: Integer): Integer;
begin
  Result:=fpsend(FSocket, @AValue, ASize, 0);
end;

function TBTManager.Receive(out AValue; ASize: Integer): Integer;
begin
  Result:=fprecv(FSocket, @AValue, ASize, 0);
end;

function TBTManager.ReceiveComplete(out AValue; ASize: Integer): Boolean;
var
  AValue2: Pointer;
  ARead  : Integer;
begin
  AValue2:=@AValue;
  repeat
    ARead:=Receive(AValue2^, ASize);
    if ARead < 0 then begin
      Result:=false;
      exit;
    end;
    ASize-=ARead;
    AValue2:=AValue2 + ARead;
  until ASize <= 0;
  Result:=true;
end;

function TBTManager.GetDeviceName(AIndex: Integer): string;
begin
  Result:=FDeviceNames[AIndex];
end;

function TBTManager.GetDeviceAddress(AIndex: Integer): TBTAddress;
begin
  Result:=FScanInfo[AIndex].bdaddr;
end;

function TBTManager.isConnected: Boolean;
begin
  Result:=FSocket <> 0;
end;

{%ENDREGION}

end.

