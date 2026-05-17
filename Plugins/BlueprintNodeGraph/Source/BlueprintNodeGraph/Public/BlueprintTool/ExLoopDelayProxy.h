// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExLatentActionManager.h"
#include "ExLoopDelayProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExLoopDelayCompletedDelegate);

/**
 * Wait Then Execute
 * If you use this, you will have at least 1 frame delay
 */
UCLASS(meta=(HideThen))
class BLUEPRINTNODEGRAPH_API UExLoopDelayProxy : public UExLatentActionProxyBase
{
	GENERATED_BODY()

	UPROPERTY()
	float SelfDuration=2.f;
	int32 DelayOverCount=0;
	int32 CallCount=0;
	
public:
	/**
	 * CreateProxy Function - (UExK2Node_WaitInput - create pins)
	 * Function params should Equal to PinNames
	 * UUIDPinName
	 * Duration - Time to delay
	 * bFirstDelay - should Delay at first call
	 */
	UE_DEPRECATED(4.26, "UExLoopDelayProxy Node has been deprecated.  Use ExForLoopWithDelay instead")
	UFUNCTION(BlueprintCallable, Category="Utilities|FlowControl", meta=(WorldContext="WorldContextObject", HidePin="UUID"))
	static UExLoopDelayProxy* CreateProxy(UObject* WorldContextObject, FString UUID, float Duration=2.f, bool bNeedFirstDelay=false);
	
	// /** Called to trigger the action once the delegates have been bound */
	// virtual void Activate() override;

	virtual void OnBranchesFinished() override;

	UFUNCTION()
	void OnTimeOver();
};
