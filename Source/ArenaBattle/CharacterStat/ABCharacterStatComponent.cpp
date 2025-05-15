// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStat/ABCharacterStatComponent.h"
#include "GameData/ABGameSingleton.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

// Sets default values for this component's properties
UABCharacterStatComponent::UABCharacterStatComponent()
{
	CurrentLevel = 1;
	AttackRadius = 50.0f;

	bWantsInitializeComponent = true;

	// 컴포넌트 리플리케이션 활성화.
	SetIsReplicated(true);
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetLevelStat(CurrentLevel);
	SetHp(BaseStat.MaxHp);
}

void UABCharacterStatComponent::BeginPlay()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::BeginPlay();
}

void UABCharacterStatComponent::ReadyForReplication()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::ReadyForReplication();
}

void UABCharacterStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 프로퍼티를 리플리케이션에 등록.
	DOREPLIFETIME(UABCharacterStatComponent, CurrentHp);
}

void UABCharacterStatComponent::OnRep_CurrentHp()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 서버로부터 받은 변경된 CurrentHp 정보를 델리게이트를 통해 알림.
	OnHpChanged.Broadcast(CurrentHp);

	// 죽었는 지 확인.
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}
}

void UABCharacterStatComponent::SetLevelStat(int32 InNewLevel)
{
	CurrentLevel = FMath::Clamp(InNewLevel, 1, UABGameSingleton::Get().CharacterMaxLevel);
	SetBaseStat(UABGameSingleton::Get().GetCharacterStat(CurrentLevel));
	check(BaseStat.MaxHp > 0.0f);
}

float UABCharacterStatComponent::ApplyDamage(float InDamage)
{
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);

	SetHp(PrevHp - ActualDamage);
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}

	return ActualDamage;
}

void UABCharacterStatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, BaseStat.MaxHp);
	
	OnHpChanged.Broadcast(CurrentHp);
}

