class TestBot extends ScriptedController native;

var int ChosenSkin;

function PostBeginPlay(){
	Super.PostBeginPlay();
}

function Destroyed(){
	Super.Destroyed();
}

function PawnDied(Pawn P){
	Super.PawnDied(P);
	GotoState('Dead');
}

function ServerRestartPlayer(){
	local MPPawn MPP;

	Level.Game.RestartPlayer(self);

	MPP = MPPawn(Pawn);

	if(MPP != None){
		MPP.ChosenSkin = ChosenSkin;
		MPP.DoCustomizations();
		MPP.PatrolRoute = BotSupport(Owner).BotPatrolRoute;
		GotoState('Idle');
	}
}

function StopFiring(){
	if((Pawn != None) && (Pawn.Weapon != None) && Pawn.Weapon.IsFiring())
		Pawn.Weapon.ServerStopFire();

	bFire = 0;
	bAltFire = 0;
}

/*
 * States
 */

// Idle
auto state Idle{
	function SeePlayer(Pawn Seen){
		log("SeePlayer: " $ GetHumanReadableName() $ " -> " $ Seen.GetHumanReadableName());
		Focus = Seen;
		Target = Seen;
		GotoState('Combat');
	}
}

// Combat
state Combat{
Begin:
	if(Pawn.Weapon.HasAmmo() && !Pawn.Weapon.IsFiring())
		Pawn.Fire();

	sleep(0);
	Goto('Begin');
}

// Dead
state Dead{
	ignores SeePlayer, HearNoise, KilledBy;

	function Timer(){}

	function BeginState(){
		if(Level.Game.TooManyBots(self)){
			Destroy();

			return;
		}

		Enemy = None;
		StopFiring();
	}

Begin:
	if(Level.Game.bGameEnded)
		GotoState('GameEnded');

	Sleep(0.25 + MPGame(Level.Game).RespawnWaitTime);
	ServerRestartPlayer();
	Goto('Begin');
MPStart:
	Sleep(0.75 + FRand());
	ServerRestartPlayer();
	Goto('Begin');
}

// GameEnded
state GameEnded{
	ignores SeePlayer, HearNoise, KilledBy, NotifyBump, HitWall, NotifyHeadVolumeChange, NotifyPhysicsVolumeChange, Falling, TakeDamage;

	function ServerReStartPlayer(){}

	function BeginState(){
		if(Pawn != None ){
			StopFiring();

			if(Pawn.Weapon != None)
				Pawn.Weapon.HolderDied();

			Pawn.bPhysicsAnimUpdate = false;
			Pawn.StopAnimating();
			Pawn.SimAnim[0].RateScale = 0;
			Pawn.SetCollision(true,false,false);
			Pawn.Velocity = vect(0,0,0);
			Pawn.SetPhysics(PHYS_None);
			Pawn.bIgnoreForces = true;

			UnPossess();
		}
	}
}

cpptext
{
	virtual INT Tick(FLOAT DeltaTime, ELevelTick TickType);
}

defaultproperties
{
	bIsPlayer=true
}
