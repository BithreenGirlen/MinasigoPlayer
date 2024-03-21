#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <memory>

#include "d2_drawer.h"
#include "view_manager.h"
#include "adv.h"
#include "mf_media_player.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	const wchar_t* m_swzClassName = L"Minasigo player window";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam);
	LRESULT OnTimer(WPARAM wParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu
	{
		kOpenFolder = 1,
		kAudioLoop, kAudioSetting
	};
	enum MenuBar
	{
		kFolder, kAudio
	};
	enum EventMessage
	{
		kAudioPlayer = WM_USER + 1
	};
	enum Timer
	{
		kText = 1,
	};

	POINT m_CursorPos{};
	bool m_bLeftDowned = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;

	bool m_bTextHidden = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	std::vector<ImageInfo> m_imageInfo;
	size_t m_nImageIndex = 0;

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();
	void MenuOnNextFolder();
	void MenuOnForeFolder();

	void MenuOnAudioLoop();
	void MenuOnAudioSetting();

	void SwitchWindowStyle();

	bool SetupScenarioResources(const wchar_t* pwzFolderPath);
	bool CreateImageList(const wchar_t* pwzFolderPath);
	bool CreateMessageList(const wchar_t* pwzFolderPath);
	bool CreateFolderList(const wchar_t* pwzFolderPath);

	void UpdateScreen();

	std::unique_ptr<CViewManager> m_pViewManager;

	std::unique_ptr<CD2Drawer> m_pD2Drawer;
	void ShiftImage(bool bForward);

	std::unique_ptr<CMfMediaPlayer> m_pMfAudioPlayer;
	void ShiftText(bool bForward);
	void UpdateText();
	void OnAudioPlayerEvent(unsigned long ulEvent);
	void AutoTexting();
};

#endif //MAIN_WINDOW_H_