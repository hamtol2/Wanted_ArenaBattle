// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "Player/ABPlayerController.h"
#include "ArenaBattle.h"
#include "ABGameState.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "ABPlayerState.h"

AABGameMode::AABGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Script/Engine.Blueprint'/Game/ArenaBattle/Blueprint/BP_ABCharacterPlayer.BP_ABCharacterPlayer_C'"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/ArenaBattle.ABPlayerController"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}

	// ���� ������Ʈ Ŭ���� ����.
	GameStateClass = AABGameState::StaticClass();

	// �÷��̾� ������Ʈ Ŭ���� ����.
	PlayerStateClass = AABPlayerState::StaticClass();
}

FTransform AABGameMode::GetRandomStartTransform() const
{
	// PlayerStartArray �迭 ������ �ʱ�ȭ �ȵƴٸ�, �⺻ ��ġ ��ȯ.
	if (PlayerStartArray.Num() == 0)
	{
		return FTransform(FVector(0.0f, 0.0f, 230.0f));
	}

	// �������� �ε��� ����.
	int32 RandIndex = FMath::RandRange(0, PlayerStartArray.Num() - 1);

	// ������ �ε����� �ش��ϴ� �÷��̾� ��ŸƮ ������ Ʈ������ ��ȯ.
	return PlayerStartArray[RandIndex]->GetActorTransform();
}

void AABGameMode::OnPlayerKilled(
	AController* Killer, AController* KilledPlayer, APawn* KilledPawn)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// �ٸ� �÷��̾ óġ�� �÷��̾��� ������Ʈ ��������.
	APlayerState* KillerPlayerState = Killer->PlayerState;
	if (KillerPlayerState)
	{
		// 1�� �߰�.
		KillerPlayerState->SetScore(KillerPlayerState->GetScore() + 1);

		// ��� ���� ���� Ȯ��.
		if (KillerPlayerState->GetScore() >= 2)
		{
			// �׷���, 5�� �Ŀ� �ٸ� ������ �̵���.
			FinishMatch();
		}
	}
}

void AABGameMode::PreLogin(
	const FString& Options,
	const FString& Address,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("=========================="));
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	// ErrorMessage�� �ƹ��� ���� �Է����� ������, �α����� �����Ŵ.
	// ������ ErrorMessage�� ���� �Ҵ��ϸ�, �̸� ������ ������.
	//ErrorMessage = TEXT("Server is Full");

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

APlayerController* AABGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	APlayerController* NewPlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));

	return NewPlayerController;
}

void AABGameMode::PostLogin(APlayerController* NewPlayer)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PostLogin(NewPlayer);

	// NetDriver ��������.
	UNetDriver* NetDriver = GetNetDriver();
	if (NetDriver)
	{
		// Ŭ���̾�Ʈ�� ������ ���� ���.
		if (NetDriver->ClientConnections.Num() == 0)
		{
			AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No client connection"));
		}
		// Ŭ���̾�Ʈ�� ������ �ִ� ���.
		else
		{
			for (const auto& Connection : NetDriver->ClientConnections)
			{
				AB_LOG(LogABNetwork, Log, TEXT("Client connections: %s"),
					*Connection->GetName());
			}
		}
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No NetDriver"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABGameMode::StartPlay()
{
	Super::StartPlay();

	// ���Ϳ� �ִ� �÷��̾� ��ŸƮ ���� ��ȸ.
	for (APlayerStart* PlayerStart : TActorRange<APlayerStart>(GetWorld()))
	{
		// �迭�� �߰�.
		PlayerStartArray.Add(PlayerStart);
	}
}
void AABGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Ÿ�̸� ����.
	GetWorld()->GetTimerManager().SetTimer(
		GameTimerHandle,
		this,
		&AABGameMode::DefaultGameTimer,
		GetWorldSettings()->GetEffectiveTimeDilation(),
		true
	);
}


void AABGameMode::DefaultGameTimer()
{
	// ��⿡�� ���� �ð� ���.
	AABGameState* ABGameState = GetGameState<AABGameState>();
	if (ABGameState && ABGameState->RemainingTime > 0)
	{
		// DefaultGameTimer �Լ��� 1�ʸ��� �ݺ��Ǳ� ������
		// 1�ʾ� ���� ó��.
		ABGameState->RemainingTime -= 1;

		// ���� �ð� �α� ���.
		AB_LOG(LogABNetwork, Log, TEXT("RemainingTime: %d"),
			ABGameState->RemainingTime);

		// ���� �ð��� ������, ��� ���� ����.
		if (ABGameState->RemainingTime <= 0)
		{
			// ���� �ð��� ���µ�, ���� ������ ���� ���̶��, ���� ���� ó��.
			if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();
			}
			// ������ �̹� ������ ���� ������ ��ٸ��� ���̶��, �ٸ� ����� �̵�.
			else if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				// ������ Ŭ���̾�Ʈ�� ��� �ٸ� ������ �������� �Լ�.
				GetWorld()->ServerTravel(
					TEXT("/Game/ArenaBattle/Maps/Part3Step2?listen")
				);
			}
		}
	}
}

void AABGameMode::FinishMatch()
{
	AABGameState* ABGameState = GetGameState<AABGameState>();

	if (ABGameState && IsMatchInProgress())
	{
		// ��� ���� ó��.
		EndMatch();

		// ��� ���� �� ��� ����� �ð����� Ÿ�̸� �ð� �ٽ� ����.
		ABGameState->RemainingTime = ABGameState->ShowResultWaitingTime;
	}
}