#include "Widgets/SImguiWidget.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Logging.h"
#include "ImguiWrap/ImguiInputAdapter.h"
#include "ImguiWrap/ImguiContext.h"
#include "ImguiWrap/ImguiHelp.h"
#include "ImguiWrap/ImguiResourceManager.h"
#include "ImguiWrap/ImguiUEWrap.h"
#include "Widgets/Input/SEditableText.h"

void SImguiWidgetBase::Construct(const FArguments& InArgs)
{
	Context = InArgs._InContext;
	Adapter = InArgs._InAdapter;
	bBlockInput = InArgs._BlockInput;

	// validate 
	if (!Context)
	{
		
	}
	if (!Adapter)
	{
		Adapter = NewObject<UImguiInputAdapter>();
	}
	Adapter->SetContext(Context);
}

SImguiWidgetBase::~SImguiWidgetBase()
{
	(Context);
}

void SImguiWidgetBase::SetContext(UImguiContext* InContext)
{
	check(InContext != nullptr);
	Context = InContext;
	if (Adapter) Adapter->SetContext(Context);
}

void SImguiWidgetBase::SetAdapter(UImguiInputAdapter* InAdapter)
{
	check(InAdapter != nullptr);
	Adapter = InAdapter;
	if (Context) Adapter->SetContext(Context);	
}

void SImguiWidgetBase::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Context);
	Collector.AddReferencedObject(Adapter);
}

FString SImguiWidgetBase::GetReferencerName() const
{
	return TEXT("ImguiWidget");
}

FReply SImguiWidgetBase::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	Super::OnKeyChar(MyGeometry, InCharacterEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnKeyChar(InCharacterEvent);
}

FReply SImguiWidgetBase::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	Super::OnKeyDown(MyGeometry, InKeyEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnKeyDown(InKeyEvent);
}

FReply SImguiWidgetBase::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	Super::OnKeyUp(MyGeometry, InKeyEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnKeyUp(InKeyEvent);
}

FReply SImguiWidgetBase::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseButtonDown(MyGeometry, MouseEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnMouseButtonDown(MouseEvent);
}

FReply SImguiWidgetBase::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseButtonDown(MyGeometry, MouseEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnMouseButtonUp(MouseEvent);}

FReply SImguiWidgetBase::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	Super::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnMouseButtonDoubleClick(InMouseEvent);
}

FReply SImguiWidgetBase::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseWheel(MyGeometry, MouseEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnMouseWheel(MouseEvent);
}

FReply SImguiWidgetBase::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseMove(MyGeometry, MouseEvent);
	if (!Adapter) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : Adapter->OnMouseMove(this, MyGeometry, MouseEvent);
}

bool SImguiWidgetBase::SupportsKeyboardFocus() const
{
	return Adapter && Adapter->CanReceiveKeyboardInput();
}

FCursorReply SImguiWidgetBase::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	Super::OnCursorQuery(MyGeometry, CursorEvent);
	return Adapter->OnCursorQuery(CursorEvent);
}

void SImguiWidgetRenderProxy::Construct(const FArguments& InArgs)
{
	HSizingRule = InArgs._HSizingRule;
	VSizingRule = InArgs._VSizingRule;
	bAutoSetWidgetPos = InArgs._AutoSetWidgetPos;
	TopWndID = InArgs._ProxyWndName.IsEmpty() ? 0 : ImHashStr(TCHAR_TO_UTF8(*InArgs._ProxyWndName));
	
	// construct parent 
	Super::Construct(
        Super::FArguments()
        .InContext(InArgs._InContext)
        .InAdapter(InArgs._InAdapter)
        .BlockInput(InArgs._BlockInput));
}

FReply SImguiWidgetRenderProxy::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseMove(MyGeometry, MouseEvent);
	if (!GetAdapter()) return FReply::Unhandled();
	return !bBlockInput ? FReply::Unhandled() : GetAdapter()->OnMouseMove(FVector2D::ZeroVector, MouseEvent);
}

int32 SImguiWidgetRenderProxy::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	UImguiContext* UECtx = GetContext();
	if (!UECtx) return LayerId;
	ImGuiContext* Ctx = UECtx->GetContext();
	ImGuiWindow* Wnd = static_cast<ImGuiWindow*>(Ctx->WindowsById.GetVoidPtr(TopWndID));
	if (!Wnd) return LayerId;
	
	FSlateRenderTransform AccumulatedTran = GetCachedGeometry().GetAccumulatedRenderTransform();
	
	if (bAutoSetWidgetPos)
	{
		Wnd->Pos = { AccumulatedTran.GetTranslation().X, AccumulatedTran.GetTranslation().Y };
	}
	else
	{
		AccumulatedTran.SetTranslation(FVector2D(Wnd->Pos.x, Wnd->Pos.y));
	}

	switch (HSizingRule)
	{
	case EImguiSizingRule::UESize:
		Wnd->Size.x = AllottedGeometry.GetAbsoluteSize().X;			
		break;
	}

	switch (VSizingRule)
	{
	case EImguiSizingRule::UESize:
		Wnd->Size.y = AllottedGeometry.GetAbsoluteSize().Y;
		break;
	}
	
	static TArray<ImDrawList*> AllDrawList;
	AllDrawList.Reset();
	for (ImGuiID ID : WndID)
	{	
		AllDrawList.Add(static_cast<ImGuiWindow*>(Ctx->WindowsById.GetVoidPtr(ID))->DrawList);
	}

	// cache window
	CachedWnd = StaticCastSharedRef<SWindow>(OutDrawElements.GetPaintWindow()->AsShared());
	
	return UEImguiDraw::MakeImgui(
        OutDrawElements,
        LayerId,
        AccumulatedTran.Inverse().Concatenate(AllottedGeometry.GetAccumulatedRenderTransform()),
        MyCullingRect,
        AllDrawList);
}

