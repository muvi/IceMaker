unit IceType;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils;

const
  BT_MSGM_CHECK         = 'C';
  BT_MSGM_QUERYSTATE    = 'Q';
  BT_MSGM_QUERYSTATES   = 'Y';
  BT_MSGM_SETSTATES     = 'S';

  BT_MSGS_CHECKRESPONSE = 'K';
  BT_MSGS_STATEINFO     = 'I';
  BT_MSGS_STATES        = 'S';
  BT_MSGS_SETSTATESOK   = 'O';
  BT_MSGS_SETSTATESERROR= 'E';

  NUMICEMODES           = 15;
  TIMEOFF               = LongWord($FFFFFFFF);

  STATEFLAG_CHANGEON_EVAPORATORCOLD = 2;
  STATEFLAG_CHANGEON_EVAPORATORHOT = 3;
  STATEFLAG_CHANGEON_BINEMPTY = 24;
  STATEFLAG_CHANGEON_BINFULL = 16;

  STATEFLAG_EVAPORATORHOT     = 1;
  STATEFLAG_EVAPORATORENABLED = 2;
  STATEFLAG_BINEMPTY          = 8;
  STATEFLAG_BINENABLED        = 16;
  STATEFLAG_INCCOUNTER = 64;

  RELAIS_COMPRESSOR = 8;
  RELAIS_WATER = 4;
  RELAIS_PUMP = 2;
  RELAIS_HOTGAS = 1;

type
  TIceMsgStateInfo = packed record
    TimeLeft   : LongWord;
    Batches    : Word;
    Description: array [0..20] of Char;
  end;

  TIceState        = packed record
    Time       : LongWord;
    Flags      : Byte;
    //Compressor, Water, Pump, Hot Gas
    RelaisState: Byte;
    NextState  : Byte;
    Reserved   : LongWord;
    Description: array [0..20] of Char;
  end;

  TIceModes        = packed record
    states    : array [0..NUMICEMODES-1] of TIceState;
    Mode2Entry: Byte;
  end;

  PIceState        = ^TIceState;

function HasFlag(AFlags, AFlag: Byte): Boolean;
procedure SetFlag(var AFlags: Byte; AFlag: Byte);
procedure RemoveFlag(var AFlags: Byte; AFlag: Byte);

implementation

function HasFlag(AFlags, AFlag: Byte): Boolean;
begin
  Result:=(AFlags and AFlag) = AFlag;
end;

procedure SetFlag(var AFlags: Byte; AFlag: Byte);
begin
  AFlags:=AFlags or AFlag;
end;

procedure RemoveFlag(var AFlags: Byte; AFlag: Byte);
begin
  AFlags:=AFlags and (not AFlag);
end;

end.

