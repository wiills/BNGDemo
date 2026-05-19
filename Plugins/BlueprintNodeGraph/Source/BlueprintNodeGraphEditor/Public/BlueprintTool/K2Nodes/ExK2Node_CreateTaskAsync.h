// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_CustomEvent.h"
#include "ExK2Node_CreateTaskAsync.generated.h"


struct FAsyncTaskHelper
{
	struct FOutputPinAndLocalVariable
	{
		UEdGraphPin* OutputPin;
		UK2Node_TemporaryVariable* TempVar;

		FOutputPinAndLocalVariable(UEdGraphPin* Pin, UK2Node_TemporaryVariable* Var) : OutputPin(Pin), TempVar(Var) {}
	};

	static bool ValidDataPin(const UEdGraphPin* Pin, EEdGraphPinDirection Direction);
	static bool CreateDelegateForNewFunction(UEdGraphPin* DelegateInputPin, FName FunctionName, UK2Node* CurrentNode, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);
	static bool CopyEventSignature(UK2Node_CustomEvent* CENode, UFunction* Function, const UEdGraphSchema_K2* Schema);
	static bool HandleDelegateImplementation(
		const FMulticastDelegateProperty* CurrentProperty,
		UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraphPin*& OutLastActivatedThenPin,
		UK2Node* CurrentNode, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

	static FName GetAsyncTaskProxyName();
};

/**
 * 
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_CreateTaskAsync : public UK2Node_ConstructObjectFromClass
{
	GENERATED_BODY()
	
	// The name of the function to call to create a proxy object
	UPROPERTY()
	FName ProxyFactoryFunctionName;
	// The class containing the proxy object functions
	UPROPERTY()
	UClass* ProxyFactoryClass;
	// The type of proxy object that will be created
	UPROPERTY()
	UClass* ProxyClass;
	// The name of the 'go' function on the proxy object that will be called after delegates are in place, can be NAME_None
	UPROPERTY()
	FName ProxyActivateFunctionName;
	
public:
	UExK2Node_CreateTaskAsync();
	
	/* UEdGraphNode interface */
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// Override node visual style
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	/* UEdGraphNode interface */
	
	/* UK2Node interface */
	virtual FText GetMenuCategory() const override;
	// Register this node in the context menu
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void AllocateDefaultPins() override;
	virtual void EarlyValidation(class FCompilerResultsLog& MessageLog) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	/* UK2Node interface */

	virtual void AllocateDelegatePins();
	
	/* ConstructObject interface */
	virtual UClass* GetClassPinBaseClass() const override;
	virtual bool UseOuter() const override { return true; }
	virtual bool IsHideThen() {return false;}
	/* ConstructObject interface */


	/************ Handle Delegates ************/
protected:
	/** Expand out the logic to handle the delegate output pins */
	virtual bool HandleDelegates(UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

private:
	/** Get the actor owner pin */
	UEdGraphPin* GetOwnerPin() const;
};

