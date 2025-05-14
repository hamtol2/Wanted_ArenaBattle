// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
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

	// �Ÿ� ������ ����ϴ� �� ���̱�.
	// 4000000 > �뷫 20����.
	NetCullDistanceSquared = 4000000.0f;

	// �޸� ���·� �����ϵ��� ������ �� ����.
	//NetDormancy = DORM_Initial;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();

	// ���� ����.
	if (HasAuthority())
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]()
			{
				//// 4000 ����Ʈ�� ������ ����.
				//BigData.Init(BigDataElement, 1000);

				//// ������ ������ ���� �� ����.
				//BigDataElement += 1.0f;

				// ���� ���� �������� ����.
				ServerLightColor = FLinearColor(
					FMath::RandRange(0.0f, 1.0f),
					FMath::RandRange(0.0f, 1.0f),
					FMath::RandRange(0.0f, 1.0f),
					1.0f
				);

				OnRep_ServerLightColor();

			}
		), 1.0f, true);

		// �޸� ���¸� ����� ���� ����� Ÿ�̸�.
		FTimerHandle Handle2;
		GetWorld()->GetTimerManager().SetTimer(
			Handle2,
			FTimerDelegate::CreateLambda([&]()
				{
					//FlushNetDormancy();
				}
			), 10.0f, false);
	}
}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ������ �Ӽ� ���� �߰�.
	DOREPLIFETIME(AABFountain, ServerRotationYaw);

	//DOREPLIFETIME(AABFountain, BigData);

	DOREPLIFETIME(AABFountain, ServerLightColor);

	//DOREPLIFETIME_CONDITION(AABFountain, ServerLightColor, COND_InitialOnly);
}

void AABFountain::OnActorChannelOpen(
	FInBunch& InBunch,
	UNetConnection* Connection)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

bool AABFountain::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	bool NetRelevantResult
		= Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	if (!NetRelevantResult)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Not Relevant: [%s] %s"),
			*RealViewer->GetName(),
			*SrcLocation.ToCompactString()
		);
	}

	return NetRelevantResult;
}

void AABFountain::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PreReplication(ChangedPropertyTracker);
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

void AABFountain::OnRep_ServerLightColor()
{
	if (!HasAuthority())
	{
		AB_LOG(LogABNetwork, Log, TEXT("LightColor: %s"),
			*ServerLightColor.ToString());
	}

	// ������Ʈ �˻�.
	UPointLightComponent* PointLight
		= Cast<UPointLightComponent>(GetComponentByClass(UPointLightComponent::StaticClass()));

	if (PointLight)
	{
		// �������� ������ ����Ʈ ���� ����.
		PointLight->SetLightColor(ServerLightColor);
	}
}