FVector2D SImguiWidgetRenderProxy::ComputeDesiredSize(float) const
{
	UImguiContext* UECtx = GetContext();
	if (!UECtx) return FVector2D::ZeroVector;
	ImGuiContext* Ctx = UECtx->GetContext();

	FVector2D OriginPoint = GetCachedGeometry().GetAccumulatedRenderTransform().GetTranslation();
	FVector2D NewDesiredSize(0);

	for (ImGuiID ID : WndID)
	{
		ImGuiWindow* Wnd = (ImGuiWindow*)Ctx->WindowsById.GetVoidPtr(ID);
		// HSizing 
		switch (HSizingRule)
		{
		case EImguiSizingRule::ImSize:
			NewDesiredSize.X = FMath::Max(NewDesiredSize.X, Wnd->Size.x);
			break;
		case EImguiSizingRule::ImContentSize:
			NewDesiredSize.X = FMath::Max(NewDesiredSize.X, Wnd->ContentSize.x + (Wnd->WindowBorderSize + Wnd->WindowPadding.x) * 2);
			break;
		default: ;
		}

		// VSizing
		switch (VSizingRule)
		{
		case EImguiSizingRule::ImSize:
			NewDesiredSize.Y = FMath::Max(NewDesiredSize.Y, Wnd->Size.y);
			break;
		case EImguiSizingRule::ImContentSize:
			NewDesiredSize.Y = FMath::Max(NewDesiredSize.Y, Wnd->ContentSize.y + (Wnd->WindowBorderSize + Wnd->WindowPadding.y) * 2);
			break;
		default: ;
		}
	}

	return NewDesiredSize;
}

void SImguiWidgetRenderProxy::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	if (!GetContext()) return;
	// change context 
	ImGuiContext* LastCtx = ImGui::GetCurrentContext();
	ImGuiContext* Ctx = GetContext()->GetContext();
	GetContext()->ApplyContext();
	ImGuiWindow* Wnd = (ImGuiWindow*)Ctx->WindowsById.GetVoidPtr(TopWndID);
	
	// remove focus
	if (Ctx->ActiveIdWindow && Ctx->ActiveIdWindow->RootWindow != Wnd)
		ImGui::FocusWindow(nullptr);
	
	// resume context 
	ImGui::SetCurrentContext(LastCtx);
}

void SGlobalImguiWidget::Construct(const FArguments& InArgs)
{
	DrawCallBack = InArgs._OnDraw;
	Super::Construct(Super::FArguments()
		.InContext(InArgs._InContext)
		.InAdapter(InArgs._InAdapter)
		.HSizingRule(InArgs._HSizingRule)
		.VSizingRule(InArgs._VSizingRule)
		.AutoSetWidgetPos(InArgs._AutoSetWidgetPos)
		.ProxyWndName(InArgs._WndName));
	
	WindowId = UImguiGlobalContextService::Get().GetGlobalContext()->AddGlobalWindow(FDrawGlobalImgui::CreateLambda([this]
	{
		if (DrawCallBack.IsBound())
		{
			// set size 
			ImVec2 SizeConstraint(-1, -1);
			FVector2D UESize = GetCachedGeometry().GetAbsoluteSize();
			switch (HSizingRule)
			{
			case EImguiSizingRule::UESize:
				SizeConstraint.x = UESize.X;
				break;
			}
			switch (VSizingRule)
            {
            case EImguiSizingRule::UESize:
            	SizeConstraint.y = UESize.Y;
            	break;
            }
			ImGui::SetNextWindowSizeConstraints(SizeConstraint, SizeConstraint);
		}
		DrawCallBack.ExecuteIfBound();
		return true;
	}));

	UImguiGlobalContextService::Get().GetGlobalContext()->AddRenderProxy(StaticCastSharedRef<SImguiWidgetRenderProxy>(this->AsShared()));
}

SGlobalImguiWidget::~SGlobalImguiWidget()
{
	UImguiGlobalContextService::Get().GetGlobalContext()->RemoveGlobalWindow(WindowId);
}
