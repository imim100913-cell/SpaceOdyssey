// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceFlightHUD.h"
#include "SpaceshipPawn.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"

ASpaceFlightHUD::ASpaceFlightHUD()
{
	static ConstructorHelpers::FObjectFinder<UFont> FontObj(TEXT("/Engine/EngineFonts/Roboto"));
	if (FontObj.Succeeded())
	{
		MainHUDFont = FontObj.Object;
	}
}

void ASpaceFlightHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	ASpaceshipPawn* Spaceship = Cast<ASpaceshipPawn>(GetOwningPawn());
	if (!Spaceship) return;

	float CanvasW = Canvas->SizeX;
	float CanvasH = Canvas->SizeY;

	FLinearColor Cyan = FLinearColor(0.2f, 0.85f, 1.0f, 0.9f);
	FLinearColor Gold = FLinearColor(1.0f, 0.75f, 0.2f, 0.95f);
	FLinearColor Green = FLinearColor(0.3f, 1.0f, 0.5f, 0.9f);
	FLinearColor White = FLinearColor(1.0f, 1.0f, 1.0f, 0.85f);
	FLinearColor BgPanelColor = FLinearColor(0.02f, 0.05f, 0.1f, 0.65f);

	// ==========================================
	// 1. 画面上部：現在の展望モード＆ステータスバー
	// ==========================================
	FString ViewName = TEXT("操縦席 (FRONT COCKPIT)");
	switch (Spaceship->CurrentCameraView)
	{
	case EFlightCameraView::ObservationCabin: ViewName = TEXT("側面展望窓 (SIDE CABIN VIEW)"); break;
	case EFlightCameraView::CupolaDome: ViewName = TEXT("頭上ドーム窓 (TOP CUPOLA DOME)"); break;
	case EFlightCameraView::ExternalCinematic: ViewName = TEXT("機外シネマティック (EXTERNAL VIEW)"); break;
	default: break;
	}

	FString CruiseText = Spaceship->bIsCruiseAutopilot ? TEXT("☕ 自動遊覧モード中（絶景をお楽しみください）") : TEXT("🕹️ マニュアル操縦");
	FString TopBarText = FString::Printf(TEXT("視点: [ %s ]   |   モード: [ %s ]"), *ViewName, *CruiseText);

	// 背景ボックス
	DrawRect(BgPanelColor, 20.f, 15.f, CanvasW - 40.f, 35.f);
	DrawText(TopBarText, Spaceship->bIsCruiseAutopilot ? Gold : Cyan, 35.f, 22.f, MainHUDFont, 1.05f);

	// ==========================================
	// 2. 画面右上：いつでも見れる【操作ヒント パネル】
	// ==========================================
	if (bShowHelpOverlay)
	{
		float PanelW = 340.f;
		float PanelH = 260.f;
		float PanelX = CanvasW - PanelW - 25.f;
		float PanelY = 65.f;

		// スタイリッシュな半透明背景
		DrawRect(BgPanelColor, PanelX, PanelY, PanelW, PanelH);
		DrawRect(Cyan, PanelX, PanelY, PanelW, 2.f); // 上部アクセントライン

		// タイトル
		DrawText(TEXT("🎮 操作ガイド (CONTROLS)"), Gold, PanelX + 15.f, PanelY + 12.f, MainHUDFont, 1.0f);
		DrawText(TEXT("[H] キーでヒント表示/非表示"), White, PanelX + PanelW - 170.f, PanelY + 14.f, MainHUDFont, 0.75f);

		// キーガイド一覧
		float ItemY = PanelY + 45.f;
		float LineSpacing = 28.f;

		DrawKeyBadge(PanelX + 15.f, ItemY, TEXT("W / S"), TEXT("前進 / 減速 (スロットル)"), Cyan); ItemY += LineSpacing;
		DrawKeyBadge(PanelX + 15.f, ItemY, TEXT("A / D"), TEXT("旋回 / ロール"), Cyan); ItemY += LineSpacing;
		DrawKeyBadge(PanelX + 15.f, ItemY, TEXT("Shift"), TEXT("🚀 ブースト (大気圏突破)"), Gold); ItemY += LineSpacing;
		DrawKeyBadge(PanelX + 15.f, ItemY, TEXT("V"), TEXT("🪟 窓・視点の切り替え"), Green); ItemY += LineSpacing;
		DrawKeyBadge(PanelX + 15.f, ItemY, TEXT("C"), TEXT("☕ オートクルーズ (鑑賞)"), Gold); ItemY += LineSpacing;
		DrawKeyBadge(PanelX + 15.f, ItemY, TEXT("中クリック"), TEXT("👁️ 首振り (窓の外を見渡す)"), White); ItemY += LineSpacing;
		DrawKeyBadge(PanelX + 15.f, ItemY, TEXT("J"), TEXT("🌙 月軌道へワープ"), Gold); ItemY += LineSpacing;
	}

	// ==========================================
	// 3. 画面左下：フライト・大気テレメトリ
	// ==========================================
	float AltitudeKm = Spaceship->CurrentAltitude / 1000.0f;
	float SpeedKmh = (Spaceship->CurrentSpeed / 100.0f) * 3.6f;

	float TelPanelW = 320.f;
	float TelPanelH = 125.f;
	float TelX = 25.f;
	float TelY = CanvasH - TelPanelH - 25.f;

	DrawRect(BgPanelColor, TelX, TelY, TelPanelW, TelPanelH);
	DrawRect(Cyan, TelX, TelY, TelPanelW, 2.f);

	FString AtmStatus = TEXT("🌍 地上・対流圏 (青空と雲)");
	if (AltitudeKm > 80.0f)
	{
		AtmStatus = TEXT("🌌 宇宙空間 (漆黒の真空)");
	}
	else if (AltitudeKm > 50.0f)
	{
		AtmStatus = TEXT("✨ 中間圏 (青から漆黒へ)");
	}
	else if (AltitudeKm > 10.0f)
	{
		AtmStatus = TEXT("❄️ 成層圏 (窓に霜・氷結)");
	}

	DrawText(TEXT("📊 計器テレメトリ (TELEMETRY)"), Gold, TelX + 15.f, TelY + 10.f, MainHUDFont, 0.95f);
	DrawText(FString::Printf(TEXT("高度: %8.1f km"), AltitudeKm), Cyan, TelX + 15.f, TelY + 35.f, MainHUDFont, 1.05f);
	DrawText(FString::Printf(TEXT("速度: %8.0f km/h"), SpeedKmh), Cyan, TelX + 15.f, TelY + 60.f, MainHUDFont, 1.05f);
	DrawText(FString::Printf(TEXT("環境: %s"), *AtmStatus), White, TelX + 15.f, TelY + 88.f, MainHUDFont, 0.95f);
}

void ASpaceFlightHUD::DrawKeyBadge(float X, float Y, const FString& KeyName, const FString& ActionDesc, const FLinearColor& BadgeColor)
{
	// キーバッジ背景
	DrawRect(FLinearColor(0.1f, 0.2f, 0.35f, 0.8f), X, Y, 65.f, 22.f);
	DrawText(KeyName, BadgeColor, X + 8.f, Y + 3.f, MainHUDFont, 0.85f);
	DrawText(ActionDesc, FLinearColor(1.0f, 1.0f, 1.0f, 0.9f), X + 75.f, Y + 3.f, MainHUDFont, 0.85f);
}
