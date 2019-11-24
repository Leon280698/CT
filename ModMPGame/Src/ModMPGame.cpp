#include "../Inc/ModMPGame.h"

static ABotSupport* GBotSupport = NULL;

/*
 * FBotSupportExecHook
 */
static struct FBotSupportExecHook : FExec{
	FExec* OldGExec;

	FBotSupportExecHook() : OldGExec(GExec){ GExec = this; }

	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){
		if(GBotSupport){
			if(ParseCommand(&Cmd, "SHOWPATHS")){
				GBotSupport->bHidden = 0;

				return 1;
			}else if(ParseCommand(&Cmd, "HIDEPATHS")){
				GBotSupport->bHidden = 1;

				return 1;
			}else if(ParseCommand(&Cmd, "IMPORTPATHS")){
				GBotSupport->ImportPaths();

				return 1;
			}else if(ParseCommand(&Cmd, "EXPORTPATHS")){
				ABotSupport::ExportPaths(GBotSupport->Level);

				return 1;
			}else if(ParseCommand(&Cmd, "BUILDPATHS")){
				GBotSupport->BuildPaths();

				return 1;
			}else if(ParseCommand(&Cmd, "PUTPATHNODE")){
				TObjectIterator<UViewport> It;
				check(It);
				FVector Loc;

				if(It->Actor->Pawn)
					Loc = It->Actor->Pawn->Location;
				else
					Loc = It->Actor->Location;

				GBotSupport->SpawnNavigationPoint(APathNode::StaticClass(), Loc);

				return 1;
			}else if(ParseCommand(&Cmd, "PUTCOVERPOINT")){
				TObjectIterator<UViewport> It;
				check(It);
				FVector Loc;
				FRotator Rot(0, 0, 0);

				if(It->Actor->Pawn){
					Loc = It->Actor->Pawn->Location;
					Rot.Yaw = It->Actor->Pawn->Rotation.Yaw;
				}else{
					Loc = It->Actor->Location;
					Rot.Yaw = It->Actor->Rotation.Yaw;
				}

				GBotSupport->SpawnNavigationPoint(ACoverPoint::StaticClass(), Loc, Rot);

				return 1;
			}
		}

		return OldGExec->Exec(Cmd, Ar);
	}
} BotSupportExecHook;

static FFilename GetPathFileName(const FString MapName){
	return GFileManager->GetDefaultDirectory() * ".." * "Maps" * "Paths" * FFilename(MapName).GetBaseFilename() + ".ctp";
}

enum ENavPtType{
	NAVPT_PathNode,
	NAVPT_CoverPoint
};

struct FNavPtInfo{
	ENavPtType Type;
	FVector Location;
	FRotator Rotation;

	friend FArchive& operator<<(FArchive& Ar, FNavPtInfo& N){
		int Type = N.Type;

		Ar << Type << N.Location << N.Rotation;
		N.Type = static_cast<ENavPtType>(Type);

		return Ar;
	}
};

/*
 * ABotSupport::SpawnNavigationPoint
 * Spawns a navigation point at the specified position. Can be called during gameplay
 */
void ABotSupport::SpawnNavigationPoint(UClass* NavPtClass, const FVector& Location, const FRotator& Rotation){
	guard(ABotSupport::SpawnNavigationPoint);
	check(NavPtClass->IsChildOf(ANavigationPoint::StaticClass()));

	NavPtClass->GetDefaultActor()->bStatic = 0;
	NavPtClass->GetDefaultActor()->bNoDelete = 0;

	// For some reason the game crashes when two navigation points have the same name
	// SpawnActor should take care of this, but whatever...
	// This guarantees a unique name
	static int NavPtNum = 0;
	ANavigationPoint* NavPt = Cast<ANavigationPoint>(XLevel->SpawnActor(
		NavPtClass,
		FName(FString::Printf("CustomNavPt%i", NavPtNum++)),
		Location,
		Rotation
	));

	if(NavPt){
		NavPt->bStatic = 1;
		NavPt->bNoDelete = 1;
	}else{
		GLog->Logf(NAME_Error, "Failed to spawn %s", *NavPtClass->FriendlyName);
		NavPtFailLocations.AddItem(Location);
	}

	NavPtClass->GetDefaultActor()->bStatic = 1;
	NavPtClass->GetDefaultActor()->bNoDelete = 1;

	unguard;
}

void ABotSupport::ImportPaths(){
	guard(ABotSupport::ImportPaths);

	FString Filename = GetPathFileName(Level->Title);
	FArchive* Ar = GFileManager->CreateFileReader(*Filename);

	if(Ar){
		TArray<FNavPtInfo> NavPtInfo;

		*Ar << NavPtInfo;

		for(int i = 0; i < NavPtInfo.Num(); ++i){
			SpawnNavigationPoint(
				NavPtInfo[i].Type == NAVPT_CoverPoint ? ACoverPoint::StaticClass() : APathNode::StaticClass(),
				NavPtInfo[i].Location,
				NavPtInfo[i].Rotation
			);
		}
	}else{
		GLog->Logf(NAME_Error, "Cannot import paths from file '%s'", *Filename);
	}

	unguard;
}

/*
 * ABotSupport::BuildPaths
 * Does the same as the build paths option in the editor
 */
void ABotSupport::BuildPaths(){
	guard(ABotSupport::BuildPaths);

	int isEd = GIsEditor;
	int begunPlay = Level->bBegunPlay;

	Level->bBegunPlay = 0;
	GIsEditor = 1;

	GPathBuilder.definePaths(XLevel);

	GIsEditor = isEd;
	Level->bBegunPlay = begunPlay;

	unguard;
}

