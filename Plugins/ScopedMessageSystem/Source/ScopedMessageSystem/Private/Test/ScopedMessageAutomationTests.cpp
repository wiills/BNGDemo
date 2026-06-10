#include "Misc/AutomationTest.h"

#include "ScopedMessageSubsystem.h"
#include "Test/ScopedMessageAutomationTestTypes.h"
#include "Test/ScopedMessagePoiDemoTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScopedMessagePayloadAutomationTest,
	"ScopedMessageSystem.Payload.EncodeDecode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScopedMessagePayloadAutomationTest::RunTest(const FString& Parameters)
{
	FScopedMessageDemoTerminalActivatedPayload Input;
	Input.TerminalId = TEXT("TerminalA");
	Input.ActivationCount = 3;
	Input.Location = FVector(1.0, 2.0, 3.0);

	const FScopedMessagePayload Payload = FScopedMessagePayload::Make(Input);
	TestTrue(TEXT("Payload should be valid"), Payload.IsValid());
	TestTrue(TEXT("Payload should resolve as terminal activation payload"), Payload.IsPayloadOfType(FScopedMessageDemoTerminalActivatedPayload::StaticStruct()));

	FScopedMessageDemoTerminalActivatedPayload Output;
	TestTrue(TEXT("Payload should decode"), Payload.TryDecode(Output));
	TestEqual(TEXT("Decoded TerminalId"), Output.TerminalId, Input.TerminalId);
	TestEqual(TEXT("Decoded ActivationCount"), Output.ActivationCount, Input.ActivationCount);
	TestEqual(TEXT("Decoded Location"), Output.Location, Input.Location);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScopedMessageRoutingAutomationTest,
	"ScopedMessageSystem.Routing.ScopeIsolationAndPartialMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScopedMessageRoutingAutomationTest::RunTest(const FString& Parameters)
{
	UScopedMessageSubsystem* Subsystem = NewObject<UScopedMessageSubsystem>();
	UScopedMessageAutomationScopeObject* ScopeA = NewObject<UScopedMessageAutomationScopeObject>();
	UScopedMessageAutomationScopeObject* ScopeB = NewObject<UScopedMessageAutomationScopeObject>();
	ScopeA->ScopeId = FScopedMessageScopeId(FName(TEXT("Automation.Scope.A")));
	ScopeB->ScopeId = FScopedMessageScopeId(FName(TEXT("Automation.Scope.B")));

	const FGameplayTag ParentChannel = FGameplayTag::RequestGameplayTag(TEXT("Poi.Demo.Terminal"));
	const FGameplayTag ActivationChannel = FGameplayTag::RequestGameplayTag(TEXT("Poi.Demo.Terminal.Activated"));

	int32 ExactCountA = 0;
	int32 ExactCountB = 0;
	int32 PartialCountA = 0;

	FScopedMessageListenerHandle ExactA = Subsystem->Subscribe<FScopedMessageDemoTerminalActivatedPayload>(
		ScopeA,
		ActivationChannel,
		[&ExactCountA](FGameplayTag, const FScopedMessageDemoTerminalActivatedPayload&)
		{
			ExactCountA++;
		});

	FScopedMessageListenerHandle ExactB = Subsystem->Subscribe<FScopedMessageDemoTerminalActivatedPayload>(
		ScopeB,
		ActivationChannel,
		[&ExactCountB](FGameplayTag, const FScopedMessageDemoTerminalActivatedPayload&)
		{
			ExactCountB++;
		});

	FScopedMessageListenerHandle PartialA = Subsystem->Subscribe<FScopedMessageDemoTerminalActivatedPayload>(
		ScopeA,
		ParentChannel,
		[&PartialCountA](FGameplayTag, const FScopedMessageDemoTerminalActivatedPayload&)
		{
			PartialCountA++;
		},
		EScopedMessageMatch::PartialMatch);

	FScopedMessageDemoTerminalActivatedPayload Payload;
	Payload.TerminalId = TEXT("TerminalA");
	Subsystem->BroadcastMessage(ScopeA, ActivationChannel, Payload, EScopedMessageReplication::LocalOnly);

	TestEqual(TEXT("Scope A exact listener should receive"), ExactCountA, 1);
	TestEqual(TEXT("Scope B exact listener should not receive"), ExactCountB, 0);
	TestEqual(TEXT("Scope A partial listener should receive child channel"), PartialCountA, 1);

	ExactA.Unregister();
	ExactB.Unregister();
	PartialA.Unregister();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScopedMessageResolverAutomationTest,
	"ScopedMessageSystem.ScopeResolver.CustomResolverOverridesDefault",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScopedMessageResolverAutomationTest::RunTest(const FString& Parameters)
{
	UScopedMessageSubsystem* Subsystem = NewObject<UScopedMessageSubsystem>();
	UObject* ContextObject = NewObject<UObject>();

	const FScopedMessageScopeId CustomScope(FName(TEXT("Automation.Scope.Custom")));
	const FDelegateHandle ResolverHandle = Subsystem->RegisterScopeResolver(
		FScopedMessageScopeResolver::CreateLambda([CustomScope, ContextObject](UObject* ScopeContext, FScopedMessageScopeId& OutScopeId)
		{
			if (ScopeContext == ContextObject)
			{
				OutScopeId = CustomScope;
				return true;
			}
			return false;
		}));

	TestTrue(TEXT("Resolver handle should be valid"), ResolverHandle.IsValid());
	TestEqual(TEXT("Custom resolver should resolve context"), Subsystem->ResolveScopeId(ContextObject), CustomScope);

	Subsystem->UnregisterScopeResolver(ResolverHandle);
	TestFalse(TEXT("Context should fall back to empty default scope after unregister"), Subsystem->ResolveScopeId(ContextObject).IsValid());
	return true;
}

#endif
