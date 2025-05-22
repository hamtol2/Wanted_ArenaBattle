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
#include "EngineUtils.h"

#include "ABCharacterMovementComponent.h"

AABCharacterPlayer::AABCharacterPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UABCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionTeleportRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Teleport.IA_Teleport'"));
	if (nullptr != InputActionTeleportRef.Object)
	{
		TeleportAction = InputActionTeleportRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;

	// ������ ���� ������ �����ϵ��� ����.
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

	// PossessedBy �Լ� ȣ�� �� ������ ���� Ȯ��.
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

	// PossessedBy �Լ� ȣ�� �� ������ ���� Ȯ��.
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

	// Super::OnRep_Owner() �Լ� ȣ�� �� ���� Ȯ��.
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
	
	// �ڷ���Ʈ �Է� ���ε�.
	EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Teleport);
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
	// ������ �Ұ����� ���� �̵��� ���ϵ��� ����.
	if (!bCanAttack)
	{
		return;
	}

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
	// ������ �Ұ����� ���� �̵��� ���ϵ��� ����.
	if (!bCanAttack)
	{
		return;
	}

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

	// ������Ƽ�� ���ø����̼ǿ� ���.
	DOREPLIFETIME(AABCharacterPlayer, bCanAttack);
}

void AABCharacterPlayer::Attack()
{
	//ProcessComboCommand();

	if (bCanAttack)
	{
		// Ŭ���̾�Ʈ ����.
		// Ŭ���̾�Ʈ ���忡���� Local:AutonomouseProxy/Remote:Authority.
		if (!HasAuthority())
		{
			// ���� �ٽ� ���ϰ� ����.
			bCanAttack = false;

			// bCanAttack ���� ������� �ʾ�, ���ø����̼��� ���� �ʱ� ������
			// OnRep_ �Լ� ȣ���� ������� �ʰ�, ���� ��� ����.
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

			// ���� ���Ḧ ���� Ÿ�̸ӵ� Ŭ���̾�Ʈ���� ����.
			// ����ؾ��� ��:
			// ĳ���� �����Ʈ �����̳� ���� �ð� ���� 
			// Ŭ���̾�Ʈ�� ���������� ����(����)�� �� �ִٴ� ����� �����ؾ� �Ѵ�.
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(
				Handle,
				FTimerDelegate::CreateLambda([&]()
					{
						// ������ ������ ó��.
						bCanAttack = true;

						GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
					})
				, AttackTime, false
			);

			// �ִϸ��̼� ���.
			PlayAttackAnimation();
		}

		// ���� �Է��� ������, Server RPC�� ȣ���� �˸�.
		// ������ ������ ������ �˸� �� Ŭ���̾�Ʈ�� ��û�� �ð��� ������ ������.
		// GetWorld()->GetTimeSeconds() �Լ��� ���� ������ �ð��� ��ȯ.
		// ������ �ð��� �������� �ؾ���.
		ServerRPCAttack(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
	}
}

void AABCharacterPlayer::Teleport()
{
	AB_LOG(LogABTeleport, Log, TEXT("%s"), TEXT("Begin"));

	UABCharacterMovementComponent* ABMovement 
		= Cast<UABCharacterMovementComponent>(GetCharacterMovement());

	if (ABMovement)
	{
		ABMovement->SetTeleportCommand();
	}
}

void AABCharacterPlayer::PlayAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	// ������ ��� ���� ��Ÿ�ִ� ����.
	AnimInstance->StopAllMontages(0.0f);
	// ��Ÿ�� �ּ� ���.
	AnimInstance->Montage_Play(ComboActionMontage);
}

void AABCharacterPlayer::AttackHitCheck()
{
	// ���� ������ ���������� ����.
	//if (HasAuthority())
	if (IsLocallyControlled())
	{
		// �α� ���.
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

		// �ϴ� ���� Ŭ������ �ִ� ������ �״�� �ű�.
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

		// ���� ���� Ÿ�̹��� ����.
		float HitCheckTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

		// Ŭ���̾�Ʈ.
		if (!HasAuthority())
		{
			// ������ ���� �� RPC�� ���� ������ �Բ� ������.
			// �̶� ���� �ð��� ������.

			if (HitDetected)
			{
				// �浹 ������ �� ���, ������ �浹�� �߻��ߴٰ� �˸�.
				// �̶� ���� ���� ���� �ð��� �Բ� ����.
				ServerRPCNotifyHit(OutHitResult, HitCheckTime);
			}
			else
			{
				// �浹 ������ �ȵ� ��쿡�� ������ ����.
				ServerRPCNotifyMiss(Start, End, Forward, HitCheckTime);
			}

		}

		// ����.
		else
		{
			// ����׷� ���� ���� �׷��ֱ�.
			FColor DebugColor = HitDetected ? FColor::Green : FColor::Red;
			DrawDebugAttackRange(DebugColor, Start, End, Forward);

			// ������ ��쿡�� �������� �ʰ� �״�� ����� ���� ���� ����.
			if (HitDetected)
			{
				AttackHitConfirm(OutHitResult.GetActor());
			}
		}
	}
}

