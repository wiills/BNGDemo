// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "KismetNodes/SGraphNodeK2Base.h"

/**
 * Graph Node Style
 */
class BLUEPRINTNODEGRAPHEDITOR_API SGraphNode_ShowBase : public SGraphNodeK2Base
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_ShowBase){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, UK2Node* InNode);

protected:
	virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override;
	virtual void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	virtual FReply OnAddPin() override;
	virtual void UpdateGraphNode() override;
};

/**
 * 
 */
class BLUEPRINTNODEGRAPHEDITOR_API SGraphNode_AddInputPin : public SGraphNode_ShowBase
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_AddInputPin){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, UK2Node* InNode);

protected:
	virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override;
	virtual FReply OnAddPin() override;
};
