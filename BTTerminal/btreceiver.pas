unit BTReceiver;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, SyncObjs, BTManager;

const BUFFERSIZE = 1024;

type
  TBTReceiver    = class (TThread)
  private
    FBluetooth : TBTManager;
    FBuffer    : array [0..BUFFERSIZE-1] of byte;
    FBufferSize: Integer;
    FBufferLock: TCriticalSection;
    FOnReceive : TNotifyEvent;
  protected
    procedure Execute; override;
  public
    constructor Create(ABluetooth: TBTManager);
    destructor Destroy; override;
    function Receive(var Value): Integer;
    procedure Stop;

    property OnReceive: TNotifyEvent read FOnReceive write FOnReceive;
  end;

implementation

{%REGION TBTReceiver}

constructor TBTReceiver.Create(ABluetooth: TBTManager);
begin
  inherited Create(false);
  FBluetooth:=ABluetooth;
  FreeOnTerminate:=false;
  FBufferSize:=0;
  FBufferLock:=TCriticalSection.Create;
end;

destructor TBTReceiver.Destroy;
begin
  FBufferLock.Destroy;
  inherited Destroy;
end;

function TBTReceiver.Receive(var Value): Integer;
begin
  FBufferLock.Enter;

  Result:=FBufferSize;
  Move(FBuffer, Value, FBufferSize);
  FBufferSize:=0;
  if Terminated then Start;

  FBufferLock.Leave;
end;

procedure TBTReceiver.Stop;
begin
  FBluetooth.Disconnect;
  FreeOnTerminate:=true;
  Terminate;
end;

procedure TBTReceiver.Execute;
var
  Val: Byte;
begin
  while not Terminated do begin
    if FBluetooth.Receive(Val, 1) = 1 then begin
      FBufferLock.Enter;

      FBuffer[FBufferSize]:=Val;
      Inc(FBufferSize);
      if FBufferSize >= BUFFERSIZE then begin
        FBufferLock.Leave;
        exit;
      end;

      FBufferLock.Leave;

      if Assigned(FOnReceive)
        then FOnReceive(Self);
    end;
  end;
end;

{%ENDREGION}

end.

