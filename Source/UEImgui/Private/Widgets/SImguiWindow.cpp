﻿#include "Widgets/SImguiWindow.h"
#include "imgui_internal.h"
#include "ImguiWrap/ImguiHelp.h"
#include "ImguiWrap/ImguiUEWrap.h"

void SImguiWindow::Construct(const FArguments& InArgs)
{
	BoundContext = InArgs._Context;
	BoundAdapter = InArgs._Adapter;
	BoundViewport = InArgs._Viewport;
	
	Super::Construct( Super::FArguments()
        .LayoutBorder(FMargin(0))
        .HasCloseButton(false)
		.Type(InArgs._IsToolTip ? EWindowType::ToolTip : EWindowType::Normal)
		.IsTopmostWindow(InArgs._IsToolTip)
		.IsPopupWindow(InArgs._IsToolTip || InArgs._IsPopup)
        .SizingRule(ESizingRule::Autosized)
        .SupportsTransparency(FWindowTransparency(EWindowTransparency::PerWindow))
        .HasCloseButton(false)
        .CreateTitleBar(false)
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .FocusWhenFirstShown(InArgs._TakeFocusWhenShow));
}

FReply SImguiWindow::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	return Adapter ? Adapter->OnKeyChar(InCharacterEvent) : FReply::Unhandled();
}

FReply SImguiWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	Super::OnKeyDown(MyGeometry, InKeyEvent);
	if (!Adapter) return FReply::Unhandled();
	return Adapter->OnKeyDown(InKeyEvent);
}

FReply SImguiWindow::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	Super::OnKeyUp(MyGeometry, InKeyEvent);
	if (!Adapter) return FReply::Unhandled();
	return Adapter->OnKeyUp(InKeyEvent);
}

FReply SImguiWindow::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	if (!Adapter) return FReply::Unhandled();
	return Adapter->OnMouseButtonDown(MouseEvent);
}

FReply SImguiWindow::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	if (!Adapter) return FReply::Unhandled();
	return Adapter->OnMouseButtonUp(MouseEvent);
}

FReply SImguiWindow::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	if (!Adapter) return FReply::Unhandled();
	return Adapter->OnMouseButtonDoubleClick(InMouseEvent);
}

FReply SImguiWindow::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	if (!Adapter) return FReply::Unhandled();
	return Adapter->OnMouseWheel(MouseEvent);
}

FReply SImguiWindow::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UImguiInputAdapter* Adapter = GetAdapter();
	if (!Adapter) return FReply::Unhandled();
	return Adapter->OnMouseMove(FVector2D::ZeroVector, MouseEvent);
}

bool SImguiWindow::SupportsKeyboardFocus() const
{
	return true;
}

void SImguiWindow::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	bInFocus = false;
}

FReply SImguiWindow::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	bInFocus = true;
}

FCursorReply SImguiWindow::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	UImguiInputAdapter* Adapter = GetAdapter();
	return Adapter ? Adapter->OnCursorQuery(CursorEvent) : FCursorReply::Cursor(EMouseCursor::Default);
}

FVector2D SImguiWindow::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	if (!BoundViewport) return FVector2D::ZeroVector;
	return FVector2D(BoundViewport->Size.x, BoundViewport->Size.y);
}

int32 SImguiWindow::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	if (!BoundViewport) return LayerId;
	
	return UEImguiDraw::MakeImgui(
		OutDrawElements,
		LayerId,
		FSlateRenderTransform(),
		MyCullingRect,
		BoundViewport->DrawData);
}

void SImguiWindow::Show(TSharedPtr<SWindow> InParent)
{
	if (GetNativeWindow().IsValid())
	{
		ShowWindow();
		return;
	}
	if (InParent.IsValid())
	{
		InParent->AddChildWindow(StaticCastSharedRef<SWindow>(this->AsShared()));
	}
	FSlateApplication::Get().AddWindow(StaticCastSharedRef<SWindow>(this->AsShared()));
}

void SImguiWindow::Update()
{
}

FVector2D SImguiWindow::GetPos()
{
	return ScreenPosition;
}

void SImguiWindow::SetPos(FVector2D InPos)
{
	MoveWindowTo(InPos);
}

FVector2D SImguiWindow::GetSize()
{
	return SWindow::Size;
}

void SImguiWindow::SetSize(FVector2D InSize)
{
	Morpher.Sequence.JumpToEnd();

	InSize.X = FMath::Max(SizeLimits.GetMinWidth().Get(InSize.X), InSize.X);
	InSize.X = FMath::Min(SizeLimits.GetMaxWidth().Get(InSize.X), InSize.X);

	InSize.Y = FMath::Max(SizeLimits.GetMinHeight().Get(InSize.Y), InSize.Y);
	InSize.Y = FMath::Min(SizeLimits.GetMaxHeight().Get(InSize.Y), InSize.Y);

	// ReshapeWindow W/H takes an int, so lets move our new W/H to int before checking if they are the same size
	FIntPoint CurrentIntSize = FIntPoint(FMath::CeilToInt(Size.X), FMath::CeilToInt(Size.Y));
	FIntPoint NewIntSize     = FIntPoint(FMath::CeilToInt(InSize.X), FMath::CeilToInt(InSize.Y));

	if (CurrentIntSize != NewIntSize)
	{
		if (NativeWindow.IsValid())
		{
			NativeWindow->ReshapeWindow(FMath::TruncToInt(ScreenPosition.X), FMath::TruncToInt(ScreenPosition.Y), NewIntSize.X, NewIntSize.Y);
		}
		else
		{
			InitialDesiredSize = InSize;
		}
	}
	SetCachedSize(InSize);
}

bool SImguiWindow::GetFocus()
{
	return bInFocus;
}

void SImguiWindow::SetFocus()
{
	FSlateApplication::Get().SetUserFocus(0, AsShared());
}

bool SImguiWindow::GetMinimized()
{
	return false;
}

void SImguiWindow::SetTitle(const char* InTitle)
{
	Title = FText::FromString(InTitle);
}

void SImguiWindow::SetAlpha(float InAlpha)
{
	SetOpacity(InAlpha);
}

void SImguiWindow::SetupViewport(ImGuiViewport* InViewport)
{
	BoundViewport = InViewport;
}

void SImguiWindow::SetupInputAdapter(UImguiInputAdapter* ImguiInputAdapter)
{
	SetAdapter(ImguiInputAdapter);
}
