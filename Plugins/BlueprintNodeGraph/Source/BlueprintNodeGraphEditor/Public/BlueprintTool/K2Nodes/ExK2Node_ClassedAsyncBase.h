// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_AsyncBase.h"
#include "ExK2Node_ClassedAsyncBase.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraphPin;
class UEdGraphSchema;
class UEdGraphSchema_K2;

/**
 * Base class for async nodes that select a Class and expose its ExposeOnSpawn properties.
 * Replaces left-side Class/parameter pins with Detail-panel editing (pins are hidden).
 */
UCLASS(Abstract)
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_ClassedAsyncBase : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_ClassedAsyncBase(const FObjectInitializer& ObjectInitializer);

	/** Subclass override: return the base UClass used to validate/filter the Class pin selection. */
	virtual UClass* GetValidBaseClass() const { return nullptr; }

	/** Subclass override: additional per-node class validation (e.g. exclude specific subclasses). */
	virtual bool IsSpawnClassValid(UClass* InClass) const { return true; }

	/** Whether to hide the Class pin on the node (moved to Detail panel). */
	virtual bool ShouldHideClassPin() const { return true; }

	/** Whether to hide the spawn-parameter pins on the node (moved to Detail panel). */
	virtual bool ShouldHideSpawnParamPins() const { return true; }

	virtual void AllocateDefaultPins() override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void PostLoad() override;
	virtual void PostReconstructNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual bool ShouldShowNodeProperties() const override { return true; }

	UEdGraphPin* GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	UClass* GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	void CreatePinsForClass(UClass* InClass);
	bool SynchronizePinsForClass(UClass* InClass);
	bool EnsureSpawnParamPinsUpToDate();

	/** Gather all ExposeOnSpawn properties for a given class (used by Detail Customization). */
	void GetExposedPropertiesForClass(UClass* InClass, TArray<const FProperty*>& OutProperties) const;

protected:
	UPROPERTY()
	TArray<FName> SpawnParamPins;

	/** Hide spawn param pins if configured to do so. Called after CreatePinsForClass. */
	void ApplyPinVisibility();
	bool SynchronizePinsForCurrentClass(bool bNotifyGraph, bool bMarkBlueprintModified);
};
