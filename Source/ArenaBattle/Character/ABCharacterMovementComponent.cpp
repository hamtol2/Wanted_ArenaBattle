// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterMovementComponent.h"
#include "ArenaBattle.h"
#include "GameFramework/Character.h"

UABCharacterMovementComponent::UABCharacterMovementComponent()
{
	// �ڷ���Ʈ �̵� �Ÿ� ��(6����).
	TeleportOffset = 600.0f;

	// �ڷ���Ʈ ��Ÿ��(3��).
	TeleportCooltime = 3.0f;

	// Bool ���� false�� �ʱ�ȭ.
	bPressedTeleport = false;
	bDidTeleport = false;
}

void UABCharacterMovementComponent::SetTeleportCommand()
{
	// �ڷ���Ʈ �Է��� ������, true�� ����.
	bPressedTeleport = true;
}

void UABCharacterMovementComponent::ABTeleport()
{
	// �ڷ���Ʈ �̵� ó��.
	if (CharacterOwner)
	{
		// �α� ���.
		AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport Begin"));

		// �ڷ���Ʈ ��ġ.
		FVector TargetLocation
			= CharacterOwner->GetActorLocation()
			+ CharacterOwner->GetActorForwardVector() * TeleportOffset;

		// �ڷ���Ʈ ó��.
		CharacterOwner->TeleportTo(
			TargetLocation,
			CharacterOwner->GetActorRotation()
		);

		// ��Ÿ�� ������ ���� ����.
		bDidTeleport = true;

		// ��Ÿ�� ������ ���� Ÿ�̸� ����.
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([&]()
				{
					// ��Ÿ�� ��.
					bDidTeleport = false;

					// �ڷ���Ʈ ������ �α� ���.
					AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport End"));
				}
			), TeleportCooltime, false
		);
	}
}

void UABCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	// �ڷ���Ʈ ���� ������ Ȯ���� �Ŀ� �ڷ���Ʈ ó��.
	if (bPressedTeleport && !bDidTeleport)
	{
		ABTeleport();
	}

	if (bPressedTeleport)
	{
		bPressedTeleport = false;
	}
}

void UABCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// Ŭ���̾�Ʈ���� ���ڵ�(����)�ؼ� ���� �÷��׸� ���ڵ� ó��.
	bPressedTeleport = ((Flags & FSavedMove_Character::FLAG_Custom_0) != 0);
	bDidTeleport = ((Flags & FSavedMove_Character::FLAG_Custom_1) != 0);

	// ���� Ȯ�� �Ŀ� �ڷ���Ʈ ó��.
	// ������.
	if (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		if (bPressedTeleport && !bDidTeleport)
		{
			AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport Begin"));

			// �ڷ���Ʈ ����.
			ABTeleport();
		}
	}
}

void FABSavedMove_Character::Clear()
{
	Super::Clear();

	// �ʱ�ȭ.
	bPressedTeleport = false;
	bDidTeleport = false;
}

void FABSavedMove_Character::SetInitialPosition(ACharacter* Character)
{
	Super::SetInitialPosition(Character);

	UABCharacterMovementComponent* ABMovement
		= Cast<UABCharacterMovementComponent>(Character->GetCharacterMovement());

	if (ABMovement)
	{
		// ���� ĳ���� �����Ʈ ������Ʈ�� �ڷ���Ʈ�� ó���ϱ� ����
		// �����ϴ� ���� �״�� ����.
		// ���Ŀ� ���� RPC�� ���� ������ ���޵�.
		bPressedTeleport = ABMovement->bPressedTeleport;
		bDidTeleport = ABMovement->bDidTeleport;
	}
}

uint8 FABSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	// ���� RPC�� ���ؼ� �����͸� ������ �� �����ؼ� ������.
	// Ư�� Ư�� �������� ��� �÷��� ���� ����Ʈ�� ������.

	if (bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}

	if (bDidTeleport)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}

FNetworkPredictionData_Client* UABCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UABCharacterMovementComponent* MutableThis 
			= const_cast<UABCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FABNetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}

FABNetworkPredictionData_Client_Character::FABNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FABNetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new FABSavedMove_Character());
}