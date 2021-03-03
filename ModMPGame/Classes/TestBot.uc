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
		GotoState('BotAI');
	}
}

/*
 * States
 */

// Default state
auto state BotAI{

}

// Dead
state Dead{
	ignores SeePlayer, HearNoise, KilledBy;

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

defaultproperties
{
	bIsPlayer=true
}
