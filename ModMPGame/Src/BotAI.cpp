#include "../Inc/ModMPGame.h"

void ABotSupport::ControllerTick(FLOAT DeltaTime){
	for(AController* C = Level->ControllerList; C; C = C->nextController){
		if(C->IsA<APlayerController>()){
			if(C->SightCounter < 0.0f)
				C->SightCounter += 0.2f;
		}else if(C->SightCounter < 0.0f){
			C->SightCounter += 0.15f + appFrand() * 0.1f;
		}

		C->SightCounter -= DeltaTime;

		if(C->Pawn && !C->Pawn->bHidden)
			ControllerShowSelf(C);
	}
}

bool ABotSupport::ControllerSeePawn(AController* C, APawn* Other, bool bMaySkipChecks){
	if(!Other || !C->Pawn)
		return false;

	if(Other == C->Enemy)
		return C->LineOfSightTo(Other) != 0;

	C->bLOSflag = !C->bLOSflag;

	APawn* Pawn = C->Pawn;
	FLOAT maxdist = Pawn->SightRadius * Min(1.0f, (FLOAT)(Other->Visibility * 0.0078125f)); // * 1/128

	// fixed max sight distance
	if((Other->Location - Pawn->Location).SizeSquared() > maxdist * maxdist)
		return false;

	FLOAT dist = (Other->Location - Pawn->Location).Size();

	// may skip if more than 1/5 of maxdist away (longer time to acquire)
	if(bMaySkipChecks && (appFrand() * dist > 0.1f * maxdist))
		return false;

	// check field of view
	FVector SightDir = (Other->Location - Pawn->Location).SafeNormal();
	FVector LookDir = Rotation.Vector();
	FLOAT Stimulus = (SightDir | LookDir);

	if(Stimulus < Pawn->PeripheralVision)
		return false;

	if(bMaySkipChecks && (appFrand() * dist > 0.1f * maxdist)){
		// lower FOV vertically
		SightDir.Z *= 2.f;
		SightDir.Normalize();

		if((SightDir | LookDir) < Pawn->PeripheralVision)
			return false;

		// notice other pawns at very different heights more slowly
		FLOAT heightMod = Abs(Other->Location.Z - Pawn->Location.Z);

		if(appFrand() * dist < heightMod)
			return false;
	}

	return C->LineOfSightTo(Other, bMaySkipChecks) != 0;
}

void ABotSupport::ControllerShowSelf(AController* C){
	for(AController* Other = Level->ControllerList; Other; Other = Other->nextController){
		if(Other != C && (Other->bIsPlayer || C->bIsPlayer) && Other->SightCounter < 0.0f && ControllerSeePawn(Other, C->Pawn))
			Other->SeePlayer(C->Pawn);
	}
}

INT ATestBot::Tick(FLOAT DeltaTime, ELevelTick TickType){
	/*
	 * This is really stupid but for some reason the movement code
	 * doesn't update the Pawn's rotation on a dedicated server.
	 * The only solution is to pretend we're in SP while calling UpdateMovementAnimation.
	 */
	if(Level->NetMode == NM_DedicatedServer &&
	   Pawn &&
	   !Pawn->bInterpolating &&
	   Pawn->bPhysicsAnimUpdate &&
	   Pawn->Mesh){
		BYTE Nm = Level->NetMode;

		Level->NetMode = NM_Standalone;
		Pawn->UpdateMovementAnimation(DeltaTime);
		Level->NetMode = Nm;
	}

	return Super::Tick(DeltaTime, TickType);
}
