// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ABCharacterBase.h"
#include "InputActionValue.h"
#include "Interface/ABCharacterHUDInterface.h"
#include "ABCharacterPlayer.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABCharacterPlayer : public AABCharacterBase, public IABCharacterHUDInterface
{
	GENERATED_BODY()
	
public:
	AABCharacterPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetDead() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Owner() override;
	virtual void PostNetInit() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// Character Control Section
protected:
	void ChangeCharacterControl();
	void SetCharacterControl(ECharacterControlType NewCharacterControlType);
	virtual void SetCharacterControlData(const class UABCharacterControlData* CharacterControlData) override;

// Camera Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

// Input Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChangeControlAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuaterMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	void ShoulderMove(const FInputActionValue& Value);
	void ShoulderLook(const FInputActionValue& Value);

	void QuaterMove(const FInputActionValue& Value);

	ECharacterControlType CurrentCharacterControlType;

	// 프로퍼티 리플리케이션 등록을 위한 함수 오버라이딩.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Attack();

	// 공격 애니메이션을 재생하는 함수.
	void PlayAttackAnimation();

	// AttackHitCheck 함수 오버라이드.
	virtual void AttackHitCheck() override;

	// 공격 판정 확인 함수.
	void AttackHitConfirm(AActor* HitActor);

	// Debug Draw 함수.
	void DrawDebugAttackRange(
		const FColor& DrawColor,
		FVector TraceStart,
		FVector TraceEnd,
		FVector Forward
	);

	// 공격 명령 처리를 위한 Server RPC 선언.
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerRPCAttack(float AttackStartTime);

	// 클라이언트에 공격 명령 전달을 위한 멀티캐스트 RPC 선언.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCAttack();

	// 공격 애니메이션을 재생 요청할 때 사용할 Client RPC 함수.
	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayAnimation(AABCharacterPlayer* CharacterToPlay);


	// 클라이언트에서 액터가 맞았을 때 서버로 판정을 보내는 함수.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyHit(const FHitResult& HitResult, float HitCheckTime);

	// 클라이언트에서 충돌 판정 후 미스가 발생했을 때 보내는 함수.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyMiss(
		FVector_NetQuantize TraceStart, 
		FVector_NetQuantize TraceEnd,
		FVector_NetQuantizeNormal TraceDir,
		float HitCheckTime
	);

	UFUNCTION()
	void OnRep_CanAttack();

	// 현재 공격 중인지를 판단할 때 사용할 변수.
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	// 첫 번째 공격 애니메이션의 재생 길이 값.
	// 타이머를 사용해서 이 시간이 지나면, 공격을 종료하도록 처리.
	float AttackTime = 1.4667f;

	// 시간 관련 변수.
	// 클라이언트가 공격한 시간(공격을 요청한)을 기록하기 위한 변수.
	float LastAttackStartTime = 0.0f;

	// 클라이언트와 서버의 시간 차를 기록하기 위한 변수.
	float AttackTimeDifference = 0.0f;

	// 공격 판정에 사용할 거리 값.
	float AttackCheckDistance = 300.0f;

	// 공격을 시작한 후에 이정도 시간은 지나야 판정이 가능하다고 판단할 기준 값.
	float AcceptMinCheckTime = 0.15f;

// UI Section
protected:
	virtual void SetupHUDWidget(class UABHUDWidget* InHUDWidget) override;
};
