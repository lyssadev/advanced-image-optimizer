#pragma once

#include <windows.h>
#include <shlobj.h>
#include <memory>
#include <vector>
#include <string>
#include "optimizer.h"

// {B742C7EF-8C47-4F47-8E1C-7AEBE1E88A9B}
DEFINE_GUID(CLSID_OptimizerShellExt, 
    0xb742c7ef, 0x8c47, 0x4f47, 0x8e, 0x1c, 0x7a, 0xeb, 0xe1, 0xe8, 0x8a, 0x9b);

class ATL_NO_VTABLE COptimizerShellExt :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<COptimizerShellExt, &CLSID_OptimizerShellExt>,
    public IShellExtInit,
    public IContextMenu,
    public IShellIconOverlayIdentifier,
    public IDropTarget
{
public:
    COptimizerShellExt();
    virtual ~COptimizerShellExt();

    DECLARE_REGISTRY_RESOURCEID(IDR_OPTIMIZERSHELLEXT)
    DECLARE_NOT_AGGREGATABLE(COptimizerShellExt)

    BEGIN_COM_MAP(COptimizerShellExt)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)
        COM_INTERFACE_ENTRY(IShellIconOverlayIdentifier)
        COM_INTERFACE_ENTRY(IDropTarget)
    END_COM_MAP()

    // IShellExtInit
    STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID);

    // IContextMenu
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst,
                              UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pwReserved,
                              LPSTR pszName, UINT cchMax);

    // IShellIconOverlayIdentifier
    STDMETHOD(IsMemberOf)(LPCWSTR pwszPath, DWORD dwAttrib);
    STDMETHOD(GetOverlayInfo)(LPWSTR pwszIconFile, int cchMax, int* pIndex,
                             DWORD* pdwFlags);
    STDMETHOD(GetPriority)(int* pPriority);

    // IDropTarget
    STDMETHOD(DragEnter)(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt,
                        DWORD* pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt,
                   DWORD* pdwEffect);

private:
    // Helper methods
    HRESULT LoadFileList(IDataObject* pdtobj);
    HRESULT ProcessFiles(bool showProgress = true);
    HRESULT ShowOptionsDialog();
    HRESULT ShowProgressDialog();
    void UpdateOverlayIcon(const std::wstring& path);

    // Context menu commands
    enum {
        ID_OPTIMIZE = 0,
        ID_OPTIMIZE_ADVANCED,
        ID_OPTIMIZE_HERE,
        ID_OPTIMIZE_TO,
        ID_SETTINGS,
        ID_LAST
    };

    // Member variables
    std::vector<std::wstring> m_Files;
    optimizer_options_t m_Options;
    bool m_IsDragging;
    HWND m_hwndProgress;
    
    // Progress callback
    static void CALLBACK ProgressCallback(float progress, void* userData) {
        auto* pThis = static_cast<COptimizerShellExt*>(userData);
        pThis->OnProgress(progress);
    }
    
    void OnProgress(float progress);

    // Registry helpers
    static HRESULT RegisterComObject();
    static HRESULT UnregisterComObject();
    static HRESULT UpdateRegistry(bool bRegister);
    
    // Shell icon helpers
    static HRESULT LoadShellIcon(HICON* phIcon);
    static void GetModulePath(LPWSTR pszPath, DWORD cchPath);
};

OBJECT_ENTRY_AUTO(__uuidof(COptimizerShellExt), COptimizerShellExt) 