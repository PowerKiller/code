#include <iostream>

#include "inexor/ui/ceflayer.hpp"

InexorCefLayer::InexorCefLayer(std::string name, int x, int y, int width, int height, std::string url)
    : name(name),
      url(url),
      is_visible(false),
      is_accepting_input(false),
      has_focus(false),
      browser_id(-1),
      browser_count(0),
      is_closing(false)
{

    window_info.x = x;
    window_info.y = y;
    window_info.width = width;
    window_info.height = height;
    cookie_manager = CefCookieManager::CreateManager("/tmp/inexorc", false);
    render_handler = new InexorCefRenderHandler(true, x, y, width, height);
    browser = CefBrowserHost::CreateBrowserSync(window_info, this, url, browser_settings, NULL);
    if (browser.get()) {
        std::cerr << "init: cef: created layer " << name << "\n";
        browser->GetHost()->SendFocusEvent(has_focus);
    }
}

InexorCefLayer::~InexorCefLayer() { }

void InexorCefLayer::SetVisibility(bool is_visible)
{
	this->is_visible = is_visible;
	browser->GetHost()->SetWindowVisibility(is_visible);
	browser->GetHost()->WasHidden(!is_visible);
}

void InexorCefLayer::SetFocus(bool has_focus)
{
	this->has_focus = has_focus;
	browser->GetHost()->SendFocusEvent(has_focus);
}

void InexorCefLayer::SetIsAcceptingInput(bool is_accepting_input)
{
	this->is_accepting_input = is_accepting_input;
}

void InexorCefLayer::Destroy()
{
    std::cerr << "InexorCefLayer::Destroy()\n";
    browser->GetHost()->CloseBrowser(true);
    // DoClose(browser);
}

void InexorCefLayer::Copy()
{
    browser->GetFocusedFrame()->Copy();
}

void InexorCefLayer::Paste()
{
    // SDL_SetClipboardText()
    // SDL_GetClipboardText()
    // browser->GetMainFrame()
    browser->GetFocusedFrame()->Paste();
}

void InexorCefLayer::Cut()
{
    browser->GetFocusedFrame()->Cut();
}

void InexorCefLayer::ShowDevTools()
{
    browser->GetHost()->ShowDevTools(window_info, this, browser_settings, CefPoint());
}

void InexorCefLayer::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    if (!browser.get())   {
        // Keep a reference to the main browser.
        this->browser = browser;
        browser_id = browser->GetIdentifier();
    }
    // Keep track of how many browsers currently exist.
    browser_count++;
}

bool InexorCefLayer::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed description of this
    // process.
    if (browser_id == browser->GetIdentifier()) {
        // Notify the browser that the parent window is about to close.
        // browser->GetHost()->ParentWindowWillClose();
        // Set a flag to indicate that the window close should be allowed.
        is_closing = true;
    }
    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void InexorCefLayer::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    if (browser_id == browser->GetIdentifier()) {
        // Free the browser pointer so that the browser can be destroyed.
        browser = NULL;
    }
    if (--browser_count == 0) {
        // All browser windows have closed. Quit the application message loop.
        // CefQuitMessageLoop();
        std::cerr << "InexorCefLayer::OnBeforeClose\n";
    }
}

void InexorCefLayer::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();
    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED)
        return;
    std::cerr << "Failed to load URL " << failedUrl.ToString() << ", " <<  errorText.ToString() << "\n";
    // Display a load error message.
    std::stringstream error_message;
    error_message << "<html><body><h2>Failed to load URL " << std::string(failedUrl)
                  << " with error " << std::string(errorText) << " (" << errorCode
                  << ").</h2></body></html>";
    frame->LoadString(error_message.str(), failedUrl);
}

bool InexorCefLayer::OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& key_event, CefEventHandle os_event, bool* is_keyboard_shortcut) {
    CEF_REQUIRE_UI_THREAD();
    return false;
}

bool InexorCefLayer::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& key_event, CefEventHandle os_event) {
    CEF_REQUIRE_UI_THREAD();
    return false;
}

void InexorCefLayer::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    CEF_REQUIRE_UI_THREAD();
    std::cerr << "address change: " << url.ToString() << "\n";
}

void InexorCefLayer::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
    CEF_REQUIRE_UI_THREAD();
    std::cerr << "status: " << value.ToString() << "\n";
}

bool InexorCefLayer::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
    CEF_REQUIRE_UI_THREAD();
    std::cerr << "jsconsole [" << source.ToString() << " (" << line << ")]: " << message.ToString() << "\n";
    return true;
}
