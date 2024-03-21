
#include <atlbase.h>
#include <wincodec.h>

#include "d2_drawer.h"

CD2Drawer::CD2Drawer(HWND hWnd)
	:m_hRetWnd(hWnd)
{
	m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(m_hrComInit))return;

	CComPtr<ID3D11Device>pD3d11Device;
	HRESULT hr = ::D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED, nullptr, 0, D3D11_SDK_VERSION,
		&pD3d11Device, nullptr, nullptr);
	if (FAILED(hr))return;

	CComPtr<IDXGIDevice1> pDxgDevice1;
	hr = pD3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDxgDevice1);
	if (FAILED(hr))return;

	hr = pDxgDevice1->SetMaximumFrameLatency(1);
	if (FAILED(hr))return;

	CComPtr<IDXGIAdapter> pDxgiAdapter;
	hr = pDxgDevice1->GetAdapter(&pDxgiAdapter);
	if (FAILED(hr))return;

	CComPtr<IDXGIFactory2> pDxgiFactory2;
	hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory2));
	if (FAILED(hr))return;

	hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2d1Factory1);
	if (FAILED(hr))return;

	CComPtr<ID2D1Device> pD2d1Device;
	hr = m_pD2d1Factory1->CreateDevice(pDxgDevice1, &pD2d1Device);
	if (FAILED(hr))return;

	hr = pD2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2d1DeviceContext);
	if (FAILED(hr))return;

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	hr = pDxgiFactory2->CreateSwapChainForHwnd(pDxgDevice1, m_hRetWnd, &desc, nullptr, nullptr, &m_pDxgiSwapChain1);
	if (FAILED(hr))return;

	m_pD2d1DeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_pD2d1DeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
	m_pD2d1DeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
	D2D1_RENDERING_CONTROLS sRenderings{};
	m_pD2d1DeviceContext->GetRenderingControls(&sRenderings);
	sRenderings.bufferPrecision = D2D1_BUFFER_PRECISION_8BPC_UNORM_SRGB;
	m_pD2d1DeviceContext->SetRenderingControls(sRenderings);
}

CD2Drawer::~CD2Drawer()
{
	if (m_pD2Text != nullptr)
	{
		delete m_pD2Text;
		m_pD2Text = nullptr;
	}

	ReleaseBitmap();

	if (m_pDxgiSwapChain1 != nullptr)
	{
		m_pDxgiSwapChain1->Release();
		m_pDxgiSwapChain1 = nullptr;
	}

	if (m_pD2d1DeviceContext != nullptr)
	{
		m_pD2d1DeviceContext->Release();
		m_pD2d1DeviceContext = nullptr;
	}

	if (m_pD2d1Factory1 != nullptr)
	{
		m_pD2d1Factory1->Release();
		m_pD2d1Factory1 = nullptr;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
	}
}

