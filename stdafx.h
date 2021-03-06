// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展

#include <afxdisp.h>        // MFC 自动化类

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持
#include <afx.h>

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//新加头文件
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES // for C++
#endif
//#include <cmath>
#include <math.h>

//内存检测
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include<stdlib.h>
#include<crtdbg.h>
#endif 

//全局变量************************
extern BYTE gLaserType;
extern float gProcessSpeed;
extern float gProcessPower;
extern float gProcessPulseWidth;
extern float gProcessWaitTime;
extern UINT gProcessFrequncy;
extern BYTE gProcessTimes;
extern BYTE gProcessMode;
extern float gLaserOnDelay;
extern float gLaserOffDelay;
extern float gBeforMarkDelay;
extern float gAfterMarkDelay;
extern float gPolylineDelay;
extern int	gDupNumber;
extern float gGapDistance;
extern byte	gFillOutlinePrior;
extern bool	gFillEnable0;
extern double gFillAngle0;
extern double gFillGap0;
extern byte	gGapMode0;
extern byte	gFillMode0;
extern double gCurtail0;
extern bool	gFillEnable1;
extern double gFillAngle1;
extern double gFillGap1;
extern byte	gGapMode1;
extern byte	gFillMode1;
extern double gCurtail1;
extern bool	gFillEnable2;
extern double gFillAngle2;
extern double gFillGap2;
extern byte	gGapMode2;
extern byte	gFillMode2;
extern double gCurtail2;
extern bool	gFillEnable3;
extern double gFillAngle3;
extern double gFillGap3;
extern byte	gGapMode3;
extern byte	gFillMode3;
extern double gCurtail3;