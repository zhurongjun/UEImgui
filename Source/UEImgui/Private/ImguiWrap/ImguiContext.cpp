﻿#include "ImguiWrap/ImguiContext.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImguiWrap/ImguiInputAdapter.h"
#include "ImguiWrap/ImguiUEWrap.h"
#include "Widgets/SImguiWindow.h"
#include "Window/ImguiWindowWrapper.h"

void UImguiContext::BeginDestroy()
{
	Super::BeginDestroy();
	
	ShutDown();
}

UImguiContext::UImguiContext(const FObjectInitializer& InInitializer)
{
}

void UImguiContext::Init(ImFontAtlas* InDefaultFontAtlas, bool bEnableDocking)
{
	// create context 
	Context = ImGui::CreateContext(InDefaultFontAtlas);

	// set up context 
	_SetupImguiContext(bEnableDocking);

	// set up key map 
	UImguiInputAdapter::CopyUnrealKeyMap(GetIO());
}

void UImguiContext::ShutDown()
{
	// destroy context 
	if (Context) ImGui::DestroyContext(Context);

	// clean reference
	Context = nullptr;
}

ImGuiIO* UImguiContext::GetIO() const
{
	return &Context->IO;
}

ImGuiStyle* UImguiContext::GetStyle() const
{
	return &Context->Style;
}

void UImguiContext::ApplyContext()
{
	ImGui::SetCurrentContext(Context);
}

void UImguiContext::DrawGlobal()
{
	for (int32 i = 0; i < AllDrawCallBack.Num(); ++i)
	{
		auto& CurCallBack = AllDrawCallBack[i];
		
		if (!CurCallBack.IsBound()) continue;
		bool bNeedClose = CurCallBack.Execute();
		if (bNeedClose)
		{
			CurCallBack.Unbind();
		}
	}
}

void UImguiContext::UpdateViewport()
{
	FImguiWindowWrapper::CurContext = this;
	ImGui::UpdatePlatformWindows();
}

void UImguiContext::_CreateWindow(ImGuiViewport* viewport, UImguiInputAdapter* InInputAdapter)
{
	// use proxy
	{
		auto Proxy = _DispatchProxy(viewport, InInputAdapter);
		if (Proxy.IsValid())
		{
			// setup platform data
			viewport->PlatformHandle = viewport->PlatformHandleRaw = Proxy.Pin().Get();
			viewport->PlatformUserData = InInputAdapter;
			
			// add to array 
			ImViewportToUE.Add(viewport, Proxy);
			return;
		}
	}

	// create a real window
	TSharedPtr<SImguiWindow> UEWnd = SNew(SImguiWindow)
	.Context(this)
	.Adapter(InInputAdapter)
	.Viewport(viewport)
	.IsToolTip(viewport->Flags & ImGuiViewportFlags_TopMost)
	.IsPopup(viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
	.TakeFocusWhenShow(!(viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing));

	// add to array
	AllDispatchedViewport.Add(UEWnd);
	ImViewportToUE.Add(viewport, UEWnd);

	// setup platform data
	viewport->PlatformHandle = viewport->PlatformHandleRaw = UEWnd.Get();
	viewport->PlatformUserData = InInputAdapter;
}

void UImguiContext::_DestroyWindow(ImGuiViewport* viewport)
{
	// find UE widget
	auto UEWidget = ImViewportToUE[viewport];

	// process dispatched window case 
	if (UEWidget.IsValid() && !UEWidget.Pin()->IsPersist())
	{
		AllDispatchedViewport.Remove(UEWidget.Pin());
	}
	
	// remove from map 
	ImViewportToUE.Remove(viewport);

	// disable platform data 
	viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

void UImguiContext::_ShowWindow(ImGuiViewport* viewport)
{
	// find UE widget
	auto UEWidget = _SafeFindViewport(viewport, false);

	// find parent 
	auto ParentViewPort = ImGui::FindViewportByID(viewport->ParentViewportId);
	auto ParentWidget = _SafeFindViewport(ParentViewPort);

	// show it!!!
	UEWidget->Show(ParentWidget->GetWindow());
}

void UImguiContext::_UpdateWindow(ImGuiViewport* viewport)
{
	_SafeFindViewport(viewport)->Update();
}

ImVec2 UImguiContext::_GetWindowPos(ImGuiViewport* viewport)
{
	return *(ImVec2*)&_SafeFindViewport(viewport)->GetPos();
}

void UImguiContext::_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
	_SafeFindViewport(viewport)->SetPos(*(FVector2D*)&pos);
}

ImVec2 UImguiContext::_GetWindowSize(ImGuiViewport* viewport)
{
	return *(ImVec2*)&_SafeFindViewport(viewport)->GetSize();
}

void UImguiContext::_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
	_SafeFindViewport(viewport)->SetSize(*(FVector2D*)&size);
}

void UImguiContext::_SetWindowFocus(ImGuiViewport* viewport)
{
	_SafeFindViewport(viewport)->SetFocus();
}

bool UImguiContext::_GetWindowFocus(ImGuiViewport* viewport)
{
	_SafeFindViewport(viewport)->GetFocus();
}

bool UImguiContext::_GetWindowMinimized(ImGuiViewport* viewport)
{
	_SafeFindViewport(viewport)->GetMinimized();
}

