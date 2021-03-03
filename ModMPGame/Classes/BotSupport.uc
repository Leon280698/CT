class BotSupport extends AdminService native;

#exec OBJ LOAD FILE="MPGame.u"

var() config int            NumBots;             // Number of bots which are spawned when the game begins
var() config float          BotAccuracy;
var Array<TestBot>          Bots;
var() config bool           bBotsCountAsPlayers; // If this is true, adding bots will increase the player count of the server

var config bool             bAutoImportPaths;
var config bool             bAutoBuildPaths;
var config bool             bShowPaths;

var bool                    bPathsImported;       // Paths were imported using ImportPaths
var bool                    bPathsHaveChanged;
var bool                    bShowPathsOnClients;
var Array<vector>           NavPtFailLocations;   // Used to debug Navigation points which failed to spawn
var Array<Pawn.PatrolPoint> BotPatrolRoute;
var Array<Actor>            NavigationPointIcons; // Intangible actors used to make navigation points visible to clients

function PostBeginPlay(){
	local int i;

	BotAccuracy = FClamp(BotAccuracy, 0.0, 1.0);

	for(i = 0; i < NumBots; ++i)
		AddBot();
}

function ShowPathsClient(){
	local NavigationPoint N;
	local Actor A;

	foreach AllActors(class'NavigationPoint', N){
		A = Spawn(class'IntangibleActor',,, N.Location);

		if(A != None){
			A.SetDrawType(DT_Sprite);
			A.Texture = N.Texture;
			NavigationPointIcons[NavigationPointIcons.Length] = A;
		}
	}

	bPathsHaveChanged = false;
}

function HidePathsClient(){
	local int i;

	for(i = 0; i < NavigationPointIcons.Length; ++i)
		NavigationPointIcons[i].Destroy();

	NavigationPointIcons.Length = 0;
}

function bool ExecCmd(String Cmd, optional PlayerController PC){
	local int i;
	local class<Pawn> PawnClass;
	local string StringParam;
	local NavigationPoint N;
	local vector SpawnPos;
	local rotator SpawnRot;
	local Pawn P;

	if(ParseCommand(Cmd, "ADDBOT")){
		ParseStringParam(Cmd, "NAME=", StringParam);
		ParseIntParam(Cmd, "TEAM=", i);
		AddBot(StringParam, i);

		return true;
	}else if(ParseCommand(Cmd, "REMOVEBOT")){
		ParseStringParam(Cmd, "NAME=", StringParam);
		RemoveBot(StringParam);

		return true;
	}else if(ParseCommand(Cmd, "REMOVEALLBOTS")){
		while(Bots.Length > 0)
			RemoveBot();

		return true;
	}else if(ParseCommand(Cmd, "SETBOTACCURACY")){
		BotAccuracy = FClamp(float(Cmd), 0.0, 1.0);

		//for(i = 0; i < Bots.Length; ++i)
			//Bots[i].SetAccuracy(BotAccuracy);

		return true;
	}else if(ParseCommand(Cmd, "SPAWN")){
		StringParam = ParseToken(Cmd);

		if(StringParam != ""){
			PawnClass = class<Pawn>(DynamicLoadObject(StringParam, class'Class', true));

			// Allowing omitting package name for convenience for 'CTCharacters' and 'Properties'

			if(PawnClass == None)
				PawnClass = class<Pawn>(DynamicLoadObject("CTCharacters." $ StringParam, class'Class', true));

			if(PawnClass == None)
				PawnClass = class<Pawn>(DynamicLoadObject("Properties." $ StringParam, class'Class', true));

			if(PawnClass != None){
				if(PC != None){
					if(PC.Pawn != None){
						SpawnRot = PC.GetViewRotation();
						SpawnRot.Pitch = 0;
						SpawnRot.Roll = 0;
						SpawnPos = PC.Pawn.Location + vector(SpawnRot) * (PC.Pawn.CollisionRadius + PawnClass.default.CollisionRadius + 8);
					}else{
						SpawnPos = PC.Location;
					}
				}else{ // Pick a random navigation point for pawns spawned from the server console
					for(N = Level.NavigationPointList; N != None; N = N.nextNavigationPoint){
						if(!N.taken && Rand(10) > 7){
							SpawnPos = N.Location;

							break;
						}
					}

					if(N == None)
						SpawnPos = Level.NavigationPointList.Location;
				}

				P = Spawn(PawnClass,,, SpawnPos);

				if(P != None){
					P.Controller = Spawn(P.ControllerClass);

					if(P.Controller != None){
						P.Controller.Possess(P);
						P.Controller.PlayerReplicationInfo = Spawn(class'MPPlayerReplicationInfo');
						P.Controller.InitPlayerReplicationInfo();
						P.Controller.PlayerReplicationInfo.PlayerName = string(P.Class.Name);
					}

					P.AddDefaultInventory();

					if(ParseIntParam(Cmd, "TEAM=", i))
						P.TeamIndex = i;
				}else{
					CommandFeedback(PC, "Unable to spawn pawn, try again at a different location");
				}
			}else{
				CommandFeedback(PC, "Pawn class '" $ StringParam $ "' not found");
			}
		}else if(PC != None){
			CommandFeedback(PC, "Expected class name");
		}

		return true;
	}else if(!IsLocalPlayer(PC)){
		if(ParseCommand(Cmd, "SHOWPATHS")){
			if(!bShowPathsOnClients)
				ShowPathsClient();

			bShowPathsOnClients = true;

			return true;
		}else if(ParseCommand(Cmd, "HIDEPATHS")){
			if(bShowPathsOnClients)
				HidePathsClient();

			bShowPathsOnClients = false;

			return true;
		}
	}

	if(Super.ExecCmd(Cmd, PC)){
		if(bPathsHaveChanged && bShowPathsOnClients){ // A native command might have changed the navigation points so we have to regenerate the dummy actors
			HidePathsClient();
			ShowPathsClient();
		}

		return true;
	}

	return false;
}

