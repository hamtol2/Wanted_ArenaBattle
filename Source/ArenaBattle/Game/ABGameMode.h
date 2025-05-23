// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Interface/ABGameInterface.h"
#include "ABGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode 
	: public AGameMode, public IABGameInterface
{
	GENERATED_BODY()
	
public:
	AABGameMode();

	// ���� ��忡�� ������ ó���� �� �� �������� ������ġ�� ������ �� ����� �Լ�.
	virtual FTransform GetRandomStartTransform() const override;

	// ���� ������ �׿����� �� �� �ֵ��� �Լ� Ȯ��.
	virtual void OnPlayerKilled(
		AController* Killer,
		AController* KilledPlayer,
		APawn* KilledPawn) override;

	// �α��� ���� �Լ�.
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartPlay() override;

protected:
	
	// �÷��̾� ��ŸƮ ���� �迭.
	TArray<TObjectPtr<class APlayerStart>> PlayerStartArray;

	virtual void PostInitializeComponents() override;

	// Ÿ�̸� �ݹ� �Լ�.
	virtual void DefaultGameTimer();

	// ��� ������ �� ȣ���� �Լ�.
	virtual void FinishMatch();

	// Ÿ�̸� �ڵ�.
	FTimerHandle GameTimerHandle;
};