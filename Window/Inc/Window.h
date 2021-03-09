/*=============================================================================
	Window.h: GUI window management code.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#pragma once

#include <Windows.h>
#include <commctrl.h>
#include "../../Core/Inc/Core.h"

#ifndef WINDOW_API
#define WINDOW_API DLL_IMPORT
LINK_LIB(Window)
#endif

// TODO: Check all virtual tables

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// Classes.
class WWindow;
class WControl;
class WWizardDialog;
class WWizardPage;
class WDragInterceptor;

class AActor;

// Global functions.
WINDOW_API void InitWindowing();
WINDOW_API HBITMAP LoadFileToBitmap(const TCHAR* Filename, INT& SizeX, INT& SizeY);

// Global variables.
extern WINDOW_API HBRUSH hBrushWhite;
extern WINDOW_API HBRUSH hBrushOffWhite;
extern WINDOW_API HBRUSH hBrushHeadline;
extern WINDOW_API HBRUSH hBrushBlack;
extern WINDOW_API HBRUSH hBrushCurrent;
extern WINDOW_API HBRUSH hBrushDark;
extern WINDOW_API HBRUSH hBrushGrey;
extern WINDOW_API HBRUSH hBrushGrey160;
extern WINDOW_API HBRUSH hBrushGrey180;
extern WINDOW_API HBRUSH hBrushGrey197;
extern WINDOW_API HBRUSH hBrushGreyWindow;
extern WINDOW_API HBRUSH hBrushCyanHighlight;
extern WINDOW_API HBRUSH hBrushCyanLow;
extern WINDOW_API HFONT  hFontText;
extern WINDOW_API HFONT  hFontUrl;
extern WINDOW_API HFONT  hFontHeadline;
extern WINDOW_API class  WLog* GLogWindow;
extern WINDOW_API HINSTANCE hInstanceWindow;
extern WINDOW_API UBOOL GNotify;
extern WINDOW_API UINT WindowMessageOpen;
extern WINDOW_API UINT WindowMessageMouseWheel;
extern WINDOW_API NOTIFYICONDATA NID;

/*-----------------------------------------------------------------------------
	Window class definition macros.
-----------------------------------------------------------------------------*/

inline void MakeWindowClassName(TCHAR* Result, const TCHAR* Base){
	guard(MakeWindowClassName);
	appSprintf(Result, "%sUnreal%s", appPackage(), Base);
	unguard;
}

