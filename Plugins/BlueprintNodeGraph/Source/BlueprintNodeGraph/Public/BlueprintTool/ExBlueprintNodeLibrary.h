// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ExBlueprintNodeLibrary.generated.h"

/**
 * 
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExBlueprintNodeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//=============================================================================
	// Latent Actions

	/** 
	 * Perform a latent action with a delay (specified in seconds).  Calling again while it is counting down will be ignored.
	 * 
	 * @param WorldContextObject	World context.
	 * @param LatentInfo 			The latent action.
	 * @param Duration 				length of delay (in seconds).
	 */
	UFUNCTION(BlueprintCallable, Category="LatentTasks", meta=(Latent, WorldContext="WorldContextObject", LatentInfo="LatentInfo", DisplayName="WaitAction", Keywords="WaitAction"))
	static void	WaitAction(const UObject* WorldContextObject, float Duration, struct FLatentActionInfo LatentInfo);

	/** 
	 * Perform a latent action with a delay (specified in seconds).  Calling again while it is counting down will be ignored.
	 * ## Diff from Delay, It will Execute on every call
	 * 
	 * @param WorldContext	World context.
	 * @param Duration 		length of delay (in seconds).
	 * @param LatentInfo 	The latent action.
	 */
	UFUNCTION(BlueprintCallable, Category="Utilities|FlowControl", meta=(Latent, WorldContext="WorldContext", LatentInfo="LatentInfo", Duration="0.2", Keywords="sleep"))
	static void	OnDelay(const UObject* WorldContext, float Duration, struct FLatentActionInfo LatentInfo );



	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (WorldContext = "WorldContextObject", Keywords = "gameplay tag"))
	static bool HasGameplayTag(UObject* WorldContextObject, FGameplayTag TagToCheck);

	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (WorldContext = "WorldContextObject", Keywords = "gameplay tag"))
	static bool HasAllTags(UObject* WorldContextObject, FGameplayTagContainer TagsToCheck);

	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (WorldContext = "WorldContextObject", Keywords = "gameplay tag"))
	static bool HasAnyTag(UObject* WorldContextObject, FGameplayTagContainer TagsToCheck);
};
