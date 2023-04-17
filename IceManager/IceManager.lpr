program IceManager;

{$mode objfpc}{$H+}

uses
  {$IFDEF UNIX}
  cthreads,
  {$ENDIF}
  Interfaces, // this includes the LCL widgetset
  Forms, lazcontrols, bluetoothlaz, MainUnit, BTManager, IceBluetooth, IceType,
  EditStates;

{$R *.res}

begin
  RequireDerivedFormResource:=True;
  Application.Initialize;
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TEditStatesForm, EditStatesForm);
  Application.Run;
end.

