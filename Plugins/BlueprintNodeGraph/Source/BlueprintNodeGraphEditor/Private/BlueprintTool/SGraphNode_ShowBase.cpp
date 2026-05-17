// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/SGraphNode_ShowBase.h"
#include "GraphEditorSettings.h"
#include "BlueprintTool/ExK2Node_ShowBase.h"
#include "Kismet2/BlueprintEditorUtils.h"


void SGraphNode_ShowBase::Construct(const FArguments& InArgs, UK2Node* InNode)
{
	this->GraphNode = InNode;
	this->SetCursor( EMouseCursor::CardinalCross );
	this->UpdateGraphNode();
}

void SGraphNode_ShowBase::CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox)
{
	SGraphNodeK2Base::CreateInputSideAddButton(InputBox);
}

void SGraphNode_ShowBase::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	SGraphNodeK2Base::CreateOutputSideAddButton(OutputBox);
}

FReply SGraphNode_ShowBase::OnAddPin()
{
	const FScopedTransaction Transaction(NSLOCTEXT("Kismet", "AddArgumentPin", "Add Argument Pin"));
	GraphNode->Modify();

	const auto BPNode = CastChecked<UK2Node_ShowBase>(GraphNode);
	BPNode->AddPinToNode();
	FBlueprintEditorUtils::MarkBlueprintAsModified(BPNode->GetBlueprint());

	UpdateGraphNode();
	GraphNode->GetGraph()->NotifyGraphChanged();

	return FReply::Handled();
}

void SGraphNode_ShowBase::UpdateGraphNode()
{
	SGraphNodeK2Base::UpdateGraphNode();
}


void SGraphNode_AddInputPin::Construct(const FArguments& InArgs, UK2Node* InNode)
{
	this->GraphNode = InNode;
	this->SetCursor( EMouseCursor::CardinalCross );
	this->UpdateGraphNode();
}

void SGraphNode_AddInputPin::CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox)
{
	FText Temp = FText::FromString(TEXT("Add Input"));
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(Temp, Temp);

	FMargin AddPinPadding = Settings->GetInputPinPadding();
	AddPinPadding.Top += 6.0f;

	InputBox->AddSlot()
	.AutoHeight()
	.VAlign(VAlign_Center)
	.Padding(AddPinPadding)
	[
		AddPinButton
	];
}

FReply SGraphNode_AddInputPin::OnAddPin()
{
	return SGraphNode_ShowBase::OnAddPin();
}
