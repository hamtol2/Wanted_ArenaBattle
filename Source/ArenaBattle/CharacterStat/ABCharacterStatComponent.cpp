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

	// ������Ʈ ���ø����̼� Ȱ��ȭ.
	SetIsReplicated(true);
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetLevelStat(CurrentLevel);
	MaxHp = BaseStat.MaxHp;
	SetHp(MaxHp);

	// ���� ���� �̺�Ʈ�� �ִ� ü�� ���� �Լ� ���.
	OnStatChanged.AddUObject(this, &UABCharacterStatComponent::SetNewMaxHp);
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

	// ������Ƽ�� ���ø����̼ǿ� ���.

	// HP ������Ƽ�� ��� Ŭ���̾�Ʈ���� ����.
	DOREPLIFETIME(UABCharacterStatComponent, CurrentHp);
	DOREPLIFETIME(UABCharacterStatComponent, MaxHp);

	// ĳ���͸� ������ Ŭ���̾�Ʈ���� �����ϵ��� ����.
	DOREPLIFETIME_CONDITION(UABCharacterStatComponent, BaseStat, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UABCharacterStatComponent, ModifierStat, COND_OwnerOnly);
}

void UABCharacterStatComponent::OnRep_CurrentHp()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// �����κ��� ���� ����� CurrentHp ������ ��������Ʈ�� ���� �˸�.
	OnHpChanged.Broadcast(CurrentHp, MaxHp);

	// �׾��� �� Ȯ��.
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}
}

void UABCharacterStatComponent::OnRep_MaxHp()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	OnHpChanged.Broadcast(CurrentHp, MaxHp);
}

void UABCharacterStatComponent::OnRep_BaseStat()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// ���� ���� �̺�Ʈ ����.
	OnStatChanged.Broadcast(BaseStat, ModifierStat);
}

void UABCharacterStatComponent::OnRep_ModifierStat()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// ���� ���� �̺�Ʈ ����.
	OnStatChanged.Broadcast(BaseStat, ModifierStat);
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

void UABCharacterStatComponent::SetNewMaxHp(
	const FABCharacterStat& InBaseStat, const FABCharacterStat& InModifierStat)
{
	// MaxHp ���� ����ƴ��� ����.
	float PrevMaxHp = MaxHp;
	MaxHp = GetTotalStat().MaxHp;

	// �� �� ���� �ٸ��� �̺�Ʈ ����.
	if (PrevMaxHp != MaxHp)
	{
		OnHpChanged.Broadcast(CurrentHp, MaxHp);
	}
}

void UABCharacterStatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHp);
	
	OnHpChanged.Broadcast(CurrentHp, MaxHp);
}