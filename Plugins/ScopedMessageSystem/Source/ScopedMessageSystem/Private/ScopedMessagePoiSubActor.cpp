#include "ScopedMessagePoiSubActor.h"

#include "ScopedMessageSubsystem.h"

AScopedMessagePoiSubActor::AScopedMessagePoiSubActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
}

void AScopedMessagePoiSubActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoResolveScopeOnBeginPlay)
	{
		EnsurePoiScopeReady();
	}
}

void AScopedMessagePoiSubActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScopeResolveRetryTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void AScopedMessagePoiSubActor::EnsurePoiScopeReady()
{
	const FScopedMessageScopeId ResolvedScopeId = ResolvePoiScopeId();
	if (!ResolvedScopeId.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage Poi sub actor %s waiting for valid Scope"),
			*UScopedMessageSubsystem::GetNetModePrefix(this),
			*GetName());

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				ScopeResolveRetryTimer,
				this,
				&AScopedMessagePoiSubActor::EnsurePoiScopeReady,
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

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage Poi sub actor %s resolved Scope=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*CachedPoiScopeId.ToString());

	OnPoiScopeReady(CachedPoiScopeId);
	BP_OnPoiScopeReady(CachedPoiScopeId);
}

FScopedMessageScopeId AScopedMessagePoiSubActor::ResolvePoiScopeId() const
{
	return UScopedMessageSubsystem::HasInstance(this)
		? UScopedMessageSubsystem::Get(this).ResolveScopeId(const_cast<AScopedMessagePoiSubActor*>(this))
		: FScopedMessageScopeId();
}

bool AScopedMessagePoiSubActor::IsPoiScopeReady() const
{
	return CachedPoiScopeId.IsValid();
}

void AScopedMessagePoiSubActor::OnPoiScopeReady(FScopedMessageScopeId ScopeId)
{
}
