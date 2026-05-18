// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExAsyncActionBase.h"
#include "ExParallelProxy.generated.h"

UENUM(BlueprintType)
enum class EExParallelMode : uint8
{
	All,
	Any,
	Count
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParallelCompletedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParallelBranchCompletedDelegate);

UCLASS()
class BLUEPRINTNODEGRAPH_API UExParallelProxy : public UExAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Parallel")
	FOnParallelBranchCompletedDelegate OnBranchCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Parallel")
	FOnParallelCompletedDelegate OnCompleted;

protected:
	UPROPERTY()
	EExParallelMode Mode = EExParallelMode::All;

	UPROPERTY()
	int32 ExpectedBranchCount = 0;

	UPROPERTY()
	int32 CompletedBranchCount = 0;

	UPROPERTY()
	int32 SuccessfulBranchCount = 0;

	UPROPERTY()
	TArray<FString> BranchUUIDs;

public:
	UFUNCTION(BlueprintCallable, Category = "Parallel", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Parallel"))
	static UExParallelProxy* CreateProxy(UObject* WorldContextObject, int32 InBranchCount, EExParallelMode InMode = EExParallelMode::All, int32 InRequiredCount = 1);

	UFUNCTION(BlueprintCallable, Category = "Parallel", meta = (BlueprintInternalUseOnly = "true"))
	void RegisterBranch(FString BranchUUID);

	UFUNCTION(BlueprintCallable, Category = "Parallel", meta = (BlueprintInternalUseOnly = "true"))
	void BranchCompleted(FString BranchUUID, bool bSuccess);

public:
	virtual void Activate() override;

protected:
	void CheckCompletion();

private:
	TMap<FString, bool> BranchResults;
};
