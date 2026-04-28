#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "MinimapWidget.generated.h"

/**
 * 미니맵 위젯
 *
 * [배경 교체 방법]
 *  SceneCapture(실시간 촬영) → SetRenderTarget(RenderTarget2D) 호출
 *  정적 약도 텍스처          → SetStaticBackground(Texture2D) 호출
 *  두 방식 모두 MinimapBackground 이미지 하나만 교체하므로 마커 로직은 공통입니다.
 *
 * [BP 위젯 계층 구조] (BP_MinimapWidget)
 *  SizeBox (200 × 200)
 *  └─ Overlay
 *      ├─ Image       "MinimapBackground"  ← 배경
 *      └─ CanvasPanel "MarkerCanvas"       ← 마커 오버레이
 *          ├─ Image   "DroneMarker"        ← 드론 (초록 점)
 *          ├─ Image   "TargetMarker"       ← 목적지 (노랑 점)
 *          └─ Image   "PackageMarker"      ← 화물 (하늘색 점)
 */
UCLASS()
class APOCALYPSEDELIVERY_API UMinimapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ── 설정값 ──

    // 드론 중심으로부터 화면 끝까지의 거리 (cm)
    // 탑뷰 시: SceneCapture의 OrthoWidth / 2
    // 약도 시: 이미지 가로 폭의 절반
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MinimapRange = 5000.0f;

    // 위젯의 실제 크기 (SizeBox와 일치)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MinimapSize = 200.0f;

    /* 점 마커 한 변의 크기 (px). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MarkerSize = 12.0f;

    // ── 배경 설정 (둘 중 하나만 호출) ────────────────────────────────────────

    /* SceneCapture RenderTarget을 배경으로 설정합니다. */
    UFUNCTION(BlueprintCallable, Category = "Minimap|Background")
    void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);

    /* 정적 약도 텍스처를 배경으로 설정합니다. */
    UFUNCTION(BlueprintCallable, Category = "Minimap|Background")
    void SetStaticBackground(UTexture2D* InTexture);

    // ── 매 틱 마커 갱신 ──────────────────────────────────────────────────────

    /**
     * @param DroneWorldPos   드론 월드 위치 (미니맵 중심 고정)
     * @param bHasTarget      목적지 플랫폼 존재 여부
     * @param TargetWorldPos  목적지 플랫폼 월드 위치
     * @param bHasPackage     화물 존재 여부 (드론 미부착 상태일 때만 true)
     * @param PackageWorldPos 화물 월드 위치
     */
    void UpdateMarkers(
        FVector DroneWorldPos/*,
        bool    bHasTarget, FVector TargetWorldPos,
        bool    bHasPackage, FVector PackageWorldPos*/
    );

protected:
    virtual void NativeConstruct() override;

private:
    // ── BindWidget ───────────────────────────────────────────────────────────
    UPROPERTY(meta = (BindWidget))
    UImage* MinimapBackground = nullptr;

    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* MarkerCanvas = nullptr;

    UPROPERTY(meta = (BindWidget))
    UImage* DroneMarker = nullptr;

    //UPROPERTY(meta = (BindWidget))
    //UImage* TargetMarker = nullptr;

    //UPROPERTY(meta = (BindWidget))
    //UImage* PackageMarker = nullptr;

    // ── 내부 유틸 ────────────────────────────────────────────────────────────

    /** UObject(RenderTarget 또는 Texture2D)를 Brush에 적용합니다. */
    void ApplyBackgroundResource(UObject* Resource);

    /** 월드 위치 → 미니맵 픽셀 좌표 변환 (드론 기준). */
    FVector2D WorldToMinimapPos(FVector WorldPos, FVector DronePos) const;

    /** 마커를 지정 좌표로 이동하거나 숨깁니다. */
    void PlaceMarker(UImage* Marker, FVector2D WidgetPos, bool bVisible);
};