/*
 * ABotSupport::ExportPaths
 * Exports all navigation points from the current map to a file
 */
void ABotSupport::ExportPaths(ALevelInfo* LevelInfo){
	guard(ABotSupport::ExportPaths);

	TArray<FNavPtInfo> NavPts;
	ANavigationPoint* NavPt = LevelInfo->NavigationPointList;

	while(NavPt){
		FNavPtInfo NavPtInfo;

		NavPtInfo.Type = NavPt->IsA(ACoverPoint::StaticClass()) ? NAVPT_CoverPoint : NAVPT_PathNode;
		NavPtInfo.Location = NavPt->Location;
		NavPtInfo.Rotation = NavPt->Rotation;
		NavPt = NavPt->nextNavigationPoint;
		NavPts.AddItem(NavPtInfo);
	}

	if(NavPts.Num() > 0){
		if(LevelInfo->Title.Len() > 0){
			FFilename Filename = GetPathFileName(LevelInfo->Title);
			GFileManager->MakeDirectory(*Filename.GetPath(), 1);
			FArchive* Ar = GFileManager->CreateFileWriter(*Filename);

			if(Ar){
				*Ar << NavPts;

				delete Ar;
			}else{
				GLog->Logf(NAME_Error, "Failed to open file '%s' for writing", *Filename);
			}
		}else{
			GLog->Log(NAME_Error, "LevelInfo.Title must not be empty");
		}
	}else{
		GLog->Log("Map does not contain any path nodes");
	}

	unguard;
}

void ABotSupport::Spawned(){
	GBotSupport = this;
}

void ABotSupport::Destroy(){
	if(GBotSupport == this)
		GBotSupport = NULL;

	Super::Destroy();
}

UBOOL ABotSupport::Tick(FLOAT DeltaTime, ELevelTick TickType){
	guard(ABotSupport::Tick);

	/*
	 * Keeping the BotSupport Actor always in the players view so that it is always rendered
	 * which is needed when the ShowPaths command was used
	 */
	if(!bHidden){
		TObjectIterator<UViewport> It;

		if(It){
			FVector Loc;

			if(It->Actor->Pawn){
				Loc = It->Actor->Pawn->Location;
				Loc.Z += It->Actor->Pawn->EyeHeight;
			}else{
				Loc = It->Actor->Location;
			}

			XLevel->FarMoveActor(this, Loc + It->Actor->Rotation.Vector());
		}
	}

	return Super::Tick(DeltaTime, TickType);

	unguard;
}

/*
 * ABotSupport::PostRender
 * Draws the connections between navigation points like the show paths option in the Editor
 */
void ABotSupport::PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI){
	guard(ABotSupport::PostRender);
	Super::PostRender(SceneNode, RI);

	FLineBatcher LineBatcher(RI);

	for(int i = 0; i < NavPtFailLocations.Num(); ++i)
		LineBatcher.DrawLine(NavPtFailLocations[i], FVector(0, 0, WORLD_MAX), FPlane(1.0f, 0.0f, 0.0f, 1.0f));

	for(ANavigationPoint *Nav=Level->NavigationPointList; Nav; Nav=Nav->nextNavigationPoint){
		for(INT i=0; i<Nav->PathList.Num(); i++){
			UReachSpec* ReachSpec = Nav->PathList[i];

			if(ReachSpec->Start && ReachSpec->End){
				LineBatcher.DrawLine(
					ReachSpec->Start->Location + FVector(0, 0, 8),
					ReachSpec->End->Location,
					ReachSpec->PathColor()
				);

				// make arrowhead to show L.D direction of path
				FVector Dir = ReachSpec->End->Location - ReachSpec->Start->Location - FVector(0, 0, 8);
				Dir.Normalize();

				LineBatcher.DrawLine(
					ReachSpec->End->Location - 12 * Dir + FVector(0, 0, 3),
					ReachSpec->End->Location - 6 * Dir,
					ReachSpec->PathColor()
				);

				LineBatcher.DrawLine(
					ReachSpec->End->Location - 12 * Dir - FVector(0, 0, 3),
					ReachSpec->End->Location - 6 * Dir,
					ReachSpec->PathColor()
				);

				LineBatcher.DrawLine(
					ReachSpec->End->Location - 12 * Dir + FVector(0, 0, 3),
					ReachSpec->End->Location - 12 * Dir - FVector(0, 0, 3),
					ReachSpec->PathColor()
				);
			}
		}
	}

	unguard;
}

/*
 * UExportPathsCommandlet
 */
class UExportPathsCommandlet : public UCommandlet{
	DECLARE_CLASS(UExportPathsCommandlet, UCommandlet, 0, ModMPGame);

	void StaticConstructor(){
		LogToStdout = 0;
		IsServer = 1;
		IsClient = 1;
		IsEditor = 1;
		LazyLoad = 1;
		ShowErrorCount = 0;
		ShowBanner = 0;
	}

	virtual INT Main(const TCHAR* Parms){
		FString MapName;

		if(Parse(Parms, "map=", MapName)){
			UPackage* Package = UObject::LoadPackage(NULL, *MapName, LOAD_NoFail);
			ANavigationPoint* NavPt = NULL;
			ALevelInfo* LevelInfo = NULL;

			for(TObjectIterator<ALevelInfo> It; It; ++It){
				if(It->IsIn(Package)){
					NavPt = It->NavigationPointList;
					LevelInfo = *It;

					break;
				}
			}

			ABotSupport::ExportPaths(LevelInfo);
		}else{
			GWarn->Log(NAME_Error, "Map to export paths from must be specified with 'map=<MapName>'");
		}

		return 0;
	}
};

IMPLEMENT_CLASS(UExportPathsCommandlet);