	//Overrides
	virtual int CheckAnimFinished(int);
	virtual int* GetOptimizedRepList(unsigned char*, struct FPropertyRetirement*, int*, class UPackageMap*, class UActorChannel*);
	virtual void StartAnimPoll();
	virtual int Tick(float, enum ELevelTick);

	//Virtual Functions
	virtual bool AppearsDead() const;
	virtual bool SetEnemy(class AActor*);
	virtual int InCombat() const;
	virtual void CheckHearSound(class AActor*, class USound*, int);
	virtual int SafeToShoot(class AActor*, const FVector&);
	virtual class AActor* GetViewTarget();
	virtual void WeaponFired();
	virtual class FVector GetFireStart();
	virtual void SetAnchor(class ANavigationPoint*);
	virtual bool CrossesMyFire(const FVector&, const FVector&);
	virtual int AcceptNearbyPath(class AActor*);
	virtual void AdjustFromWall(const FVector&, class AActor*);
	virtual void SetAdjustLocation(const FVector&);
	virtual int LocalPlayerController();
	virtual int WantsLedgeCheck();
	virtual int StopAtLedge();
	virtual void DebugDraw(class FLineBatcher&);
	virtual int ScriptedGoalsFinished() const;
	virtual int IsAvailable() const;

	// Events
	void MayFall();
	FColor GetDebugColor();
	void SoakStop(const FString& problem);
	void LongFall();
	UBOOL NotifyPhysicsVolumeChange(class APhysicsVolume* NewVolume);
	UBOOL NotifyHeadVolumeChange(class APhysicsVolume* NewVolume);
	UBOOL NotifyLanded(const FVector& HitNormal);
	UBOOL NotifyHitWall(const FVector& HitNormal, class AActor* Wall);
	UBOOL NotifyBump(class AActor* Other);
	void NotifyHitMover(const FVector& HitNormal, class AMover* Wall);
	void NotifyJumpApex();
	void NotifyMissedJump();
	void PrepareForMove(class ANavigationPoint* Goal, class UReachSpec* Path);
	void HearNoise(FLOAT Loudness, class AActor* NoiseMaker);
	void SeePlayer(class APawn* Seen);
	void StopShake();
	void ShakeView(FLOAT InTime, FLOAT SustainTime, FLOAT OutTime, FLOAT XMag, FLOAT YMag, FLOAT ZMag, FLOAT YawMag, FLOAT PitchMag, FLOAT Frequency);
	void ThrowScav();
	void SwitchGrenade(FLOAT F);
	void ServerReStartPlayer();
	void MonitoredPawnAlert();

	//Functions
	int CanHear(const FVector&, float, float, float, class AActor*);
	int CanHear(class UStimulus*);
	int CanHearSound(const FVector&, class AActor*, float);
	void CheckFears();
	void EnableTurning(bool, bool, bool);
	FVector GetFocusLocation();
	FRotator GetFocusRotation();
	DWORD LineOfSightTo(class AActor* Other, int bUseLOSFlag = 0);
	int SafeToShoot(class AActor*);
	int SeeActor(class AActor*, int);
	void SetFocus(const FRotator&);
	void SetFocus(const FVector&);
	void SetFocus(class AActor*);
	void SetupMoveToward(class AActor*, float, int);