#define DECLARE_WINDOWCLASS(cls,parentcls,pkg) \
	public: \
		void GetWindowClassName(TCHAR* Result){ MakeWindowClassName(Result,#cls); } \
		~cls(){ MaybeDestroy(); } \
		virtual const TCHAR* GetPackageName(){ return #pkg; }

#define DECLARE_WINDOWSUBCLASS(cls,parentcls,pkg) \
	DECLARE_WINDOWCLASS(cls,parentcls,pkg) \
	static WNDPROC SuperProc;

#define IMPLEMENT_WINDOWCLASS(cls,clsf) \
{\
	TCHAR Temp[256];\
	MakeWindowClassName(Temp,#cls);\
	cls::RegisterWindowClass(Temp, clsf);\
}

#define IMPLEMENT_WINDOWSUBCLASS(cls,wincls) \
	{TCHAR Temp[256]; MakeWindowClassName(Temp,#cls); cls::SuperProc = cls::RegisterWindowClass(Temp, wincls);}

/*-----------------------------------------------------------------------------
	FRect.
-----------------------------------------------------------------------------*/

struct FPoint{
	INT X, Y;

	FPoint(){}
	FPoint(INT InX, INT InY) : X(InX),
							   Y(InY){}

	operator POINT*() const{ return (POINT*)this; }
	const INT& operator[](INT i) const{ return (&X)[i]; }
	INT& operator[](INT i){ return (&X)[i]; }
	UBOOL operator==(const FPoint& Other) const{ return X==Other.X && Y==Other.Y; }
	UBOOL operator!=(const FPoint& Other) const{ return X!=Other.X || Y!=Other.Y; }
	FPoint& operator+=(const FPoint& Other){
		X += Other.X;
		Y += Other.Y;
		return *this;
	}
	FPoint& operator-=(const FPoint& Other){
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}
	FPoint operator+(const FPoint& Other) const{ return FPoint(*this) += Other; }
	FPoint operator-(const FPoint& Other) const{ return FPoint(*this) -= Other; }

	static FPoint ZeroValue(){ return FPoint(0,0); }
	static FPoint NoneValue(){ return FPoint(INDEX_NONE,INDEX_NONE); }
	static INT Num(){ return 2; }
};

struct FRect{
	FPoint Min, Max;

	FRect(){}
	FRect(INT X0, INT Y0, INT X1, INT Y1) : Min(X0, Y0),
											Max(X1, Y1){}
	FRect(FPoint InMin, FPoint InMax) : Min(InMin),
										Max(InMax){}
	FRect(RECT R) : Min(R.left, R.top),
					Max(R.right, R.bottom){}
	operator RECT*() const{ return (RECT*)this; }
	const FPoint& operator[](INT i) const{ return (&Min)[i]; }
	FPoint& operator[](INT i){ return (&Min)[i]; }
	UBOOL operator==(const FRect& Other) const{ return Min == Other.Min && Max == Other.Max; }
	UBOOL operator!=(const FRect& Other) const{ return Min != Other.Min || Max != Other.Max; }
	FRect Right(INT Width){ return FRect(::Max(Min.X, Max.X - Width), Min.Y, Max.X, Max.Y); }
	FRect Bottom(INT Height){ return FRect(Min.X, ::Max(Min.Y, Max.Y - Height), Max.X, Max.Y); }
	FPoint Size(){ return FPoint(Max.X-Min.X, Max.Y-Min.Y); }
	INT Width(){ return Max.X-Min.X; }
	INT Height(){ return Max.Y-Min.Y; }
	FRect& operator+=(const FPoint& P){
		Min += P;
		Max += P;
		return *this;
	}
	FRect& operator-=(const FPoint& P){
		Min -= P;
		Max -= P;
		return *this;
	}
	FRect operator+(const FPoint& P) const{ return FRect(Min+P, Max+P); }
	FRect operator-(const FPoint& P) const{ return FRect(Min-P, Max-P); }
	FRect operator+(const FRect& R) const{ return FRect(Min+R.Min, Max+R.Max); }
	FRect operator-(const FRect& R) const{ return FRect(Min-R.Min, Max-R.Max); }
	FRect Inner(FPoint P) const{ return FRect(Min+P, Max-P); }
	UBOOL Contains(FPoint P) const{ return P.X >= Min.X && P.X < Max.X && P.Y >= Min.Y && P.Y < Max.Y; }

	static INT Num(){ return 2; }
};

/*-----------------------------------------------------------------------------
	FControlSnoop.
-----------------------------------------------------------------------------*/

/*
 * For forwarding interaction with a control to an object.
 */
class WINDOW_API FControlSnoop{
public:
	// FControlSnoop interface.
	virtual void SnoopChar(WWindow* Src, INT Char){}
	virtual void SnoopKeyDown(WWindow* Src, INT Char){}
	virtual void SnoopLeftMouseDown(WWindow* Src, FPoint P){}
	virtual void SnoopRightMouseDown(WWindow* Src, FPoint P){}
};

/*-----------------------------------------------------------------------------
	FCommandTarget.
-----------------------------------------------------------------------------*/

/*
 * Interface for accepting commands.
 */
class WINDOW_API FCommandTarget{
public:
	virtual void Unused(){}
};

// Delegate function pointers.
typedef void(FCommandTarget::*TDelegate)();
typedef void(FCommandTarget::*TDelegateInt)(INT);

// Simple bindings to an object and a member function of that object.
struct WINDOW_API FDelegate{
	FCommandTarget* TargetObject;
	void (FCommandTarget::*TargetInvoke)();

	FDelegate(FCommandTarget* InTargetObject = NULL, TDelegate InTargetInvoke = NULL);

	virtual void operator()();
};

struct WINDOW_API FDelegateInt{
	FCommandTarget* TargetObject;
	void (FCommandTarget::*TargetInvoke)(INT);

	FDelegateInt(FCommandTarget* InTargetObject = NULL, TDelegateInt InTargetInvoke = NULL);

	virtual void operator()(INT I);
};

// Load a menu and localize its text.
WINDOW_API HMENU LoadLocalizedMenu(HINSTANCE hInstance, INT Id, const TCHAR* Name, const TCHAR* Package = GPackage);

/*-----------------------------------------------------------------------------
	FWindowsBitmap.
-----------------------------------------------------------------------------*/

/*
 * A bitmap.
 */
class WINDOW_API FWindowsBitmap{
public:
	INT SizeX, SizeY, Keep;

	FWindowsBitmap(UBOOL InKeep = 0);
	~FWindowsBitmap();

	UBOOL LoadFile(const TCHAR* Filename);
	HBITMAP GetBitmapHandle();

private:
	HBITMAP hBitmap;

	void operator=(FWindowsBitmap&){}
};

/*-----------------------------------------------------------------------------
	WWindow.
-----------------------------------------------------------------------------*/

#define W_DECLARE_ABSTRACT_CLASS(a,b,c) public:
#define W_DECLARE_CLASS(a,b,c) public:
#define W_IMPLEMENT_CLASS(a)

/*
 * An operating system window.
 */
class WINDOW_API WWindow : public FCommandTarget{
	W_DECLARE_ABSTRACT_CLASS(WWindow,UObject,CLASS_Transient);

	// Variables.
	HWND					hWnd;
	FName					PersistentName;
	WORD					ControlId, TopControlId;
	BITFIELD				Destroyed:1;
	BITFIELD				MdiChild:1;
	WWindow*				OwnerWindow;
	FNotifyHook*			NotifyHook;
	FControlSnoop*			Snoop;
	TArray<class WControl*>	Controls;
	UBOOL bShow;
	WPARAM LastwParam;
	LPARAM LastlParam;
	INT UDNHelpTopic;		// Matches one of the ID's registed to the UDN help topic map in UnrealEd\src\main.cpp

	// Static.
	static INT              ModalCount;
	static TArray<WWindow*> _Windows;
	static TArray<WWindow*> _DeleteWindows;
	//Use this procedure for modeless dialogs.
	static INT_PTR CALLBACK StaticDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LONG APIENTRY StaticWndProc(HWND hWnd, UINT Message, UINT wParam, LONG lParam);
	static WNDPROC RegisterWindowClass(const TCHAR* Name, DWORD Style);

	// Structors.
	WWindow(FName InPersistentName = NAME_None, WWindow* InOwnerWindow = NULL);

	FRect GetClientRect() const;
	void MoveWindow(FRect R, UBOOL bRepaint);
	void MoveWindow(INT Left, INT Top, INT Width, INT Height, UBOOL bRepaint);
	void ResizeWindow(INT Width, INT Height, UBOOL bRepaint);
	FRect GetWindowRect(UBOOL bConvert = 1) const;
	FPoint ClientToScreen(const FPoint& InP);
	FPoint ScreenToClient(const FPoint& InP);
	FRect ClientToScreen(const FRect& InR);
	FRect ScreenToClient(const FRect& InR);
	FPoint GetCursorPos();
	void SetNotifyHook(FNotifyHook* NotifyHook);

	// Virtual functions
	virtual ~WWindow();
	virtual void Show(UBOOL Show);
	virtual void Serialize(FArchive& Ar);
	virtual const TCHAR* GetPackageName();
	virtual void DoDestroy();
	virtual void GetWindowClassName(TCHAR* Result) = 0;
	virtual LONG WndProc(UINT Message, UINT wParam, LONG lParam);
	virtual INT CallDefaultProc(UINT Message, UINT wParam, LONG lParam);
	virtual UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
	virtual FString GetText();
	virtual void SetText(const TCHAR* Text);
	virtual INT GetLength();
	virtual void OnCopyData(HWND hWndSender, COPYDATASTRUCT* CD);
	virtual void OnSetFocus(HWND hWndLosingFocus);
	virtual void OnKillFocus(HWND hWndGaininFocus);
	virtual void OnSize(DWORD Flags, INT NewX, INT NewY);
	virtual void OnWindowPosChanging(INT* NewX, INT* NewY, INT* NewWidth, INT* NewHeight);
	virtual void OnMove(INT NewX, INT NewY);
	virtual void OnCommand(INT Command);
	virtual INT OnSysCommand(INT Command);
	virtual void OnActivate(UBOOL Active);
	virtual void OnChar(TCHAR Ch);
	virtual void OnKeyDown(TCHAR Ch);
	virtual void OnCut();
	virtual void OnCopy();
	virtual void OnPaste();
	virtual void OnShowWindow(UBOOL bShow);
	virtual void OnUndo();
	virtual UBOOL OnEraseBkgnd();
	virtual void OnVScroll(WPARAM wParam, LPARAM lParam);
	virtual void OnHScroll(WPARAM wParam, LPARAM lParam);
	virtual void OnKeyUp(WPARAM wParam, LPARAM lParam);
	virtual void OnPaint();
	virtual void OnCreate();
	virtual void OnDrawItem(DRAWITEMSTRUCT* Info);
	virtual void OnMeasureItem(MEASUREITEMSTRUCT* Info);
	virtual void OnInitDialog();
	virtual void OnEnterIdle();
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnMouseHover();
	virtual void OnTimer();
	virtual void OnReleaseCapture();
	virtual void OnMdiActivate(UBOOL Active);
	virtual void OnMouseMove(DWORD Flags, FPoint Location);
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonDoubleClick();
	virtual void OnMiddleButtonDoubleClick();
	virtual void OnRightButtonDoubleClick();
	virtual void OnRightButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnRightButtonUp();
	virtual void OnFinishSplitterDrag(WDragInterceptor* Drag, UBOOL Success);
	virtual INT OnSetCursor();
	virtual UBOOL OnClose();
	virtual void OnDestroy();
	virtual void MyDrawEdge(HDC hdc, LPRECT qrc, UBOOL bRaised);

	void SaveWindowPos();
	void MaybeDestroy();
	void _CloseWindow();
	void SetFont(HFONT hFont);
	void PerformCreateWindowEx(DWORD dwExStyle, LPCTSTR lpWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);
	void VerifyPosition();
	void SetRedraw(UBOOL Redraw);

	operator HWND() const{ return hWnd; }
};

/*-----------------------------------------------------------------------------
	WControl.
-----------------------------------------------------------------------------*/

/*
 * A control which exists inside an owner window.
 */
class WINDOW_API WControl : public WWindow{
	W_DECLARE_ABSTRACT_CLASS(WControl,WWindow,CLASS_Transient);

	// Variables.
	WNDPROC WindowDefWndProc;

	// Structors.
	WControl(){}
	WControl(WWindow* InOwnerWindow, INT InId, WNDPROC InSuperProc);
	~WControl();

	virtual INT CallDefaultProc(UINT Message, UINT wParam, LONG lParam);

	static WNDPROC RegisterWindowClass(const TCHAR* Name, const TCHAR* WinBaseClass);
};

/*-----------------------------------------------------------------------------
	WTabControl.
-----------------------------------------------------------------------------*/

class WINDOW_API WTabControl : public WControl{
	W_DECLARE_CLASS(WTabControl,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WTabControl,WControl,Window)

	FDelegate SelectionChangeDelegate;

	// Constructor.
	WTabControl(){}
	WTabControl(WWindow* InOwner, INT InId=0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OpenWindow(UBOOL Visible, DWORD dwExtraStyle = 0);
	void AddTab(FString InText, INT InlParam);
	void Empty();
	INT GetCount();
	INT SetCurrent(INT Index);
	INT GetCurrent();
	INT GetIndexFromlParam(INT InlParam);
	FString GetString(INT Index);
	INT GetlParam(INT Index);
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
};

/*-----------------------------------------------------------------------------
	WLabel.
-----------------------------------------------------------------------------*/

/*
 * A non-interactive label control.
 */
class WINDOW_API WLabel : public WControl{
	W_DECLARE_CLASS(WLabel,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WLabel,WControl,Window)

	// Constructor.
	WLabel(){}
	WLabel(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OpenWindow(UBOOL Visible, UBOOL ClientEdge = 1, DWORD dwExtraStyle = 0);
};

/*-----------------------------------------------------------------------------
	WGroupBox.
-----------------------------------------------------------------------------*/

class WINDOW_API WGroupBox : public WControl{
	W_DECLARE_CLASS(WGroupBox,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WGroupBox,WControl,Window)

	// Constructor.
	WGroupBox(){}
	WGroupBox(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OpenWindow(UBOOL Visible, DWORD dwExtraStyle = 0);
};

/*-----------------------------------------------------------------------------
	WPropertyPage.

    A WPropertySheet has 1 or more of these on it.  Each tab
	represents one page.  A page uses a dialog box resource as it's
	template to determine where to place child controls.
-----------------------------------------------------------------------------*/

/*
 * Helper struct that stores the window positions
 * of the various controls this page contains.
 */
struct WPropertyPageCtrl{
	INT id;
	RECT rect;
	FString Caption;
	LONG Style;
	LONG ExStyle;

	WPropertyPageCtrl(INT InId, RECT InRect, FString InCaption, LONG InStyle, LONG InExStyle);
};

class WINDOW_API WPropertyPage : public WWindow{
	DECLARE_WINDOWCLASS(WPropertyPage,WWindow,Window)

	TArray<WPropertyPageCtrl> Ctrls;
	TArray<WLabel*> Labels;
	FString Caption;
	INT id;

	// Structors.
	WPropertyPage(WWindow* InOwnerWindow) : WWindow("WPropertyPage", InOwnerWindow){}

	virtual void OpenWindow(INT InDlgId, HMODULE InHMOD);
	void PlaceControl(WControl* InControl);
	void Finalize();
	void Cleanup();
	FString GetCaption();
	INT GetID();
	virtual void Refresh();
};

/*-----------------------------------------------------------------------------
	WPropertySheet.
-----------------------------------------------------------------------------*/
class FContainer;

class WINDOW_API WPropertySheet : public WControl{
	W_DECLARE_CLASS(WPropertySheet,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WPropertySheet,WControl,Window)

	WTabControl* Tabs;
	TArray<WPropertyPage*> Pages;
	FContainer* ParentContainer;
	UBOOL bResizable;
	UBOOL bMultiLine;

	// Constructor.
	WPropertySheet();
	WPropertySheet(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, UBOOL InResizable, DWORD dwExtraStyle = 0);
	void OnCreate();
	void OnSize(DWORD Flags, INT NewX, INT NewY);
	void RefreshPages();
	void Empty();
	void AddPage(WPropertyPage* InPage);
	INT SetCurrent(INT Index);
	INT SetCurrent(WPropertyPage* Page);
	void OnTabsSelChange();
};

/*-----------------------------------------------------------------------------
	WCustomLabel.
-----------------------------------------------------------------------------*/

class WINDOW_API WCustomLabel : public WLabel{
	W_DECLARE_CLASS(WCustomLabel,WLabel,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WCustomLabel,WLabel,Window)

	// Constructor.
	WCustomLabel(){}
	WCustomLabel(WWindow* InOwner, INT InId=0, WNDPROC InSuperProc = NULL) : WLabel(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OnPaint();
};

/*-----------------------------------------------------------------------------
	WButton.
-----------------------------------------------------------------------------*/

/*
 * A button.
 */
class WINDOW_API WButton : public WControl{
	W_DECLARE_CLASS(WButton,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WButton,WControl,Window)

	// Delegates.
	FDelegate ClickDelegate;
	FDelegate DoubleClickDelegate;
	FDelegate PushDelegate;
	FDelegate UnPushDelegate;
	FDelegate SetFocusDelegate;
	FDelegate KillFocusDelegate;
	HBITMAP hbm;
	UBOOL bChecked, bOwnerDraw;

	// Constructor.
	WButton(){}
	WButton(WWindow* InOwner, INT InId = 0, FDelegate InClicked = FDelegate(), WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, INT X, INT Y, INT XL, INT YL, const TCHAR* Text, UBOOL bBitmap = 0, DWORD dwExtraStyle = 0);
	void SetVisibleText(const TCHAR* Text);
	void SetBitmap(HBITMAP Inhbm);
	virtual void Clicked();
	void OnDrawItem(DRAWITEMSTRUCT* Item);
	UBOOL IsChecked(void);
	void SetCheck(INT iCheck);
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
};

/*-----------------------------------------------------------------------------
	WBitmapButton.
-----------------------------------------------------------------------------*/

/*
 * Works like a normal button, but has a bitmap on it instead of text.
 */
class WINDOW_API WBitmapButton : public WButton{
	W_DECLARE_CLASS(WBitmapButton,WButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WBitmapButton,WButton,Window)

	HBITMAP hbmSource;
	FRect EnabledRect, DownRect, DisabledRect; // the position/size to read from the source bitmap
	UBOOL bIsAutoCheckBox;

	// Constructor.
	WBitmapButton(){}
	WBitmapButton(WWindow* InOwner, INT InId = 0, FDelegate InClicked = FDelegate(), WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, DWORD InType, DWORD dwExtraStyle, HBITMAP InhbmSource, FRect InEnabledRect, FRect InDownRect, FRect InDisabledRect);
	void OnDestroy();
	void OnDrawItem(DRAWITEMSTRUCT* Item);
	virtual void Clicked();
};

/*-----------------------------------------------------------------------------
	WColorButton.
-----------------------------------------------------------------------------*/

class WINDOW_API WColorButton : public WButton{
	W_DECLARE_CLASS(WColorButton,WButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WColorButton,WButton,Window)

	INT R, G, B;

	// Constructor.
	WColorButton(){}
	WColorButton(WWindow* InOwner, INT InId = 0, FDelegate InClicked = FDelegate(), WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, DWORD dwExtraStyle);
	void OnDrawItem(DRAWITEMSTRUCT* Item);
	void SetColor(INT InR, INT InG, INT InB);
	void GetColor(INT& InR, INT& InG, INT& InB);
};

/*-----------------------------------------------------------------------------
	WToolTip
-----------------------------------------------------------------------------*/

/*
 * Tooltip window - easy way to create tooltips for standard controls.
 */
class WINDOW_API WToolTip : public WControl{
	W_DECLARE_CLASS(WToolTip,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WToolTip,WControl,Window)

	// Constructor.
	WToolTip(){}
	WToolTip(WWindow* InOwner, INT InId = 900, WNDPROC InSuperProc = NULL);

	void OpenWindow();
	void AddTool(HWND InHwnd, FString InToolTip, INT InId, RECT* InRect = NULL);
};

/*-----------------------------------------------------------------------------
	WThinScrollBar.
-----------------------------------------------------------------------------*/
/*
 * A small, MAX-like scrollbar.
 */
class WINDOW_API WThinScrollBar : public WWindow{
	DECLARE_WINDOWCLASS(WThinScrollBar,WWindow,Window)

	// Delegates.
	FDelegate PosChangedDelegate;

	FLOAT ThumbPosPct;
	FLOAT ThumbSzPct;	// The pct's where the thumb sits and how large it is
	INT ID;				// The controls ID
	INT MaxVal;			// The maximum value the scrollbar represents

	HBRUSH ThumbBrush;
	FPoint OldMouseLocation;

	// Structors.
	WThinScrollBar(WWindow* InOwnerWindow, INT InID);

	void OpenWindow(INT X, INT Y, INT XL, INT YL);
	void OnLeftButtonDoubleClick();
	void OnLeftButtonDown();
	void OnMouseMove(DWORD Flags, FPoint MouseLocation);
	void OnLeftButtonUp();
	void OnDestroy();
	INT OnSetCursor();
	FRect GetThumbRect();
	void OnPaint();
	INT GetPos();
	void SetPos(INT InPos, FLOAT InThumbSzPct);
	void SetRange(INT InMaxVal);
};

/*-----------------------------------------------------------------------------
	WTimeScrollBar.
-----------------------------------------------------------------------------*/
/*
 * Custom scrollbar style control for Matinee.
 */
class WINDOW_API WTimeScrollBar : public WWindow{
	DECLARE_WINDOWCLASS(WTimeScrollBar,WWindow,Window)

	// Delegates.
	FDelegate PosChangedDelegate;

	FLOAT ThumbPosPct;	// The position of the thumb expressed a %
	INT ID;				// The controls ID
	INT MaxVal;			// The maximum value the scrollbar represents

	HBITMAP hbmThumb, hbmLeft, hbmBody, hbmRight;
	FPoint OldMouseLocation;
	BITMAP ThumbInfo, LeftInfo, BodyInfo, RightInfo;

	// Structors.
	WTimeScrollBar(WWindow* InOwnerWindow, INT InID);

	void OpenWindow(INT X, INT Y, INT XL);
	void OnLeftButtonDoubleClick();
	void OnLeftButtonDown();
	void OnMouseMove(DWORD Flags, FPoint MouseLocation);
	void OnLeftButtonUp();
	void OnDestroy();
	INT OnSetCursor();
	FRect GetThumbRect();
	void OnPaint();
	INT GetPos();
	FLOAT GetPct();
	void SetPos(INT InPos);
	void SetPct(FLOAT InPct);
	void SetRange(INT InMaxVal);
	virtual void OnRightButtonDown();
};

/*-----------------------------------------------------------------------------
	WPictureButton.
-----------------------------------------------------------------------------*/

class WINDOW_API WPictureButton : public WWindow{
	DECLARE_WINDOWCLASS(WPictureButton,WWindow,Window)

	HBITMAP hbmOn, hbmOff;
	RECT ClientPos, BmpOffPos, BmpOnPos;
	FString ToolTipText;
	INT ID;
	WToolTip* ToolTipCtrl;
	UBOOL bOn, bHasBeenSetup;

	// Structors.
	WPictureButton(WWindow* InOwnerWindow);

	void OpenWindow();
	void OnSize(DWORD Flags, INT NewX, INT NewY);
	void OnDestroy();
	void OnPaint();
	void OnLeftButtonDown();
	void SetUp(FString InToolTipText, INT InID,
		INT InClientLeft, INT InClientTop, INT InClientRight, INT InClientBottom,
		HBITMAP InHbmOff, INT InBmpOffLeft, INT InBmpOffTop, INT InBmpOffRight, INT InBmpOffBottom,
		HBITMAP InHbmOn, INT InBmpOnLeft, INT InBmpOnTop, INT InBmpOnRight, INT InBmpOnBottom);
};

/*-----------------------------------------------------------------------------
	WUDNWindow.
-----------------------------------------------------------------------------*/

/*
 * Creating to capture the mouse so UDN help topics can be requested.
 */
class WINDOW_API WUDNWindow : public WWindow{
	DECLARE_WINDOWCLASS(WUDNWindow,WWindow,Window)

	// Structors.
	WUDNWindow(WWindow* InOwnerWindow) : WWindow("UDN", InOwnerWindow){}

	void OpenWindow();
	void Capture();
	void Release();
	void OnLeftButtonDown();
	virtual void OnKeyDown(TCHAR Ch);
	virtual void OnMouseMove(DWORD Flags, FPoint Location);
	INT GetCurrentTopicID();
};

/*-----------------------------------------------------------------------------
	WCheckBox.
-----------------------------------------------------------------------------*/

/*
 * A checkbox.
 */
class WINDOW_API WCheckBox : public WButton{
	W_DECLARE_CLASS(WCheckBox,WButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WCheckBox,WButton,Window)

	UBOOL bAutocheck;

	// Constructor.
	WCheckBox(){}
	WCheckBox(WWindow* InOwner, INT InId = 0, FDelegate InClicked = FDelegate()) : WButton(InOwner, InId, InClicked){}

	// WWindow interface.
	void OpenWindow(UBOOL Visible, INT X, INT Y, INT XL, INT YL, const TCHAR* Text, UBOOL InbAutocheck = 1, UBOOL bBitmap = 0, DWORD dwExtraStyle = 0);
	void OnCreate();
	virtual void OnRightButtonDown();
	virtual void Clicked();
};

/*-----------------------------------------------------------------------------
	WScrollBar.
-----------------------------------------------------------------------------*/

/*
 * A vertical scrollbar.
 */
class WINDOW_API WScrollBar : public WControl{
	W_DECLARE_CLASS(WScrollBar,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WScrollBar,WControl,Window)

	// Constructor.
	WScrollBar(){}
	WScrollBar(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OpenWindow(UBOOL Visible, INT X, INT Y, INT XL, INT YL, UBOOL bVertical = 1);
};

/*-----------------------------------------------------------------------------
	WTreeView.
-----------------------------------------------------------------------------*/

/*
 * A tree control
 */
class WINDOW_API WTreeView : public WControl{
	W_DECLARE_CLASS(WTreeView,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WTreeView,WControl,Window)

	// Delegates.
	FDelegate ItemExpandingDelegate;
	FDelegate SelChangedDelegate;
	FDelegate DblClkDelegate;

	// Constructor.
	WTreeView(){}
	WTreeView(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OpenWindow(UBOOL Visible, INT X, INT Y, INT XL, INT YL, DWORD dwExtraStyle = 0);
	void Empty(void);
	HTREEITEM AddItem(const TCHAR* pName, HTREEITEM hti, UBOOL bHasChildren);
	virtual void OnRightButtonDown();
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
};

/*-----------------------------------------------------------------------------
	WCoolButton.
-----------------------------------------------------------------------------*/

/*
 * Frame showing styles.
 */
enum EFrameFlags{
	CBFF_ShowOver	= 0x01,
	CBFF_ShowAway	= 0x02,
	CBFF_DimAway    = 0x04,
	CBFF_UrlStyle	= 0x08,
	CBFF_NoCenter   = 0x10
};

/*
 * A coolbar-style button.
 */
class WINDOW_API WCoolButton : public WButton{
	W_DECLARE_CLASS(WCoolButton,WButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WCoolButton,WButton,Window)

	// Variables.
	static WCoolButton* GlobalCoolButton;

	HICON hIcon;
	DWORD FrameFlags;

	// Constructor.
	WCoolButton(){}
	WCoolButton(WWindow* InOwner, INT InId = 0, FDelegate InClicked = FDelegate(), DWORD InFlags = CBFF_ShowOver | CBFF_DimAway);

	void OpenWindow(UBOOL Visible, INT X, INT Y, INT XL, INT YL, const TCHAR* Text);
	void OnDestroy();
};

/*-----------------------------------------------------------------------------
	WUrlButton.
-----------------------------------------------------------------------------*/

/*
 * A URL button.
 */
class WINDOW_API WUrlButton : public WCoolButton{
	W_DECLARE_CLASS(WUrlButton,WCoolButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WUrlButton,WCoolButton,Window)

	// Variables.
	FString URL;

	// Constructor.
	WUrlButton(){}
	WUrlButton(WWindow* InOwner, const TCHAR* InURL, INT InId = 0);

	void OnClick();
};

/*-----------------------------------------------------------------------------
	WComboBox.
-----------------------------------------------------------------------------*/

/*
 * A combo box control.
 */
class WINDOW_API WComboBox : public WControl{
	W_DECLARE_CLASS(WComboBox,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WComboBox,WControl,Window)

	// Delegates.
	FDelegate DoubleClickDelegate;
	FDelegate DropDownDelegate;
	FDelegate CloseComboDelegate;
	FDelegate EditChangeDelegate;
	FDelegate EditUpdateDelegate;
	FDelegate SetFocusDelegate;
	FDelegate KillFocusDelegate;
	FDelegate SelectionChangeDelegate;
	FDelegate SelectionEndOkDelegate;
	FDelegate SelectionEndCancelDelegate;

	// Constructor.
	WComboBox(){}
	WComboBox(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OpenWindow(UBOOL Visible, UBOOL Sort = FALSE, UINT InListType = CBS_DROPDOWNLIST);
	virtual LONG WndProc(UINT Message, UINT wParam, LONG lParam);
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
	virtual void AddString(const TCHAR* Str);
	virtual FString GetString(INT Index);
	virtual INT GetCount();
	virtual void SetCurrent(INT Index);
	virtual void SetCurrent(const TCHAR* String);
	virtual INT GetCurrent();
	virtual INT FindString(const TCHAR* String);
	virtual INT FindStringExact(const TCHAR* String);
	void Empty();
};

/*-----------------------------------------------------------------------------
	WEdit.
-----------------------------------------------------------------------------*/

/*
 * A single-line or multiline edit control.
 */
class WINDOW_API WEdit : public WControl{
	W_DECLARE_CLASS(WEdit,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WEdit,WControl,Window)

	// Variables.
	FDelegate ChangeDelegate;
    FDelegate BlurDelegate;

	// Constructor.
	WEdit(){}
	WEdit(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, UBOOL Multiline, UBOOL ReadOnly, UBOOL HorizScroll = FALSE, UBOOL NoHideSel = FALSE);
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
	UBOOL GetReadOnly();
	void SetReadOnly(UBOOL ReadOnly);
	INT GetLineCount();
	INT GetLineIndex(INT Line);
	void GetSelection(INT& Start, INT& End);
	void SetSelection(INT Start, INT End);
	void SetSelectedText(const TCHAR* Text);
	UBOOL GetModify();
	void SetModify(UBOOL Modified);
	void ScrollCaret();
};

/*-----------------------------------------------------------------------------
	WRichEdit.
-----------------------------------------------------------------------------*/

/*
 * A single-line or multiline edit control.
 */
class WINDOW_API WRichEdit : public WControl{
	W_DECLARE_CLASS(WRichEdit,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WRichEdit,WControl,Window)

	// Variables.
	FDelegate ChangeDelegate;

	// Constructor.
	WRichEdit(){}
	WRichEdit(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc?InSuperProc:SuperProc){}

	void StreamTextIn(char* _StreamSrc, INT _iSz);
	void StreamTextOut(char* _StreamDst, INT _iSz);
	void OpenWindow(UBOOL Visible, UBOOL ReadOnly);
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
	void SetTextLimit(INT _Limit);
	void SetReadOnly(UBOOL ReadOnly);
};

/*-----------------------------------------------------------------------------
	WTerminal.
-----------------------------------------------------------------------------*/

/*
 * Base class of terminal edit windows.
 */
class WINDOW_API WTerminalBase : public WWindow{
	W_DECLARE_ABSTRACT_CLASS(WTerminalBase,WWindow,CLASS_Transient);
	DECLARE_WINDOWCLASS(WTerminalBase,WWindow,Window)

	// Constructor.
	WTerminalBase(){}
	WTerminalBase(FName InPersistentName, WWindow* InOwnerWindow) : WWindow(InPersistentName, InOwnerWindow){}

	// WTerminalBase interface.
	virtual void TypeChar(TCHAR Ch) = 0;
	virtual void Paste() = 0;
};

/*
 * A terminal edit window.
 */
class WINDOW_API WEditTerminal : public WEdit{
	W_DECLARE_ABSTRACT_CLASS(WEditTerminal,WEdit,CLASS_Transient)
	DECLARE_WINDOWCLASS(WEditTerminal,WEdit,Window)

	// Variables.
	WTerminalBase* OwnerTerminal;

	// Constructor.
	WEditTerminal(WTerminalBase* InOwner = NULL);

	void OnChar(TCHAR Ch);
	void OnRightButtonDown();
	void OnPaste();
	void OnUndo();
};

/*
 * A terminal window.
 */
class WINDOW_API WTerminal : public WTerminalBase, public FOutputDevice{
	W_DECLARE_CLASS(WTerminal,WTerminalBase,CLASS_Transient);
	DECLARE_WINDOWCLASS(WTerminal,WTerminalBase,Window)

	// Variables.
	WEditTerminal Display;
	FExec* Exec;
	INT MaxLines, SlackLines;
	TCHAR Typing[256];
	UBOOL Shown;

	// Structors.
	WTerminal(){}
	WTerminal(FName InPersistentName, WWindow* InOwnerWindow);

	void Serialize(const TCHAR* Data, EName MsgType);
	void OnShowWindow(UBOOL bShow);
	void OnCreate();
	void OpenWindow(UBOOL bMdi=0, UBOOL AppWindow=0);
	void OnSetFocus(HWND hWndLoser);
	void OnSize(DWORD Flags, INT NewX, INT NewY);
	void Paste();
	void TypeChar(TCHAR Ch);
	void SelectTyping();
	void UpdateTyping();
	void SetExec(FExec* InExec);
};

/*-----------------------------------------------------------------------------
	WLog.
-----------------------------------------------------------------------------*/

class WINDOW_API WLog : public WTerminal{
	W_DECLARE_CLASS(WLog,WTerminal,CLASS_Transient);
	DECLARE_WINDOWCLASS(WLog,WTerminal,Window)

	// Variables.
	UINT NidMessage;
	FArchive*& LogAr;
	FString LogFilename;
	char Padding[1024];
	// Functions.
	WLog();
	WLog(const TCHAR* InLogFilename, FArchive*& InLogAr, FName InPersistentName, WWindow* InOwnerWindow = NULL);

	void OnCreate();
	void SetText(const TCHAR* Text);
	void OnShowWindow(UBOOL bShow);
	void OpenWindow(UBOOL bShow, UBOOL bMdi);
	void OnDestroy();
	void OnCopyData(HWND hWndSender, COPYDATASTRUCT* CD);
	UBOOL OnClose();
	void OnCommand(INT Command);
	LONG WndProc(UINT Message, UINT wParam, LONG lParam);
};

/*-----------------------------------------------------------------------------
	WDialog.
-----------------------------------------------------------------------------*/

/*
 * A dialog window, always based on a Visual C++ dialog template.
 */
class WINDOW_API WDialog : public WWindow{
	W_DECLARE_ABSTRACT_CLASS(WDialog,WWindow,CLASS_Transient);

	// Constructors.
	WDialog(){}
	WDialog(FName InPersistentName, INT InDialogId, WWindow* InOwnerWindow = NULL);

	// WWindow interface
	virtual void OnInitDialog();
	virtual void Show(UBOOL Show);

	// WDialog interface
	virtual void LocalizeText(const TCHAR* Section, const TCHAR* Package=GPackage);

	void EndDialogTrue();
	void EndDialogFalse();
	void EndDialog(INT Result);
	INT CallDefaultProc(UINT Message, UINT wParam, LONG lParam);
	void OpenChildWindow(INT InControlId, UBOOL Visible);
	void CenterInOwnerWindow();
	INT DoModal(HINSTANCE hInst = hInstanceWindow);

	static UBOOL CALLBACK LocalizeTextEnumProc(HWND hInWmd, LPARAM lParam);
};

/*-----------------------------------------------------------------------------
	WCrashBoxDialog.
-----------------------------------------------------------------------------*/

/*
 * A crash dialog box.
 */
class WINDOW_API WCrashBoxDialog : public WDialog{
	W_DECLARE_CLASS(WCrashBoxDialog,WDialog,CLASS_Transient);
	DECLARE_WINDOWCLASS(WCrashBoxDialog,WDialog,Window)

	// Controls.
	WEdit TextEdit;
	WCoolButton OkButton, CopyButton, BugReportButton;
	FString Caption, Message;

	// Constructor.
	WCrashBoxDialog(){}
	WCrashBoxDialog(const TCHAR* InCaption, const TCHAR* InMessage);

	void OnInitDialog();
	void OnCopy();
	void OnBugReport();
};

/*-----------------------------------------------------------------------------
	WTrackBar.
-----------------------------------------------------------------------------*/

class WINDOW_API WTrackBar : public WControl{
	W_DECLARE_CLASS(WTrackBar,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WTrackBar,WControl,Window)

	UBOOL ManualTicks;

	// Delegates.
	FDelegate ThumbTrackDelegate;
	FDelegate ThumbPositionDelegate;

	// Constructor.
	WTrackBar(){}
	WTrackBar(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, UBOOL InClientEdge = 1);
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
	void SetTicFreq(INT TicFreq);
	void SetTicks(INT* Ticks, INT TickCount);
	void SetRange(INT Min, INT Max);
	void SetPos(INT Pos);
	INT GetPos();
};

/*-----------------------------------------------------------------------------
	WProgressBar.
-----------------------------------------------------------------------------*/

/*
 * A non-interactive label control.
 */
class WINDOW_API WProgressBar : public WControl{
	W_DECLARE_CLASS(WProgressBar,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WProgressBar,WControl,Window)

	// Variables.
	INT Percent;

	// Constructor.
	WProgressBar(){}
	WProgressBar(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible);
	void SetProgress(SQWORD InCurrent, SQWORD InMax);
};

/*-----------------------------------------------------------------------------
	WListBox.
-----------------------------------------------------------------------------*/

class WINDOW_API WListBox : public WControl{
	W_DECLARE_CLASS(WListBox,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WListBox,WControl,Window)

	// Delegates.
	FDelegate DoubleClickDelegate;
	FDelegate SelectionChangeDelegate;
	FDelegate SelectionCancelDelegate;
	FDelegate SetFocusDelegate;
	FDelegate KillFocusDelegate;
	FDelegate RightClickDelegate;

	UBOOL m_bMultiSel;

	// Constructor.
	WListBox(){}
	WListBox(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, UBOOL Integral, UBOOL MultiSel, UBOOL OwnerDrawVariable, UBOOL Sort = 0, DWORD dwExtraStyle = 0);
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
	void OnRightButtonDown();
	FString GetString(INT Index);
	void* GetItemData(INT Index);
	void SetItemData(INT Index, void* Value);
	void SetItemData(INT Index, INT Value);
	INT GetCurrent();
	void ClearSel();
	INT SetCurrent(INT Index, UBOOL bScrollIntoView = 1);
	INT GetTop();
	void SetTop(INT Index);
	void DeleteString(INT Index);
	INT GetCount();
	INT GetItemHeight(INT Index);
	INT ItemFromPoint(FPoint P);
	FRect GetItemRect(INT Index);
	void Empty();
	UBOOL GetSelected(INT Index);
	INT GetSelectedItems(INT Count, INT* Buffer);
	INT GetSelectedCount();
	INT AddString(const TCHAR* C);
	void InsertString(INT Index, const TCHAR* C);
	INT FindStringExact(const TCHAR* C);
	INT FindString(const TCHAR* C);
	INT FindStringChecked(const TCHAR* C);
	void InsertStringAfter(const TCHAR* Existing, const TCHAR* New);
	INT AddItem(const void* C);
	void InsertItem(INT Index, const void* C);
	INT FindItem(const void* C);
	INT FindItemChecked(const void* C);
	void InsertItemAfter(const void* Existing, const void* New);
};

/*-----------------------------------------------------------------------------
	WCheckListBox.
-----------------------------------------------------------------------------*/

/*
 * A list box where each item has a checkbox beside it.
 */
class WINDOW_API WCheckListBox : public WListBox{
	W_DECLARE_CLASS(WCheckListBox,WListBox,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WCheckListBox,WListBox,Window)

	HBITMAP hbmOff, hbmOn;
	INT bOn;

	// Constructor.
	WCheckListBox(){}
	WCheckListBox(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL);

	void OpenWindow(UBOOL Visible, UBOOL Integral, UBOOL MultiSel, UBOOL Sort = FALSE, DWORD dwExtraStyle = 0);
	void OnDestroy();
	void OnDrawItem(DRAWITEMSTRUCT* Item);
	void OnLeftButtonDown();
};

/*-----------------------------------------------------------------------------
	FTreeItemBase.
-----------------------------------------------------------------------------*/

class WINDOW_API FTreeItemBase : public FCommandTarget, public FControlSnoop{
public:
	virtual void Draw(HDC hDC) = 0;
	virtual INT GetHeight() = 0;
	virtual void SetSelected(UBOOL NewSelected) = 0;
};

/*-----------------------------------------------------------------------------
	WItemBox.
-----------------------------------------------------------------------------*/

/*
 * A list box contaning list items.
 */
class WINDOW_API WItemBox : public WListBox{
	W_DECLARE_CLASS(WItemBox,WListBox,CLASS_Transient);
	DECLARE_WINDOWCLASS(WItemBox,WListBox,Window)

	// Constructors.
	WItemBox(){}
	WItemBox(WWindow* InOwner, INT InId = 0) : WListBox(InOwner, InId){
		check(OwnerWindow);
	}

	void OnDrawItem(DRAWITEMSTRUCT* Info);
	void OnMeasureItem(MEASUREITEMSTRUCT* Info);
	UBOOL OnEraseBkgnd();
};

/*-----------------------------------------------------------------------------
	WListView.
-----------------------------------------------------------------------------*/

/*
 * A list view.
 */
class WINDOW_API WListView : public WControl{
	W_DECLARE_CLASS(WListView,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WListView,WControl,Window)

	FDelegate DblClkDelegate;
	FDelegate SelChangedDelegate;

	// Constructor.
	WListView(){}
	WListView(WWindow* InOwner, INT InId = 0, WNDPROC InSuperProc = NULL) : WControl(InOwner, InId, InSuperProc ? InSuperProc : SuperProc){}

	void OpenWindow(UBOOL Visible, DWORD dwExtraStyle = 0);
	void Empty();
	UBOOL InterceptControlCommand(UINT Message, UINT wParam, LONG lParam);
	INT GetCurrent();
};

/*-----------------------------------------------------------------------------
	WPropertiesBase.
-----------------------------------------------------------------------------*/

class WINDOW_API WPropertiesBase : public WWindow, public FControlSnoop{
	W_DECLARE_ABSTRACT_CLASS(WPropertiesBase,WWindow,CLASS_Transient);

	// Variables.
	UBOOL ShowTreeLines;
	WItemBox List;
	class FTreeItem* FocusItem;

	// Structors.
	WPropertiesBase(){}
	WPropertiesBase(FName InPersistentName, WWindow* InOwnerWindow);

	// Overrides
	virtual UBOOL OnClose();

	// Virtual functions
	virtual FTreeItem* GetRoot() = 0;
	virtual INT GetDividerWidth() = 0;
	virtual void ResizeList() = 0;
	virtual void SetItemFocus(UBOOL FocusCurrent) = 0;
	virtual void ForceRefresh() = 0;
	virtual void BeginSplitterDrag() = 0;

	FTreeItem* GetListItem(INT i);
};

/*-----------------------------------------------------------------------------
	WDragInterceptor.
-----------------------------------------------------------------------------*/

/*
 * Splitter drag handler.
 */
class WINDOW_API WDragInterceptor : public WWindow{
	W_DECLARE_CLASS(WDragInterceptor,WWindow,CLASS_Transient);
	DECLARE_WINDOWCLASS(WDragInterceptor,WWindow,Window)

	// Variables.
	FPoint		OldMouseLocation;
	FPoint		DragIndices;
	FPoint		DragPos;
	FPoint		DragStart;
	FPoint		DrawWidth;
	FRect		DragClamp;
	UBOOL		Success;

	// Constructor.
	WDragInterceptor(){}
	WDragInterceptor(WWindow* InOwner, FPoint InDragIndices, FRect InDragClamp, FPoint InDrawWidth);

	virtual void OpenWindow();
	virtual void ToggleDraw(HDC hInDC);
	void OnKeyDown(TCHAR Ch);
	void OnMouseMove(DWORD Flags, FPoint MouseLocation);
	void OnReleaseCapture();
	void OnLeftButtonUp();
};

/*-----------------------------------------------------------------------------
	FTreeItem.
-----------------------------------------------------------------------------*/

/*
 * Base class of list items.
 */
class WINDOW_API FTreeItem : public FTreeItemBase{
public:
	// Variables.
	class WPropertiesBase*	OwnerProperties;
	FTreeItem*				Parent;
	UBOOL					Expandable;
	UBOOL					Expanded;
	UBOOL					Sorted;
	UBOOL					Selected;
	INT						ButtonWidth;
	TArray<WCoolButton*>	Buttons;
	TArray<FTreeItem*>		Children;

	// Structors.
	FTreeItem(){}
	FTreeItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UBOOL InExpandable);
	virtual ~FTreeItem();

	virtual HBRUSH GetBackgroundBrush(UBOOL Selected);
	virtual COLORREF GetTextColor(UBOOL Selected);
	virtual void Serialize(FArchive& Ar);
	virtual INT OnSetCursor();
	void EmptyChildren();
	virtual FRect GetRect();
	virtual void Redraw();
	virtual void OnItemSetFocus();
	virtual void OnItemKillFocus(UBOOL Abort);
	virtual void AddButton(const TCHAR* Text, FDelegate Action);
	virtual void OnItemLeftMouseDown(FPoint P);
	virtual void OnItemRightMouseDown(FPoint P);
	INT GetIndent();
	INT GetUnitIndentPixels();
	virtual INT GetIndentPixels(UBOOL Text);
	virtual FRect GetExpanderRect();
	virtual UBOOL GetSelected();
	void SetSelected(UBOOL InSelected);
	virtual void DrawTreeLines(HDC hDC, FRect Rect, UBOOL bTopLevelHeader);
	virtual void Collapse();
	virtual void Expand();
	virtual void ToggleExpansion();
	virtual void OnItemDoubleClick();
	virtual BYTE* GetBase(BYTE* Base);
	virtual BYTE* GetContents(BYTE* Base);
	virtual BYTE* GetReadAddress(class FPropertyItem* Child, UBOOL RequireSingleSelection=0);
	virtual void NotifyChange();
	virtual void SetProperty(FPropertyItem* Child, const TCHAR* Value);
	virtual void GetStates(TArray<FName>& States);
	virtual UBOOL AcceptFlags(DWORD InFlags);
	virtual QWORD GetId() const = 0;
	virtual FString GetCaption() const = 0;
	virtual void OnItemHelp();
	virtual void SetFocusToItem();
	virtual void SetValue(const TCHAR* Value);
	void SnoopChar(WWindow* Src, INT Char);
	void SnoopKeyDown(WWindow* Src, INT Char);
	virtual UObject* GetParentObject();
};

/*
 * Property list item.
 */
class WINDOW_API FPropertyItem : public FTreeItem{
public:
	// Variables.
	UProperty*      Property;
	INT				_Offset;
	INT				ArrayIndex;
	WEdit*			EditControl;
	WTrackBar*		TrackControl;
	WComboBox*		ComboControl;
	WLabel*			HolderControl;
	UBOOL			ComboChanged;
	UBOOL			EditInline;
	UBOOL			EditInlineUse;
	UBOOL			EditInlineNotify;

	UBOOL			EdFindable;
	UBOOL			SingleSelect;
	FName			Name;
	FString			Equation;				// A work area to store editor equations

	// Constructors.
	FPropertyItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UProperty* InProperty, FName InName, INT InOffset, INT InArrayIndex, INT SixSixSix);

	void Expand();
	void Serialize(FArchive& Ar);
	QWORD GetId() const;
	FString GetCaption() const;
	virtual INT OnSetCursor();
	virtual void OnItemLeftMouseDown(FPoint P);
	BYTE* GetBase(BYTE* Base);
	BYTE* GetContents(BYTE* Base);
	void GetPropertyText(TCHAR* Str);
	void SetValue(const TCHAR* Value);
	void Draw(HDC hDC);
	INT GetHeight();
	void SetFocusToItem();
	void OnItemDoubleClick();
	void OnItemSetFocus();
	void OnItemKillFocus(UBOOL Abort);
	void Collapse();
	void SnoopChar(WWindow* Src, INT Char);
	void ComboSelectionEndCancel();
	void ComboSelectionEndOk();
	void OnTrackBarThumbTrack();
	void OnTrackBarThumbPosition();
	void OnChooseColorButton();
	void OnArrayAdd();
	void OnArrayEmpty();
	void OnArrayInsert();
	void OnArrayDelete();
	void OnBrowseButton();
	void OnUseCurrentButton();
	void OnPickColorButton();
	void OnFindButton();
	void OnClearButton();
	virtual void Advance();
	virtual void SendToControl();
	virtual void ReceiveFromControl();
};

/*
 * An abstract list header.
 */
class WINDOW_API FHeaderItem : public FTreeItem{
public:
	// Constructors.
	FHeaderItem(){}
	FHeaderItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UBOOL InExpandable) : FTreeItem(InOwnerProperties, InParent, InExpandable){}

	void Draw(HDC hDC);
	FVector ToHSL(FVector RGB);
	FVector FromHSV(BYTE H, BYTE S, BYTE V);
	virtual void SetValue(const TCHAR* Value);
	void OnChooseHSLColorButton();
	void OnPickColorButton();
	void OnItemSetFocus();
	virtual void OnItemKillFocus(UBOOL Abort);
	INT GetHeight();
};

/*
 * A category header list item.
 */
class WINDOW_API FCategoryItem : public FHeaderItem{
public:
	// Variables.
	FName Category;
	UClass* BaseClass;

	// Constructors.
	FCategoryItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UClass* InBaseClass, FName InCategory, UBOOL InExpandable);

	void Serialize(FArchive& Ar);
	QWORD GetId() const;
	virtual FString GetCaption() const;
	void Expand();
	void Collapse();
};

/*
 * Item to create a new object for inline editing.
 */
class WINDOW_API FNewObjectItem : public FHeaderItem{
public:
	// Variables.
	UObjectProperty*	Property;
	WComboBox*			ComboControl;
	WLabel*				HolderControl;
	FString				NewClass;

	// Constructors.
	FNewObjectItem(){}
	FNewObjectItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UObjectProperty* InProperty);

	void OnItemSetFocus();
	void OnItemKillFocus(UBOOL Abort);
	void ComboSelectionEndCancel();
	void ComboSelectionEndOk();
	void OnNew();
	void Draw(HDC hDC);
	FString GetCaption() const;
	QWORD GetId() const;
};

/*-----------------------------------------------------------------------------
	WProperties.
-----------------------------------------------------------------------------*/

/*
 * General property editing control.
 */
class WINDOW_API WProperties : public WPropertiesBase{
	W_DECLARE_ABSTRACT_CLASS(WProperties,WPropertiesBase,CLASS_Transient);
	DECLARE_WINDOWCLASS(WProperties,WWindow,Window)

	// Variables.
	TArray<QWORD>		Remembered;
	QWORD				SavedTop, SavedCurrent;
	WDragInterceptor*	DragInterceptor;
	INT					DividerWidth;
	static TArray<WProperties*> PropertiesWindows;
	UBOOL bAllowForceRefresh;

	// Structors.
	WProperties(){}
	WProperties(FName InPersistentName, WWindow* InOwnerWindow = NULL);

	void OpenChildWindow(INT InControlId);
	void OpenWindow(HWND hWndParent = NULL);

	// Overrides
	virtual void SnoopChar(WWindow* Src, INT Char);
	virtual void SnoopKeyDown(WWindow* Src, INT Char);
	virtual void SnoopLeftMouseDown(WWindow* Src, FPoint P);
	virtual void SnoopRightMouseDown(WWindow* Src, FPoint P);
	virtual void Serialize(FArchive& Ar);
	virtual void DoDestroy();
	virtual void OnSize(DWORD Flags, INT NewX, INT NewY);
	virtual void OnActivate(UBOOL Active);
	virtual void OnPaint();
	virtual void OnFinishSplitterDrag(WDragInterceptor* Drag, UBOOL Success);
	virtual INT OnSetCursor();
	virtual void OnDestroy();
	virtual INT GetDividerWidth();
	virtual void ResizeList();
	virtual void SetItemFocus(UBOOL FocusCurrent);
	virtual void ForceRefresh();
	virtual void BeginSplitterDrag();

	// Virtual functions
	virtual void SetValue(const TCHAR* Value);
    virtual void ExpandAll();
	virtual void RemoveActor(AActor* Actor);
};

/*-----------------------------------------------------------------------------
	FPropertyItemBase.
-----------------------------------------------------------------------------*/

class WINDOW_API FPropertyItemBase : public FHeaderItem{
public:
	// Variables.
	FString Caption;
	DWORD FlagMask;
	UClass* BaseClass;

	// Structors.
	FPropertyItemBase(){}
	FPropertyItemBase(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const TCHAR* InCaption);

	void Serialize(FArchive& Ar);
	UBOOL AcceptFlags(DWORD InFlags);
	void GetStates(TArray<FName>& States);
	void Collapse();
	FString GetCaption() const;
	QWORD GetId() const;
};

/*-----------------------------------------------------------------------------
	WObjectProperties.
-----------------------------------------------------------------------------*/

/*
 * Object properties root.
 */
class WINDOW_API FObjectsItem : public FPropertyItemBase{
public:
	// Variables.
	UBOOL ByCategory;
	UBOOL NotifyParent;
	TArray<UObject*> _Objects;

	// Structors.
	FObjectsItem(){}
	FObjectsItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const TCHAR* InCaption, UBOOL InByCategory, UBOOL InNotifyParent);

	void Serialize(FArchive& Ar);
	BYTE* GetBase(BYTE* Base);
	BYTE* GetReadAddress(FPropertyItem* Child, UBOOL RequireSingleSelection =  0);
	virtual void NotifyChange();
	void SetProperty(FPropertyItem* Child, const TCHAR* Value);
	void Expand();
	FString GetCaption() const;
	// TODO(Leon): Fix vtables
	virtual void vtpad1(){}
	virtual void vtpad2(){}
	virtual void vtpad3(){}
	virtual void vtpad4(){}
	virtual void vtpad5(){}
	virtual void SetObjects(UObject** InObjects, INT Count);
	virtual UObject* GetParentObject();
	UBOOL Eval(FString Str, FLOAT* pResult);
	UBOOL SubEval(FString* pStr, FLOAT* pResult, INT Prec);
	FString GrabChar(FString* pStr);
	FLOAT Val(FString Value);
};

/*
 * Multiple selection object properties.
 */
class WINDOW_API WObjectProperties : public WProperties{
	W_DECLARE_CLASS(WObjectProperties,WProperties,CLASS_Transient);
	DECLARE_WINDOWCLASS(WObjectProperties,WProperties,Window)

	char PADDING[4];

	// Variables.
	FObjectsItem Root;

	// Structors.
	WObjectProperties(){}
	WObjectProperties(FName InPersistentName, DWORD InFlagMask, const TCHAR* InCaption, WWindow* InOwnerWindow, UBOOL InByCategory);

	// Overrides
	virtual void Show(UBOOL Show);
	virtual FTreeItem* GetRoot();
	virtual void RemoveActor(AActor* Actor);
};

/*-----------------------------------------------------------------------------
	WClassProperties.
-----------------------------------------------------------------------------*/

/*
 * Class properties root.
 */
class WINDOW_API FClassItem : public FPropertyItemBase{
public:
	// Structors.
	FClassItem(){}
	FClassItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const TCHAR* InCaption, UClass* InBaseClass);

	BYTE* GetBase(BYTE* Base);
	BYTE* GetReadAddress(class FPropertyItem* Child, UBOOL RequireSingleSelection=0);
	void SetProperty(FPropertyItem* Child, const TCHAR* Value);
	void Expand();
};

/*
 * Multiple selection class properties.
 */
class WINDOW_API WClassProperties : public WProperties{
	W_DECLARE_CLASS(WClassProperties,WProperties,CLASS_Transient);
	DECLARE_WINDOWCLASS(WClassProperties,WProperties,Window)

	// Variables.
	FClassItem Root;

	// Structors.
	WClassProperties(){}
	WClassProperties(FName InPersistentName, DWORD InFlagMask, const TCHAR* InCaption, UClass* InBaseClass);

	FTreeItem* GetRoot();
};

/*-----------------------------------------------------------------------------
	WConfigProperties.
-----------------------------------------------------------------------------*/

/*
 * Object configuration header.
 */
class WINDOW_API FObjectConfigItem : public FPropertyItemBase{
public:
	// Variables.
	FString  ClassName;
	FName    CategoryFilter;
	UClass*  Class;
	UBOOL	 Failed;
	UBOOL    Immediate;

	// Structors.
	FObjectConfigItem(WPropertiesBase* InOwnerProperties, FTreeItem* InParent, const TCHAR* InCaption, const TCHAR* InClass, UBOOL InImmediate, FName InCategoryFilter);

	BYTE* GetBase(BYTE* Base);
	BYTE* GetReadAddress(FPropertyItem* Child, UBOOL RequireSingleSelection = 0);
	void SetProperty(FPropertyItem* Child, const TCHAR* Value);
	void OnResetToDefaultsButton();
	void OnItemSetFocus();
	void Expand();
	void LazyLoadClass();
	void Serialize(FArchive& Ar);
};

/*
 * A configuration list item.
 */
class WINDOW_API FConfigItem : public FHeaderItem{
public:
	// Variables.
	FPreferencesInfo Prefs;

	// Constructors.
	FConfigItem(){}
	FConfigItem(const FPreferencesInfo& InPrefs, WPropertiesBase* InOwnerProperties, FTreeItem* InParent);

	// FTreeItem interface.
	QWORD GetId() const;
	virtual FString GetCaption() const;
	void Expand();
	void Collapse();
};

/*
 * Configuration properties.
 */
class WINDOW_API WConfigProperties : public WProperties{
	W_DECLARE_CLASS(WConfigProperties,WProperties,CLASS_Transient);
	DECLARE_WINDOWCLASS(WConfigProperties,WProperties,Window)

	// Variables.
	FConfigItem Root;
	char        Padding[8]; // PADDING!

	// Structors.
	WConfigProperties(){}
	WConfigProperties(FName InPersistentName, const TCHAR* InTitle);

	FTreeItem* GetRoot();
};

/*-----------------------------------------------------------------------------
	WWizardPage.
-----------------------------------------------------------------------------*/

/*
 * A wizard page.
 */
class WINDOW_API WWizardPage : public WDialog{
	W_DECLARE_ABSTRACT_CLASS(WWizardPage,WDialog,CLASS_Transient);
	DECLARE_WINDOWCLASS(WWizardPage,WDialog,Window)

	// Variables.
	WWizardDialog* Owner;

	// Constructor.
	WWizardPage(){}
	WWizardPage(const TCHAR* PageName, INT ControlId, WWizardDialog* InOwner);

	virtual void OnCurrent();
	virtual WWizardPage* GetNext();
	virtual const TCHAR* GetBackText();
	virtual const TCHAR* GetNextText();
	virtual const TCHAR* GetFinishText();
	virtual const TCHAR* GetCancelText();
	virtual UBOOL GetShow();
	virtual void OnCancel();
};

/*-----------------------------------------------------------------------------
	WWizardDialog.
-----------------------------------------------------------------------------*/

/*
 * The wizard frame dialog.
 */
class WINDOW_API WWizardDialog : public WDialog{
	W_DECLARE_CLASS(WWizardDialog,WDialog,CLASS_Transient);
	DECLARE_WINDOWCLASS(WWizardDialog,WDialog,Window)

	// Variables.
	WCoolButton BackButton;
	WCoolButton NextButton;
	WCoolButton FinishButton;
	WCoolButton CancelButton;
	WLabel PageHolder;
	TArray<WWizardPage*> Pages;
	WWizardPage* CurrentPage;

	// Constructor.
	WWizardDialog();

	void OnInitDialog();
	virtual void Advance(WWizardPage* NewPage);
	virtual void RefreshPage();
	virtual void OnDestroy();
	virtual void OnBack();
	virtual void OnNext();
	virtual void OnFinish();
	virtual void OnCancel();
	UBOOL OnClose();
};

/*-----------------------------------------------------------------------------
	Window container classes.

	Control the position/size of child controls in a resizable window.
-----------------------------------------------------------------------------*/

#define STANDARD_CTRL_HEIGHT	21
#define STANDARD_SB_WIDTH		16
#define STANDARD_TOOLBAR_HEIGHT	28
#define STANDARD_BUTTON_HEIGHT	23
#define STANDARD_BUTTON_WIDTH	77

enum EAnchorPos{
	ANCHOR_NONE		= 0,	// No change
	ANCHOR_TOP		= 1,	// Anchor relative to top of window
	ANCHOR_LEFT		= 2,	// Anchor relative to left of window
	ANCHOR_BOTTOM	= 4,	// Anchor relative to bottom of window
	ANCHOR_RIGHT	= 8,	// Anchor relative to right of window
	ANCHOR_WIDTH	= 16,	// Use a hardcoded width
	ANCHOR_HEIGHT	= 32,	// Use a hardcoded height

	ANCHOR_TL		= ANCHOR_TOP | ANCHOR_LEFT,
	ANCHOR_BR		= ANCHOR_BOTTOM | ANCHOR_RIGHT,
};

class WINDOW_API FWindowAnchor{
public:
	FWindowAnchor(){}
	FWindowAnchor(HWND InRefWindow, HWND InWindow,
		INT InPosFlags, INT InXPos, INT InYPos,
		INT InSzFlags, INT InXSz, INT InYSz);
	~FWindowAnchor(){}

	FWindowAnchor operator=(const FWindowAnchor& Other);

	HWND RefWindow;		// The window to anchor to

	HWND Window;		// The window we are anchoring
	INT PosFlags;		// ANCHOR_
	INT XPos, YPos;		// The offsets for the top/left corner

	INT SzFlags;		// ANCHOR_
	INT XSz, YSz;		// Either the offsets for the bottom/right corner of the window, or the hardcoded width/height
};

class WINDOW_API FContainer{
public:
	TMap<DWORD, FWindowAnchor>* Anchors;

	// Structors.
	FContainer();

	void SetAnchors(TMap<DWORD, FWindowAnchor>* InAnchors);
	void RefreshControls();
};

/*-----------------------------------------------------------------------------
	WSplitterPane.
-----------------------------------------------------------------------------*/

/*
 * The window that sits on either side of a WSPplitter.
 */
class WINDOW_API WSplitterPane : public WWindow{
	DECLARE_WINDOWCLASS(WSplitterPane,WWindow,Window)

	// Constructor.
	WSplitterPane(){}
	WSplitterPane(WWindow* InOwnerWindow);

	// WWindow interface.
	void OpenWindow();
	void OnPaint();
};

/*-----------------------------------------------------------------------------
	WSplitter.
-----------------------------------------------------------------------------*/

#define STANDARD_SPLITTER_SZ 6

/*
 * The bar that sits between the panes being split.
 */
class WINDOW_API WSplitter : public WWindow{
	DECLARE_WINDOWCLASS(WSplitter,WWindow,Window)

	UBOOL bVertical;
	FLOAT Pct;	// 0.0f-100.0f

	// Constructor.
	WSplitter();
	WSplitter(WWindow* InOwnerWindow);

	// WWindow interface.
	void OpenWindow(UBOOL InVertical = 1);
	void OnPaint();
	virtual INT OnSetCursor();
	void OnLeftButtonDown();
	void OnMouseMove(DWORD Flags, FPoint MouseLocation);
	void OnLeftButtonUp();
	void PositionSplitter();
};

/*-----------------------------------------------------------------------------
	WSplitterContainer.
-----------------------------------------------------------------------------*/

/*
 * A window which allows you to have 2 windows with a splitter bar between them.
 *
 * Dragging the splitter control will resize the windows.
 */
class WINDOW_API WSplitterContainer : public WWindow{
	DECLARE_WINDOWCLASS(WSplitterContainer,WWindow,Window)

	WSplitter* SplitterBar;
	WSplitterPane *Pane1, *Pane2;
	FContainer* ParentContainer;

	// Structors.
	WSplitterContainer(WWindow* InOwnerWindow);

	// WWindow interface.
	virtual void OpenWindow(UBOOL InVertical = 1);
	void SetPct(FLOAT InPct);
	virtual void OnDestroy();
	void PositionPanes();
	void OnSize(DWORD Flags, INT NewX, INT NewY);
	void OnPaint();
};

/*-----------------------------------------------------------------------------
	UDN Help.
-----------------------------------------------------------------------------*/

/*
 * Helper class to assist with context sensitive UDN help.
 */
class WINDOW_API FUDNHelpTopic{
public:
	FString MenuDesc, URL;

	FUDNHelpTopic(){}
	FUDNHelpTopic(FString InMenuDesc, FString InURL);
	~FUDNHelpTopic(){}
};

/*-----------------------------------------------------------------------------
	FWaitCursor.
-----------------------------------------------------------------------------*/

/*
 * Simple class to handle the hourglass cursor.
 */
class WINDOW_API FWaitCursor{
public:
	HCURSOR SaveCursor;

	FWaitCursor();
	~FWaitCursor();

	void Restore();

};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
