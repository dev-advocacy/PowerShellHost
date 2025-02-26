#pragma once

#include <memory>


class HostfxrContextSingleton {
public:
    static HostfxrContextSingleton& getInstance() {
        static HostfxrContextSingleton instance;
        return instance;
    }

    HOSTFXR_CONTEXT* getContext()
    {
		if (_hostfxr_context == nullptr) {
			_hostfxr_context = new HOSTFXR_CONTEXT();
		}
        return _hostfxr_context;
    }

    void setContext(HOSTFXR_CONTEXT *context) {
        _hostfxr_context = context;
    }

private:
    ~HostfxrContextSingleton() {
        if (_hostfxr_context != nullptr) {
            delete _hostfxr_context;
            _hostfxr_context = nullptr;
        }
    }

private:
    HostfxrContextSingleton() = default;
    HostfxrContextSingleton(const HostfxrContextSingleton&) = delete;
    HostfxrContextSingleton& operator=(const HostfxrContextSingleton&) = delete;
    HOSTFXR_CONTEXT* _hostfxr_context = nullptr;
};
