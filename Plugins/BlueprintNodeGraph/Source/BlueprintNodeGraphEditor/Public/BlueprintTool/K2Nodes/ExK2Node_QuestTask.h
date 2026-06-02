// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_ClassedAsyncBase.h"
#include "ExK2Node_QuestTask.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraphPin;

/**
 * @class UExK2Node_QuestTask
 * @brief Quest latent task K2 node (CreateQuestProxy on UExLatentTask_Quest, not CreateLatentTask).
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_QuestTask : public UExK2Node_ClassedAsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_QuestTask(const FObjectInitializer& ObjectInitializer);

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

	UEdGraphPin* GetResultPin() const;

	virtual UClass* GetValidBaseClass() const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
