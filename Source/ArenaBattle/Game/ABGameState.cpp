// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameState.h"
#include "ArenaBattle.h"
#include "Net/UnrealNetwork.h"

AABGameState::AABGameState()
{
	// 타이머에 사용할 RemainingTime 변수에 경기 시간 값 설정.
	RemainingTime = MatchPlayTime;
}

void AABGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABGameState, RemainingTime);
}

//void AABGameState::HandleBeginPlay()
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::HandleBeginPlay();
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
//
//void AABGameState::OnRep_ReplicatedHasBegunPlay()
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::OnRep_ReplicatedHasBegunPlay();
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}