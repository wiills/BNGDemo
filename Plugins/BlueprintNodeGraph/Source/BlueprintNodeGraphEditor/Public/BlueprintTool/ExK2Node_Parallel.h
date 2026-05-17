// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "ExK2Node_Parallel.generated.h"

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_Parallel : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_Parallel(const FObjectInitializer& ObjectInitializer);

	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	
protected:
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	UEdGraphPin* GetCompletedPin() const;
};
