#define UNICODE
#pragma comment(lib,"dxguid")
#pragma comment(linker,"/opt:nowin98")
#include<windows.h>
#include<dxdiag.h>

TCHAR szClassName[]=TEXT("Window");

HRESULT GetStringValue(IDxDiagContainer* pObject,WCHAR* wstrName,WCHAR* strValue,int nStrLen)
{
	HRESULT hr;
	VARIANT var;
	VariantInit(&var);
	if(FAILED(hr=pObject->GetProp(wstrName,&var)))
	{
		return hr;
	}
	if(var.vt !=VT_BSTR)
	{
		return E_INVALIDARG;
	}
	wcsncpy(strValue,var.bstrVal,nStrLen-1);
	strValue[nStrLen-1]=L'\0';
	VariantClear(&var);
	return S_OK;
}

HRESULT GetDisplayDevices(HWND hList)
{
	HRESULT hr;
	IDxDiagProvider*pDxDiagProvider=0;
	IDxDiagContainer*pDxDiagRoot=0;
	IDxDiagContainer*pContainer=0;
	DXDIAG_INIT_PARAMS dxDiagInitParam;
	hr=CoInitialize(NULL);
	if(FAILED(hr))
	{
		goto LCleanup;
	}
	hr=CoCreateInstance(CLSID_DxDiagProvider,NULL,CLSCTX_INPROC_SERVER,IID_IDxDiagProvider,(LPVOID*) &pDxDiagProvider);
	if(FAILED(hr)||pDxDiagProvider==NULL)
	{
		goto LCleanup;
	}
	ZeroMemory(&dxDiagInitParam,sizeof(DXDIAG_INIT_PARAMS));
	dxDiagInitParam.dwSize=sizeof(DXDIAG_INIT_PARAMS);
	dxDiagInitParam.dwDxDiagHeaderVersion=DXDIAG_DX9_SDK_VERSION;
	dxDiagInitParam.bAllowWHQLChecks=FALSE;
	dxDiagInitParam.pReserved=NULL;
	hr=pDxDiagProvider->Initialize(&dxDiagInitParam);
	if(FAILED(hr))
	{
		goto LCleanup;
	}
	hr=pDxDiagProvider->GetRootContainer(&pDxDiagRoot);
	if(FAILED(hr))
	{
		goto LCleanup;
	}
	hr=pDxDiagRoot->GetChildContainer(L"DxDiag_DisplayDevices",&pContainer);
	if(FAILED(hr))
	{
		goto LCleanup;
	}
	DWORD nInstanceCount;
	hr=pContainer->GetNumberOfChildContainers(&nInstanceCount);
	if(FAILED(hr))
	{
		goto LCleanup;
	}
	WCHAR wszContainer[1024];
	WCHAR szDeviceName[1024];
	IDxDiagContainer* pObject;
	DWORD nItem;
	for(nItem=0;nItem<nInstanceCount;nItem++)
	{
		hr=pContainer->EnumChildContainerNames(nItem,wszContainer,1024);
		if(FAILED(hr))
		{
			continue;
		}
		hr=pContainer->GetChildContainer(wszContainer,&pObject);
		if(FAILED(hr)||pObject==NULL)
		{
			continue;
		}
		if(SUCCEEDED(hr=GetStringValue(pObject,L"szDeviceName",szDeviceName,1024)))
		{
			SendMessageW(hList,LB_ADDSTRING,0,(LPARAM)szDeviceName);
		}
		pObject->Release();
	}
LCleanup:
	if(pContainer)pContainer->Release();
	if(pDxDiagRoot)pDxDiagRoot->Release();
	if(pDxDiagProvider)pDxDiagProvider->Release();
	CoUninitialize();
	return hr;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HWND hList;
	switch(msg)
	{
	case WM_CREATE:
		hList=CreateWindow(TEXT("LISTBOX"),0,WS_VISIBLE|WS_CHILD,0,0,0,0,hWnd,0,((LPCREATESTRUCT)lParam)->hInstance,0);
		__try{
			GetDisplayDevices(hList);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			MessageBox(hWnd,TEXT("デバイス名の取得ができませんでした。"),0,0);// ここに来る
			return -1;
		}
		break;
	case WM_SIZE:
		MoveWindow(hList,0,0,LOWORD(lParam),HIWORD(lParam),1);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPreInst,LPSTR pCmdLine,int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass={
		CS_HREDRAW|CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd=CreateWindow(
		szClassName,
		TEXT("ディスプレイ デバイス 一覧"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
