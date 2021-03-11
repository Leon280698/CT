/*===========================================================================
    C++ class definitions exported from UnrealScript.
    This is automatically generated by the tools.
    DO NOT modify this manually! Edit the corresponding .uc files instead!
===========================================================================*/

#ifndef MODMPGAME_NATIVE_DEFS
#define MODMPGAME_NATIVE_DEFS

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

#ifndef MODMPGAME_API
#define MODMPGAME_API DLL_IMPORT
#endif



class MODMPGAME_API AAdminControl : public AActor
{
public:
    TArrayNoInit<FString> ServiceClasses;
    TArrayNoInit<class AAdminService*> Services;
    FStringNoInit EventLogFile;
    BITFIELD AppendEventLog:1 GCC_PACK(4);
    BITFIELD EventLogTimestamp:1;
    BITFIELD bPrintCommands:1;
    TArrayNoInit<FString> CurrentCommands GCC_PACK(4);
    class UFunctionOverride* GameInfoPostLoginOverride;
    class UFunctionOverride* GameInfoLogoutOverride;
    TArrayNoInit<class APlayerController*> Players;
    void execEventLog(FFrame& Stack, void* Result);
    void execSaveStats(FFrame& Stack, void* Result);
    void execRestoreStats(FFrame& Stack, void* Result);
    UBOOL ExecCmd(FString const& Cmd, class APlayerController* PC)
    {
        DECLARE_NAME(ExecCmd);
        struct {
            FString Cmd;
            class APlayerController* PC;
            UBOOL ReturnValue;
        } Parms;
        Parms.Cmd=Cmd;
        Parms.PC=PC;
        Parms.ReturnValue=0;
        ProcessEvent(NExecCmd, &Parms);
        return Parms.ReturnValue;
    }
    DECLARE_CLASS(AAdminControl,AActor,0|CLASS_Config,ModMPGame)
	// Overrides
	virtual void Spawned();
	virtual void Destroy();

	void EventLog(const TCHAR* Msg, FName Event);
    DECLARE_NATIVES(AAdminControl)
};


class MODMPGAME_API AAdminService : public AActor
{
public:
    class UClass* RelevantGameInfoClass;
    BITFIELD bRequiresAdminPermissions:1 GCC_PACK(4);
    class AAdminControl* AdminControl GCC_PACK(4);
    void execParseCommand(FFrame& Stack, void* Result);
    void execParseToken(FFrame& Stack, void* Result);
    void execParseIntParam(FFrame& Stack, void* Result);
    void execParseFloatParam(FFrame& Stack, void* Result);
    void execParseStringParam(FFrame& Stack, void* Result);
    void execExecCmd(FFrame& Stack, void* Result);
    void execEventLog(FFrame& Stack, void* Result);
    void CommandFeedback(class APlayerController* PC, FString const& Msg, UBOOL DontWriteToLog)
    {
        DECLARE_NAME(CommandFeedback);
        struct {
            class APlayerController* PC;
            FString Msg;
            UBOOL DontWriteToLog;
        } Parms;
        Parms.PC=PC;
        Parms.Msg=Msg;
        Parms.DontWriteToLog=DontWriteToLog;
        ProcessEvent(NCommandFeedback, &Parms);
    }
    DECLARE_CLASS(AAdminService,AActor,0|CLASS_Config,ModMPGame)
	// AAdminService interface
	virtual bool ExecCmd(const TCHAR* Cmd, class APlayerController* PC = NULL){ return false; }

	void EventLog(const TCHAR* Msg);

	/*
	 * Admin services should prefer this function over 'ParseCommand' as it can also collect the command strings to display as help text
	 */
	bool CheckCommand(const TCHAR** Stream, const TCHAR* Match);
    DECLARE_NATIVES(AAdminService)
};


class MODMPGAME_API ABotSupport : public AAdminService
{
public:
    INT NumBots;
    FLOAT BotAccuracy;
    TArrayNoInit<class ATestBot*> Bots;
    BITFIELD bBotsCountAsPlayers:1 GCC_PACK(4);
    BITFIELD bAutoImportPaths:1;
    BITFIELD bAutoBuildPaths:1;
    BITFIELD bShowPaths:1;
    BITFIELD bPathsImported:1;
    BITFIELD bPathsHaveChanged:1;
    BITFIELD bShowPathsOnClients:1;
    TArrayNoInit<FVector> NavPtFailLocations GCC_PACK(4);
    TArrayNoInit<FPatrolPoint> BotPatrolRoute;
    TArrayNoInit<class AActor*> NavigationPointIcons;
    void SetupPatrolRoute()
    {
        DECLARE_NAME(SetupPatrolRoute);
        ProcessEvent(NSetupPatrolRoute, NULL);
    }
    DECLARE_CLASS(ABotSupport,AAdminService,0|CLASS_Config,ModMPGame)
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

	// Bot AI
	void ControllerTick(FLOAT DeltaTime); // Implementation of controller features that are present in Unreal but were removed from RC
	bool ControllerSeePawn(AController* C, APawn* Other, bool MaySkipChecks = true); // Check if this controller can see the specified pawn
	void ControllerShowSelf(AController* C); // Show this controller to other's that can see its pawn
};


class MODMPGAME_API AMPBot : public ACTBot
{
public:
    INT ChosenSkin;
    FLOAT Accuracy;
    void execUpdatePawnAccuracy(FFrame& Stack, void* Result);
    DECLARE_CLASS(AMPBot,ACTBot,0|CLASS_Config,ModMPGame)
	virtual int Tick(FLOAT DeltaTime, ELevelTick TickType);
    DECLARE_NATIVES(AMPBot)
};


class MODMPGAME_API ATestBot : public AScriptedController
{
public:
    INT ChosenSkin;
    DECLARE_CLASS(ATestBot,AScriptedController,0|CLASS_Config,ModMPGame)
	virtual INT Tick(FLOAT DeltaTime, ELevelTick TickType);
};


class MODMPGAME_API APatrolPoint : public ANavigationPoint
{
public:
    DECLARE_CLASS(APatrolPoint,ANavigationPoint,0,ModMPGame)
    NO_DEFAULT_CONSTRUCTOR(APatrolPoint)
};


class MODMPGAME_API ASmallNavigationPoint : public ANavigationPoint
{
public:
    DECLARE_CLASS(ASmallNavigationPoint,ANavigationPoint,0,ModMPGame)
    NO_DEFAULT_CONSTRUCTOR(ASmallNavigationPoint)
};


class MODMPGAME_API AInventorySpot : public ASmallNavigationPoint
{
public:
    DECLARE_CLASS(AInventorySpot,ASmallNavigationPoint,0,ModMPGame)
    NO_DEFAULT_CONSTRUCTOR(AInventorySpot)
};



#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#if __STATIC_LINK

#define AUTO_INITIALIZE_REGISTRANTS_MODMPGAME \
	AAdminControl::StaticClass(); \
	AAdminService::StaticClass(); \
	ABotSupport::StaticClass(); \
	AMPBot::StaticClass(); \
	ATestBot::StaticClass(); \
	APatrolPoint::StaticClass(); \
	ASmallNavigationPoint::StaticClass(); \
	AInventorySpot::StaticClass(); \
	UExportPathsCommandlet::StaticClass(); \

#endif // __STATIC_LINK

#endif // CORE_NATIVE_DEFS
