#include "../../Core/Inc/Core.h"
#include "../../Core/Inc/FOutputDeviceFile.h"
#include "../../Core/Inc/FOutputDeviceWindowsError.h"
#include "../../Core/Inc/FFeedbackContextCmd.h"
#include "../../Core/Inc/FConfigCacheIni.h"
#include "../../Engine/Inc/Engine.h"
#include "../../Window/Inc/Window.h"
#include "../../GameSpyMgr/Inc/GameSpyMgr.h"

static struct FExecHook : public FExec{
	UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){
		guard(FExecHook::Exec);

		if(ParseCommand(&Cmd, "SHOWLOG")){
			GLogWindow->Show(1);
			SetFocus(*GLogWindow);
			GLogWindow->Display.ScrollCaret();

			return 1;
		}else if(ParseCommand(&Cmd, "HIDELOG")){
			if(GLogWindow)
				GLogWindow->Show(0);

			return 1;
		}else if(ParseCommand(&Cmd, "EDITOBJ")){
			UObject* Obj = FindObject<UObject>(ANY_PACKAGE, Cmd);

			if(Obj){
				WObjectProperties* P = new WObjectProperties("EditObj", 0, "", NULL, 1);
				P->OpenWindow((HWND)GEngine->Client->Viewports[0]->GetWindow());
				P->Root.SetObjects(&Obj, 1);
				P->Show(1);
			}else{
				Ar.Logf("Object \"%s\" not found", Cmd);
			}

			return 1;
		}else if(ParseCommand(&Cmd, "EDITACTOR")){
			UClass* Class;
			UObject* Found = NULL;
			FName ActorName;
			APlayerController* Player = GEngine->Client->Viewports[0]->Actor;

			if(ParseObject<UClass>(Cmd, "Class=", Class, ANY_PACKAGE)){
				FLOAT   MinDist = 999999.0f;

				for(TObjectIterator<AActor> It; It; ++It){
					FLOAT Dist = Player ? FDist(It->Location, Player->Pawn ? Player->Pawn->Location : Player->Location) : 0.0f;

					if((!Player || It->GetLevel() == Player->GetLevel()) &&
					   !It->bDeleteMe &&
					   It->IsA(Class) &&
					   Dist < MinDist){
						MinDist = Dist;
						Found   = *It;
					}
				}
			}else if(Parse(Cmd, "Name=", ActorName)){
				for(TObjectIterator<AActor> It; It; ++It){
					if(!It->bDeleteMe && It->GetFName() == ActorName){
						Found = *It;

						break;
					}
				}
			}else if(Player){
				Found = Player->Target;
			}

			if(Found){
				WObjectProperties* P = new WObjectProperties("EditActor", 0, "", NULL, 1);
				P->OpenWindow((HWND)GEngine->Client->Viewports[0]->GetWindow());
				P->Root.SetObjects((UObject**)&Found, 1);
				P->Show(1);
			}else{
				Ar.Logf("Target not found");
			}

			return 1;
		}else if(ParseCommand(&Cmd, "PREFERENCES")){
			WConfigProperties* Preferences = new WConfigProperties("Preferences", LocalizeGeneral("AdvancedOptionsTitle", "Window"));

			Preferences->OpenWindow((HWND)GEngine->Client->Viewports[0]->GetWindow());
			Preferences->ForceRefresh();
			Preferences->Show(1);
			SetFocus(*Preferences);

			return 1;
		}else if(ParseCommand(&Cmd, "USERENDEV")){
			FString RenderDeviceClass = Cmd;

			if(RenderDeviceClass == "D3D")
				RenderDeviceClass = "D3DDrv.D3DRenderDevice";
			else if(RenderDeviceClass == "OpenGL")
				RenderDeviceClass = "OpenGLDrv.OpenGLRenderDevice";
			else if(RenderDeviceClass == "Mod")
				RenderDeviceClass = "Mod.ModRenderDevice";

			UClass* Class = LoadClass<URenderDevice>(NULL, *RenderDeviceClass, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);

			if(Class){
				if(GEngine->GRenDev && GEngine->GRenDev->GetClass() != Class){
					// UViewport::TryRenderDevice only works if GEngine->GRenDev is not the same as the current render device
					GEngine->GRenDev = ConstructObject<URenderDevice>(Class);
					GEngine->GRenDev->Init();

					// We need to reset all FCanvasUtils since they still reference the old render interface
					for(TObjectIterator<UCanvas> It; It; ++It){
						if(It->pCanvasUtil){
							delete It->pCanvasUtil;
							It->pCanvasUtil = NULL;
						}
					}

					UViewport* Viewport = GEngine->Client->Viewports[0];
					Viewport->TryRenderDevice(*RenderDeviceClass, Viewport->SizeX, Viewport->SizeY, Viewport->IsFullscreen());
				}
			}else{
				Ar.Logf("Unable to find render device class '%s'", *RenderDeviceClass);
			}

			return 1;
		}

		return 0;

		unguard;
	}
} LauncherExecHook;

