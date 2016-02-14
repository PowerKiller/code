#ifndef _CEF_LAYER_MANAGER_H
#define _CEF_LAYER_MANAGER_H

#include <list>

#include "include/cef_app.h"
#include "include/cef_runnable.h"

#include "inexor/ui/cefcontextprovider.hpp"
#include "inexor/ui/ceflayer.hpp"
#include "inexor/ui/ceflayerprovider.hpp"
#include "inexor/util/Logging.hpp"

class InexorCefLayerManager : public InexorCefContextProvider
{

    public:
        InexorCefLayerManager(int width, int height);

        // Rendering
        void Render();
        void RenderLayer(std::string name);
        void SetScreenSize(int width, int height);

        // Layers
        void InitializeLayers();
        void DestroyLayers();
        void AddLayerProvider(CefRefPtr<InexorCefLayerProvider> layer_provider);

        CefRefPtr<InexorCefLayer> CreateLayer(std::string name, std::string url);
        CefRefPtr<InexorCefLayer> CreateLayer(std::string name, int x, int y, int width, int height, std::string url);
        CefRefPtr<InexorCefLayer> GetLayer(std::string name);
        std::list<std::string> GetLayers();
        bool LayerExists(std::string name);
        void ShowLayer(std::string name);
        void HideLayer(std::string name);
        void BringToFront(std::string name);
        void SendToBack(std::string name);
        void BringForward(std::string name);
        void SendBackward(std::string name);

        // Input events
        void SendKeyEvent(CefKeyEvent event);
        void SendMouseClickEvent(const CefMouseEvent& event, CefBrowserHost::MouseButtonType type, bool mouseUp, int clickCount);
        void SendMouseMoveEvent(const CefMouseEvent& event, bool mouseLeave);
        void SendMouseWheelEvent(const CefMouseEvent& event, int deltaX, int deltaY);

        // InexorCefContextProvider
        void InitializeContext();
        bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception);
        bool Get(const CefString& name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval, CefString& exception);
        bool Set(const CefString& name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception);
        std::string GetContextName() { return "layer"; };

    private:
        std::list<CefRefPtr<InexorCefLayer> > layers;
        std::list<CefRefPtr<InexorCefLayerProvider> > layer_providers;

        int width;
        int height;

        std::list<CefRefPtr<InexorCefLayer> >::iterator GetIterator(std::string name);
        void _CreateLayer(std::string name, std::string url);

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(InexorCefLayerManager);

};

#endif  // _CEF_LAYER_MANAGER_H
