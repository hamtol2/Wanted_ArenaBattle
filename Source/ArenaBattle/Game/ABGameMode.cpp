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

	// 게임 스테이트 클래스 지정.
	GameStateClass = AABGameState::StaticClass();

	// 플레이어 스테이트 클래스 지정.
	PlayerStateClass = AABPlayerState::StaticClass();
}

FTransform AABGameMode::GetRandomStartTransform() const
{
	// PlayerStartArray 배열 정보가 초기화 안됐다면, 기본 위치 반환.
	if (PlayerStartArray.Num() == 0)
	{
		return FTransform(FVector(0.0f, 0.0f, 230.0f));
	}

	// 랜덤으로 인덱스 선택.
	int32 RandIndex = FMath::RandRange(0, PlayerStartArray.Num() - 1);

	// 선택한 인덱스에 해당하는 플레이어 스타트 액터의 트랜스폼 반환.
	return PlayerStartArray[RandIndex]->GetActorTransform();
}

void AABGameMode::OnPlayerKilled(
	AController* Killer, AController* KilledPlayer, APawn* KilledPawn)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 다른 플레이어를 처치한 플레이어의 스테이트 가져오기.
	APlayerState* KillerPlayerState = Killer->PlayerState;
	if (KillerPlayerState)
	{
		// 1점 추가.
		KillerPlayerState->SetScore(KillerPlayerState->GetScore() + 1);

		// 경기 종료 조건 확인.
		if (KillerPlayerState->GetScore() >= 2)
		{
			// 그러면, 5초 후에 다른 맵으로 이동함.
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

	// ErrorMessage에 아무런 값을 입력하지 않으면, 로그인을 통과시킴.
	// 하지만 ErrorMessage에 값을 할당하면, 이를 오류로 간주함.
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

	// NetDriver 가져오기.
	UNetDriver* NetDriver = GetNetDriver();
	if (NetDriver)
	{
		// 클라이언트의 접속이 없는 경우.
		if (NetDriver->ClientConnections.Num() == 0)
		{
			AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No client connection"));
		}
		// 클라이언트의 접속이 있는 경우.
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

	// 액터에 있는 플레이어 스타트 액터 순회.
	for (APlayerStart* PlayerStart : TActorRange<APlayerStart>(GetWorld()))
	{
		// 배열에 추가.
		PlayerStartArray.Add(PlayerStart);
	}
}
void AABGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 타이머 설정.
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
	// 경기에서 남은 시간 계산.
	AABGameState* ABGameState = GetGameState<AABGameState>();
	if (ABGameState && ABGameState->RemainingTime > 0)
	{
		// DefaultGameTimer 함수는 1초마다 반복되기 때문에
		// 1초씩 감소 처리.
		ABGameState->RemainingTime -= 1;

		// 남은 시간 로그 출력.
		AB_LOG(LogABNetwork, Log, TEXT("RemainingTime: %d"),
			ABGameState->RemainingTime);

		// 남은 시간이 없으면, 경기 상태 변경.
		if (ABGameState->RemainingTime <= 0)
		{
			// 남은 시간이 없는데, 현재 게임이 진행 중이라면, 게임 종료 처리.
			if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();
			}
			// 게임을 이미 종료해 다음 게임을 기다리는 중이라면, 다른 월드로 이동.
			else if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				// 접속한 클라이언트를 모두 다른 맵으로 데려가는 함수.
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
		// 경기 종료 처리.
		EndMatch();

		// 경기 종료 후 잠시 대기할 시간으로 타이머 시간 다시 설정.
		ABGameState->RemainingTime = ABGameState->ShowResultWaitingTime;
	}
}