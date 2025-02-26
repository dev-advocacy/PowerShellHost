#include "pch.h"
#include "CheckFailure.h"
#include "osutility.h"


namespace os
{


   

    HRESULT svg::Init2D2()
    {
        HRESULT hr = S_OK;

        if (FAILED(hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(&m_pWICFactory))))
        {
            ATLTRACE(L"CoCreateInstance CLSID_WICImagingFactory failed");
            return hr;
        }


        if (FAILED(hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory)))
        {
            ATLTRACE(L"D2D1CreateFactory failed");
            return hr;
        }
        return (hr);
    }  
    HRESULT svg::GetSvgDocumentSize(std::wstring str_sourcefile, D2D1_SIZE_F& imagesize)
    {
        HRESULT						hr = S_OK;
        wil::com_ptr<ID2D1SvgDocument>	svgDocument = nullptr;
        wil::com_ptr<IStream>			pstream = nullptr;
        CComQIPtr<ID2D1SvgElement>	    svgelement = nullptr;
        wil::com_ptr<ID2D1RenderTarget> pRT = nullptr;
        wil::com_ptr<IStream>			ptemplate = nullptr;
        wil::com_ptr<IWICBitmap>		pWICBitmap = nullptr;
        FLOAT						    tmpheight = 0;
        FLOAT						    tmpwidth = 0;
        D2D1_SIZE_F					s = { 100,100 };

        if (FAILED(hr = m_pWICFactory->CreateBitmap(static_cast<UINT>(s.height), static_cast<UINT>(s.width), GUID_WICPixelFormat32bppBGR, WICBitmapCacheOnLoad, &pWICBitmap)))
        {
            OutputDebugString(L"pWICFactory failed");
            return (hr);
        }
        if (FAILED(hr = m_pD2DFactory->CreateWicBitmapRenderTarget(pWICBitmap.get(), D2D1::RenderTargetProperties(), &pRT)))
        {
            OutputDebugString(L"pD2DFactory failed");
            return (hr);
        }

        if (FAILED(hr = SHCreateStreamOnFileEx(str_sourcefile.c_str(), STGM_READ, 0, FALSE, ptemplate.get(), &pstream)))
        {            
            return (hr);
        }
        wil::com_ptr<ID2D1DeviceContext5> spDeviceContext5 = nullptr;
        pRT->QueryInterface(&spDeviceContext5);

        if (FAILED(hr = spDeviceContext5->CreateSvgDocument(pstream.get(), s, &svgDocument)))
        {            
            return hr;
        }

        svgDocument->GetRoot(&svgelement);

        D2D1_SVG_VIEWBOX viewBox;

        if (FAILED(hr = svgelement->GetAttributeValue(L"viewBox", D2D1_SVG_ATTRIBUTE_POD_TYPE_VIEWBOX, static_cast<void*>(&viewBox), sizeof(viewBox))))
        {
            viewBox.height = viewBox.width = 0;
        }

        if (FAILED(hr = svgelement->GetAttributeValue(L"width", &tmpwidth)))
        {
            tmpwidth = 0;
        }

        if (FAILED(hr = svgelement->GetAttributeValue(L"height", &tmpheight)))
        {

            tmpheight = 0;
        }

        if (viewBox.height != 0 && viewBox.width != 0)
        {
            imagesize.height = viewBox.height;
            imagesize.width = viewBox.width;
        }

        if (tmpheight != 0 && tmpwidth != 0)
        {
            imagesize.height = tmpheight;
            imagesize.width = tmpwidth;
        }

        if (imagesize.width == 0 && imagesize.height == 0)
        {
            return E_FAIL;
        }

        return hr;
    }
    HRESULT svg::DrawImage(wil::com_ptr<ID2D1RenderTarget> pRT, std::wstring str_sourcefile, D2D1_SIZE_F imagesize, FLOAT factor)
    {
        IStream* pstream = nullptr;
        IStream* ptemplate = nullptr;
        HRESULT hr = S_OK;

        if (FAILED(hr = SHCreateStreamOnFileEx(str_sourcefile.c_str(), STGM_READ, 0, FALSE, ptemplate, &pstream)))
        {
            return hr;
        }

        wil::com_ptr<ID2D1DeviceContext5> spDeviceContext = nullptr;
            
        pRT->QueryInterface(&spDeviceContext);

        D2D1_SIZE_F s = { imagesize.height, imagesize.width };
        wil::com_ptr<ID2D1SvgDocument> svgDocument;

        if (FAILED(hr = spDeviceContext->CreateSvgDocument(pstream, s, &svgDocument)))
        {
            return hr;
        }
        //FLOAT radio = (svg_Height * 100) / (FLOAT) size.cx;

        D2D1_SIZE_F viewportsize = svgDocument->GetViewportSize();
        auto transform = D2D1::Matrix3x2F::Scale(D2D1::SizeF(factor, factor), D2D1::Point2F(0, 0));
        spDeviceContext->SetTransform(transform);

        if (svgDocument == nullptr)
        {
            return hr;
        }
        spDeviceContext->DrawSvgDocument(svgDocument.get());
        spDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
        return (hr);
    }
    void svg::SaveImage(std::wstring str_sourcefile, std::wstring str_destfile, D2D1_SIZE_F imagesize, FLOAT factor)
    {
        
        HRESULT hr = S_OK;
        wil::com_ptr<ID2D1RenderTarget> pRT = nullptr;

        if (m_pWICBitmap != nullptr)
        {
            m_pWICBitmap = nullptr;
        }

        if (FAILED(hr = m_pWICFactory->CreateBitmap(static_cast<UINT>(imagesize.width * factor), static_cast<UINT>(imagesize.height * factor), GUID_WICPixelFormat32bppBGR, WICBitmapCacheOnLoad, &m_pWICBitmap)))
        {
            OutputDebugString(L"pWICFactory failed");
            return;
        }

        if (FAILED(hr = m_pD2DFactory->CreateWicBitmapRenderTarget(m_pWICBitmap.get(), D2D1::RenderTargetProperties(), &pRT)))
        {
            OutputDebugString(L"pD2DFactory failed");
            return;
        }

        pRT->BeginDraw();
        pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
        DrawImage(pRT, str_sourcefile, imagesize, factor);
        hr = pRT->EndDraw();
        ExportImage(str_destfile, imagesize, factor);
    }
    void svg::ExportImage(std::wstring str_destfile, D2D1_SIZE_F imagesize, FLOAT factor)
    {
        HRESULT hr;
        wil::com_ptr<IWICStream> pStream = nullptr;
        wil::com_ptr<IWICBitmapEncoder> pEncoder = nullptr;
        wil::com_ptr<IWICBitmapFrameEncode> pFrameEncode = nullptr;

        if (FAILED(hr = m_pWICFactory->CreateStream(&pStream)))
        {
            OutputDebugString(L"CreateStream failed");
            return;
        }

        WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;

        if (FAILED(hr = pStream->InitializeFromFilename(str_destfile.c_str(), GENERIC_WRITE)))
        {
            OutputDebugString(L"InitializeFromFilename failed");
            return;
        }
        if (FAILED(hr = m_pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder)))
        {
            OutputDebugString(L"CreateEncoder failed");
            return;
        }
        if (FAILED(hr = pEncoder->Initialize(pStream.get(), WICBitmapEncoderNoCache)))
        {
            OutputDebugString(L"Initialize failed");
            return;
        }
        if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL)))
        {
            OutputDebugString(L"CreateNewFrame failed");
            return;
        }
        if (FAILED(hr = pFrameEncode->Initialize(NULL)))
        {
            OutputDebugString(L"Initialize failed");
            return;
        }
        if (FAILED(hr = pFrameEncode->SetSize(static_cast<UINT>(imagesize.width * factor), static_cast<UINT>(imagesize.height * factor))))
        {
            OutputDebugString(L"SetSize failed");
            return;
        }
        if (FAILED(hr = pFrameEncode->SetPixelFormat(&format)))
        {
            OutputDebugString(L"SetPixelFormat failed");
            return;
        }
        if (FAILED(hr = pFrameEncode->WriteSource(m_pWICBitmap.get(), NULL)))
        {
            OutputDebugString(L"WriteSource failed");
            return;
        }
        if (FAILED(hr = pFrameEncode->Commit()))
        {
            OutputDebugString(L"Commit failed");
            return;
        }
        if (FAILED(hr = pEncoder->Commit()))
        {
            OutputDebugString(L"Commit failed");
            return;
        }

    }
    /// <summary>
    /// Return the SVG folder
    /// </summary>
    /// <returns></returns>
    std::wstring utility::GetSvgFolder()
    {
        TCHAR buffer[MAX_PATH * sizeof(TCHAR)];
        std::wstring wstrPath;

        GetModuleFileName(nullptr, buffer, MAX_PATH);
        std::wstring::size_type pos = std::wstring(buffer).find_last_of(_T("\\/"));

        wstrPath = std::wstring(buffer).substr(0, pos);

        return wstrPath + _T("\\") + L"media\\svg";
    }

    std::wstring utility::GetimageFolder(imagesize isize)
    {
        TCHAR buffer[MAX_PATH * sizeof(TCHAR)];
        std::wstring wstrPath;

        GetModuleFileName(nullptr, buffer, MAX_PATH);
        std::wstring::size_type pos = std::wstring(buffer).find_last_of(_T("\\/"));

        wstrPath = std::wstring(buffer).substr(0, pos);

        if (isize == imagesize::size24)
        {
            return wstrPath + _T("\\") + L"media\\im24";
        }
        if (isize == imagesize::size32)
        {
            return wstrPath + _T("\\") + L"media\\im32";
        }
        return std::wstring(L"");
        
    }

    void utility::GetImages(imagesize isize)
    {
        
        D2D1_SIZE_F size = { 24,24 };

       fs::path svrfolder = GetSvgFolder();

       if (fs::exists(svrfolder))
       {
           for (const auto& entry : fs::directory_iterator(svrfolder))
           {
               if (fs::path(entry).extension() == ".svg")
               {
                    auto imfolder = GetimageFolder(isize);
                    auto imfile = fs::path(entry).stem();

                    std::wstring sourceimg = entry.path().native();
                    std::wstring destimage = imfolder + L"\\" + imfile.native() + L".bmp";
                    if (!fs::exists(imfolder))
                       fs::create_directories(imfolder);

                    if (fs::exists(sourceimg) && !fs::exists(destimage))
                    {
                        svg util;
                        D2D1_SIZE_F imagesize;
                        HRESULT hr;

                        util.Init2D2();

                        hr = util.GetSvgDocumentSize(sourceimg, imagesize);

                        util.SaveImage(sourceimg, destimage, size, 1.0f);
                    }
               }

               
           }
               
       }

    }


    colortheme::colortheme()
    {
        m_brush_dark.CreateSolidBrush(RGB(74, 74, 74));
        m_brush_light.CreateSolidBrush(RGB(255, 255, 255));
    }

    void utility::SetWindowBackgroud(HWND hwnd)
    {
        ColorMode mode = os::theme::get_instance()->get_color_mode();

        BOOL btrue = TRUE;
        BOOL bfalse = FALSE;
        if (::IsWindow(hwnd))
        {
            mode == ColorMode::Dark ? ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &btrue, sizeof(btrue)) :
                                      ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bfalse, sizeof(bfalse));
        }
    }

    LRESULT colortheme::SetWindowBackgroudColor(WPARAM wParam)
    {
        ColorMode mode = os::theme::get_instance()->get_color_mode();

        SetBkMode((HDC)wParam, TRANSPARENT);

        if (mode == ColorMode::Dark)
        {
            SetTextColor((HDC)wParam, RGB(255, 255, 255));
            return (LRESULT)(m_brush_dark.m_hBrush);
        }
        else
        {
            SetTextColor((HDC)wParam, RGB(0, 0, 0));
            return (LRESULT)(m_brush_light.m_hBrush);
        }
    }

    
   



    

 
    
}