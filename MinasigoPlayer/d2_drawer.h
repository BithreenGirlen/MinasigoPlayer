#ifndef D2_DRAWER_H_
#define D2_DRAWER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dwrite_1.h>

#include <vector>

struct ImageInfo
{
	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	int iStride = 0;
	std::vector<unsigned char> pixels;
};

class CD2TextWriter;

class CD2Drawer
{
public:
	CD2Drawer(HWND hWnd);
	~CD2Drawer();

	void Clear(const D2D1::ColorF &colour = D2D1::ColorF(255, 255, 255, 255));
	bool DrawImage(const ImageInfo &imageInfo, D2D_VECTOR_2F fOffset, float fScale);
	bool DrawString(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
	void Display();
private:
	HWND m_hRetWnd = nullptr;

	HRESULT m_hrComInit = E_FAIL;
	ID2D1Factory1* m_pD2d1Factory1 = nullptr;
	ID2D1DeviceContext* m_pD2d1DeviceContext = nullptr;
	IDXGISwapChain1* m_pDxgiSwapChain1 = nullptr;
	ID2D1Bitmap* m_pD2d1Bitmap = nullptr;

	unsigned int m_uiWindowWidth = 0;
	unsigned int m_uiWindowHeight = 0;

	void ReleaseBitmap();
	bool CheckBitmapSize(const ImageInfo& imageInfo);
	bool CreateBitmapForDrawing(const ImageInfo& imageInfo);
	bool CheckBufferSize();
	bool ResizeBuffer();

	CD2TextWriter* m_pD2Text = nullptr;
};

namespace image_loader
{
	bool LoadImageToMemory(const wchar_t* wpzFilePath, ImageInfo* pImageInfo, float fScale);
}

class CD2TextWriter
{
public:
	CD2TextWriter(ID2D1Factory1* pD2d1Factory1, ID2D1DeviceContext* pD2d1DeviceContext);
	~CD2TextWriter();

	bool SetupOutLinedDrawing(const wchar_t* pwzFontFilePath);

	void NoBorderDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
	void OutLinedDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
	void LayedOutDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
private:
	ID2D1Factory1* m_pStoredD2d1Factory1 = nullptr;
	ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;

	IDWriteFactory* m_pDWriteFactory = nullptr;
	IDWriteTextFormat* m_pDWriteFormat = nullptr;
	IDWriteFontFace* m_pDWriteFontFace = nullptr;

	ID2D1SolidColorBrush* m_pD2d1SolidColorBrush = nullptr;
	ID2D1SolidColorBrush* m_pD2dSolidColorBrushForOutline = nullptr;

	const wchar_t* m_swzFontFamilyName = L"yumin";
	const float m_fStrokeWidth = 4.f;
	float m_fFontSize = 24.f;

	float PointSizeToDip(float fPointSize)const { return (fPointSize / 72.f) * 96.f; };

	bool SetupFont(const wchar_t * pwzFontFilePath);
	bool CreateBrushes();

	bool SingleLineGlyphDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_POINT_2F& fRawPos = D2D1_POINT_2F{});
};

#endif // !D2_DRAWER_H_
