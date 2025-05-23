// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ABGameState.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameState : public AGameState
{
	GENERATED_BODY()
	
public:

	AABGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//virtual void HandleBeginPlay() override;
	//virtual void OnRep_ReplicatedHasBegunPlay() override;

	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	// ��� �ð�.
	int32 MatchPlayTime = 2000;

	// ���� ����� �� ��� ����� �ð�.
	int32 ShowResultWaitingTime = 5;
};