/*âÊñ è¡ãé*/
void CD2Drawer::Clear(const D2D1::ColorF& colour)
{
	if (m_pD2d1DeviceContext != nullptr)
	{
		bool bRet = CheckBufferSize();
		if (!bRet)return;
		m_pD2d1DeviceContext->BeginDraw();
		m_pD2d1DeviceContext->Clear(colour);
		m_pD2d1DeviceContext->EndDraw();
	}
}
/*âÊëúï`âÊ*/
bool CD2Drawer::DrawImage(const ImageInfo& imageInfo, D2D_VECTOR_2F fOffset, float fScale)
{
	if ( m_pD2d1DeviceContext == nullptr || m_pDxgiSwapChain1 == nullptr)
	{
		return false;
	}

	bool bRet = CheckBitmapSize(imageInfo);
	if (!bRet)return false;

	bRet = CheckBufferSize();
	if (!bRet)return false;

	const ImageInfo& s = imageInfo;
	HRESULT hr = E_FAIL;
	UINT uiWidth = s.uiWidth;
	UINT uiHeight = s.uiHeight;
	INT iStride = s.iStride;

	D2D1_RECT_U rc = { 0, 0, uiWidth, uiHeight };
	hr = m_pD2d1Bitmap->CopyFromMemory(&rc, s.pixels.data(), s.iStride);
	if (SUCCEEDED(hr))
	{
		CComPtr<ID2D1Effect> pD2d1Effect;
		hr = m_pD2d1DeviceContext->CreateEffect(CLSID_D2D1Scale, &pD2d1Effect);
		pD2d1Effect->SetInput(0, m_pD2d1Bitmap);
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_CENTER_POINT, fOffset);
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(fScale, fScale));
		m_pD2d1DeviceContext->BeginDraw();
		m_pD2d1DeviceContext->DrawImage(pD2d1Effect, D2D1::Point2F(0.f, 0.f), D2D1::RectF(fOffset.x, fOffset.y, uiWidth * fScale, uiHeight * fScale), D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, D2D1_COMPOSITE_MODE_SOURCE_COPY);
		m_pD2d1DeviceContext->EndDraw();
	}

	return SUCCEEDED(hr);
}
/*ï∂éöóÒï`âÊ*/
bool CD2Drawer::DrawString(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pD2d1DeviceContext == nullptr || m_pDxgiSwapChain1 == nullptr)
	{
		return false;
	}

	if (m_pD2Text == nullptr)
	{
		m_pD2Text = new CD2TextWriter(m_pD2d1Factory1, m_pD2d1DeviceContext);
		m_pD2Text->SetupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");
	}

	m_pD2d1DeviceContext->BeginDraw();
	if (m_pD2Text != nullptr)
	{
		D2D1_RECT_F fRect = { 0.f, 0.f, static_cast<float>(m_uiWindowWidth), m_uiWindowWidth /4.f };
		m_pD2Text->OutLinedDraw(wszText, ulTextLength, fRect);
	}
	m_pD2d1DeviceContext->EndDraw();
	return false;
}
/*ì]é */
void CD2Drawer::Display()
{
	if (m_pDxgiSwapChain1 != nullptr)
	{
		DXGI_PRESENT_PARAMETERS params{};
		m_pDxgiSwapChain1->Present1(1, 0, &params);
	}
}
/*ï°é ògâï˙*/
void CD2Drawer::ReleaseBitmap()
{
	if (m_pD2d1Bitmap != nullptr)
	{
		m_pD2d1Bitmap->Release();
		m_pD2d1Bitmap = nullptr;
	}
}
/*ï°é ògê°ñ@ämîF*/
bool CD2Drawer::CheckBitmapSize(const ImageInfo& imageInfo)
{
	if (m_pD2d1Bitmap == nullptr)
	{
		return CreateBitmapForDrawing(imageInfo);
	}
	else
	{
		const D2D1_SIZE_U& uBitmapSize = m_pD2d1Bitmap->GetPixelSize();
		if (imageInfo.uiWidth > uBitmapSize.width && imageInfo.uiHeight > uBitmapSize.height)
		{
			return CreateBitmapForDrawing(imageInfo);
		}
		else
		{
			return true;
		}
	}
	return false;
}
/*ï°é ògçÏê¨*/
bool CD2Drawer::CreateBitmapForDrawing(const ImageInfo& imageInfo)
{
	ReleaseBitmap();

	UINT uiWidth = imageInfo.uiWidth;
	UINT uiHeight = imageInfo.uiHeight;

	HRESULT hr = m_pD2d1DeviceContext->CreateBitmap(D2D1::SizeU(uiWidth, uiHeight),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&m_pD2d1Bitmap);

	return SUCCEEDED(hr);
}
/*å¥î≈ê°ñ@ämîF*/
bool CD2Drawer::CheckBufferSize()
{
	RECT rc;
	::GetClientRect(m_hRetWnd, &rc);

	unsigned int uiWidth = rc.right - rc.left;
	unsigned int uiHeight = rc.bottom - rc.top;

	if (m_uiWindowWidth != uiWidth || m_uiWindowHeight != uiHeight)
	{
		m_uiWindowWidth = uiWidth;
		m_uiWindowHeight = uiHeight;
		return ResizeBuffer();
	}
	else
	{
		return true;
	}
	return false;
}
/*å¥î≈ê°ñ@ïœçX*/
bool CD2Drawer::ResizeBuffer()
{
	if (m_pDxgiSwapChain1 != nullptr && m_pD2d1DeviceContext != nullptr && m_hRetWnd != nullptr)
	{
		m_pD2d1DeviceContext->SetTarget(nullptr);

		HRESULT hr = m_pDxgiSwapChain1->ResizeBuffers(0, m_uiWindowWidth, m_uiWindowHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

		CComPtr<IDXGISurface> pDxgiSurface;
		hr = m_pDxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&pDxgiSurface));

		CComPtr<ID2D1Bitmap1> pD2d1Bitmap1;
		hr = m_pD2d1DeviceContext->CreateBitmapFromDxgiSurface(pDxgiSurface, nullptr, &pD2d1Bitmap1);

		m_pD2d1DeviceContext->SetTarget(pD2d1Bitmap1);
		return SUCCEEDED(hr);
	}
	return false;
}
// class CD2Drawer

