// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_ClassedAsyncBase.h"
#include "ExK2Node_TimerTask.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraphPin;

/**
 * Dedicated K2 node for countdown latent tasks (UExLatentTask_Timer).
 * Class defaults to UExLatentTask_Timer; Duration and other ExposeOnSpawn pins are shown on the node.
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_TimerTask : public UExK2Node_ClassedAsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_TimerTask(const FObjectInitializer& ObjectInitializer);

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	virtual void AllocateDefaultPins() override;
	virtual void PostPlacedNewNode() override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

	UEdGraphPin* GetResultPin() const;

	virtual UClass* GetValidBaseClass() const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