void UImguiContext::_SetWindowTitle(ImGuiViewport* viewport, const char* title)
{
	_SafeFindViewport(viewport)->SetTitle(title);
}

void UImguiContext::_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
{
	_SafeFindViewport(viewport)->SetAlpha(alpha);
}

float UImguiContext::_GetWindowDpiScale(ImGuiViewport* viewport)
{
	_SafeFindViewport(viewport)->GetDpiScale();
}

void UImguiContext::_OnChangedViewport(ImGuiViewport* viewport)
{
	_SafeFindViewport(viewport)->OnChangeViewport();
}

void UImguiContext::_SetImeInputPos(ImGuiViewport* viewport, ImVec2 pos)
{
	
}

void UImguiContext::_SetupImguiContext(bool bEnableDocking)
{
	// enable keyboard control
	GetIO()->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// enable docking
	if (bEnableDocking)
	{
		GetIO()->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		GetIO()->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		GetIO()->ConfigViewportsNoAutoMerge = true;
	}
	
	// set backend flags
	GetIO()->BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	GetIO()->BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	GetIO()->BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	GetIO()->BackendPlatformName = "Unreal Backend";
	GetIO()->BackendPlatformName = "Unreal Widget";

	// set up style
	ImGui::StyleColorConfig(&GetContext()->Style);

	// disable .ini by default
	GetIO()->IniFilename = nullptr;

	// setup platform interface
	FImguiWindowWrapper::SetupPlatformInterface(&Context->PlatformIO);

	// setup monitor
	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
	for (FMonitorInfo& Info : DisplayMetrics.MonitorInfo)
	{
		ImGuiPlatformMonitor Monitor;
		Monitor.DpiScale = 1.f;
		Monitor.MainPos.x = Info.DisplayRect.Left;
		Monitor.MainPos.y = Info.DisplayRect.Top;
		Monitor.MainSize.x = Info.DisplayRect.Right - Info.DisplayRect.Left;
		Monitor.MainSize.y = Info.DisplayRect.Bottom - Info.DisplayRect.Top;
		Monitor.WorkPos.x = Info.WorkArea.Left;
		Monitor.WorkPos.y = Info.WorkArea.Top;
		Monitor.MainSize.x = Info.WorkArea.Right - Info.WorkArea.Left;
		Monitor.MainSize.y = Info.WorkArea.Bottom - Info.WorkArea.Top;
		Context->PlatformIO.Monitors.push_back(Monitor);
	}
}

ImFontAtlas& GetDefaultFontAtlas()
{
	static ImFontAtlas FontAtlas;
	if (!FontAtlas.IsBuilt())
	{
		ImFontConfig FontConfig = {};
		FontConfig.SizePixels = FMath::RoundFromZero(13.f);
		FontAtlas.AddFontDefault(&FontConfig);

		unsigned char* Pixels;
		int Width, Height, Bpp;
		FontAtlas.GetTexDataAsRGBA32(&Pixels, &Width, &Height, &Bpp);
	}
	return FontAtlas;
}

void UImguiContext::_SetUpDefaultFont()
{
	if (!GetIO()->Fonts->IsBuilt())
	{
		ImFontConfig FontConfig = {};
		FontConfig.SizePixels = FMath::RoundFromZero(13.f);
		GetIO()->Fonts->AddFontDefault(&FontConfig);
		
		unsigned char* Pixels;
		int Width, Height, Bpp;
		GetIO()->Fonts->GetTexDataAsRGBA32(&Pixels, &Width, &Height, &Bpp);
	}
}

TSharedPtr<IImguiViewport> UImguiContext::_SafeFindViewport(ImGuiViewport* InViewport, bool bNeedShow)
{
	// early out
	if (!InViewport) return nullptr;
	
	// find weak reference 
	auto UEWidget = ImViewportToUE[InViewport];

	// return it directly 
	if (UEWidget.IsValid()) return UEWidget.Pin();

	// create
	_CreateWindow(InViewport, (UImguiInputAdapter*)InViewport->PlatformUserData);

	// get shared ptr
	auto PinedWidget = ImViewportToUE[InViewport].Pin();
	
	// show it
	if (bNeedShow)
	{
		auto ParentViewPort = ImGui::FindViewportByID(InViewport->ParentViewportId);
		auto ParentWidget = _SafeFindViewport(ParentViewPort, bNeedShow);
		PinedWidget->Show(ParentWidget->GetWindow());
	}

	return PinedWidget;
}

TWeakPtr<IImguiViewport> UImguiContext::_DispatchProxy(ImGuiViewport* InViewport, UImguiInputAdapter* InInputAdapter)
{
	for (int32 i = 0; i < AllRenderProxy.Num();)
	{
		auto Proxy = AllRenderProxy[i];

		// check if valid 
		if (!Proxy.IsValid())
		{
			AllRenderProxy.RemoveAtSwap(i);
			continue;
		}

		// find window
		auto PinedProxy = Proxy.Pin();
		check(PinedProxy->IsPersist());
		ImGuiWindow* Window = ImGui::FindWindowByID(PinedProxy->GetPersistWindowID());
		if (Window && Window->Viewport == InViewport)
		{
			PinedProxy->SetupViewport(InViewport);
			PinedProxy->SetupInputAdapter(InInputAdapter);
			return PinedProxy;
		}
		
		++i;
	}

	return nullptr;
}