/// <summary>
/// âÊëfèÓïÒéÊÇËçûÇ›
/// </summary>
/// <param name="wpzFilePath: ">éZí†åoòH</param>
/// <param name="pImageInfo: ">âÊëfèÓïÒèëÇ´çûÇ›êÊ</param>
/// <param name="fScale: ">ägèkìx</param>
/// <returns>true: ê¨å˜, false: é∏îs</returns>
bool image_loader::LoadImageToMemory(const wchar_t* wpzFilePath, ImageInfo* pImageInfo, float fScale)
{
	if (pImageInfo == nullptr)return false;

	ImageInfo *s = pImageInfo;

	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
	hr = pWicImageFactory->CreateDecoderFromFilename(wpzFilePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWicBitmapDecoder);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapFrameDecode> pWicFrameDecode;
	hr = pWicBitmapDecoder->GetFrame(0, &pWicFrameDecode);
	if (FAILED(hr))return false;

	CComPtr<IWICFormatConverter> pWicFormatConverter;
	hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
	if (FAILED(hr))return false;

	pWicFormatConverter->Initialize(pWicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr))return false;

	hr = pWicFormatConverter->GetSize(&s->uiWidth, &s->uiHeight);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapScaler> pWicBmpScaler;
	hr = pWicImageFactory->CreateBitmapScaler(&pWicBmpScaler);
	if (FAILED(hr))return false;

	hr = pWicBmpScaler->Initialize(pWicFormatConverter, static_cast<UINT>(s->uiWidth * fScale), static_cast<UINT>(s->uiHeight * fScale), WICBitmapInterpolationMode::WICBitmapInterpolationModeCubic);
	if (FAILED(hr))return false;
	hr = pWicBmpScaler.p->GetSize(&s->uiWidth, &s->uiHeight);

	CComPtr<IWICBitmap> pWicBitmap;
	hr = pWicImageFactory->CreateBitmapFromSource(pWicBmpScaler, WICBitmapCacheOnDemand, &pWicBitmap);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapLock> pWicBitmapLock;
	WICRect wicRect{ 0, 0, static_cast<INT>(s->uiWidth), static_cast<INT>(s->uiHeight) };
	hr = pWicBitmap->Lock(&wicRect, WICBitmapLockRead, &pWicBitmapLock);
	if (FAILED(hr))return false;

	UINT uiStride;
	hr = pWicBitmapLock->GetStride(&uiStride);
	if (FAILED(hr))return false;

	s->iStride = static_cast<INT>(uiStride);
	s->pixels.resize(static_cast<size_t>(s->iStride * s->uiHeight));
	hr = pWicBitmap->CopyPixels(nullptr, uiStride, static_cast<UINT>(s->pixels.size()), s->pixels.data());
	if (FAILED(hr))return false;

	return true;
}
// namespace image_loader

CD2TextWriter::CD2TextWriter(ID2D1Factory1* pD2d1Factory1, ID2D1DeviceContext* pD2d1DeviceContext)
	:m_pStoredD2d1Factory1(pD2d1Factory1), m_pStoredD2d1DeviceContext(pD2d1DeviceContext)
{
	HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
	if (FAILED(hr))return;

	hr = m_pDWriteFactory->CreateTextFormat(m_swzFontFamilyName, nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL,
		PointSizeToDip(m_fFontSize), L"", &m_pDWriteFormat);
	if (FAILED(hr))return;

	CreateBrushes();
}

