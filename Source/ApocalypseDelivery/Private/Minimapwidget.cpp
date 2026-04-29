#include "MinimapWidget.h"
#include "Components/CanvasPanelSlot.h"

// ────────────────────────────────────────────────────────────────────────────
// 초기화
// ────────────────────────────────────────────────────────────────────────────

void UMinimapWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 마커 기본 색상 설정 (BP에서 텍스처를 지정해도 되고, 이 색상만 써도 됩니다)
    if (DroneMarker)   DroneMarker->SetColorAndOpacity(FLinearColor(0.1f, 1.0f, 0.2f));   // 초록
    if (TargetMarker)  TargetMarker->SetColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.0f));  // 노랑

    // 목적지 정보가 들어오기 전까지 숨김
    if (TargetMarker)  TargetMarker->SetVisibility(ESlateVisibility::Hidden);
}

// ────────────────────────────────────────────────────────────────────────────
// 배경 설정 (두 방식 공통 내부 함수로 위임)
// ────────────────────────────────────────────────────────────────────────────

void UMinimapWidget::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
    if (InRenderTarget) ApplyBackgroundResource(InRenderTarget);
}

void UMinimapWidget::SetStaticBackground(UTexture2D* InTexture)
{
    if (InTexture) ApplyBackgroundResource(InTexture);
}

void UMinimapWidget::ApplyBackgroundResource(UObject* Resource)
{
    if (!MinimapBackground || !Resource) return;

    FSlateBrush Brush;
    Brush.SetResourceObject(Resource);
    Brush.ImageSize = FVector2D(MinimapSize, MinimapSize);
    MinimapBackground->SetBrush(Brush);
}

// ────────────────────────────────────────────────────────────────────────────
// 마커 갱신
// ────────────────────────────────────────────────────────────────────────────

void UMinimapWidget::UpdateMarkers(FVector DroneWorldPos, const TArray<FVector>& PlatformPositions)
{
    APawn* PlayerPawn = GetOwningPlayerPawn();
    if (!PlayerPawn || !MinimapBackground || !MarkerCanvas) return;

    float DroneYaw = PlayerPawn->GetActorRotation().Yaw;
    /*
    // ---------------------------------------------------------
    // [버전 1] 드론이 보는 방향에 따라 미니맵 회전
    // ---------------------------------------------------------
    // 배경과 마커 캔버스 전체를 드론 회전의 반대로 돌려 드론이 항상 위를 보게 함
    if (MinimapBackground) MinimapBackground->SetRenderTransformAngle(-DroneYaw);
    if (MarkerCanvas) MarkerCanvas->SetRenderTransformAngle(-DroneYaw);
    if (DroneMarker) DroneMarker->SetRenderTransformAngle(0.0f); // 드론 아이콘은 고정
    */
    
    // ---------------------------------------------------------
    // [버전 2] 미니맵은 항상 북쪽 고정
    // ---------------------------------------------------------
    if (MinimapBackground) MinimapBackground->SetRenderTransformAngle(0.0f);
    if (MarkerCanvas) MarkerCanvas->SetRenderTransformAngle(0.0f);
    if (DroneMarker) DroneMarker->SetRenderTransformAngle(DroneYaw); // 드론 아이콘만 회전
    

    // 1. 드론 마커 (항상 중앙)
    float HalfSize = MinimapSize * 0.5f;
    PlaceMarker(DroneMarker, FVector2D(HalfSize, HalfSize), true);

    // 2. 목적지 마커 (드론 기준 상대 위치)
    //드론 마커 위치 업데이트
    PlaceMarker(DroneMarker, WorldToMinimapPos(DroneWorldPos, DroneWorldPos), true);
    
    //전달된 플랫폼 개수보다 마커가 부족하다면 기존 마커를 복사해서 동적 생성
    while (ActivePlatformMarkers.Num() < PlatformPositions.Num())
    {
        UImage* NewMarker = NewObject<UImage>(MarkerCanvas);
        if (TargetMarker)
        {
            //원본 TargetMarker의 브러시와 색상 설정 복사
            NewMarker->SetBrush(TargetMarker->GetBrush());
            NewMarker->SetColorAndOpacity(TargetMarker->ColorAndOpacity);
        }

        //캔버스 패널에 새로 만든 마커 추가
        UCanvasPanelSlot* NewSlot = Cast<UCanvasPanelSlot>(MarkerCanvas->AddChildToCanvas(NewMarker));
        UCanvasPanelSlot* TargetSlot = Cast<UCanvasPanelSlot>(TargetMarker->Slot);

        //슬롯 크기 및 정렬 정보 복사
        if (NewSlot && TargetSlot)
        {
            NewSlot->SetSize(TargetSlot->GetSize());
            NewSlot->SetAlignment(TargetSlot->GetAlignment());
            NewSlot->SetAnchors(TargetSlot->GetAnchors());
        }

        ActivePlatformMarkers.Add(NewMarker);
    }

    //복제 틀로 사용된 원본 TargetMarker는 화면에서 숨김 처리
    if (TargetMarker)
    {
        TargetMarker->SetVisibility(ESlateVisibility::Hidden);
    }

    //배열을 돌며 위치 적용 및 가시성 갱신
    for (int32 i = 0; i < ActivePlatformMarkers.Num(); ++i)
    {
        if (i < PlatformPositions.Num())
        {
            //유효한 플랫폼 개수 안쪽이면 위치 갱신 후 표시
            PlaceMarker(ActivePlatformMarkers[i], WorldToMinimapPos(PlatformPositions[i], DroneWorldPos), true);
        }
        else
        {
            //남는 마커는 화면에서 숨김
            PlaceMarker(ActivePlatformMarkers[i], FVector2D::ZeroVector, false);
        }
    }
}

// ────────────────────────────────────────────────────────────────────────────
// 내부 유틸
// ────────────────────────────────────────────────────────────────────────────

FVector2D UMinimapWidget::WorldToMinimapPos(FVector WorldPos, FVector DronePos) const
{
    float HalfSize = MinimapSize * 0.5f;

    // 드론과의 상대적 거리(cm) 계산
    float RelativeX = WorldPos.X - DronePos.X;
    float RelativeY = WorldPos.Y - DronePos.Y;

    // 정규화 (MinimapRange 내에서 -1 ~ 1)
    float NormX = FMath::Clamp(RelativeX / MinimapRange, -1.0f, 1.0f);
    float NormY = FMath::Clamp(RelativeY / MinimapRange, -1.0f, 1.0f);

    // 위젯 좌표계 매핑 (언리얼 X -> UI -Y / 언리얼 Y -> UI +X)
    return FVector2D(
        HalfSize + (NormY * HalfSize),
        HalfSize - (NormX * HalfSize)
    );
}

void UMinimapWidget::PlaceMarker(UImage* Marker, FVector2D MinimapPos, bool bVisible)
{
    if (!Marker) return;

    if (!bVisible)
    {
        Marker->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    Marker->SetVisibility(ESlateVisibility::Visible);

    // 마커의 슬롯을 찾아 위치를 직접 강제 바인딩
    UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
    if (MarkerSlot)
    {
        // 계산된 픽셀 좌표를 슬롯 위치에 대입
        MarkerSlot->SetPosition(MinimapPos);
    }
}