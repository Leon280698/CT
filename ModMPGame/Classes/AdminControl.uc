class AdminControl extends Actor native config(ModMPGame);

var() config array<String> ServiceClasses;
var array<AdminService>    Services;

var() config string EventLogFile;
var() config bool   AppendEventLog;
var() config bool   EventLogTimestamp;

var bool          bPrintCommands;  // Commands are not executed but instead displayed (e.g. when the 'help' command is used)
var array<string> CurrentCommands; // Only used as temporary storage when bPrintCommands == true

var FunctionOverride GameInfoPostLoginOverride;
var FunctionOverride GameInfoLogoutOverride;

var array<PlayerController> Players;

native final function EventLog(coerce string Msg, name Event);
native final function SaveStats(PlayerController PC);
native final function RestoreStats(PlayerController PC);

function GameInfoPostLogin(PlayerController NewPlayer){
	Players[Players.Length] = NewPlayer;

	Level.Game.PostLogin(NewPlayer);

	EventLog(NewPlayer.PlayerReplicationInfo.PlayerName $ " entered the game", 'Join');
	RestoreStats(NewPlayer);
}

function GameInfoLogout(Controller Exiting){
	local PlayerController PC;
	local int i;

	PC = PlayerController(Exiting);

	if(PC != None){
		SaveStats(PC);

		if(!Level.Game.bGameEnded) // No need to log this at the end of the game when all players leave automatically
			EventLog(PC.PlayerReplicationInfo.PlayerName $ " left the game", 'Leave');

		for(i = 0; i < Players.Length; ++i){
			if(Players[i] == PC)
				Players.Remove(i, 1);
		}
	}

	Level.Game.Logout(Exiting);
}

function PostBeginPlay(){
	local int i;
	local Class<AdminService> ServiceClass;
	local AdminService Service;

	GameInfoPostLoginOverride = new class'FunctionOverride';
	GameInfoPostLoginOverride.Init(Level.Game, 'PostLogin', self, 'GameInfoPostLogin');
	GameInfoLogoutOverride = new class'FunctionOverride';
	GameInfoLogoutOverride.Init(Level.Game, 'Logout', self, 'GameInfoLogout');

	SaveConfig();

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
		Services[Services.Length] = Service;
	}
}

function bool DispatchCmd(PlayerController PC, string Cmd){
	local int i;
	local int j;
	local int NumLines;
	local bool RecognizedCmd;
	local bool bAdmin;

	bAdmin = PC == None || PC.PlayerReplicationInfo.bAdmin;

	for(i = 0; i < Services.Length; ++i){
		if(Services[i].bRequiresAdminPermissions && !bAdmin)
			continue;

		if(Services[i].ExecCmd(Cmd, PC))
			RecognizedCmd = true;

		if(CurrentCommands.Length > 0){ // 'help' command was used, so display the list of commands
			Services[i].CommandFeedback(PC, string(Services[i].Class.Name) $ ":", PC != None);
			++NumLines;

			for(j = 0; j < CurrentCommands.Length; ++j){
				Services[i].CommandFeedback(PC, "  - " $ CurrentCommands[j], PC != None);
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
	local int i;
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
		for(i = 0; i < Services.Length; ++i)
			Services[i].SaveConfig();

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
	EventLogFile="../Save/ServerEvents.log"
	AppendEventLog=true
	EventLogTimestamp=true
	ServiceClasses(0)="ModMPGame.AdminAuthentication"
	ServiceClasses(1)="ModMPGame.AdminCommands"
	ServiceClasses(2)="ModMPGame.BotSupport"
}
