

#include "framework.h"
#include "MinasigoPlayer.h"
#include "main_window.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	int iRet = 0;
	CMainWindow* pWindow = new CMainWindow();
	if (pWindow != nullptr)
	{
		bool bRet = pWindow->Create(hInstance, L"Minasigo player");
		if (bRet)
		{
			::ShowWindow(pWindow->GetHwnd(), nCmdShow);
			iRet = pWindow->MessageLoop();
		}
		delete pWindow;
	}

	return iRet;
}