void AABCharacterPlayer::AttackHitConfirm(AActor* HitActor)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// ���������� ����.
	if (HasAuthority())
	{
		// ���� ������� ���ȿ���.
		const float AttackDamage = Stat->GetTotalStat().Attack;

		FDamageEvent DamageEvent;
		HitActor->TakeDamage(
			AttackDamage,
			DamageEvent,
			GetController(),
			this
		);
	}
}

void AABCharacterPlayer::DrawDebugAttackRange(
	const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward)
{
#if ENABLE_DRAW_DEBUG

	const float AttackRange = Stat->GetTotalStat().AttackRange;
	const float AttackRadius = Stat->GetAttackRadius();

	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	//FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

	DrawDebugCapsule(
		GetWorld(),
		CapsuleOrigin,
		CapsuleHalfHeight,
		AttackRadius,
		FRotationMatrix::MakeFromZ(Forward).ToQuat(),
		DrawColor,
		false,
		5.0f
	);
#endif
}

void AABCharacterPlayer::ClientRPCPlayAnimation_Implementation(
	AABCharacterPlayer* CharacterToPlay)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	if (CharacterToPlay)
	{
		CharacterToPlay->PlayAttackAnimation();
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(const FHitResult& HitResult, float HitCheckTime)
{
	// Ŭ���̾�Ʈ�κ��� ���� ���� �ð� ���� ���� �ȵ�����, ���� ����.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// ������ ������ �� ������ ������ �ð� ���̿� �ɸ� �ð���
	// ���� ���� �ּ� ������ �Ѿ�� �� Ȯ��.
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}

void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(const FHitResult& HitResult, float HitCheckTime)
{
	// ���� ����.
	AActor* HitActor = HitResult.GetActor();

	if (IsValid(HitActor))
	{
		// ���� ���� ������ ����� ���� ����.
		// ���� ��ġ ��.
		const FVector HitLocation = HitResult.Location;

		// ���� ĳ������ �ٿ�� �ڽ� ���� ��������.
		const FBox HitBox = HitActor->GetComponentsBoundingBox();

		// �߽� ��ġ.
		const FVector ActorBoxCenter = HitBox.GetCenter();

		// Ŭ���̾�Ʈ�� ���� ������ �������,
		// ���� ��ġ�� ���� ���� ������ �Ÿ��� ���� ���� �Ÿ����� ������ ��.
		if (FVector::DistSquared(HitLocation, ActorBoxCenter) 
			<= AttackCheckDistance * AttackCheckDistance)
		{
			// ����� ���� ��� ����.
			AttackHitConfirm(HitActor);
		}
		else
		{
			AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("HitResult Reject!"));
		}

		// Debug Draw�� ���� �����ֱ�.

#if ENABLE_DRAW_DEBUG

		// ���� ������ ��ġ�� �ϴû����� ǥ�� (������).
		DrawDebugPoint(
			GetWorld(),
			ActorBoxCenter,
			30.0f,
			FColor::Cyan,
			false,
			5.0f
		);

		// ���� ������ ��ġ�� ��ȫ������ ǥ�� (������).
		DrawDebugPoint(
			GetWorld(),
			HitLocation,
			30.0f,
			FColor::Magenta,
			false,
			5.0f
		);

#endif

		// ����� ��ο�� ���� ���� �����ֱ�.
		DrawDebugAttackRange(
			FColor::Green, 
			HitResult.TraceStart, 
			HitResult.TraceEnd, 
			HitActor->GetActorForwardVector()
		);
	}
}

bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(
	FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime)
{
	// Ŭ���̾�Ʈ�κ��� ���� ���� �ð� ���� ���� �ȵ�����, ���� ����.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// ������ ������ �� ������ ������ �ð� ���̿� �ɸ� �ð���
	// ���� ���� �ּ� ������ �Ѿ�� �� Ȯ��.
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}

