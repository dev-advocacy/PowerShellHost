#include "pch.h"
#include "CheckFailure.h"

void ShowFailure(HRESULT hr, const std::wstring& message)
{
    std::wstringstream formattedMessage;
    formattedMessage << message << ": 0x" << std::hex << std::setw(8) << hr;
    MessageBox(nullptr, formattedMessage.str().c_str(), nullptr, MB_OK);
}
void CheckFailure(HRESULT hr, const std::wstring& message)
{
    if (FAILED(hr))
    {
        ShowFailure(hr, message);
        FAIL_FAST();
    }
}
void FeatureNotAvailable()
{
    MessageBox(nullptr,
        L"This feature is not available in the browser version currently being used.",
        L"Feature Not Available", MB_OK);
}
