#pragma once

#include "K2Node_AsyncAction.h"
#include "K2Node_AsyncAction_ListenForScopedMessages.generated.h"

class FBlueprintActionDatabaseRegistrar;
class FKismetCompilerContext;
class FMulticastDelegateProperty;
class UEdGraph;
class UEdGraphPin;

/**
 * Dedicated Blueprint node for scoped message listening.
 *
 * The runtime async action stores payloads as type-erased UStruct memory. This
 * editor node mirrors the engine GameplayMessage node by turning the selected
 * Payload Type into a strongly typed Payload output pin.
 */
UCLASS()
class UK2Node_AsyncAction_ListenForScopedMessages : public UK2Node_AsyncAction
{
	GENERATED_BODY()

	virtual void PostReconstructNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;

protected:
	virtual bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs,
		UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin,
		UEdGraph* SourceGraph,
		FKismetCompilerContext& CompilerContext) override;

private:
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualScopeIdVar,
		UEdGraphPin*& InOutLastActivatedThenPin,
		UEdGraph* SourceGraph,
		FKismetCompilerContext& CompilerContext);

	void RefreshOutputPayloadType();
	void HideUnlinkedGeneratedPins();

	UEdGraphPin* GetPayloadPin() const;
	UEdGraphPin* GetPayloadTypePin() const;
	UEdGraphPin* GetOutputChannelPin() const;
	UEdGraphPin* GetOutputScopeIdPin() const;
};
