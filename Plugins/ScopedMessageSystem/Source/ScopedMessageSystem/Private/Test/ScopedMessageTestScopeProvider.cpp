#include "Test/ScopedMessageTestScopeProvider.h"

#include "GameFramework/Actor.h"
#include "Misc/Guid.h"
#include "Net/UnrealNetwork.h"

AScopedMessageTestScopeProvider::AScopedMessageTestScopeProvider()
{
	PrimaryActorTick.bCanEverTick = false;

	// Establish a default root component.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	bReplicates = true;
}

void AScopedMessageTestScopeProvider::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !ScopeId.IsValid())
	{
		const FString GeneratedName = FString::Printf(TEXT("TestPOI.%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		ScopeId = FScopedMessageScopeId(FName(*GeneratedName));
	}
}

void AScopedMessageTestScopeProvider::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AScopedMessageTestScopeProvider, ScopeId);
}

FScopedMessageScopeId AScopedMessageTestScopeProvider::GetScopeId_Implementation() const
{
	return ScopeId;
}
