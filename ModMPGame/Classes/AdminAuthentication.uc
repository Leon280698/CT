class AdminAuthentication extends AdminService;

function bool ExecCmd(String Cmd, optional PlayerController PC){
	if(PC != None){
		if(ParseCommand(Cmd, "LOGIN")){
			if(PC.PlayerReplicationInfo.bAdmin){
				CommandFeedback(PC, "You are already logged in!", true);
			}else if(Level.Game.AccessControl.AdminLogin(PC, Cmd)){
				PC.PlayerReplicationInfo.bAdmin = true;
				AdminControl.SaveStats(PC);
			}else{
				CommandFeedback(PC, "Wrong password!");
			}

			return true;
		}else if(ParseCommand(Cmd, "LOGOUT")){
			if(PC.PlayerReplicationInfo.bAdmin){
				PC.PlayerReplicationInfo.bAdmin = false;
				CommandFeedback(PC, "You are no longer an admin!", true);
				AdminControl.SaveStats(PC);
			}else{
				CommandFeedback(PC, "You are not logged in!", true);
			}

			return true;
		}
	}

	return false;
}

defaultproperties
{
	bRequiresAdminPermissions=false // This must be false or players won't be able to use the /login command
}
