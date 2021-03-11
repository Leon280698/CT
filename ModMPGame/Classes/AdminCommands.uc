class AdminCommands extends AdminService;

var() config bool bAllowConsoleCommands;

function bool ExecCmd(String Cmd, optional PlayerController PC){
	local PlayerReplicationInfo PRI;
	local String CommandResult;
	local int IntParam;
	local string StringParam;
	local Controller C;

	if(ParseCommand(Cmd, "CMD")){
		if(bAllowConsoleCommands || IsLocalPlayer(PC)){ // The host can always execute console commands
			StringParam = Cmd;

			if(IsLocalPlayer(PC)){
				CommandResult = ConsoleCommand(Cmd);
			}else if(ParseCommand(StringParam, "GET") || ParseCommand(StringParam, "SET")){
				if(InStr(Caps(StringParam), "ADMINPASSWORD") == -1) // Security measure
					CommandResult = ConsoleCommand(Cmd);
				else
					CommandResult = "Remote players are not allowed to access the admin password";
			}else{
				CommandResult = "Remote players are only allowed to use the get and set commands"; // Might still mess things up but at least some access should be provided
			}

			if(CommandResult != "")
				CommandFeedback(PC, CommandResult);
		}else{
			CommandFeedback(PC, "Console commands are not allowed!");
		}

		return true;
	}else if(ParseCommand(Cmd, "LISTPLAYERS")){ // Mostly useful for the console since ingame you can just look at the scoreboard
		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None){
				PRI = C.PlayerReplicationInfo;
				CommandResult = "name=" $ PRI.PlayerName $ " id=" $ PRI.PlayerID $ " kills=" $ int(PRI.Score) $ " deaths=" $ int(PRI.Deaths);

				CommandFeedback(PC, CommandResult, PC != None);
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "KICK")){
		StringParam = ParseToken(Cmd);

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC){
				PRI = C.PlayerReplicationInfo;

				if(PRI.PlayerName ~= StringParam)
					AdminAccessControl(Level.Game.AccessControl).KickPlayerController(PlayerController(C), Cmd);
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "KICKALL")){
		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC) // Don't kick yourself
				AdminAccessControl(Level.Game.AccessControl).KickPlayerController(PlayerController(C));
		}

		return true;
	}else if(ParseCommand(Cmd, "KICKID")){
		IntParam = int(ParseToken(Cmd));

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC){
				PRI = C.PlayerReplicationInfo;

				if(PRI.PlayerID == IntParam){
					AdminAccessControl(Level.Game.AccessControl).KickPlayerController(PlayerController(C), Cmd);

					break;
				}
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "KICKSCORE")){
		IntParam = int(Cmd);

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC){
				PRI = C.PlayerReplicationInfo;

				if(PRI.Score == IntParam)
					AdminAccessControl(Level.Game.AccessControl).KickPlayerController(PlayerController(C));
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "KICKSCOREBELOW")){
		IntParam = int(ParseToken(Cmd));

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC){
				PRI = C.PlayerReplicationInfo;

				if(PRI.Score < IntParam)
					AdminAccessControl(Level.Game.AccessControl).KickPlayerController(PlayerController(C));
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "KICKSCOREABOVE")){
		IntParam = int(Cmd);

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC){
				PRI = C.PlayerReplicationInfo;

				if(PRI.Score > IntParam)
					AdminAccessControl(Level.Game.AccessControl).KickPlayerController(PlayerController(C));
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "BAN")){
		StringParam = ParseToken(Cmd);

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC){
				PRI = C.PlayerReplicationInfo;

				if(PRI.PlayerName ~= StringParam)
					AdminAccessControl(Level.Game.AccessControl).BanPlayerController(PlayerController(C), Cmd);
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "BANID")){
		IntParam = int(ParseToken(Cmd));

		for(C = Level.ControllerList; C != None; C = C.nextController){
			if(PlayerController(C) != None && C != PC){
				PRI = C.PlayerReplicationInfo;

				if(PRI.PlayerID == IntParam){
					AdminAccessControl(Level.Game.AccessControl).BanPlayerController(PlayerController(C), Cmd);

					break;
				}
			}
		}

		return true;
	}else if (ParseCommand(Cmd, "PROMOTE")){
		for(C = Level.ControllerList; C != None; C = C.nextController){
			PRI = C.PlayerReplicationInfo;

			if(C.IsA('PlayerController') && !PRI.bAdmin && PRI.PlayerName ~= Cmd){
				PRI.bAdmin = true;

				if(PC != None)
					StringParam = PRI.PlayerName;
				else
					StringParam = "the server";

				CommandResult = PRI.PlayerName $ " was promoted to admin by " $ StringParam;

				Log(CommandResult);
				Level.Game.Broadcast(self, CommandResult);
				AdminControl.SaveStats(PlayerController(C));

				break;
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "DEMOTE")){
		for(C = Level.ControllerList; C != None; C = C.nextController){
			PRI = C.PlayerReplicationInfo;

			if(PRI.bAdmin && PRI.PlayerName ~= Cmd){
				PRI.bAdmin = false;

				if(PC != None)
					StringParam = PRI.PlayerName;
				else
					StringParam = "the server";

				CommandResult = PRI.PlayerName $ "'s admin priviledges were revoked by " $ StringParam;

				Log(CommandResult);
				Level.Game.Broadcast(self, CommandResult);
				AdminControl.SaveStats(PlayerController(C));

				break;
			}
		}

		return true;
	}else if(ParseCommand(Cmd, "SWITCHMAP")){
		Level.Game.Broadcast(self, "Switching map");
		Level.ServerTravel(Cmd, false);

		return true;
	}else if(ParseCommand(Cmd, "RESTARTMAP")){
		Level.Game.Broadcast(self, "Restarting map");
		Level.ServerTravel("?restart", false);

		return true;
	}

	return false;
}

defaultproperties
{
	bAllowConsoleCommands=true
}