CD2TextWriter::~CD2TextWriter()
{
	if (m_pD2dSolidColorBrushForOutline != nullptr)
	{
		m_pD2dSolidColorBrushForOutline->Release();
		m_pD2dSolidColorBrushForOutline = nullptr;
	}

	if (m_pD2d1SolidColorBrush != nullptr)
	{
		m_pD2d1SolidColorBrush->Release();
		m_pD2d1SolidColorBrush = nullptr;
	}

	if (m_pDWriteFontFace != nullptr)
	{
		m_pDWriteFontFace->Release();
		m_pDWriteFontFace = nullptr;
	}

	if (m_pDWriteFormat != nullptr)
	{
		m_pDWriteFormat->Release();
		m_pDWriteFormat = nullptr;
	}

	if (m_pDWriteFactory != nullptr)
	{
		m_pDWriteFactory->Release();
		m_pDWriteFactory = nullptr;
	}
}
/*âèóLÇËï`âÊéñëOê›íË*/
bool CD2TextWriter::SetupOutLinedDrawing(const wchar_t* pwzFontFilePath)
{
	if (m_pDWriteFontFace == nullptr)
	{
		bool bRet = SetupFont(pwzFontFilePath);
		if (!bRet)return false;
	}

	m_pStoredD2d1DeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

	return true;
}
/*íPèÉï`âÊ*/
void CD2TextWriter::NoBorderDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}
	m_pStoredD2d1DeviceContext->DrawText(wszText, ulTextLength, m_pDWriteFormat, &rect, m_pD2d1SolidColorBrush);
}
/*âèóLÇËï`âÊ*/
void CD2TextWriter::OutLinedDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr
		|| m_pD2d1SolidColorBrush == nullptr || m_pD2dSolidColorBrushForOutline == nullptr
		|| m_pDWriteFontFace == nullptr)
	{
		return;
	}

	/*ëºÇÃï`âÊñ@Ç∆à·Ç¡Çƒêßå‰ÉRÅ[ÉhÇ‡ï∂éöóÒÇ∆ÇµÇƒå©ÇƒÇµÇ‹Ç§ÇÃÇ≈àÍçsñàÇ…ï`âÊÇ∑ÇÈÅB*/
	if (wszText == nullptr)return;

	std::vector<std::vector<wchar_t>> lines;
	for (size_t nRead = 0, nLen = 0;;)
	{
		const wchar_t* p = wcsstr(&wszText[nRead], L"\r\n");
		if (p == nullptr)
		{
			nLen = ulTextLength - nRead;
			std::vector<wchar_t> wchars;
			wchars.reserve(nLen);
			for (size_t i = nRead; i < ulTextLength; ++i)
			{
				wchars.push_back(wszText[i]);
			}
			lines.push_back(wchars);
			break;
		}
		nLen = p - &wszText[nRead];
		std::vector<wchar_t> wchars;
		wchars.reserve(nLen);
		for (size_t i = 0; i < nLen; ++i)
		{
			wchars.push_back(wszText[nRead + i]);
		}
		lines.push_back(wchars);
		nRead += nLen + 2;
	}

	for (size_t i = 0; i < lines.size(); ++i)
	{
		D2D1_POINT_2F fPos{ rect.left, rect.top + i * PointSizeToDip(m_fFontSize) };
		SingleLineGlyphDraw(lines.at(i).data(), static_cast<unsigned long>(lines.at(i).size()), fPos);
	}
}
/*ï∂éöä‘äuéwíËï`âÊ*/
void CD2TextWriter::LayedOutDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}

	CComPtr<IDWriteTextLayout>pDWriteTextLayout;
	HRESULT hr = m_pDWriteFactory->CreateTextLayout(wszText, ulTextLength, m_pDWriteFormat, rect.right - rect.left, rect.bottom - rect.top, &pDWriteTextLayout);
	CComPtr<IDWriteTextLayout1>pDWriteTextLayout1;
	hr = pDWriteTextLayout->QueryInterface(__uuidof(IDWriteTextLayout1), (void**)&pDWriteTextLayout1);

	DWRITE_TEXT_RANGE sRange{ 0, ulTextLength };
	hr = pDWriteTextLayout1->SetCharacterSpacing(1.f, 1.f, 2.f, sRange);
	pDWriteTextLayout1->SetFontWeight(DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_EXTRA_BOLD, sRange);

	m_pStoredD2d1DeviceContext->DrawTextLayout(D2D1_POINT_2F{ rect.left, rect.top }, pDWriteTextLayout1, m_pD2d1SolidColorBrush);
}
/*éöëÃÉtÉ@ÉCÉãê›íË*/
bool CD2TextWriter::SetupFont(const wchar_t* pwzFontFilePath)
{
	if (m_pDWriteFactory == nullptr)return false;

	CComPtr<IDWriteFontFile> pDWriteFontFile;
	HRESULT hr = m_pDWriteFactory->CreateFontFileReference(pwzFontFilePath, nullptr, &pDWriteFontFile);
	if (FAILED(hr))return false;

	IDWriteFontFile* pDWriteFontFiles[] = { pDWriteFontFile };
	hr = m_pDWriteFactory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE, 1U, pDWriteFontFiles, 0, DWRITE_FONT_SIMULATIONS_BOLD | DWRITE_FONT_SIMULATIONS_OBLIQUE, &m_pDWriteFontFace);

	return SUCCEEDED(hr);
}
/*ìhÇËÇ¬Ç‘ÇµêFçÏê¨*/
bool CD2TextWriter::CreateBrushes()
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	if (m_pD2d1SolidColorBrush != nullptr)
	{
		m_pD2d1SolidColorBrush->Release();
		m_pD2d1SolidColorBrush = nullptr;
	}
	if (m_pD2dSolidColorBrushForOutline != nullptr)
	{
		m_pD2dSolidColorBrushForOutline->Release();
		m_pD2dSolidColorBrushForOutline = nullptr;
	}

	HRESULT hr = m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pD2d1SolidColorBrush);
	if (SUCCEEDED(hr))
	{
		hr = m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pD2dSolidColorBrushForOutline);
	}

	return SUCCEEDED(hr);
}
/*àÍçsí§çè*/
bool CD2TextWriter::SingleLineGlyphDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_POINT_2F& fRawPos)
{
	std::vector<UINT32> codePoints;
	codePoints.reserve(ulTextLength);
	for (unsigned long i = 0; i < ulTextLength; ++i)
	{
		codePoints.push_back(wszText[i]);
	}

	std::vector<UINT16> glyphai;
	glyphai.resize(ulTextLength);
	HRESULT hr = m_pDWriteFontFace->GetGlyphIndicesW(codePoints.data(), static_cast<unsigned long>(codePoints.size()), glyphai.data());
	if (FAILED(hr))return false;

	CComPtr<ID2D1PathGeometry>pD2d1PathGeometry;
	hr = m_pStoredD2d1Factory1->CreatePathGeometry(&pD2d1PathGeometry);
	if (FAILED(hr))return false;

	CComPtr<ID2D1GeometrySink> pD2d1GeometrySink;
	hr = pD2d1PathGeometry->Open(&pD2d1GeometrySink);
	if (FAILED(hr))return false;

	pD2d1GeometrySink->SetFillMode(D2D1_FILL_MODE::D2D1_FILL_MODE_WINDING);
	pD2d1GeometrySink->SetSegmentFlags(D2D1_PATH_SEGMENT::D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);

	hr = m_pDWriteFontFace->GetGlyphRunOutline(PointSizeToDip(m_fFontSize), glyphai.data(), nullptr, nullptr, static_cast<unsigned long>(glyphai.size()), FALSE, FALSE, pD2d1GeometrySink);
	if (FAILED(hr))return false;

	pD2d1GeometrySink->Close();

	D2D1_RECT_F fGeoRect{};
	pD2d1PathGeometry->GetBounds(nullptr, &fGeoRect);
	D2D1_POINT_2F fPos = { fRawPos.x - fGeoRect.left, fRawPos.y - fGeoRect.top };
	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(fPos.x, fPos.y));
	m_pStoredD2d1DeviceContext->DrawGeometry(pD2d1PathGeometry, m_pD2dSolidColorBrushForOutline, PointSizeToDip(m_fStrokeWidth));
	m_pStoredD2d1DeviceContext->FillGeometry(pD2d1PathGeometry, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0.f, 0.f));
	return true;
}
