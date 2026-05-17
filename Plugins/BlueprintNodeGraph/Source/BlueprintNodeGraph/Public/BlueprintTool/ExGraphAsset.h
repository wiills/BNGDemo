// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ExGraphAsset.generated.h"

UCLASS(BlueprintType, Blueprintable)
class BLUEPRINTNODEGRAPH_API UExGraphAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "FlowGraph")
	FText GraphName;

	UPROPERTY(BlueprintReadWrite, Category = "FlowGraph")
	FText Description;

	UPROPERTY(BlueprintReadWrite, Category = "FlowGraph")
	TArray<FName> EntryPointNames;

	UPROPERTY(BlueprintReadWrite, Category = "FlowGraph")
	bool bAutoStart = false;

	UPROPERTY(BlueprintReadWrite, Category = "FlowGraph")
	float DefaultTimeout = 30.f;

	UFUNCTION(BlueprintCallable, Category = "FlowGraph")
	void AddEntryPoint(FName EntryName);

	UFUNCTION(BlueprintCallable, Category = "FlowGraph")
	void RemoveEntryPoint(FName EntryName);
};
