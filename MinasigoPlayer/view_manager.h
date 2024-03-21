#ifndef VIEW_MANAGER_H_
#define VIEW_MANAGER_H_

#include <Windows.h>

class CViewManager
{
public:
    CViewManager(HWND hWnd);
    ~CViewManager();

    void SetBaseSize(unsigned int uiWidth, unsigned int uiHeight);
    void Rescale(bool bUpscale);
    void SetOffset(int iX, int iY);
    void ResetZoom();
    void OnStyleChanged();

    float GetScale() const { return m_fScale; };
    int GetXOffset() const { return m_iXOffset; };
    int GetYOffset() const { return m_iYOffset; };
private:
    enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

    HWND m_hRetWnd = nullptr;

    unsigned int m_uiBaseWidth = Constants::kBaseWidth;
    unsigned int m_uiBaseHeight = Constants::kBaseHeight;
    float m_fDefaultScale = 1.f;
    float m_fThresholdScale = 1.f;

    float m_fScale = 1.f;
    int m_iXOffset = 0;
    int m_iYOffset = 0;

    void WorkOutDefaultScale();
    void ResizeWindow();
    void AdjustOffset();
    void RequestRedraw();

    bool bIsWidowBarHidden();
};

#endif // !VIEW_MANAGER_H_