void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(
	FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime)
{
	DrawDebugAttackRange(FColor::Red, TraceStart, TraceEnd, TraceDir);
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	// ���� Ÿ�ֿ̹� ���� ���� �߰�.
	// ���� ���� Ÿ�̹� ���� ������ ���� ���ϰ� ���.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// ������ ��ϵ� ���� �ð��� �̹��� ��û�� ���� �ð����� ���̰�
	// ���� �ִϸ��̼� ���̺��� ū�� Ȯ��.
	// �� ���� ���� �ִϸ��̼� ���̺��� �۴ٸ�, Ŭ���̾�Ʈ�� �ǽ��غ� �� �ִ� ��Ȳ.
	return (AttackStartTime - LastAttackStartTime) > AttackTime;
}

void AABCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// �������� ó�� �� ��.
	// ���� ���̶�� �ǹ̷� �÷��� ����.
	bCanAttack = false;

	// ���� �߿��� �̵� ����.
	OnRep_CanAttack();

	// ����-Ŭ���̾�Ʈ�� �ð� ���� ���.
	AttackTimeDifference = GetWorld()->GetTimeSeconds() - AttackStartTime;

	// �ð� ���� ���� �α׷� Ȯ��.
	AB_LOG(LogABNetwork, Log, TEXT("LagTime: %f"), AttackTimeDifference);

	// ����ó��.
	// ��Ʈ��ũ ���°� �ʹ� ������ ���, AttackTimeDifference ���� �ʹ� ũ�ԵǸ�,
	// �Ʒ��� Ÿ�̸Ӱ� �������� ���� �� �ֱ� ������ �ּ����� �ð� ���� ����.
	AttackTimeDifference = FMath::Clamp(
		AttackTimeDifference,
		0.0f,
		AttackTime - 0.01f
	);

	// Ÿ�̸� ����.
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

	// Ŭ���̾�Ʈ�� ���� ��û�� �� �ð� �� ����.
	LastAttackStartTime = AttackStartTime;

	// ������ �ִϸ��̼� ���.
	PlayAttackAnimation();

	// �α� ���.
	//AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// Ŭ���̾�Ʈ�� ���� �Է��� �ؼ� ������ ȣ���� ��û�� �� �����.
	// ���� ������ ��쿡�� ���ÿ��� ����Ǳ� ������ ��ٷ� �����.
	// �� ���� �ٸ� Ŭ���̾�Ʈ�� ��Ʈ��ũ�κ��� ��ȣ�� �޾� �����.
	//MulticastRPCAttack();

	// ������ Multicast > Client RPC�� ����.
	for (auto* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		//UE_LOG(LogABNetwork, Log, TEXT("GetController: %s | PlayerController: %s | IsLocalController: %s"),
		//	*GetController()->GetName(),
		//	*PlayerController->GetName(),
		//	(PlayerController->IsLocalPlayerController() ? )
		//);

		// ������ �ִ� Ŭ���̾�Ʈ�� �ش��ϴ� �÷��̾� ��Ʈ�ѷ� �Ÿ���.
		if (PlayerController && GetController() != PlayerController)
		{
			// ���������� ��Ʈ�ѷ� �Ÿ���.
			if (!PlayerController->IsLocalController())
			{
				// ����� �Ѿ�� �÷��̾� ��Ʈ�ѷ���
				// ������ �ƴϰ�, ���� Ŭ���̾�Ʈ�� �ƴ�.
				AABCharacterPlayer* OtherPlayer
					= Cast<AABCharacterPlayer>(PlayerController->GetPawn());
				if (OtherPlayer)
				{
					// Client RPC ����.
					//ClientRPCPlayAnimation(OtherPlayer);
					OtherPlayer->ClientRPCPlayAnimation(this);
				}
			}
		}
	}

}

void AABCharacterPlayer::MulticastRPCAttack_Implementation()
{
	// ������ ����(�Է��� ������) Ŭ���̾�Ʈ�� �̹�
	// ���� ���� ���� �����Ʈ ��� ���� ������.
	// ���⿡���� ������ Ŭ���̾�Ʈ�� ���� ó���� ����.
	// ���� ���� ���� �����Ʈ ��� ���� ������Ƽ ���ø����̼����� ���޵Ǿ� ó���� �� ��.
	// ������, ���� �ִϸ��̼��� ������� �ʾ����� ����ؾ� ��.
	// ������ Ŭ���̾�Ʈ���� �ľ�.
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
		InHUDWidget->UpdateHpBar(Stat->GetCurrentHp(), Stat->GetMaxHp());

		Stat->OnStatChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateStat);
		Stat->OnHpChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateHpBar);
	}
}