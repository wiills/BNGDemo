// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Subsystems/ExLatentActionManager.h"
#include "ExProxy_ForLoopWithDelay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExForLoopWithDelayProxyDelegate, int32, LoopIndex);

/**
 * For Loop Count, With Loop Interval\n, only execute once when delay running
 */
UCLASS(Blueprintable, BlueprintType, meta=(HideThen))
class BLUEPRINTNODEGRAPH_API UExProxy_ForLoopWithDelay : public UExBase_FlowProxy
{
	GENERATED_BODY()

	float SelfLoopInterval=1.f;
	int32 SelfLoopCount=1;
	bool bSelfNeedFistDelay=false;
	
	int32 LoopCallCount=0;
	bool bIsRunning=false;

	FTimerHandle TimerHandle;
	
public:
	UPROPERTY(BlueprintAssignable, meta=(DisplayName="OnLoopBody"))
	FExForLoopWithDelayProxyDelegate OnLoopBodyDelegate;

	virtual void BeginDestroy() override;
	
	/**
	 * CreateProxy Function - (UExK2Node_WaitInput - create pins)
	 * Function params should Equal to PinNames
	 * UUIDPinName
	 * InputCount - Input Branches
	 * Duration	  - Time to delay
	 * bFistDelay - should Delay at first call
	 */
	UFUNCTION(BlueprintCallable, Category="Utilities|FlowControl", meta=(WorldContext="WorldContextObject", HidePin="UUID,InputCount"))
	static UExProxy_ForLoopWithDelay* CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount, int32 LoopCount=1, float LoopInterval=1.f, bool bNeedFistDelay=false);
	
protected:
	// wait branches
	virtual void OnBranchesFinished() override;
	
	UFUNCTION()
	void OnLoopCall();
	
	// UFUNCTION()
	// void OnLoopCompleted();
	virtual void OnFinishCall() override;

	void MakeClean();
};
