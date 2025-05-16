// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABCharacterControlData.h"
#include "UI/ABHUDWidget.h"
#include "CharacterStat/ABCharacterStatComponent.h"
#include "Interface/ABGameInterface.h"
#include "ArenaBattle.h"
#include "Components/CapsuleComponent.h"
#include "Physics/ABCollision.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

#include "GameFramework/GameStateBase.h"

AABCharacterPlayer::AABCharacterPlayer()
{
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Input
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputChangeActionControlRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ChangeControl.IA_ChangeControl'"));
	if (nullptr != InputChangeActionControlRef.Object)
	{
		ChangeControlAction = InputChangeActionControlRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderMove.IA_ShoulderMove'"));
	if (nullptr != InputActionShoulderMoveRef.Object)
	{
		ShoulderMoveAction = InputActionShoulderMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderLook.IA_ShoulderLook'"));
	if (nullptr != InputActionShoulderLookRef.Object)
	{
		ShoulderLookAction = InputActionShoulderLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionQuaterMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_QuaterMove.IA_QuaterMove'"));
	if (nullptr != InputActionQuaterMoveRef.Object)
	{
		QuaterMoveAction = InputActionQuaterMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;

	// 시작할 때는 공격이 가능하도록 설정.
	bCanAttack = true;

	bReplicates = true;
}

void AABCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
	}

	SetCharacterControl(CurrentCharacterControlType);
}

void AABCharacterPlayer::SetDead()
{
	Super::SetDead();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

void AABCharacterPlayer::PossessedBy(AController* NewController)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// PossessedBy 함수 호출 전 액터의 소유 확인.
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner: %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("No Owner"));
	}

	Super::PossessedBy(NewController);

	// PossessedBy 함수 호출 후 액터의 소유 확인.
	OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner: %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("No Owner"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::OnRep_Owner()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnRep_Owner();

	// Super::OnRep_Owner() 함수 호출 후 오너 확인.
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner: %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("No Owner"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::PostNetInit()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s %s"), TEXT("Begin"), *GetName());

	Super::PostNetInit();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(ChangeControlAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ChangeCharacterControl);
	EnhancedInputComponent->BindAction(ShoulderMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderMove);
	EnhancedInputComponent->BindAction(ShoulderLookAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderLook);
	EnhancedInputComponent->BindAction(QuaterMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::QuaterMove);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Attack);
}

void AABCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
	{
		SetCharacterControl(ECharacterControlType::Shoulder);
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Shoulder)
	{
		SetCharacterControl(ECharacterControlType::Quater);
	}
}

void AABCharacterPlayer::SetCharacterControl(ECharacterControlType NewCharacterControlType)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UABCharacterControlData* NewCharacterControl = CharacterControlManager[NewCharacterControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	CurrentCharacterControlType = NewCharacterControlType;
}

void AABCharacterPlayer::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);

	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch = CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw = CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll = CharacterControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest = CharacterControlData->bDoCollisionTest;
}

void AABCharacterPlayer::ShoulderMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AABCharacterPlayer::ShoulderLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AABCharacterPlayer::QuaterMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	float InputSizeSquared = MovementVector.SquaredLength();
	float MovementVectorSize = 1.0f;
	float MovementVectorSizeSquared = MovementVector.SquaredLength();
	if (MovementVectorSizeSquared > 1.0f)
	{
		MovementVector.Normalize();
		MovementVectorSizeSquared = 1.0f;
	}
	else
	{
		MovementVectorSize = FMath::Sqrt(MovementVectorSizeSquared);
	}

	FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	AddMovementInput(MoveDirection, MovementVectorSize);
}

void AABCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 프로퍼티를 리플리케이션에 등록.
	DOREPLIFETIME(AABCharacterPlayer, bCanAttack);
}

void AABCharacterPlayer::Attack()
{
	//ProcessComboCommand();

	if (bCanAttack)
	{
		// 클라이언트 로직.
		// 클라이언트 입장에서는 Local:AutonomouseProxy/Remote:Authority.
		if (!HasAuthority())
		{
			// 공격 다시 못하게 설정.
			bCanAttack = false;

			// bCanAttack 값이 변경되지 않아, 리플리케이션이 되지 않기 때문에
			// OnRep_ 함수 호출을 기대하지 않고, 직접 모드 변경.
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

			// 공격 종료를 위한 타이머도 클라이언트에서 설정.
			// 고려해야할 것:
			// 캐릭터 무브먼트 설정이나 공격 시간 등은 
			// 클라이언트가 악의적으로 변경(변조)할 수 있다는 사실을 가정해야 한다.
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(
				Handle,
				FTimerDelegate::CreateLambda([&]()
					{
						// 공격이 끝나면 처리.
						bCanAttack = true;

						GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
					})
				, AttackTime, false
			);

			// 애니메이션 재생.
			PlayAttackAnimation();
		}

		// 공격 입력이 들어오면, Server RPC를 호출해 알림.
		// 이제는 공격을 서버에 알릴 때 클라이언트가 요청한 시간을 서버에 전송함.
		// GetWorld()->GetTimeSeconds() 함수는 현재 월드의 시간을 반환.
		// 서버의 시간을 기준으로 해야함.
		ServerRPCAttack(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
	}
}

void AABCharacterPlayer::PlayAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	// 기존에 재생 중인 몽타주는 중지.
	AnimInstance->StopAllMontages(0.0f);
	// 몽타주 애셋 재생.
	AnimInstance->Montage_Play(ComboActionMontage);
}

