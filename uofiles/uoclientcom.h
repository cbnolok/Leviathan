#ifndef UOCOM_H
#define UOCOM_H

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winuser.h>
#endif


class UOClientCom
{
public:
    enum UOClientType
    {
        CLIENT_UNK      = -1,
        CLIENT_CLASSIC  = 0,
        CLIENT_ENHANCED = 1,
        CLIENT_QTY
    };

    enum UOClientComError
    {
        UOCOMERR_OK         = 0,    // no error
        UOCOMERR_NOWINDOW   = 1,
        UOCOMERR_QTY
    };

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
    // don't mind the "defined but not used" compiler warning: it's not used here, but it is elsewhere
    static const char *UOClientComErrorString[];    // defined in the .cpp
    #pragma GCC diagnostic pop

    //UOClientCom();
    UOClientComError m_error = UOCOMERR_OK;
    UOClientType m_clientType = CLIENT_UNK;
    const char *getErrorString();
    static const char *UOClientWindowTitles[];
    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const char *string);

private:
    long *m_uoHandle = nullptr;
#ifdef _WIN32
    static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam);
#endif
    bool findUOWindow();
    bool canSend();
};


#endif // UOCOM_H