event SetupPatrolRoute(){
	local NavigationPoint NavPt;
	local Pawn.PatrolPoint P;

	BotPatrolRoute.Length = 0;

	for(NavPt = Level.NavigationPointList; NavPt != None; NavPt = NavPt.nextNavigationPoint){
		if(PatrolPoint(NavPt) != None){
			P.Node = NavPt;
			P.RunToNode = true;
			P.ShootWhileMoving = true;
			P.OrientToNode = true;
			P.PatrolPriorityOverride = 0.3;
			BotPatrolRoute[BotPatrolRoute.Length] = P;
		}
	}
}

function AddBot(optional string Name, optional int Team){
	local TestBot Bot;

	if(bBotsCountAsPlayers && Level.Game.NumPlayers >= Level.Game.MaxPlayers){
		Log("Game is full and bBotsCountAsPlayers == true");

		return;
	}

	Bot = Spawn(class'TestBot', self);

	if(Bot != None){
		//Bot.Accuracy = BotAccuracy;

		if(Name == "")
			Bot.PlayerReplicationInfo.PlayerName = "Bot" $ Bots.Length;
		else
			Bot.PlayerReplicationInfo.PlayerName = Name;

		//Bot.bCanGesture = false;
		Bot.ChosenSkin = Rand(5);
		Bot.GotoState('Dead', 'MPStart');
		Bots[Bots.Length] = Bot;

		BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, Bot.PlayerReplicationInfo);

		if(bBotsCountAsPlayers)
			++Level.Game.NumPlayers;

		++Level.Game.NumBots;

		if(Level.Game.bTeamGame)
			Level.Game.ChangeTeam(Bot, Team, false, false);
	}
}

function RemoveBot(optional string Name){
	local int i;

	if(Bots.Length > 0){
		if(Name == ""){
			i = Bots.Length - 1;
		}else{
			do{
				if(Bots[i].PlayerReplicationInfo.PlayerName ~= Name)
					break;

				++i;
			}until(i == Bots.Length);
		}

		if(i < Bots.Length){
			if(Bots[i].Pawn != None)
				Bots[i].Pawn.Destroy();

			Bots[i].Destroy();
			Bots.Length = i;

			if(bBotsCountAsPlayers)
				--Level.Game.NumPlayers;

			--Level.Game.NumBots;
		}
	}
}

cpptext
{
	void SpawnNavigationPoint(UClass* NavPtClass, const FVector& Location, const FRotator& Rotation = FRotator(0, 0, 0));
	void ImportPaths();
	void ExportPaths();
	void BuildPaths();
	void ClearPaths();

	// Overrides
	virtual void Spawned();
	virtual UBOOL Tick(FLOAT DeltaTime, ELevelTick TickType);
	virtual void PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI);
	virtual bool ExecCmd(const char* Cmd, class APlayerController* PC);
}

defaultproperties
{
	BotAccuracy=1.0
	bHidden=true
	bAutoImportPaths=true
	bAutoBuildPaths=false
	bShowPaths=false
}
