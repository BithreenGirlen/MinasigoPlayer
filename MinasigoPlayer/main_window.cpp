
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "mnsg.h"
#include "media_setting_dialogue.h"

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    //wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_APP));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    //wcex.lpszMenuName = MAKEINTRESOURCE(IDI_ICON_APP);
    wcex.lpszClassName = m_swzClassName;
    //wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_APP));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;

        m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

	return false;
}

int CMainWindow::MessageLoop()
{
    MSG msg;

    for (;;)
    {
        BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
        if (bRet > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else if (bRet == 0)
        {
            /*ループ終了*/
            return static_cast<int>(msg.wParam);
        }
        else
        {
            /*ループ異常*/
            std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
            return -1;
        }
    }
    return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis != nullptr)
    {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd);
    case WM_DESTROY:
        return OnDestroy();
    case WM_CLOSE:
        return OnClose();
    case WM_PAINT:
        return OnPaint();
    case WM_ERASEBKGND:
        return 1;
    case WM_KEYUP:
        return OnKeyUp(wParam, lParam);
    case WM_COMMAND:
        return OnCommand(wParam);
    case WM_TIMER:
        return OnTimer(wParam);
    case WM_MOUSEWHEEL:
        return OnMouseWheel(wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, lParam);
    case WM_MBUTTONUP:
        return OnMButtonUp(wParam, lParam);
    case EventMessage::kAudioPlayer:
        OnAudioPlayerEvent(static_cast<unsigned long>(lParam));
        break;
    default:
        break;
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    m_pD2Drawer = std::make_unique<CD2Drawer>(m_hWnd);
    m_pViewManager = std::make_unique<CViewManager>(m_hWnd);
    m_pMfAudioPlayer = std::make_unique<CMfMediaPlayer>(m_hWnd, EventMessage::kAudioPlayer);

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    ::PostQuitMessage(0);

    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_swzClassName, m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    if (m_pD2Drawer.get() != nullptr)
    {
        m_pD2Drawer->Clear();
        if (!m_imageInfo.empty() && m_pViewManager.get() != nullptr)
        {
            m_pD2Drawer->DrawImage(m_imageInfo.at(m_nImageIndex), { static_cast<float>(m_pViewManager->GetXOffset()), static_cast<float>(m_pViewManager->GetYOffset()) }, m_pViewManager->GetScale());
        }
        if (!m_textData.empty() && !m_bTextHidden)
        {
            const adv::TextDatum& t = m_textData.at(m_nTextIndex);
            std::wstring wstr = t.wstrText + L"\r\n " + std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
            m_pD2Drawer->DrawString(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
        }
        m_pD2Drawer->Display();
    }

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
    return 0;
}
/*WM_KEYUP*/
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_ESCAPE:
        ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        break;
    case VK_UP:
        MenuOnForeFolder();
        break;
    case VK_DOWN:
        MenuOnNextFolder();
        break;
    case 0x54:// T key
        m_bTextHidden ^= true;
        UpdateScreen();
        break;
    }
    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam)
{
    int wmKind = HIWORD(wParam);
    int wmId = LOWORD(wParam);
    if (wmKind == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kOpenFolder:
            MenuOnOpenFolder();
            break;
        case Menu::kAudioLoop:
            MenuOnAudioLoop();
            break;
        case Menu::kAudioSetting:
            MenuOnAudioSetting();
            break;
        default:
            break;
        }
    }
    if (wmKind > 1)
    {
        /*Controls*/
    }

    return 0;
}
/*WM_TIMER*/
LRESULT CMainWindow::OnTimer(WPARAM wParam)
{
    switch (wParam)
    {
    case Timer::kText:
        AutoTexting();
        break;
    default:
        break;
    }
    return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
    WORD wKey = LOWORD(wParam);

    if (wKey == 0)
    {
        if (m_pViewManager.get() != nullptr && !m_imageInfo.empty())
        {
            m_pViewManager->Rescale(iScroll > 0);
        }
    }

    if (wKey == MK_LBUTTON)
    {
        ShiftImage(iScroll > 0);
    }

    if (wKey == MK_RBUTTON)
    {
        ShiftText(iScroll > 0);
    }

    return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    ::GetCursorPos(&m_CursorPos);

    m_bLeftDowned = true;

    return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);

    if (usKey == MK_RBUTTON && m_bBarHidden)
    {
        ::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_DOWN;
        ::SendInput(1, &input, sizeof(input));
    }

    if (usKey == 0 && m_bLeftDowned)
    {
        POINT pt{};
        ::GetCursorPos(&pt);
        int iX = m_CursorPos.x - pt.x;
        int iY = m_CursorPos.y - pt.y;

        if (iX == 0 && iY == 0)
        {

        }
        else
        {
            if (m_pViewManager.get() != nullptr)
            {
                m_pViewManager->SetOffset(iX, iY);
            }
        }
    }

    m_bLeftDowned = false;

    return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);

    if (usKey == 0)
    {
        if (m_pViewManager.get() != nullptr)
        {
            m_pViewManager->ResetZoom();
        }
    }

    if (usKey == MK_RBUTTON)
    {
        SwitchWindowStyle();
    }

    return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
    HMENU hMenuFolder = nullptr;
    HMENU hMenuAudio = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    hMenuFolder = ::CreateMenu();
    if (hMenuFolder == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kOpenFolder, "Open");
    if (iRet == 0)goto failed;

    hMenuAudio = ::CreateMenu();
    if (hMenuAudio == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioLoop, "Loop");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioSetting, "Setting");
    if (iRet == 0)goto failed;

    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;

    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFolder), "Folder");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
    if (iRet == 0)goto failed;

    iRet = ::SetMenu(m_hWnd, hMenuBar);
    if (iRet == 0)goto failed;

    m_hMenuBar = hMenuBar;

    return;

