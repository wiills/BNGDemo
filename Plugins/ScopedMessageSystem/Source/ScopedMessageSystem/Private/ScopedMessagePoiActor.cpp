#include "ScopedMessagePoiActor.h"

#include "ScopedMessageSubsystem.h"

AScopedMessagePoiActor::AScopedMessagePoiActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
}

void AScopedMessagePoiActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoResolveScopeOnBeginPlay)
	{
		EnsurePoiScopeReady();
	}
}

void AScopedMessagePoiActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScopeResolveRetryTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void AScopedMessagePoiActor::EnsurePoiScopeReady()
{
	const FScopedMessageScopeId ResolvedScopeId = ResolvePoiScopeId();
	if (!ResolvedScopeId.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage %s %s waiting for valid Scope"),
			*UScopedMessageSubsystem::GetNetModePrefix(this),
			*GetPoiActorLogLabel(),
			*GetName());

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				ScopeResolveRetryTimer,
				this,
				&AScopedMessagePoiActor::EnsurePoiScopeReady,
				FMath::Max(ScopeResolveRetryInterval, 0.01f),
				true);
		}
		return;
	}

	if (CachedPoiScopeId == ResolvedScopeId)
	{
		return;
	}

	CachedPoiScopeId = ResolvedScopeId;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScopeResolveRetryTimer);
	}

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage %s %s resolved Scope=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetPoiActorLogLabel(),
		*GetName(),
		*CachedPoiScopeId.ToString());

	OnPoiScopeReady(CachedPoiScopeId);
	BP_OnPoiScopeReady(CachedPoiScopeId);
}

FScopedMessageScopeId AScopedMessagePoiActor::ResolvePoiScopeId() const
{
	return UScopedMessageSubsystem::HasInstance(this)
		? UScopedMessageSubsystem::Get(this).ResolveScopeId(const_cast<AScopedMessagePoiActor*>(this))
		: FScopedMessageScopeId();
}

bool AScopedMessagePoiActor::IsPoiScopeReady() const
{
	return CachedPoiScopeId.IsValid();
}

FString AScopedMessagePoiActor::GetPoiActorLogLabel() const
{
	return TEXT("Poi actor");
}

void AScopedMessagePoiActor::OnPoiScopeReady(FScopedMessageScopeId ScopeId)
{
}