static void DetectRenderDevice(){
	FString RenderDeviceClass;

	// Check for a specific render device on the command line.
	if(Parse(appCmdLine(), "RenDev=", RenderDeviceClass)){
		// Allow short form of known render devices.
		if(RenderDeviceClass == "D3D")
			RenderDeviceClass = "D3DDrv.D3DRenderDevice";
		else if(RenderDeviceClass == "OpenGL")
			RenderDeviceClass = "OpenGLDrv.OpenGLRenderDevice";
		else if(RenderDeviceClass == "Mod")
			RenderDeviceClass = "Mod.ModRenderDevice";

		debugf("RenderDevice set on command line: %s", *RenderDeviceClass);
		GConfig->SetString("Engine.Engine", "RenderDevice", *RenderDeviceClass);
	}else{
		// If default render device is set to D3DRenderDevice, try locating ModRenderDevice and use it if it exists.
		GConfig->GetFString("Engine.Engine", "RenderDevice", RenderDeviceClass);

		if(RenderDeviceClass == "D3DDrv.D3DRenderDevice"){
			UClass* ModRenderDeviceClass = LoadClass<URenderDevice>(NULL, "Mod.ModRenderDevice", NULL, LOAD_NoWarn | LOAD_Quiet, NULL);

			if(ModRenderDeviceClass){
				debugf("Using Mod.ModRenderDevice");
				GConfig->SetString("Engine.Engine", "RenderDevice", "Mod.ModRenderDevice");
			}
		}
	}
}

static void InitEngine(){
	guard(InitEngine);
	check(!GEngine);

	DOUBLE LoadTime = appSeconds();

	// First-run menu.
	INT FirstRun = 0;

	GConfig->GetInt("FirstRun", "FirstRun", FirstRun);

	if(ParseParam(appCmdLine(), "FirstRun"))
		FirstRun = 0;

	// Update first-run.
	if(FirstRun < ENGINE_VERSION)
		FirstRun = ENGINE_VERSION;

	GConfig->SetInt("FirstRun", "FirstRun", FirstRun);

	DetectRenderDevice();

	// Create game engine.
	UClass* EngineClass = LoadClass<UEngine>(NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail, NULL);

	GEngine = ConstructObject<UEngine>(EngineClass);

	GLogWindow->SetExec(GEngine);
	GLogWindow->Log(NAME_Title, LocalizeGeneral("Run", "SWRepublicCommando"));

	GEngine->Init();
	debugf("Startup time: %f seconds", appSeconds() - LoadTime);
	unguard;
}

//
// Unreal's main message loop.  All windows in Unreal receive messages
// somewhere below this function on the stack.
//
static void MainLoop(){
	guard(MainLoop);
	check(GEngine);

	// Loop while running.
	GIsRunning = 1;
	DOUBLE OldTime = appSeconds();
	DOUBLE SecondStartTime = OldTime;
	INT TickCount = 0;

	while(GIsRunning && !GIsRequestingExit){
		// Update the world.
		guard(UpdateWorld);
		DOUBLE NewTime  = appSeconds();
		FLOAT DeltaTime = NewTime - OldTime;

		GEngine->Tick(DeltaTime);

		if(GWindowManager)
			GWindowManager->Tick(DeltaTime);

		OldTime = NewTime;
		TickCount++;

		if(OldTime > SecondStartTime + 1){
			GEngine->CurrentTickRate = (FLOAT)TickCount / (OldTime - SecondStartTime);
			SecondStartTime = OldTime;
			TickCount = 0;
		}
		unguard;

		// Enforce optional maximum tick rate.
		guard(EnforceTickRate);
		FLOAT MaxTickRate = GEngine->GetMaxTickRate();
		if(MaxTickRate > 0.0f){
			FLOAT Delta = (1.0f / MaxTickRate) - (appSeconds() - OldTime);
			appSleep(Max(0.0f, Delta));
		}
		unguard;

		// Handle all incoming messages.
		guard(MessagePump);
		MSG Msg;

		while(PeekMessageA(&Msg,NULL, 0, 0, PM_REMOVE)){
			if(Msg.message == WM_QUIT)
				GIsRequestingExit = 1;

			TranslateMessage(&Msg);
			DispatchMessageA( &Msg );
		}

		unguard;
	}

	GIsRunning = 0;

	unguard;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
	int ExitCode = EXIT_SUCCESS;
	FOutputDeviceFile Log;
	FOutputDeviceWindowsError Error;
	FFeedbackContextCmd Warn;

	GIsStarted = 1;

	try{
		GameSpyCDKeyResponseInterface CDKeyInterface;

		GIsGuarded = 1;

		appInit(appPackage(), lpCmdLine, &Log, &Error, &Warn, FConfigCacheIni::Factory, 1);

		GIsClient = 1;
		GIsServer = 1;
		GIsEditor = 0;
		GIsScriptable = 1;
		GLazyLoad = ParseParam(appCmdLine(), "LAZY");
		GUseFrontEnd = ParseParam(appCmdLine(), "NOUI") == 0;

		InitWindowing();

		GLogWindow = new WLog(Log.Filename, Log.LogAr, "GameLog");
		GLogWindow->OpenWindow(ParseParam(appCmdLine(), "LOG"), 0);
		GLogWindow->Log(NAME_Title, LocalizeGeneral("Start", "SWRepublicCommando"));

		GExec = &LauncherExecHook;

		InitEngine();

		if(GEngine && !GIsRequestingExit)
			MainLoop();

		GLogWindow->Log(NAME_Title, LocalizeGeneral("Exit", "SWRepublicCommando"));

		if(GLogHook == GLogWindow)
			GLogHook = NULL;

		delete GLogWindow;

		appPreExit();

		GIsGuarded = 0;
	}catch(...){
		GIsGuarded = 0;
		ExitCode = EXIT_FAILURE;
		Error.HandleError();
	}

	appExit();

	GIsStarted = 0;

	return ExitCode;
}

// Force use of dedicated GPU
extern "C"{
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}
