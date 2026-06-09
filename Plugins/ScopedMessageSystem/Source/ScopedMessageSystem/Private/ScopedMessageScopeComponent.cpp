#include "ScopedMessageScopeComponent.h"

#include "GameFramework/Actor.h"
#include "Misc/Guid.h"
#include "Net/UnrealNetwork.h"

UScopedMessageScopeComponent::UScopedMessageScopeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UScopedMessageScopeComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (bGenerateScopeIdOnAuthority && Owner && Owner->HasAuthority() && !ScopeId.IsValid())
	{
		const FString GeneratedName = FString::Printf(
			TEXT("%s.%s"),
			*GeneratedScopePrefix.ToString(),
			*FGuid::NewGuid().ToString(EGuidFormats::Digits));
		ScopeId = FScopedMessageScopeId(FName(*GeneratedName));
	}
}

void UScopedMessageScopeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UScopedMessageScopeComponent, ScopeId);
}

FScopedMessageScopeId UScopedMessageScopeComponent::GetScopeId_Implementation() const
{
	return ScopeId;
}

void UScopedMessageScopeComponent::SetScopeId(FScopedMessageScopeId InScopeId)
{
	ScopeId = InScopeId;
}
