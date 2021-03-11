class AdminControl extends GameStats native config(ModMPGame);

var() config array<String> ServiceClasses;

var() config string EventLogFile;
var() config bool   AppendEventLog;
var() config bool   EventLogTimestamp;
var() config bool   DoStatLogging;

var bool          bPrintCommands;  // Commands are not executed but instead displayed (e.g. when the 'help' command is used)
var array<string> CurrentCommands; // Only used as temporary storage when bPrintCommands == true
var AdminService  Services;        // Linked list of all currently active services

native final function EventLog(coerce string Msg, name Tag);
native final function SaveStats(PlayerController PC);
native final function RestoreStats(PlayerController PC);

function Init(){
	if(DoStatLogging)
		Super.Init();
}

function ConnectEvent(PlayerReplicationInfo Who){
	EventLog(Who.PlayerName $ " entered the game", 'Join');
	RestoreStats(PlayerController(Who.Owner));
	Super.ConnectEvent(Who);
}

function DisconnectEvent(PlayerReplicationInfo Who){
	EventLog(Who.PlayerName $ " left the game", 'Leave');
	SaveStats(PlayerController(Who.Owner));
	Super.DisconnectEvent(Who);
}

function PostBeginPlay(){
	local int i;
	local class<AdminService> ServiceClass;
	local AdminService Service;

	if(Level.Game.GameStats != None && Level.Game.GameStats != self){
		Warn("GameStats will be replaced by AdminControl!");
	}else if(Level.Game.GameStats.IsA('AdminControl')){
		Destroy();

		return;
	}

	EventLog(GetMapFileName(), 'Map');
	SaveConfig(); // Create config if it doesn't exist

	Level.Game.GameStats = self;
	Level.Game.bAdminCanPause = false;
	Level.Game.BroadcastHandlerClass = "ModMPGame.AdminControlBroadcastHandler";
	Level.Game.AccessControlClass = "ModMPGame.AdminAccessControl";

	if(Level.Game.BroadcastHandler != None && !Level.Game.BroadcastHandler.IsA('AdminControlBroadcastHandler')){
		Level.Game.BroadcastHandler.Destroy();
		Level.Game.BroadcastHandler = Spawn(class'AdminControlBroadcastHandler');
	}

	if(Level.Game.AccessControl != None && !Level.Game.AccessControl.IsA('AdminAccessControl')){
		Level.Game.AccessControl.Destroy();
		Level.Game.AccessControl = Spawn(class'AdminAccessControl');
	}

	Level.Game.SaveConfig();

	for(i = 0; i < ServiceClasses.Length; ++i){
		ServiceClass = Class<AdminService>(DynamicLoadObject(ServiceClasses[i], class'Class'));

		if(ServiceClass == None){
			Warn("'" $ ServiceClasses[i] $ "' is not a subclass of AdminService");

			continue;
		}

		if(!Level.Game.IsA(ServiceClass.default.RelevantGameInfoClass.Name))
			continue; // Service is not relevant for current game mode so don't spawn it

		Log("Spawning actor for admin service class '" $ ServiceClass $ "'");
		Service = Spawn(ServiceClass);

		if(Service == None){
			Warn("Unable to spawn admin service '" $ ServiceClass $ "'");

			continue;
		}

		Service.AdminControl = self;
		Service.nextAdminService = Services;
		Services = Service;
	}
}

function bool DispatchCmd(PlayerController PC, string Cmd){
	local int i;
	local AdminService Service;
	local int NumLines;
	local bool RecognizedCmd;
	local bool bAdmin;

	bAdmin = PC == None || PC.PlayerReplicationInfo.bAdmin;

	for(Service = Services; Service != None; Service = Service.nextAdminService){
		if(Service.bRequiresAdminPermissions && !bAdmin)
			continue;

		if(Service.ExecCmd(Cmd, PC))
			RecognizedCmd = true;

		if(CurrentCommands.Length > 0){ // 'help' command was used, so display the list of commands
			Service.CommandFeedback(PC, string(Service.Class.Name) $ ":", PC != None);
			++NumLines;

			for(i = 0; i < CurrentCommands.Length; ++i){
				Service.CommandFeedback(PC, "  - " $ CurrentCommands[i], PC != None);
				++NumLines;
			}

			CurrentCommands.Length = 0;
		}
	}

	if(bPrintCommands && PC != None && NumLines > 6) // Not all commands fit in chat but they are in the console
		PC.ClientMessage("Open console for the complete list of available commands");

	return RecognizedCmd || bPrintCommands;
}

event bool ExecCmd(string Cmd, optional PlayerController PC){
	local AdminService Service;
	local bool RecognizedCmd;
	local string CommandSource;

	// Common engine commands are ignored
	if(Left(Cmd, 5) ~= "XLIVE" || Left(Cmd, 7) ~= "GETPING" || Left(Cmd, 13) ~= "GETCURRENTRES" || Left(Cmd, 8) ~= "SETMOUSE")
		return false;

	if(Cmd ~= "HELP")
		bPrintCommands = true;

	if(PC != None)
		CommandSource = PC.PlayerReplicationInfo.PlayerName;
	else
		CommandSource = "ServerConsole";

	EventLog("(" $ CommandSource $ "): " $ Cmd, 'Command');

	if((PC == None || PC.PlayerReplicationInfo.bAdmin) && Cmd ~= "SAVECONFIG"){
		for(Service = Services; Service != None; Service = Service.nextAdminService)
			Service.SaveConfig();

		SaveConfig();

		RecognizedCmd = true;
	}else{
		RecognizedCmd = DispatchCmd(PC, Cmd);
	}

	if(PC != None && !RecognizedCmd){
		if(PC.PlayerReplicationInfo.bAdmin)
			PC.ClientMessage("Unrecognized command");
		else
			PC.ClientMessage("Unrecognized command or missing permissions");
	}

	bPrintCommands = false;

	return RecognizedCmd;
}

cpptext
{
	// Overrides
	virtual void Spawned();
	virtual void Destroy();

	void EventLog(const TCHAR* Msg, FName Event);
}

defaultproperties
{
	bHidden=true
	EventLogFile="ServerEvents.log"
	AppendEventLog=true
	EventLogTimestamp=true
	ServiceClasses(0)="ModMPGame.AdminAuthentication"
	ServiceClasses(1)="ModMPGame.AdminCommands"
	ServiceClasses(2)="ModMPGame.BotSupport"
}
