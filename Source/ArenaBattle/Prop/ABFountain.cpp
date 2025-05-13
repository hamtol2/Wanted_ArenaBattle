// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

// Sets default values
AABFountain::AABFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	// ���ø����̼� Ȱ��ȭ�� ���� �ɼ� ����.
	bReplicates = true;

	// ��Ʈ��ũ ���� �� ���� (1�ʿ� 1������ ���̱�).
	NetUpdateFrequency = 1.0f;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
	
	// ���� ����.
	//if (HasAuthority())
	//{
	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]() 
	//		{
	//			ServerRotationYaw += 1.0f;
	//		}
	//	), 1.0f, true);
	//}
}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ������ �Ӽ� ���� �߰�.
	DOREPLIFETIME(AABFountain, ServerRotationYaw);
}

void AABFountain::OnActorChannelOpen(
	FInBunch& InBunch, 
	UNetConnection* Connection)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// �������� Ȯ��.
	if (HasAuthority())
	{
		// ������ ���Ͱ� ȸ��.
		AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));

		// ����� ȸ�� ���� ������Ƽ�� ����.
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	// Ŭ���̾�Ʈ.
	else
	{
		// �����κ��� �����͸� ���� �Ŀ� ����� �ð� ������Ʈ.
		ClientTimeSinceUpdate += DeltaTime;

		// KINDA_SMALL_NUMBER�� 0�� ������ ��.
		// ���� ó���� �ϴµ� ClientTimeBetweenLastUpdate ���� �ſ� ������,
		// ������ �ǹ̰� ����.
		if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER)
		{
			return;
		}

		// ���� ��Ʈ��ũ ��Ŷ �� ������ ȸ�� ���� ����.
		const float EstimateRotationYaw
			= ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;

		// ȸ�� �� ���ϱ�.
		FRotator ClientRotator = RootComponent->GetComponentRotation();

		// ���� (Alpha, t ��).
		const float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

		// ���� �� ���ϱ�.
		const float ClientNewYaw = FMath::Lerp(
			ServerRotationYaw,
			EstimateRotationYaw,
			LerpRatio
		);

		ClientRotator.Yaw = ClientNewYaw;

		// ������Ʈ ȸ���� ����.
		RootComponent->SetWorldRotation(ClientRotator);
	}
}

void AABFountain::OnRep_ServerRotationYaw()
{
	AB_LOG(LogABNetwork, Log, TEXT("Yaw: %f"), ServerRotationYaw);

	// ����� ȸ�� ���� �ݿ��� �� ȸ�� �� ����.
	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;

	// ��Ʈ ������Ʈ ȸ���� ����.
	RootComponent->SetWorldRotation(NewRotator);

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;

	// �����κ��� �����͸� �޾ұ� ������ 0���� �ʱ�ȭ.
	ClientTimeSinceUpdate = 0.0f;
}