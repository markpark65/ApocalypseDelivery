#include "Meteor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMeteor::AMeteor()
{
	PrimaryActorTick.bCanEverTick = false;

    // 충돌체 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;
    CollisionComp->InitSphereRadius(50.f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile")); // 발사체용 충돌 프로필

    // 드론 운석 감지용 태그
    Tags.Add(FName("Meteor"));

    // 스태틱 메시 설정
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);

    // 발사체 움직임 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;

    // 초기 속도 설정
    ProjectileMovement->InitialSpeed = 1000.f;
    ProjectileMovement->MaxSpeed = 1000.f;

    // 발사 방향 설정
    ProjectileMovement->Velocity = FVector(0.f, 0.f, -1.f);

    // 발사체 회전 옵션
    ProjectileMovement->bRotationFollowsVelocity = true;

    // 중력 영향 직선
    ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AMeteor::BeginPlay()
{
	Super::BeginPlay();
    SetLifeSpan(10.f);
}

void AMeteor::SetMeteorSpeed(float NewSpeed)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = NewSpeed;
        ProjectileMovement->MaxSpeed = NewSpeed;
        ProjectileMovement->Velocity = FVector(0.f, 0.f, -1.f);
    }
}
void AMeteor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }
    Destroy();
}