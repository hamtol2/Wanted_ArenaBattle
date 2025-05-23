// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/ABGameInterface.h"
#include "ABGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode 
	: public AGameModeBase, public IABGameInterface
{
	GENERATED_BODY()
	
public:
	AABGameMode();

	// 게임 모드에서 리스폰 처리를 할 때 랜덤으로 스폰위치를 가져올 때 사용할 함수.
	virtual FTransform GetRandomStartTransform() const override;

	// 누가 누구를 죽였는지 알 수 있도록 함수 확장.
	virtual void OnPlayerKilled(
		AController* Killer,
		AController* KilledPlayer,
		APawn* KilledPawn) override;

	// 로그인 관련 함수.
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartPlay() override;

protected:
	// 플레이어 스타트 액터 배열.
	TArray<TObjectPtr<class APlayerStart>> PlayerStartArray;
};