void AABCharacterPlayer::AttackHitCheck()
{
	// 공격 판정은 서버에서만 진행.
	//if (HasAuthority())
	if (IsLocallyControlled())
	{
		// 로그 출력.
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

		// 일단 상위 클래스에 있는 로직을 그대로 옮김.
		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = Stat->GetTotalStat().AttackRange;
		const float AttackRadius = Stat->GetAttackRadius();
		const float AttackDamage = Stat->GetTotalStat().Attack;
		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + Forward * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(
			OutHitResult,
			Start,
			End,
			FQuat::Identity,
			CCHANNEL_ABACTION,
			FCollisionShape::MakeSphere(AttackRadius),
			Params
		);

		// 공격 판정 타이밍을 저장.
		float HitCheckTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

		// 클라이언트.
		if (!HasAuthority())
		{
			// 판정을 보낼 때 RPC에 여러 정보를 함께 보낸다.
			// 이때 판정 시간도 보낸다.
		}

		// 서버.
		else
		{

		}

//		if (HitDetected)
//		{
//			FDamageEvent DamageEvent;
//			OutHitResult.GetActor()->TakeDamage(
//				AttackDamage,
//				DamageEvent,
//				GetController(),
//				this
//			);
//		}
//
//#if ENABLE_DRAW_DEBUG
//
//		FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
//		float CapsuleHalfHeight = AttackRange * 0.5f;
//		FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;
//
//		DrawDebugCapsule(
//			GetWorld(),
//			CapsuleOrigin,
//			CapsuleHalfHeight,
//			AttackRadius,
//			FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(),
//			DrawColor,
//			false,
//			5.0f
//		);
//#endif
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(const FHitResult& HitResult, float HitCheckTime)
{
	return true;
}

void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(const FHitResult& HitResult, float HitCheckTime)
{
}

bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(FVector TraceStart, FVector TraceEnd, FVector TraceDir, float HitCheckTime)
{
	return true;
}

void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(FVector TraceStart, FVector TraceEnd, FVector TraceDir, float HitCheckTime)
{
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	// 공격 타이밍에 대한 검증 추가.
	// 이전 공격 타이밍 값이 없으면 검증 안하고 통과.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// 이전에 기록된 공격 시간과 이번에 요청한 공격 시간과의 차이가
	// 공격 애니메이션 길이보다 큰지 확인.
	// 이 값이 공격 애니메이션 길이보다 작다면, 클라이언트를 의심해볼 수 있는 상황.
	return (AttackStartTime - LastAttackStartTime) > AttackTime;
}

void AABCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	// 서버에서 처리 할 일.
	// 공격 중이라는 의미로 플래그 설정.
	bCanAttack = false;

	// 공격 중에는 이동 중지.
	OnRep_CanAttack();

	// 서버-클라이언트의 시간 차이 기록.
	AttackTimeDifference = GetWorld()->GetTimeSeconds() - AttackStartTime;

	// 시간 차이 값을 로그로 확인.
	AB_LOG(LogABNetwork, Log, TEXT("LagTime: %f"), AttackTimeDifference);

	// 예외처리.
	// 네트워크 상태가 너무 안좋은 경우, AttackTimeDifference 값이 너무 크게되면,
	// 아래의 타이머가 동작하지 않을 수 있기 때문에 최소한의 시간 값을 보정.
	AttackTimeDifference = FMath::Clamp(
		AttackTimeDifference,
		0.0f,
		AttackTime - 0.01f
	);

	// 타이머 설정.
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda([&]()
			{
				bCanAttack = true;
				OnRep_CanAttack();
			})
		, AttackTime - AttackTimeDifference, false
	);

	// 클라이언트가 공격 요청을 한 시간 값 저장.
	LastAttackStartTime = AttackStartTime;

	// 서버도 애니메이션 재생.
	PlayAttackAnimation();

	// 로그 출력.
	//AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 클라이언트가 공격 입력을 해서 서버로 호출을 요청할 때 실행됨.
	// 리슨 서버의 경우에는 로컬에서 실행되기 때문에 곧바로 실행됨.
	// 그 외의 다른 클라이언트는 네트워크로부터 신호를 받아 실행됨.
	MulticastRPCAttack();
}

void AABCharacterPlayer::MulticastRPCAttack_Implementation()
{
	// 서버와 본인(입력을 전달한) 클라이언트는 이미
	// 공격 상태 값과 무브먼트 모드 값을 변경함.
	// 여기에서는 나머지 클라이언트에 대한 처리를 진행.
	// 공격 상태 값과 무브먼트 모드 값은 프로퍼티 리플리케이션으로 전달되어 처리가 될 것.
	// 하지만, 공격 애니메이션은 재생하지 않았으니 재생해야 함.
	// 나머지 클라이언트인지 파악.
	if (!IsLocallyControlled())
	{
		PlayAttackAnimation();
	}
}



void AABCharacterPlayer::OnRep_CanAttack()
{
	if (bCanAttack)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}
}

void AABCharacterPlayer::SetupHUDWidget(UABHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		InHUDWidget->UpdateStat(Stat->GetBaseStat(), Stat->GetModifierStat());
		InHUDWidget->UpdateHpBar(Stat->GetCurrentHp());

		Stat->OnStatChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateStat);
		Stat->OnHpChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateHpBar);
	}
}
