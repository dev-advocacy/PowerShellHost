#pragma once

typedef struct ProfileInformation
{
    std::wstring  browserDirectory;
	std::wstring  userDataDirectory;

} ProfileInformation_t;

class CWebViewProfile
{
public:
	static HRESULT Profile(ProfileInformation_t& profile);
};

