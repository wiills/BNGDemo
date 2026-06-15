// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLGNode.h"
#include "SLGBlueprintNodeObject.h"
#include "SLGSubsystem.h"

ESLGNodeStatus FSLGNode_Wait::Execute(FSLGExecContext& Ctx) const
{
	FSLGNode_WaitState* State = Ctx.GetMutableNodeState<FSLGNode_WaitState>();
	if (!State)
	{
		// No per-instance slot allocated; treat as instantaneous to stay safe.
		return ESLGNodeStatus::Succeeded;
	}

	// DeltaTime accumulation is driven by the scheduler; M0 advances per serial pass.
	// Real delta wiring lands in M2 scheduling. Keep the logic explicit here.
	if (State->Elapsed >= Duration)
	{
		State->Elapsed = 0.f;
		return ESLGNodeStatus::Succeeded;
	}

	return ESLGNodeStatus::Running;
}

ESLGNodeStatus FSLGNode_BlueprintProxy::Execute(FSLGExecContext& Ctx) const
{
	// Slow path: only valid on GameThread because it touches UObjects.
	if (!Ctx.IsGameThread() || !NodeClass)
	{
		return ESLGNodeStatus::Failed;
	}

	UWorld* World = Ctx.World.Get();
	if (!World)
	{
		return ESLGNodeStatus::Failed;
	}

	// M0 spawns a transient executor per call. M3 will cache/pool these objects.
	USLGBlueprintNodeObject* NodeObject = NewObject<USLGBlueprintNodeObject>(World, NodeClass);
	if (!NodeObject)
	{
		return ESLGNodeStatus::Failed;
	}

	return NodeObject->BP_Execute(Ctx.ScopeId);
}
