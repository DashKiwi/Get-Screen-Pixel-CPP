#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <string>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#elif __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

// Function to convert RGB to HEX
std::string rgb_to_hex(int r, int g, int b) {
    std::stringstream ss;
    ss << "#" << std::setw(2) << std::setfill('0') << std::hex << r
       << std::setw(2) << std::setfill('0') << std::hex << g
       << std::setw(2) << std::setfill('0') << std::hex << b;
    return ss.str();
}

// Platform-specific function to get pixel color at (x, y)

// Windows
#ifdef _WIN32
COLORREF get_pixel_color(int x, int y) {
    HDC hdcScreen = GetDC(NULL);
    COLORREF color = GetPixel(hdcScreen, x, y);
    ReleaseDC(NULL, hdcScreen);
    return color;
}
#endif

// Linux
#ifdef __linux__
COLORREF get_pixel_color(int x, int y) {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        std::cerr << "Cannot open display" << std::endl;
        exit(1);
    }
    
    Window root = DefaultRootWindow(display);
    XImage* img = XGetImage(display, root, x, y, 1, 1, AllPlanes, ZPixmap);
    unsigned long pixel = XGetPixel(img, 0, 0);
    int r = (pixel & 0xFF0000) >> 16;
    int g = (pixel & 0x00FF00) >> 8;
    int b = pixel & 0x0000FF;
    XDestroyImage(img);
    XCloseDisplay(display);
    return (r << 16) | (g << 8) | b;
}
#endif

// macOS
#ifdef __APPLE__
COLORREF get_pixel_color(int x, int y) {
    CGEventRef event = CGEventCreate(NULL);
    CGPoint point = CGPointMake(x, y);
    CGEventPost(kCGEventMouseMoved, event);  // move mouse to (x, y)
    CGEventRelease(event);
    
    CGEventRef mouseEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, point, 0, 0, 0);
    CGEventPost(kCGEventMouseMoved, mouseEvent);
    
    CGEventRelease(mouseEvent);
    return 0;
}
#endif

// Platform-specific clipboard handling

#ifdef _WIN32
void copy_to_clipboard(const std::string& hex_color) {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, hex_color.size() + 1);
        memcpy(GlobalLock(hGlobal), hex_color.c_str(), hex_color.size() + 1);
        GlobalUnlock(hGlobal);
        SetClipboardData(CF_TEXT, hGlobal);
        CloseClipboard();
    }
}
#elif __linux__
void copy_to_clipboard(const std::string& hex_color) {
    std::string cmd = "echo " + hex_color + " | xclip -selection clipboard";
    system(cmd.c_str());
}
#elif __APPLE__
void copy_to_clipboard(const std::string& hex_color) {
    std::string cmd = "echo " + hex_color + " | pbcopy";
    system(cmd.c_str());
}
#endif

// Platform-specific hotkey detection

#ifdef _WIN32
bool is_hotkey_pressed() {
    return (GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
           (GetAsyncKeyState(VK_MENU) & 0x8000) && 
           (GetAsyncKeyState(0x43) & 0x8000);  // Ctrl + Alt + C
}
#elif __linux__
bool is_hotkey_pressed() {
    // Linux-specific hotkey detection using X11
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        std::cerr << "Cannot open X display!" << std::endl;
        exit(1);
    }

    XGrabKey(display, XKeysymToKeycode(display, XK_C), Mod1Mask | ControlMask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
    XEvent ev;
    XNextEvent(display, &ev);

    bool keyPressed = (ev.xkey.keycode == XKeysymToKeycode(display, XK_C));
    XCloseDisplay(display);
    return keyPressed;
}
#elif __APPLE__
bool is_hotkey_pressed() {
    // macOS hotkey detection using Quartz
    CGEventFlags flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
    if (flags & kCGEventFlagMaskControl && flags & kCGEventFlagMaskAlternate) {
        return true;
    }
    return false;
}
#endif

// Function to get mouse position and trigger color pick
void on_hotkey_press() {
    POINT p;
    if (GetCursorPos(&p)) {
        int x = p.x;
        int y = p.y;

        // Get color at mouse position
        COLORREF color = get_pixel_color(x, y);

        // Convert color to RGB
        int r = GetRValue(color);
        int g = GetGValue(color);
        int b = GetBValue(color);

        // Convert RGB to HEX
        std::string hex_color = rgb_to_hex(r, g, b);

        // Print the results
        std::cout << "\nColor Grabbed" << std::endl;
        std::cout << "Coordinates: X = " << x << ", Y = " << y << std::endl;
        std::cout << "HEX: " << hex_color << std::endl;
        std::cout << "RGB: " << r << ", " << g << ", " << b << std::endl;

        // Copy to clipboard
        copy_to_clipboard(hex_color);
    }
}

// Function to monitor hotkey presses
void monitor_hotkey() {
    while (true) {
        if (is_hotkey_pressed()) {
            on_hotkey_press();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));  // Prevents repeated triggering
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Prevent high CPU usage
    }
}

int main() {
    std::cout << "Point your mouse cursor at anywhere on the screen, press Ctrl+Alt+C\n";
    std::cout << "The HEX color code will be copied to your clipboard (and printed here)\n";

    // Start the hotkey monitoring in a separate thread
    std::thread hotkey_thread(monitor_hotkey);
    hotkey_thread.join();

    return 0;
}