failed:
    std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
    ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    if (hMenuFolder != nullptr)
    {
        ::DestroyMenu(hMenuFolder);
    }
    if (hMenuAudio != nullptr)
    {
        ::DestroyMenu(hMenuAudio);
    }
    if (hMenuBar != nullptr)
    {
        ::DestroyMenu(hMenuBar);
    }
}
/*フォルダ選択*/
void CMainWindow::MenuOnOpenFolder()
{
    if (m_pD2Drawer.get() != nullptr)
    {
        std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(m_hWnd);
        if (!wstrPickedFolder.empty())
        {
            SetupScenarioResources(wstrPickedFolder.c_str());
            CreateFolderList(wstrPickedFolder.c_str());
        }
    }
}
/*次のフォルダに移動*/
void CMainWindow::MenuOnNextFolder()
{
    if (m_folders.empty())return;

    ++m_nFolderIndex;
    if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
    SetupScenarioResources(m_folders.at(m_nFolderIndex).c_str());
}
/*前のフォルダに移動*/
void CMainWindow::MenuOnForeFolder()
{
    if (m_folders.empty())return;

    --m_nFolderIndex;
    if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
    SetupScenarioResources(m_folders.at(m_nFolderIndex).c_str());
}
/*音声ループ設定切り替え*/
void CMainWindow::MenuOnAudioLoop()
{
    if (m_pMfAudioPlayer.get() != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kAudio);
            if (hMenu != nullptr)
            {
                BOOL iRet = m_pMfAudioPlayer->SwitchLoop();
                ::CheckMenuItem(hMenu, Menu::kAudioLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*音量・再生速度変更*/
void CMainWindow::MenuOnAudioSetting()
{
    if (m_pMfAudioPlayer.get() != nullptr)
    {
        CMediaSettingDialogue sMediaSettingDialogue;
        sMediaSettingDialogue.Open(m_hInstance, m_hWnd, m_pMfAudioPlayer.get(), L"Audio");
    }
}
/*表示形式切り替え*/
void CMainWindow::SwitchWindowStyle()
{
    RECT rect;
    ::GetWindowRect(m_hWnd, &rect);
    LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    m_bBarHidden ^= true;

    if (m_bBarHidden)
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
        ::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
        ::SetMenu(m_hWnd, nullptr);
    }
    else
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
        ::SetMenu(m_hWnd, m_hMenuBar);
    }

    if (m_pViewManager.get() != nullptr)
    {
        m_pViewManager->OnStyleChanged();
    }
}
/*再生素材ファイル一覧作成*/
bool CMainWindow::SetupScenarioResources(const wchar_t* pwzFolderPath)
{
    bool bRet = CreateImageList(pwzFolderPath);
    CreateMessageList(pwzFolderPath);
    return bRet;
}
/*画像一覧作成*/
bool CMainWindow::CreateImageList(const wchar_t* pwzFolderPath)
{
    m_imageInfo.clear();
    m_nImageIndex = 0;

    std::vector<std::wstring> files;
    bool bRet = win_filesystem::CreateFilePathList(pwzFolderPath, L".jpg", files);
    if (bRet)
    {
        for (const auto& file : files)
        {
            ImageInfo s{};
            bRet = image_loader::LoadImageToMemory(file.c_str(), &s, 1.f);
            if (bRet)
            {
                m_imageInfo.emplace_back(s);
            }
        }
        if (!m_imageInfo.empty() && m_pViewManager.get() != nullptr)
        {
            const ImageInfo& s = m_imageInfo.at(0);
            m_pViewManager->SetBaseSize(s.uiWidth, s.uiHeight);
            m_pViewManager->ResetZoom();
        }
        return m_imageInfo.size() > 0;
    }
    return false;
}
/*文章一覧作成*/
bool CMainWindow::CreateMessageList(const wchar_t* pwzFolderPath)
{
    m_textData.clear();
    m_nTextIndex = 0;

    std::vector<std::wstring> textFiles;
    win_filesystem::CreateFilePathList(pwzFolderPath, L".txt", textFiles);
    if (!textFiles.empty())
    {
        mnsg::LoadScenario(textFiles.at(0), m_textData);
    }

    /*.txtなし、或いは読み取り失敗*/
    if (m_textData.empty())
    {
        std::vector<std::wstring> audioFiles;
        win_filesystem::CreateFilePathList(pwzFolderPath, L".mp3", audioFiles);
        for (const std::wstring& audioFile : audioFiles)
        {
            m_textData.emplace_back(adv::TextDatum{ L"", audioFile});
        }
    }

    UpdateText();

    return !m_textData.empty();
}
/*フォルダ一覧作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
    m_folders.clear();
    m_nFolderIndex = 0;
    win_filesystem::GetFolderListAndIndex(pwzFolderPath, m_folders, &m_nFolderIndex);

    return m_folders.size() > 0;
}
/*再描画要求*/
void CMainWindow::UpdateScreen()
{
    ::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*表示画像送り・戻し*/
void CMainWindow::ShiftImage(bool bForward)
{
    if (bForward)
    {
        ++m_nImageIndex;
        if (m_nImageIndex >= m_imageInfo.size())m_nImageIndex = 0;
    }
    else
    {
        --m_nImageIndex;
        if (m_nImageIndex >= m_imageInfo.size())m_nImageIndex = m_imageInfo.size() - 1;
    }
    UpdateScreen();
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
    if (bForward)
    {
        ++m_nTextIndex;
        if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
    }
    else
    {
        --m_nTextIndex;
        if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
    }
    UpdateText();
}
/*文章更新*/
void CMainWindow::UpdateText()
{
    if (!m_textData.empty())
    {
        const adv::TextDatum& t = m_textData.at(m_nTextIndex);
        if (!t.wstrVoicePath.empty())
        {
            if (m_pMfAudioPlayer.get() != nullptr)
            {
                m_pMfAudioPlayer->Play(t.wstrVoicePath.c_str());
            }

            ::KillTimer(m_hWnd, Timer::kText);
        }
        else
        {
            constexpr unsigned int kTimerInterval = 2000;
            ::SetTimer(m_hWnd, Timer::kText, kTimerInterval, nullptr);
        }
    }

    UpdateScreen();
}
/*IMFMediaEngineNotify::EventNotify*/
void CMainWindow::OnAudioPlayerEvent(unsigned long ulEvent)
{
    switch (ulEvent)
    {
    case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:

        break;
    case MF_MEDIA_ENGINE_EVENT_ENDED:
        AutoTexting();
        break;
    default:
        break;
    }
}
/*自動送り*/
void CMainWindow::AutoTexting()
{
    if (m_nTextIndex < m_textData.size() - 1)ShiftText(true);
}
