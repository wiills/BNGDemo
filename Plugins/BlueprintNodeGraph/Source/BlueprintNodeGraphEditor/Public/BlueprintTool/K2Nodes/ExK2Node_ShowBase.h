// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_ShowBase.generated.h"

/** Base K2 node for editor display; provides GetMenuActions. */
UCLASS(Abstract)
class BLUEPRINTNODEGRAPHEDITOR_API UK2Node_ShowBase : public UK2Node
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NodeDesc;
	
	/* UEdGraphNode interface */
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldShowNodeProperties() const override { return true; }
	// Override node visual style
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	/* UEdGraphNode interface */

	/* UK2Node interface */
	virtual FText GetMenuCategory() const override;
	// Register this node in the context menu
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	/* UK2Node interface */

	// add pin
	virtual void AllocateDefaultPins() override;
	virtual void AddPinToNode() {}
	
	/** Custom In Out Pin DataType **/
	UEdGraphPin* GetThenPin() const; // default output pin
};
