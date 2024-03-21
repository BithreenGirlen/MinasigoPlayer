

#include "view_manager.h"


CViewManager::CViewManager(HWND hWnd)
	:m_hRetWnd(hWnd)
{

}

CViewManager::~CViewManager()
{

}
/*基準長設定*/
void CViewManager::SetBaseSize(unsigned int uiWidth, unsigned int uiHeight)
{
	m_uiBaseWidth = uiWidth;
	m_uiBaseHeight = uiHeight;
	WorkOutDefaultScale();
}
/*尺度変更*/
void CViewManager::Rescale(bool bUpscale)
{
	constexpr float fScaleMin = 0.5f;
	constexpr float fScalePortion = 0.05f;
	if (bUpscale)
	{
		m_fScale += fScalePortion;
	}
	else
	{
		m_fScale -= fScalePortion;
		if (m_fScale < fScaleMin) m_fScale = fScaleMin;
	}
	ResizeWindow();
}
/*原点位置移動*/
void CViewManager::SetOffset(int iX, int iY)
{
	m_iXOffset += iX;
	m_iYOffset += iY;
	AdjustOffset();
	RequestRedraw();
}
/*原寸表示*/
void CViewManager::ResetZoom()
{
	m_fScale = 1.f;
	m_iXOffset = 0;
	m_iYOffset = 0;

	ResizeWindow();
}
/*表示形式変更通知*/
void CViewManager::OnStyleChanged()
{
	ResizeWindow();
}
/*基準尺度算出*/
void CViewManager::WorkOutDefaultScale()
{
	/*基準長がモニタ解像度より大きい場合には予め縮小する*/

	unsigned int uiMonitorWidth = static_cast<unsigned int>(::GetSystemMetricsForDpi(SM_CXSCREEN, ::GetDpiForWindow(m_hRetWnd)));
	unsigned int uiMonitorHeight = static_cast<unsigned int>(::GetSystemMetricsForDpi(SM_CYSCREEN, ::GetDpiForWindow(m_hRetWnd)));

	if (m_uiBaseWidth > uiMonitorWidth || m_uiBaseHeight > uiMonitorHeight)
	{
		if (uiMonitorWidth > uiMonitorHeight)
		{
			m_fDefaultScale = static_cast<float>(uiMonitorHeight) / m_uiBaseHeight;
			m_fThresholdScale = static_cast<float>(uiMonitorWidth) / m_uiBaseWidth;
		}
		else
		{
			m_fDefaultScale = static_cast<float>(uiMonitorWidth) / m_uiBaseWidth;
			m_fThresholdScale = static_cast<float>(uiMonitorHeight) / m_uiBaseHeight;
		}
		m_fScale = m_fDefaultScale;
	}
}
/*窓寸法調整*/
void CViewManager::ResizeWindow()
{
	if (m_hRetWnd != nullptr)
	{
		bool bBarHidden = bIsWidowBarHidden();
		RECT rect;
		if (!bBarHidden)
		{
			::GetWindowRect(m_hRetWnd, &rect);
		}
		else
		{
			::GetClientRect(m_hRetWnd, &rect);
		}

		int iX = static_cast<int>(m_uiBaseWidth * m_fScale);
		int iY = static_cast<int>(m_uiBaseHeight * m_fScale);
		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		if (!bBarHidden)
		{
			LONG lStyle = ::GetWindowLong(m_hRetWnd, GWL_STYLE);
			::AdjustWindowRect(&rect, lStyle, TRUE);
			::SetWindowPos(m_hRetWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			RECT rc;
			::GetWindowRect(m_hRetWnd, &rc);
			::MoveWindow(m_hRetWnd, rc.left, rc.top, rect.right, rect.bottom, TRUE);
		}
	}

	AdjustOffset();
	RequestRedraw();
}
/*原点位置調整*/
void CViewManager::AdjustOffset()
{
	if (m_hRetWnd != nullptr)
	{
		int iScaledWidth = static_cast<int>(m_uiBaseWidth * m_fScale);
		int iScaledHeight = static_cast<int>(m_uiBaseHeight * m_fScale);

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iXOffsetMax = iScaledWidth > iClientWidth ? static_cast<int>((iScaledWidth - iClientWidth)/ m_fScale) : 0;
		int iYOffsetMax = iScaledHeight > iClientHeight ? static_cast<int>((iScaledHeight - iClientHeight) / m_fScale) : 0;

		if (m_iXOffset < 0) m_iXOffset = 0;
		if (m_iYOffset < 0) m_iYOffset = 0;

		if (m_iXOffset > iXOffsetMax)m_iXOffset = iXOffsetMax;
		if (m_iYOffset > iYOffsetMax)m_iYOffset = iYOffsetMax;
	}
}
/*再描画要求*/
void CViewManager::RequestRedraw()
{
	if (m_hRetWnd != nullptr)
	{
		::InvalidateRect(m_hRetWnd, nullptr, FALSE);
	}
}
/*ウィンドウバー有無*/
bool CViewManager::bIsWidowBarHidden()
{
	if (m_hRetWnd != nullptr)
	{
		LONG lStyle = ::GetWindowLong(m_hRetWnd, GWL_STYLE);
		return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
	}
	return false;
}
