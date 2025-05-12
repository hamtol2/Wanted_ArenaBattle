// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "Player/ABPlayerController.h"
#include "ArenaBattle.h"
#include "ABGameState.h"

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
}

void AABGameMode::OnPlayerDead()
{

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
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::StartPlay();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}