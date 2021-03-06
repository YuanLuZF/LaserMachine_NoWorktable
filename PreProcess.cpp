#include "stdafx.h"
#include "PreProcess.h"
#include "CDlgDevCfgTabCamera.h"
#include "CDlgDevCfgTabScanner.h"
#include "CameraView.h"
#include <math.h>

CPreProcess::CPreProcess()
{
	m_vecProcEntPerGrid.clear();
	m_vecProcEntPerGridTrans.clear();

	//m_ptOrg = Point_T(0.0, 0.0);
	//m_ptOrgDxf = Point_T();
	//m_ptScale = Point_T();
	//m_fRotateDegree = 0;
	m_fMarkArcStep = ReadScannerMarkArcStep();
}

CPreProcess::~CPreProcess()
{

}

//******************************************//
//分格
void CPreProcess::CalculateGrid(std::vector<ObjRect>& vecGridRect, CMachineListContainer* pObjList, std::vector<double> vecGridX, std::vector<double> vecGridY)
{
	ObjRect rectBorder = pObjList->GetObjBound();
	int nCtGridX, nCtGridY;
	nCtGridX = vecGridX.size() - 1;
	nCtGridY = vecGridY.size() - 1;

	for (int j = 0; j < nCtGridY; j++)					//Y轴由下到上
	{
		if (0 == j % 2)										//奇数行，由左至右
		{
			for (int i = 0; i < nCtGridX; i++)
			{
				ObjRect rectTmp;
				rectTmp.min_x = vecGridX[i] + rectBorder.min_x;
				rectTmp.max_x = vecGridX[(size_t)i + 1] + rectBorder.min_x;
				rectTmp.min_y = vecGridY[j] + rectBorder.min_y;
				rectTmp.max_y = vecGridY[(size_t)j + 1] + rectBorder.min_y;
				vecGridRect.push_back(rectTmp);
			}
		}
		else												//偶数行，由右至左
		{
			for (int i = nCtGridX - 1; i >= 0; i--)
			{
				ObjRect rectTmp;
				rectTmp.min_x = vecGridX[i] + rectBorder.min_x;
				rectTmp.max_x = vecGridX[(size_t)i + 1] + rectBorder.min_x;
				rectTmp.min_y = vecGridY[j] + rectBorder.min_y;
				rectTmp.max_y = vecGridY[(size_t)j + 1] + rectBorder.min_y;
				vecGridRect.push_back(rectTmp);
			}
		}
	}
}
void CPreProcess::CalculateGrid(std::vector<ObjRect>& vecGridRect, CMachineListContainer* pObjList)
{
	ObjRect rectBorder = pObjList->GetObjBound();
	double fScannerRegion, fWktableRegion;
	int nCtGridX, nCtGridY, nCtGridMax;

	fScannerRegion = ReadScannerCaliRegion();
	//fScannerRegion = 140;
	fWktableRegion = 500;

	nCtGridMax = (int)floor(fWktableRegion / fScannerRegion);
	nCtGridX = (int)ceil(abs(rectBorder.max_x - rectBorder.min_x) / fScannerRegion);
	nCtGridY = (int)ceil(abs(rectBorder.max_y - rectBorder.min_y) / fScannerRegion);

	if (nCtGridX > nCtGridMax && nCtGridY > nCtGridMax)
	{
		AfxMessageBox(_T("加工对象超幅面"));
		return;
	}
	else if (nCtGridX <= 1 && nCtGridY <= 1)
	{
		vecGridRect.push_back(rectBorder);
	}
	else
	{
		for (int j = 0; j < nCtGridY; j++)					//Y轴由下到上
		{
			if (0 == j % 2)										//奇数行，由左至右
			{
				for (int i = 0; i < nCtGridX; i++)
				{
					ObjRect rectTmp;
					rectTmp.min_x = fScannerRegion * i + rectBorder.min_x;
					rectTmp.max_x = fScannerRegion * ((double)i + 1) + rectBorder.min_x;
					rectTmp.min_y = fScannerRegion * j + rectBorder.min_y;
					rectTmp.max_y = fScannerRegion * ((double)j + 1) + rectBorder.min_y;
					vecGridRect.push_back(rectTmp);
				}
			}
			else												//偶数行，由右至左
			{
				for (int i = nCtGridX - 1; i >= 0; i--)
				{
					ObjRect rectTmp;
					rectTmp.min_x = fScannerRegion * i + rectBorder.min_x;
					rectTmp.max_x = fScannerRegion * ((double)i + 1) + rectBorder.min_x;
					rectTmp.min_y = fScannerRegion * j + rectBorder.min_y;
					rectTmp.max_y = fScannerRegion * ((double)j + 1) + rectBorder.min_y;
					vecGridRect.push_back(rectTmp);
				}
			}
		}
	}

}
BOOL CPreProcess::DoSingleGrid(CMachineListContainer* pObjList)
{
	ObjRect rtBorder = pObjList->GetObjBound();
	double fScannerRegion;
	fScannerRegion = ReadScannerCaliRegion();
	fScannerRegion *= 1.05;

	if (rtBorder.max_x - rtBorder.min_x > fScannerRegion || rtBorder.max_y - rtBorder.min_y > fScannerRegion)
	{
		AfxMessageBox(_T("加工对象超幅面"));
		return FALSE;
	}

	//生成polyGrid，即所有分格矩形列表
	//std::vector<Polygon_T> vecPolyGrid;
	Polygon_T polyTmp;
	bg::append(polyTmp.outer(), Point_T(rtBorder.min_x, rtBorder.min_y));
	bg::append(polyTmp.outer(), Point_T(rtBorder.min_x, rtBorder.max_y));
	bg::append(polyTmp.outer(), Point_T(rtBorder.max_x, rtBorder.max_y));
	bg::append(polyTmp.outer(), Point_T(rtBorder.max_x, rtBorder.min_y));
	bg::correct(polyTmp);
	//vecPolyGrid.push_back(polyTmp);

	//生成vecEntities，即所有加工对象列表
	VecEntities vecEntities;
	LoadMachineObjList(vecEntities, pObjList);

	std::vector<MLine_T> vecMLineAll;
	for (auto val : vecEntities)
	{
		vecMLineAll.push_back(val.mlineObj);
	}


	//遍历vecEntities & vecPolyGrid，生成m_vecProcEntPerGrid
	//CutEntityByGrid(vecEntities, vecPolyGrid);
	PreProcessEntPerGrid prcEntPerGridTmp;
	prcEntPerGridTmp.polyGrid = polyTmp;
	prcEntPerGridTmp.vecEntitiesPerGrid = vecEntities;
	m_vecProcEntPerGrid.push_back(prcEntPerGridTmp);

	return TRUE;
}

void CPreProcess::DoGrid(std::vector<ObjRect>& vecGridRect, CMachineListContainer* pObjList)
{
	//生成polyGrid，即所有分格矩形列表
	std::vector<Polygon_T> vecPolyGrid;
	for (auto val : vecGridRect)
	{
		Polygon_T polyTmp;
		bg::append(polyTmp.outer(), Point_T(val.min_x, val.min_y));
		bg::append(polyTmp.outer(), Point_T(val.min_x, val.max_y));
		bg::append(polyTmp.outer(), Point_T(val.max_x, val.max_y));
		bg::append(polyTmp.outer(), Point_T(val.max_x, val.min_y));
		bg::correct(polyTmp);
		vecPolyGrid.push_back(polyTmp);
	}

	//生成vecEntities，即所有加工对象列表
	VecEntities vecEntities;
	LoadMachineObjList(vecEntities, pObjList);


	//std::vector<MLine_T> vecMLineAll;
	//for (auto val : vecEntities)
	//{
	//	vecMLineAll.push_back(val.mlineObj);
	//}


	//遍历vecEntities & vecPolyGrid，生成m_vecProcEntPerGrid
	CutEntityByGrid(vecEntities, vecPolyGrid);



	//检查结果

	//std::vector<MLine_T> vecMLinePerGrid;
	//std::vector<Polygon_T> vecPoly;
	//for (auto valList : m_vecProcEntPerGrid)
	//{
	//	Polygon_T polyTmp;
	//	VecEntities vecEntTmp;
	//	polyTmp = valList.polyGrid;
	//	vecEntTmp = valList.vecEntitiesPerGrid;
	//	vecPoly.push_back(valList.polyGrid);

	//	for (auto val : vecEntTmp)
	//	{
	//		vecMLinePerGrid.push_back(val.mlineObj);
	//	}
	//}
	//if (1) {}

	//double ptOrg[2] = { 100, 100 };
	//double ptScale[2] = { 1, 1 }; 
	//double fRotateDegree = 90;
	//DoTrans(ptOrg, ptScale, fRotateDegree);
	//std::vector<MLine_T> vecMLinePerGridTrans;
	//std::vector<Polygon_T> vecPolyTrans;
	//for (auto valListTrans : m_vecProcEntPerGridTrans)
	//{
	//	Polygon_T polyTmpTrans;
	//	VecEntities vecEntTmp;
	//	polyTmpTrans = valListTrans.polyGrid;
	//	vecEntTmp = valListTrans.vecEntitiesPerGrid;
	//	vecPolyTrans.push_back(valListTrans.polyGrid);

	//	for (auto val : vecEntTmp)
	//	{
	//		vecMLinePerGridTrans.push_back(val.mlineObj);
	//	}
	//}

	//if (1) {}

}

//void CPreProcess::DoGrid(std::vector<double*> &vecGridRect, CMachineListContainer* pObjList)
//{
//	////如果vector为空，自动计算分格
//	//if (TRUE == vecGridRect.empty())
//	//{
//	//	Polygon_T polyTmp;
//	//	ObjRect objRect = pObjList->GetObjBound();
//	//	bg::append(polyTmp.outer(), Point_T(objRect.min_x, objRect.min_y));
//	//	bg::append(polyTmp.outer(), Point_T(objRect.min_x, objRect.max_y));
//	//	bg::append(polyTmp.outer(), Point_T(objRect.max_x, objRect.max_y));
//	//	bg::append(polyTmp.outer(), Point_T(objRect.max_x, objRect.min_y));
//	//	bg::correct(polyTmp);
//	//	vecPolyGrid.push_back(polyTmp);
//	//}
//
//	//生成polyGrid，即所有分格矩形列表
//	std::vector<Polygon_T> vecPolyGrid;
//	for (auto val : vecGridRect)
//	{
//		Polygon_T polyTmp;
//		bg::append(polyTmp.outer(), Point_T(val[0], val[3]));
//		bg::append(polyTmp.outer(), Point_T(val[0], val[1]));
//		bg::append(polyTmp.outer(), Point_T(val[2], val[1]));
//		bg::append(polyTmp.outer(), Point_T(val[2], val[3]));
//		bg::correct(polyTmp);
//		vecPolyGrid.push_back(polyTmp);
//	}
//
//	//生成vecEntities，即所有加工对象列表
//	VecEntities vecEntities;
//	LoadMachineObjList(vecEntities, pObjList);
//
//	//遍历vecEntities & vecPolyGrid，生成m_vecProcEntPerGrid
//	CutEntityByGrid(vecEntities, vecPolyGrid);
//
//
//	////检查结果
//	//double ptOrg[2] = { 100, 100 };
//	//double ptScale[2] = { 1, 1 }; 
//	//double fRotateDegree = 90;
//	//DoTrans(ptOrg, ptScale, fRotateDegree);
//
//	//std::vector<MLine_T> vecMLineAll;
//	//for (auto val : vecEntities)
//	//{
//	//	vecMLineAll.push_back(val.mlineObj);
//	//}
//
//	//std::vector<MLine_T> vecMLinePerGrid;
//	//std::vector<Polygon_T> vecPoly;
//	//for (auto valList : m_vecProcEntPerGrid)
//	//{
//	//	Polygon_T polyTmp;
//	//	VecEntities vecEntTmp;
//	//	polyTmp = valList.polyGrid;
//	//	vecEntTmp = valList.vecEntitiesPerGrid;
//	//	vecPoly.push_back(valList.polyGrid);
//
//	//	for (auto val : vecEntTmp)
//	//	{
//	//		vecMLinePerGrid.push_back(val.mlineObj);
//	//	}
//	//}
//
//	//std::vector<MLine_T> vecMLinePerGridTrans;
//	//std::vector<Polygon_T> vecPolyTrans;
//	//for (auto valListTrans : m_vecProcEntPerGridTrans)
//	//{
//	//	Polygon_T polyTmpTrans;
//	//	VecEntities vecEntTmp;
//	//	polyTmpTrans = valListTrans.polyGrid;
//	//	vecEntTmp = valListTrans.vecEntitiesPerGrid;
//	//	vecPolyTrans.push_back(valListTrans.polyGrid);
//
//	//	for (auto val : vecEntTmp)
//	//	{
//	//		vecMLinePerGridTrans.push_back(val.mlineObj);
//	//	}
//	//}
//
//	//if (1) {}
//		
//}


//平移缩放旋转
void CPreProcess::CalculateTrans(int nCountMark, std::vector <CPointF> vPtPosDestinedMark, std::vector <CPointF> vPtPosRealMark, std::vector<ObjRect> vecGridRect,
	double fPosWorktableX, double fPosWorktableY, double ptOrg[2], double ptScale[2], double* fRotateDegree, double ptReal[2], int nScaleDirection)
{

	//CString str111;

	//计算平移旋转拉伸
	if (0 == nCountMark)	
	{
		//无mark点 以图中分格左下角+当前位置振镜中心为原点
		ptOrg[0] = vecGridRect[0].min_x;
		ptOrg[1] = vecGridRect[0].min_y;
		ptScale[0] = 1;
		ptScale[1] = 1;
		*fRotateDegree = 0;
		ptReal[0] = fPosWorktableX;
		ptReal[1] = fPosWorktableY;
	}
	else
	{
		//有mark点 则以第一个mark点为零点
		double fPosCameraX, fPosCameraY;
		fPosCameraX = ReadDevCameraPosX();
		fPosCameraY = ReadDevCameraPosY();
		ptOrg[0] = (double)vPtPosDestinedMark[0].x;
		ptOrg[1] = (double)vPtPosDestinedMark[0].y;
		ptReal[0] = (double)vPtPosRealMark[0].x - fPosCameraX;
		ptReal[1] = (double)vPtPosRealMark[0].y - fPosCameraY;

		//计算拉伸旋转
		if (1 == nCountMark)
		{
			ptScale[0] = 1;
			ptScale[1] = 1;
			*fRotateDegree = 0;
		}
		else if (2 == nCountMark)
		{
			CPointF ptVecReal, ptVecDestined;
			double fLengthOfVecReal, fLengthOfVecDestined;
			ptVecReal = vPtPosRealMark[1] - vPtPosRealMark[0];
			ptVecDestined = vPtPosDestinedMark[1] - vPtPosDestinedMark[0];
			fLengthOfVecReal = sqrt(pow(ptVecReal.x, 2) + pow(ptVecReal.y, 2));
			fLengthOfVecDestined = sqrt(pow(ptVecDestined.x, 2) + pow(ptVecDestined.y, 2));

			//str111.Format(_T("fLengthOfVecReal = %.3f\nfLengthOfVecDestined = %.3f\nptScale = %.3f"), fLengthOfVecReal, fLengthOfVecDestined, fLengthOfVecReal / fLengthOfVecDestined);
			//AfxMessageBox(str111);

			if (0 == nScaleDirection)
			{
				//同向拉伸
				ptScale[0] = fLengthOfVecReal / fLengthOfVecDestined;
				ptScale[1] = fLengthOfVecReal / fLengthOfVecDestined;
				*fRotateDegree = atan2((double)ptVecReal.y, (double)ptVecReal.x) - atan2((double)ptVecDestined.y, (double)ptVecDestined.x);	//ccw 弧度
				*fRotateDegree *= -180 / M_PI;	//cw 角度
			}
			else if (1 == nScaleDirection)
			{
				//X方向拉伸
				double fPreScaleX, fPreScaleY;
				fPreScaleY = ptVecDestined.y;
				fPreScaleX = sqrt(pow(fLengthOfVecReal, 2) - pow(fPreScaleY, 2));
				ptScale[0] = fPreScaleX / ptVecDestined.x;
				ptScale[1] = 1;
				*fRotateDegree = atan2((double)ptVecReal.y, (double)ptVecReal.x) - atan2(fPreScaleY, fPreScaleX);	//ccw 弧度
				*fRotateDegree *= -180 / M_PI;	//cw 角度
			}
			else if (2 == nScaleDirection)
			{
				//Y方向拉伸
				double fPreScaleX, fPreScaleY;
				fPreScaleX = ptVecDestined.x;
				fPreScaleY = sqrt(pow(fLengthOfVecReal, 2) - pow(fPreScaleX, 2));
				ptScale[0] = 1;
				ptScale[1] = fPreScaleY / ptVecDestined.y;
				*fRotateDegree = atan2((double)ptVecReal.y, (double)ptVecReal.x) - atan2(fPreScaleY, fPreScaleX);	//ccw 弧度
				*fRotateDegree *= -180 / M_PI;	//cw 角度
			}
			else
				AfxMessageBox(_T("定位参数错误"));

		}
		else if (3 == nCountMark)
		{

		}
		else if (4 == nCountMark)
		{

		}
		else
			AfxMessageBox(_T("定位参数错误"));


	}

	//switch (nCountMark)
	//{
	//case 0:
	//	*ptOrg[0] = vecGridRect[0].min_x;
	//	*ptOrg[1] = vecGridRect[0].min_y;
	//	*ptScale[0] = 1;
	//	*ptScale[1] = 1;
	//	*fRotateDegree = 0;
	//	*ptReal[0] = fPosWorktableX;
	//	*ptReal[1] = fPosWorktableY;
	//	break;
	//case 1:
	//	*ptOrg[0] = (double)vPtPosDestinedMark[0].x;
	//	*ptOrg[1] = (double)vPtPosDestinedMark[0].y;
	//	*ptScale[0] = 1;
	//	*ptScale[1] = 1;
	//	*fRotateDegree = 0;
	//	*ptReal[0] = (double)vPtPosRealMark[0].x - fPosCameraX;
	//	*ptReal[1] = (double)vPtPosRealMark[0].y - fPosCameraY;
	//	break;
	//case 2:
	//	*ptOrg[0] = (double)vPtPosDestinedMark[0].x;
	//	*ptOrg[1] = (double)vPtPosDestinedMark[0].y;
	//	//ptScale[0] = (double)((vPtPosRealMark[1] - vPtPosRealMark[0]) / (vPtPosDestinedMark[1] - vPtPosDestinedMark[0])).x;
	//	//ptScale[1] = (double)((vPtPosRealMark[1] - vPtPosRealMark[0]) / (vPtPosDestinedMark[1] - vPtPosDestinedMark[0])).y;
	//	*ptScale[0] = (vPtPosRealMark[1].x - vPtPosRealMark[0].x) / (vPtPosDestinedMark[1].x - vPtPosDestinedMark[0].x);
	//	*ptScale[1] = (vPtPosRealMark[1].y - vPtPosRealMark[0].y) / (vPtPosDestinedMark[1].y - vPtPosDestinedMark[0].y);
	//	*fRotateDegree = atan2((double)vPtPosRealMark[1].y - vPtPosRealMark[0].y, (double)vPtPosRealMark[1].x - vPtPosRealMark[0].x)
	//		- atan2((double)vPtPosDestinedMark[1].y - vPtPosDestinedMark[0].y, (double)vPtPosDestinedMark[1].x - vPtPosDestinedMark[0].x);
	//	*fRotateDegree *= -180 / M_PI;
	//	*ptReal[0] = (double)vPtPosRealMark[0].x - fPosCameraX;
	//	*ptReal[1] = (double)vPtPosRealMark[0].y - fPosCameraY;
	//	break;
	//case 3:
	//	*ptOrg[0] = (double)vPtPosDestinedMark[0].x;
	//	*ptOrg[1] = (double)vPtPosDestinedMark[0].y;
	//	*ptScale[0] = (vPtPosRealMark[1].x - vPtPosRealMark[0].x) / (vPtPosDestinedMark[1].x - vPtPosDestinedMark[0].x);
	//	*ptScale[1] = (vPtPosRealMark[1].y - vPtPosRealMark[0].y) / (vPtPosDestinedMark[1].y - vPtPosDestinedMark[0].y);
	//	*fRotateDegree = atan2((double)vPtPosRealMark[1].y - vPtPosRealMark[0].y, (double)vPtPosRealMark[1].x - vPtPosRealMark[0].x)
	//		- atan2((double)vPtPosDestinedMark[1].y - vPtPosDestinedMark[0].y, (double)vPtPosDestinedMark[1].x - vPtPosDestinedMark[0].x);
	//	*fRotateDegree *= -180 / M_PI;
	//	*ptReal[0] = (double)vPtPosRealMark[0].x - fPosCameraX;
	//	*ptReal[1] = (double)vPtPosRealMark[0].y - fPosCameraY;
	//	break;
	//case 4:
	//	*ptOrg[0] = (double)vPtPosDestinedMark[0].x;
	//	*ptOrg[1] = (double)vPtPosDestinedMark[0].y;
	//	*ptScale[0] = (vPtPosRealMark[1].x - vPtPosRealMark[0].x) / (vPtPosDestinedMark[1].x - vPtPosDestinedMark[0].x);
	//	*ptScale[1] = (vPtPosRealMark[1].y - vPtPosRealMark[0].y) / (vPtPosDestinedMark[1].y - vPtPosDestinedMark[0].y);
	//	*fRotateDegree = atan2((double)vPtPosRealMark[1].y - vPtPosRealMark[0].y, (double)vPtPosRealMark[1].x - vPtPosRealMark[0].x)
	//		- atan2((double)vPtPosDestinedMark[1].y - vPtPosDestinedMark[0].y, (double)vPtPosDestinedMark[1].x - vPtPosDestinedMark[0].x);
	//	*fRotateDegree *= -180 / M_PI;
	//	*ptReal[0] = (double)vPtPosRealMark[0].x - fPosCameraX;
	//	*ptReal[1] = (double)vPtPosRealMark[0].y - fPosCameraY;
	//	break;
	//default:
	//	AfxMessageBox(_T("定位参数错误"));
	//	break;
	//}
}

void CPreProcess::CalculateSingleTrans(CMachineListContainer* pObjList, int nCountMark, std::vector <CPointF> vPtPosDestinedMark, std::vector <CPointF> vPtPosRealMark,
	double ptOrg[2], double ptScale[2], double* fRotateDegree, double ptReal[2], int nScaleDirection)
{
	ObjRect rtBound = pObjList->GetObjBound();

	//计算平移旋转拉伸
	if (0 == nCountMark)
	{
		//无mark点 以链表外框中心为锚点，以相机视角dxf位置计算旋转偏移
		ptOrg[0] = (rtBound.max_x + rtBound.min_x) / 2;
		ptOrg[1] = (rtBound.max_y + rtBound.min_y) / 2;
		ptScale[0] = 1;
		ptScale[1] = 1;
		*fRotateDegree = -g_fDxfRotate;		//ccw to cw
		ptReal[0] = g_ptDxfTranslate.x;
		ptReal[1] = g_ptDxfTranslate.y;
		////无mark点 以图中分格左下角+当前位置振镜中心为原点
		//ptOrg[0] = (rtBound.max_x - rtBound.min_x) / 2;
		//ptOrg[1] = (rtBound.max_y - rtBound.min_y) / 2;
		//ptScale[0] = 1;
		//ptScale[1] = 1;
		//*fRotateDegree = 0;
		//ptReal[0] = 0;
		//ptReal[1] = 0;

	}
	else
	{
		//有mark点 则以第一个mark点为零点
		double fPosCameraX, fPosCameraY;
		fPosCameraX = ReadDevCameraPosX();
		fPosCameraY = ReadDevCameraPosY();
		ptOrg[0] = (double)vPtPosDestinedMark[0].x;
		ptOrg[1] = (double)vPtPosDestinedMark[0].y;
		ptReal[0] = (double)vPtPosRealMark[0].x - fPosCameraX;
		ptReal[1] = (double)vPtPosRealMark[0].y - fPosCameraY;

		//计算拉伸旋转
		if (1 == nCountMark)
		{
			ptScale[0] = 1;
			ptScale[1] = 1;
			*fRotateDegree = 0;
		}
		else if (2 == nCountMark)
		{
			CPointF ptVecReal, ptVecDestined;
			double fLengthOfVecReal, fLengthOfVecDestined;
			ptVecReal = vPtPosRealMark[1] - vPtPosRealMark[0];
			ptVecDestined = vPtPosDestinedMark[1] - vPtPosDestinedMark[0];
			fLengthOfVecReal = sqrt(pow(ptVecReal.x, 2) + pow(ptVecReal.y, 2));
			fLengthOfVecDestined = sqrt(pow(ptVecDestined.x, 2) + pow(ptVecDestined.y, 2));

			//str111.Format(_T("fLengthOfVecReal = %.3f\nfLengthOfVecDestined = %.3f\nptScale = %.3f"), fLengthOfVecReal, fLengthOfVecDestined, fLengthOfVecReal / fLengthOfVecDestined);
			//AfxMessageBox(str111);

			if (0 == nScaleDirection)
			{
				//同向拉伸
				ptScale[0] = fLengthOfVecReal / fLengthOfVecDestined;
				ptScale[1] = fLengthOfVecReal / fLengthOfVecDestined;
				*fRotateDegree = atan2((double)ptVecReal.y, (double)ptVecReal.x) - atan2((double)ptVecDestined.y, (double)ptVecDestined.x);	//ccw 弧度
				*fRotateDegree *= -180 / M_PI;	//cw 角度
			}
			else if (1 == nScaleDirection)
			{
				//X方向拉伸
				double fPreScaleX, fPreScaleY;
				fPreScaleY = ptVecDestined.y;
				fPreScaleX = sqrt(pow(fLengthOfVecReal, 2) - pow(fPreScaleY, 2));
				ptScale[0] = fPreScaleX / ptVecDestined.x;
				ptScale[1] = 1;
				*fRotateDegree = atan2((double)ptVecReal.y, (double)ptVecReal.x) - atan2(fPreScaleY, fPreScaleX);	//ccw 弧度
				*fRotateDegree *= -180 / M_PI;	//cw 角度
			}
			else if (2 == nScaleDirection)
			{
				//Y方向拉伸
				double fPreScaleX, fPreScaleY;
				fPreScaleX = ptVecDestined.x;
				fPreScaleY = sqrt(pow(fLengthOfVecReal, 2) - pow(fPreScaleX, 2));
				ptScale[0] = 1;
				ptScale[1] = fPreScaleY / ptVecDestined.y;
				*fRotateDegree = atan2((double)ptVecReal.y, (double)ptVecReal.x) - atan2(fPreScaleY, fPreScaleX);	//ccw 弧度
				*fRotateDegree *= -180 / M_PI;	//cw 角度
			}
			else
				AfxMessageBox(_T("定位参数错误"));

		}
		else if (3 == nCountMark)
		{

		}
		else if (4 == nCountMark)
		{

		}
		else
			AfxMessageBox(_T("定位参数错误"));

	}
}


void CPreProcess::DoTrans(double ptOrg[2], double ptScale[2], double fRotateDegree, double ptReal[2])
{

	for (auto valList : m_vecProcEntPerGrid)
	{
		PreProcessEntPerGrid prcEntPerGridTmp;
		Polygon_T polyTmp;
		VecEntities vecEntTmp, vecEntTransTmp;
		polyTmp = valList.polyGrid;
		vecEntTmp = valList.vecEntitiesPerGrid;

		//对每个grid操作
		Polygon_T polyTranslate, polyScale, polyRotate, polyReal;
		bgt::translate_transformer<double, 2, 2> translate(-ptOrg[0], -ptOrg[1]);
		bg::transform(polyTmp, polyTranslate, translate);
		bgt::scale_transformer<double, 2, 2> scale(ptScale[0], ptScale[1]);
		bg::transform(polyTranslate, polyScale, scale);
		bgt::rotate_transformer<boost::geometry::degree, double, 2, 2> rotate(fRotateDegree);
		bg::transform(polyScale, polyRotate, rotate);
		//grid不平移， 相当于图平移
		prcEntPerGridTmp.polyGrid = polyRotate;
		//bgt::translate_transformer<double, 2, 2> translateReal(ptReal[0], ptReal[1]);
		//bg::transform(polyRotate, polyReal, translateReal);
		//prcEntPerGridTmp.polyGrid = polyReal;

		//对每个grid中的对象操作
		for (auto valEnt : vecEntTmp)
		{
			PreProcessEntity myEntity;
			MLine_T mlineTmp;
			mlineTmp = valEnt.mlineObj;

			MLine_T mlineTranslate, mlineScale, mlineRotate, mlineReal;
			bgt::translate_transformer<double, 2, 2> translate(-ptOrg[0], -ptOrg[1]);
			bg::transform(mlineTmp, mlineTranslate, translate);
			bgt::scale_transformer<double, 2, 2> scale(ptScale[0], ptScale[1]);
			bg::transform(mlineTranslate, mlineScale, scale);
			bgt::rotate_transformer<boost::geometry::degree, double, 2, 2> rotate(fRotateDegree);
			bg::transform(mlineScale, mlineRotate, rotate);
			bgt::translate_transformer<double, 2, 2> translateReal(ptReal[0], ptReal[1]);
			bg::transform(mlineRotate, mlineReal, translateReal);
			myEntity.mlineObj = mlineReal;
			myEntity.nPenNo = valEnt.nPenNo;
			vecEntTransTmp.push_back(myEntity);
		}
		prcEntPerGridTmp.vecEntitiesPerGrid = vecEntTransTmp;

		m_vecProcEntPerGridTrans.push_back(prcEntPerGridTmp);
	}
}

void CPreProcess::DoSingleTrans(CMachineListContainer* pObjList, int nCountMark, std::vector <CPointF> vPtPosDestinedMark, std::vector <CPointF> vPtPosRealMark, int nScaleDirection)
{
	double ptOrg[2], ptScale[2], fRotateDegree, ptReal[2];
	CalculateSingleTrans(pObjList, nCountMark, vPtPosDestinedMark, vPtPosRealMark, ptOrg, ptScale, &fRotateDegree, ptReal, nScaleDirection);
	DoTrans(ptOrg, ptScale, fRotateDegree, ptReal);
}


void CPreProcess::GetGridCenter(int nGridIndex, double* pGridCenterX, double* pGridCenterY)
{
	if (nGridIndex >= m_vecProcEntPerGridTrans.size())
	{
		*pGridCenterX = 0;
		*pGridCenterY = 0;
		return;
	}

	//取分格中心，对m_vecProcEntPerGridTrans来说ptOrg已平移为坐标圆点
	Polygon_T polyGrid;
	Point_T	ptCenter;
	polyGrid = m_vecProcEntPerGridTrans.at(nGridIndex).polyGrid;
	bg::centroid(polyGrid, ptCenter);

	*pGridCenterX = ptCenter.x();
	*pGridCenterY = ptCenter.y();
	return;
}

BOOL CPreProcess::WriteEntitiesPerGridToBuffer(int nGridIndex, CMachineListContainer* pObjList)
{
	//debug333
	if (NULL == pDevCardMark)
		return FALSE;

	//debug333
	//先清空打标卡缓冲区
	if (!pDevCardMark->DeleteALLEntities())
		return FALSE;

	if (nGridIndex >= m_vecProcEntPerGridTrans.size())
		return FALSE;

	//取分格中心,m_vecProcEntPerGridTrans中第一个mark点已移至图形坐标原点
	Polygon_T polyGrid;
	Point_T	ptCenter;
	polyGrid = m_vecProcEntPerGridTrans.at(nGridIndex).polyGrid;
	bg::centroid(polyGrid, ptCenter);

	//对象排序，加工模式识别	//LayerNum_Default 加工起始层
	int nCtProcLayer = pObjList->GetLayerCount() - LayerNum_Default;
	std::vector<VecEntities> vecEntSortByLayer;
	vecEntSortByLayer.resize(nCtProcLayer);
	for (auto valEnt : m_vecProcEntPerGridTrans.at(nGridIndex).vecEntitiesPerGrid)
	{
		//暂存笔号层号、对象
		int nPenNo = valEnt.nPenNo;
		PreProcessEntity entTmp;

		//计算对象相对分格中心坐标,相当于平移
		MLine_T mlineTmp, mlineTrans;
		mlineTmp = valEnt.mlineObj;
		//bgt::translate_transformer<double, 2, 2> translate(-ptCenter.x(), -ptCenter.y());
		//bgt::translate_transformer<double, 2, 2> translate(0, 0);
		//bg::transform(mlineTmp, mlineTrans, translate);
		mlineTrans = mlineTmp;

		//将对象加入对应层号的vecEntSortByLayer
		entTmp.mlineObj = mlineTrans;
		entTmp.nPenNo = nPenNo;
		vecEntSortByLayer[nPenNo - (int)LayerNum_Default].push_back(entTmp);
	}

	//遍历每一层
	for (int nIndexLayer = 0; nIndexLayer < nCtProcLayer; nIndexLayer++)
	{
		//暂存笔号，判断加工模式和加工遍数
		int nPenNo = nIndexLayer + LayerNum_Default;
		CMachinePara_Layer* pMachineParaLayerTmp = pObjList->GetLayerOfNumber(nPenNo);

		if (0 == pMachineParaLayerTmp->m_MachineMode)								//切割模式
		{
			if (FALSE == WriteEntitiesToBuffer(vecEntSortByLayer[nIndexLayer]))
				return FALSE;
		}
		else																		//遍历模式
		{
			////方法一
			////分别计算该层所有对象的ptBuf
			//std::vector<double (*)[2]> vecPtBuf;
			//std::vector<int> vecCtPt;;
			//for (auto valEnt : vecEntSortByLayer[nIndexLayer])
			//{
			//	int nPenNo = valEnt.nPenNo;

			//	//计算ptBuf并写卡
			//	BOOST_FOREACH(Line_T const& line, valEnt.mlineObj)
			//	{
			//		int nCountPt;
			//		double(*ptBuf)[2];
			//		nCountPt = (int)line.size();
			//		ptBuf = new double[nCountPt][2];
			//		for (int i = 0; i < nCountPt; i++)
			//		{
			//			ptBuf[i][0] = line[i].x();
			//			ptBuf[i][1] = line[i].y();
			//		}

			//		vecPtBuf.push_back(ptBuf);
			//		vecCtPt.push_back(nCountPt);
			//	}
			//}

			////依次导入每个对象的ptBuf，该过程重复n编
			//int nMarkLoop = pMachineParaLayerTmp->m_MachinePara.Times;
			//while (nMarkLoop--)
			//{
			//	for (int i; i < vecPtBuf.size(); i++)
			//	{
			//		if (FALSE == pDevCardMark->AddEntityLines(vecCtPt[i], vecPtBuf[i], nPenNo))
			//		{
			//			//delete[] ptBuf;
			//			return FALSE;
			//		}
			//	}
			//}

			//方法二
			int nMarkLoop = pMachineParaLayerTmp->m_MachinePara.Times;
			while (nMarkLoop--)
			{
				if (FALSE == WriteEntitiesToBuffer(vecEntSortByLayer[nIndexLayer]))
					return FALSE;
			}
		}
	}

	//保存加工文件
	if (!pDevCardMark->SaveEntityToFile())
		return FALSE;

	return TRUE;

	/*

	//分格中所有对象相对分格中心坐标,相当于平移
	VecEntities vecEntities;
	vecEntities = m_vecProcEntPerGridTrans.at(nGridIndex).vecEntitiesPerGrid;
	for (auto valEnt : vecEntities)
	{
		int nPenNo = valEnt.nPenNo;

		MLine_T mlineTmp, mlineTrans;
		mlineTmp = valEnt.mlineObj;
		bgt::translate_transformer<double, 2, 2> translate(-ptCenter.x(), -ptCenter.y());
		bg::transform(mlineTmp, mlineTrans, translate);

		BOOST_FOREACH(Line_T const& line, mlineTrans)
		{
		   	int nCountPt;
			double(*ptBuf)[2];
			nCountPt = (int)line.size();
			ptBuf = new double[nCountPt][2];

			for (int i = 0; i < nCountPt; i++)
			{
				ptBuf[i][0] = line[i].x();
				ptBuf[i][1] = line[i].y();
			}

			//将对象写入板卡缓冲区
			if (FALSE == pDevCardMark->AddEntityLines(nCountPt, ptBuf, nPenNo))
			{
				delete[] ptBuf;
				return FALSE;
			}
		}
	}

	*/
}

BOOL CPreProcess::WriteEntitiesToBuffer(VecEntities& vecEntities)
{
	for (auto valEnt : vecEntities)
	{
		int nPenNo = valEnt.nPenNo;

		//计算ptBuf并写卡
		BOOST_FOREACH(Line_T const& line, valEnt.mlineObj)
		{
			int nCountPt = (int)line.size();
			double(*ptBuf)[2] = new double[nCountPt][2];
			for (int i = 0; i < nCountPt; i++)
			{
				ptBuf[i][0] = line[i].x();
				ptBuf[i][1] = line[i].y();
			}

			//debug333
			//将对象写入板卡缓冲区
			if (FALSE == pDevCardMark->AddEntityLines(nCountPt, ptBuf, nPenNo))
			{
				//AfxMessageBox(_T("切割模式return flase"));
				delete[] ptBuf;
				ptBuf = NULL;
				return FALSE;
			}
			delete[] ptBuf;
			ptBuf = NULL;
		}
	}
	return TRUE;
}


void CPreProcess::CutEntityByGrid1(VecEntities& vecEntities, std::vector<Polygon_T>& vecPolyGrid)
{
	//CString strTime;
	//long t1 = GetTickCount();//程序段开始前取得系统运行时间(ms)            

	for (auto valGrid : vecPolyGrid)
	{
		PreProcessEntPerGrid prcEntPerGridTmp;
		VecEntities vecEntitiesPerGrid;

		for (auto iter = vecEntities.begin(); iter != vecEntities.end(); /*iter++*/)
		{
			//mlineObj -> mlineTmp，多段线与矩形相交后，可能裂变为多条线段
			PreProcessEntity myEntityTmp;
			MLine_T mlineTmp, mlineDiffer;

			//判断对象与分格单元的位置关系
			if (TRUE == bg::covered_by((*iter).mlineObj, valGrid))
			{
				//框内
				vecEntitiesPerGrid.push_back((*iter));
				iter = vecEntities.erase(iter);
			}
			else
			{
				//框外或相交
				if(bg::intersects((*iter).mlineObj, valGrid))
				{
					bg::intersection((*iter).mlineObj, valGrid, mlineTmp);
					bg::difference((*iter).mlineObj, mlineTmp, mlineDiffer);
					myEntityTmp.mlineObj = mlineTmp;
					myEntityTmp.nPenNo = (*iter).nPenNo;
					vecEntitiesPerGrid.push_back(myEntityTmp);
					(*iter).mlineObj = mlineDiffer;
				}
				iter++;
			}
		}
		prcEntPerGridTmp.polyGrid = valGrid;
		prcEntPerGridTmp.vecEntitiesPerGrid = vecEntitiesPerGrid;
		m_vecProcEntPerGrid.push_back(prcEntPerGridTmp);
	}

	//long t2 = GetTickCount();//程序段结束后取得系统运行时间(ms) 
	//long nCtSize = 0;
	//for (auto val1 : m_vecProcEntPerGrid)
	//{
	//	for (auto val2 : val1.vecEntitiesPerGrid)
	//	{
	//		for (auto val3 : val2.mlineObj)
	//		{
	//			nCtSize += val3.size();
	//		}
	//	}
	//}
	//strTime.Format(_T("Size: %d\ntime:%dms"), nCtSize, t2 - t1);//前后之差即程序运行时间        
	//AfxMessageBox(strTime);

}

void CPreProcess::CutEntityByGrid(VecEntities& vecEntities, std::vector<Polygon_T>& vecPolyGrid)
{
	//CString strTime;
	//long t1 = GetTickCount();//程序段开始前取得系统运行时间(ms)            

	for (auto valGrid : vecPolyGrid)
	{
		PreProcessEntPerGrid prcEntPerGridTmp;
		VecEntities vecEntitiesPerGrid;

		for (auto iter = vecEntities.begin(); iter != vecEntities.end(); /*iter++*/)
		{
			//mlineObj -> mlineTmp，多段线与矩形相交后，可能裂变为多条线段
			PreProcessEntity myEntityTmp;
			MLine_T mlineTmp, mlineDiffer;

			//判断对象与分格单元的位置关系
			bg::intersection((*iter).mlineObj, valGrid, mlineTmp);
			if (TRUE == mlineTmp.empty())				//如果在框外，啥也不做
			{
				iter++;
			}
			else
			{
				myEntityTmp.mlineObj = mlineTmp;
				myEntityTmp.nPenNo = (*iter).nPenNo;
				vecEntitiesPerGrid.push_back(myEntityTmp);

				bg::difference((*iter).mlineObj, mlineTmp, mlineDiffer);
				if (TRUE == mlineDiffer.empty())		//如果包含，在原表中删掉该对象
				{
					iter = vecEntities.erase(iter);
				}
				else									//如果相交，切分被grid选中的对象
				{
					(*iter).mlineObj = mlineDiffer;
					iter++;
				}
			}
		}
		prcEntPerGridTmp.polyGrid = valGrid;
		prcEntPerGridTmp.vecEntitiesPerGrid = vecEntitiesPerGrid;
		m_vecProcEntPerGrid.push_back(prcEntPerGridTmp);
	}

	//long t2 = GetTickCount();//程序段结束后取得系统运行时间(ms) 
	//long nCtSize = 0;
	//for (auto val1 : m_vecProcEntPerGrid)
	//{
	//	for (auto val2 : val1.vecEntitiesPerGrid)
	//	{
	//		for (auto val3 : val2.mlineObj)
	//		{
	//			nCtSize += val3.size();
	//		}
	//	}
	//}
	//strTime.Format(_T("Size: %d\ntime:%dms"), nCtSize, t2 - t1);//前后之差即程序运行时间        
	//AfxMessageBox(strTime);

}

void CPreProcess::CutEntityByGridOverlap(VecEntities& vecEntities, std::vector<Polygon_T>& vecPolyGrid, double fOverlapLength)
{
	//double fUltralimit = 1;

	////遍历每个分格
	//for (auto valGrid : vecPolyGrid)
	//{
	//	PreProcessEntPerGrid prcEntPerGridTmp;		//该valGrid分格中所有的对象+valGrid
	//	VecEntities vecEntitiesPerGrid;				//该valGrid分格中所有的对象
	//	//遍历分格中的每个对象
	//	for (auto& valEntity : vecEntities)
	//	{
	//		//mlineObj -> mlineTmp，多段线与矩形相交后，可能裂变为多条线段
	//		PreProcessEntity myEntityTmp;
	//		MLine_T mlineTmp, mlineDiffer;

	//		//遍历每个对象中的
	//		BOOST_FOREACH(Line_T const& line, valEntity.mlineObj)
	//		{
	//			//
	//			for (auto valPt : line)
	//			{
	//				//如果valPt是最后一个点，则退出循环
	//				if (bg::equals(valPt, line.end()))
	//					break;
	//			}


	//			MLine_T mlineIntersect;
	//			bg::intersection(line, valGrid, mlineIntersect);

	//			if (TRUE == mlineTmp.empty())
	//				continue;


	//		}

	//		bg::intersection(valEntity.mlineObj, valGrid, mlineTmp);
	//		
	//		//相交运算后筛选线段
	//		//1. 没有被切分的 -> 不用管
	//		//2. 被切分的且没有超限的 -> 选入该grid，vecEntities里删除该线段
	//		//3. 被切分的且超限的 -> //切分的两条线，分别在交点处，延长重叠长度；
	//								 //一个选入该grid，一个加入到vecEntities中；
	//								 //vecEntities里删除原线段


	//		myEntityTmp.mlineObj = mlineTmp;
	//		myEntityTmp.nPenNo = valEntity.nPenNo;
	//		vecEntitiesPerGrid.push_back(myEntityTmp);

	//		//切分被grid选中的对象
	//		if (FALSE == mlineTmp.empty())
	//		{
	//			bg::difference(valEntity.mlineObj, mlineTmp, mlineDiffer);
	//			valEntity.mlineObj = mlineDiffer;
	//		}

	//	}
	//	prcEntPerGridTmp.polyGrid = valGrid;
	//	prcEntPerGridTmp.vecEntitiesPerGrid = vecEntitiesPerGrid;
	//	m_vecProcEntPerGrid.push_back(prcEntPerGridTmp);
	//}
}


void CPreProcess::LoadMachineObjList(VecEntities& vecEntities, CMachineListContainer* pObjList)
{
	//先清空对象表
	vecEntities.clear();

	////写加工参数
	//SetPensFromAllLayers(pObjList);

	//读list
	CMachineObj_Comm* pObj;
	POSITION pos;
	pos = pObjList->GetObjHeadPosition();
	while (pos)
	{
		pObj = pObjList->GetObjNext(pos);
		LoadMachineObj(vecEntities, pObj);
	}

	////保存数据库到文件中
	//if (!SaveEntityToFile())
	//	return;
}

void CPreProcess::LoadMachineObj(VecEntities& vecEntities, CMachineObj_Comm* pObj)
{
	//滤过不加工的层
	if (LayerNum_Default > pObj->m_ObjByLayer)
		return;

	//提前初始化供case MachineObj_Type_Polyline使用
	ObjPoint ptPolylineLast, ptPolyline;
	int nLayer = pObj->m_ObjByLayer;
	std::vector<ObjPoint> vecPolylinePtBuf;

	switch (pObj->GetObjType())
	{
	case MachineObj_Type_Point:
		//***DrawPoint***
		CMachineObjPoint* pPoint;
		pPoint = (CMachineObjPoint*)pObj;
		break;
	case MachineObj_Type_Line:
		//***DrawLine***
		CMachineObjLine* pLine;
		pLine = (CMachineObjLine*)pObj;

		double ptPos[2][2];
		ptPos[0][0] = pLine->GetLineStart().x;
		ptPos[0][1] = pLine->GetLineStart().y;
		ptPos[1][0] = pLine->GetLineEnd().x;
		ptPos[1][1] = pLine->GetLineEnd().y;

		AddEntityLines(vecEntities, 2, ptPos, nLayer);
		break;
	case MachineObj_Type_Circle:
		//***DrawCircle***
		CMachineObjCircle* pCircle;
		pCircle = (CMachineObjCircle*)pObj;

		double ptCircleCenterPos[2], fCircleRadius, fCircleAngleStart;
		int nCircleDir;
		ptCircleCenterPos[0] = pCircle->GetCircleCenter().x;
		ptCircleCenterPos[1] = pCircle->GetCircleCenter().y;
		fCircleRadius = pCircle->GetCircleRadius();
		fCircleAngleStart = 90 * (double)pCircle->GetCircleStartNum();
		nCircleDir = (int)pCircle->GetCircleDir();

		AddEntityCircle(vecEntities, ptCircleCenterPos, fCircleRadius, fCircleAngleStart, nCircleDir, nLayer);
		break;
	case MachineObj_Type_Arc:
		//***DrawArc***
		CMachineObjArc* pArc;
		pArc = (CMachineObjArc*)pObj;

		double ptArcCenterPos[2], fArcRadius, fArcAngleStart, fArcAngleEnd;
		int nArcDir;
		ptArcCenterPos[0] = pArc->GetArcCenter().x;
		ptArcCenterPos[1] = pArc->GetArcCenter().y;
		fArcRadius = pArc->GetArcRadius();
		fArcAngleStart = pArc->GetStartAngle();
		fArcAngleEnd = pArc->GetEndAngle();
		nArcDir = (int)pArc->GetArcDir();

		AddEntityArc(vecEntities, ptArcCenterPos, fArcRadius, fArcAngleStart, fArcAngleEnd, nArcDir, nLayer);
		break;
	case MachineObj_Type_Ellipse:
		//***DrawEllipse***
		CMachineObjEllipse* pEllipse;
		pEllipse = (CMachineObjEllipse*)pObj;
		break;
	case MachineObj_Type_ArcEll:
		//***DrawArcEll***
		CMachineObjArcEll* pArcEll;
		pArcEll = (CMachineObjArcEll*)pObj;
		break;
	case MachineObj_Type_TiltEllipse:
		//***DrawTiltEllipse***
		CMachineObjTiltEllipse* pTiltEllArc;
		pTiltEllArc = (CMachineObjTiltEllipse*)pObj;
		break;
	case MachineObj_Type_Polyline:
		//***DrawPolyline***
		CMachineObjPolyline* pPolyline;
		pPolyline = (CMachineObjPolyline*)pObj;

		int nVertCount;
		double fConvexityLast;
		//ObjPoint ptPolylineLast, ptPolyline;

		nVertCount = pPolyline->GetPolylineVertexCount();
		ptPolylineLast.x = pPolyline->GetPolylineStart().x;
		ptPolylineLast.y = pPolyline->GetPolylineStart().y;
		fConvexityLast = pPolyline->GetPolylineStart().convexity;

		vecPolylinePtBuf.clear();
		vecPolylinePtBuf.push_back(ObjPoint(ptPolylineLast.x, ptPolylineLast.y));

		for (int i = 1; i < nVertCount; i++)
		{
			//读当前顶点i
			ptPolyline.x = pPolyline->GetPolylinePoint(i).x;
			ptPolyline.y = pPolyline->GetPolylinePoint(i).y;

			//检查上一个顶点i-1的凸度
			if (0 == fConvexityLast)
			{
				//顶点i-1 to 顶点i 是直线
				//double ptPos[2][2];
				//ptPos[0][0] = ptPolylineLast.x;
				//ptPos[0][1] = ptPolylineLast.y;
				//ptPos[1][0] = ptPolyline.x;
				//ptPos[1][1] = ptPolyline.y;
				//AddEntityLines(vecEntities, 2, ptPos, nLayer);
				vecPolylinePtBuf.push_back(ObjPoint(ptPolyline.x, ptPolyline.y));
			}
			else
			{
				//顶点i-1 to 顶点i 是圆弧
				ObjPoint ArcCenter;
				double /*ptArcCenterPos[2],*/ fArcRadius, fArcAngleStart, fArcAngleEnd;
				int nArcDir;

				pPolyline->TranPolylineToArc(
					ptPolylineLast, ptPolyline, fConvexityLast,
					&ArcCenter, &fArcRadius, &fArcAngleStart, &fArcAngleEnd);
				//ptArcCenterPos[0] = ArcCenter.x;
				//ptArcCenterPos[1] = ArcCenter.y;
				nArcDir = (fArcAngleStart > fArcAngleEnd) ? (int)CW : (int)CCW;

				//AddEntityArc(vecEntities, ptArcCenterPos, fArcRadius, fArcAngleStart, fArcAngleEnd, nArcDir, nLayer);

				//解析圆弧上插补坐标
				int nCountPt;
				double fAngleDelt = fArcAngleEnd - fArcAngleStart;
				double fMarkArcStepAngle;
				fMarkArcStepAngle = m_fMarkArcStep / fArcRadius * 180 / M_PI;
				ObjPoint ptTmp;

				if (0 == nArcDir)//顺时针
				{
					if (0 <= fAngleDelt)//劣弧
						nCountPt = (int)(((M_PI * (360 - fAngleDelt) / 180) * fArcRadius) / m_fMarkArcStep) + 1;
					else//优弧
						nCountPt = (int)(((M_PI * abs(fAngleDelt) / 180) * fArcRadius) / m_fMarkArcStep) + 1;

					//ptBuf = new double[nCountPt][2];

					for (int i = 0; i < nCountPt - 1; i++)
					{
						//ptBuf[i][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleStart / 180);
						//ptBuf[i][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleStart / 180);

						fArcAngleStart -= fMarkArcStepAngle;
						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
						vecPolylinePtBuf.push_back(ptTmp);
					}

					//ptBuf[nCountPt - 1][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleEnd / 180);
					//ptBuf[nCountPt - 1][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleEnd / 180);
					ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
					ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
					vecPolylinePtBuf.push_back(ptTmp);
				}
				else if (1 == nArcDir)//逆时针
				{
					if (0 <= fAngleDelt)//优弧
						nCountPt = (int)(((M_PI * fAngleDelt / 180) * fArcRadius) / m_fMarkArcStep) + 1;
					else//劣弧
						nCountPt = (int)(((M_PI * (360 - abs(fAngleDelt)) / 180) * fArcRadius) / m_fMarkArcStep) + 1;

					//ptBuf = new double[nCountPt][2];

					for (int i = 0; i < nCountPt - 1; i++)
					{
						//ptBuf[i][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleStart / 180);
						//ptBuf[i][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleStart / 180);
						fArcAngleStart += fMarkArcStepAngle;
						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
						vecPolylinePtBuf.push_back(ptTmp);
					}

					//ptBuf[nCountPt - 1][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleEnd / 180);
					//ptBuf[nCountPt - 1][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleEnd / 180);
					ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
					ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
					vecPolylinePtBuf.push_back(ptTmp);

				}


			}

			//更新ptPolylineLast，fConvexityLast
			if (i < nVertCount - 1)
			{
				ptPolylineLast = ptPolyline;
				fConvexityLast = pPolyline->GetPolylinePoint(i).convexity;
			}
		}

		//所有多段线读取完成,一起写入缓冲区
		int nCountPtPolyline;
		double(*ptBuf)[2];

		nCountPtPolyline = (int)vecPolylinePtBuf.size();
		ptBuf = new double[nCountPtPolyline][2];
		for (int i = 0; i < nCountPtPolyline; i++)
		{
			ptBuf[i][0] = vecPolylinePtBuf[i].x;
			ptBuf[i][1] = vecPolylinePtBuf[i].y;
		}
		AddEntityLines(vecEntities, nCountPtPolyline, ptBuf, nLayer);
		delete[] ptBuf;
		break;

	case MachineObj_Type_WLine:
		CMachineObjWLine* pWLine;
		int nCountWLine;
		pWLine = (CMachineObjWLine*)pObj;

		double ptPosWLine[2][2];
		ptPosWLine[0][0] = pWLine->GetLineStart().x;
		ptPosWLine[0][1] = pWLine->GetLineStart().y;
		ptPosWLine[1][0] = pWLine->GetLineEnd().x;
		ptPosWLine[1][1] = pWLine->GetLineEnd().y;
		AddEntityLines(vecEntities, 2, ptPosWLine, nLayer);

		nCountWLine = pWLine->m_pDupList.size();
		for (int i = 0; i < nCountWLine; i++)
		{
			double ptPos[2][2];
			ptPos[0][0] = pWLine->m_pDupList[i]->front().x;
			ptPos[0][1] = pWLine->m_pDupList[i]->front().y;
			ptPos[1][0] = pWLine->m_pDupList[i]->back().x;
			ptPos[1][1] = pWLine->m_pDupList[i]->back().y;

			AddEntityLines(vecEntities, 2, ptPos, nLayer);
		}
		break;

	case MachineObj_Type_WArc:
		CMachineObjWArc* pWArc;
		int nCountWArc;
		pWArc = (CMachineObjWArc*)pObj;

		double ptWArcCenterPos[2];
		int nWArcDir;
		double fWArcRadius, fWArcAngleStart, fWArcAngleEnd;
		ptWArcCenterPos[0] = pWArc->GetArcCenter().x;
		ptWArcCenterPos[1] = pWArc->GetArcCenter().y;
		nWArcDir = (int)pWArc->GetArcDir();
		fWArcRadius = pWArc->GetArcRadius();
		fWArcAngleStart = pWArc->GetStartAngle();
		fWArcAngleEnd = pWArc->GetEndAngle();
		AddEntityArc(vecEntities, ptWArcCenterPos, fWArcRadius, fWArcAngleStart, fWArcAngleEnd, nWArcDir, nLayer);

		nCountWArc= pWArc->m_pDupList.size();
		for (int i = 0; i < nCountWArc; i++)
		{
			double fWArcRadius, fWArcAngleStart, fWArcAngleEnd;
			VectPoint vecWArc;
			vecWArc = *pWArc->m_pDupList[i];

			fWArcRadius = vecWArc[2].x;
			fWArcAngleStart = vecWArc[1].x;
			fWArcAngleEnd = vecWArc[1].y;
			AddEntityArc(vecEntities, ptWArcCenterPos, fWArcRadius, fWArcAngleStart, fWArcAngleEnd, nWArcDir, nLayer);
		}
		break;

	case MachineObj_Type_WPolyline:
		CMachineObjWPolyline* pWPolyline;
		int nCountWPolyline;
		pWPolyline = (CMachineObjWPolyline*)pObj;

		{
		//原图
		int nVertCount;
		double fConvexityLast;
		nVertCount = pWPolyline->GetPolylineVertexCount();
		ptPolylineLast.x = pWPolyline->GetPolylineStart().x;
		ptPolylineLast.y = pWPolyline->GetPolylineStart().y;
		fConvexityLast = pWPolyline->GetPolylineStart().convexity;

		vecPolylinePtBuf.clear();
		vecPolylinePtBuf.push_back(ObjPoint(ptPolylineLast.x, ptPolylineLast.y));
		for (int i = 1; i < nVertCount; i++)
		{
			//读当前顶点i
			ptPolyline.x = pWPolyline->GetPolylinePoint(i).x;
			ptPolyline.y = pWPolyline->GetPolylinePoint(i).y;

			//检查上一个顶点i-1的凸度
			if (0 == fConvexityLast)
			{
				//顶点i-1 to 顶点i 是直线
				vecPolylinePtBuf.push_back(ObjPoint(ptPolyline.x, ptPolyline.y));
			}
			else
			{
				//顶点i-1 to 顶点i 是圆弧
				ObjPoint ArcCenter;
				double fArcRadius, fArcAngleStart, fArcAngleEnd;
				int nArcDir;

				pWPolyline->TranPolylineToArc(
					ptPolylineLast, ptPolyline, fConvexityLast,
					&ArcCenter, &fArcRadius, &fArcAngleStart, &fArcAngleEnd);
				nArcDir = (fArcAngleStart > fArcAngleEnd) ? (int)CW : (int)CCW;

				//解析圆弧上插补坐标
				int nCountPt;
				double fAngleDelt = fArcAngleEnd - fArcAngleStart;
				double fMarkArcStepAngle;
				fMarkArcStepAngle = m_fMarkArcStep / fArcRadius * 180 / M_PI;
				ObjPoint ptTmp;

				if (0 == nArcDir)//顺时针
				{
					if (0 <= fAngleDelt)//劣弧
						nCountPt = (int)(((M_PI * (360 - fAngleDelt) / 180) * fArcRadius) / m_fMarkArcStep) + 1;
					else//优弧
						nCountPt = (int)(((M_PI * abs(fAngleDelt) / 180) * fArcRadius) / m_fMarkArcStep) + 1;

					for (int i = 0; i < nCountPt - 1; i++)
					{

						fArcAngleStart -= fMarkArcStepAngle;
						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
						vecPolylinePtBuf.push_back(ptTmp);
					}
					ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
					ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
					vecPolylinePtBuf.push_back(ptTmp);
				}
				else if (1 == nArcDir)//逆时针
				{
					if (0 <= fAngleDelt)//优弧
						nCountPt = (int)(((M_PI * fAngleDelt / 180) * fArcRadius) / m_fMarkArcStep) + 1;
					else//劣弧
						nCountPt = (int)(((M_PI * (360 - abs(fAngleDelt)) / 180) * fArcRadius) / m_fMarkArcStep) + 1;
					for (int i = 0; i < nCountPt - 1; i++)
					{
						fArcAngleStart += fMarkArcStepAngle;
						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
						vecPolylinePtBuf.push_back(ptTmp);
					}
					ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
					ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
					vecPolylinePtBuf.push_back(ptTmp);
				}
			}
			//更新ptPolylineLast，fConvexityLast
			if (i < nVertCount - 1)
			{
				ptPolylineLast = ptPolyline;
				fConvexityLast = pWPolyline->GetPolylinePoint(i).convexity;
			}
		}

		//所有多段线读取完成,一起写入缓冲区
		int nCountPtPolyline;
		double(*ptBuf)[2];
		nCountPtPolyline = (int)vecPolylinePtBuf.size();
		ptBuf = new double[nCountPtPolyline][2];
		for (int i = 0; i < nCountPtPolyline; i++)
		{
			ptBuf[i][0] = vecPolylinePtBuf[i].x;
			ptBuf[i][1] = vecPolylinePtBuf[i].y;
		}
		AddEntityLines(vecEntities, nCountPtPolyline, ptBuf, nLayer);
		delete[] ptBuf;
		//原图//

		}

		nCountWPolyline = pWPolyline->m_pDupList.size();
		for (int j = 0; j < nCountWPolyline; j++)
		{
			VectVpoint pVpoint = *pWPolyline->m_pDupList[j];
		
			//***DrawPolyline***
			//CMachineObjPolyline* pPolyline;
			//pPolyline = (CMachineObjPolyline*)pObj;

			int nVertCount;
			double fConvexityLast;
			//ObjPoint ptPolylineLast, ptPolyline;

			nVertCount = pVpoint.size();
			ptPolylineLast.x = pVpoint.front().x;
			ptPolylineLast.y = pVpoint.front().y;
			fConvexityLast = pVpoint.front().convexity;
			//nVertCount = pPolyline->GetPolylineVertexCount();
			//ptPolylineLast.x = pPolyline->GetPolylineStart().x;
			//ptPolylineLast.y = pPolyline->GetPolylineStart().y;
			//fConvexityLast = pPolyline->GetPolylineStart().convexity;

			vecPolylinePtBuf.clear();
			vecPolylinePtBuf.push_back(ObjPoint(ptPolylineLast.x, ptPolylineLast.y));

			for (int i = 1; i < nVertCount; i++)
			{
				//读当前顶点i
				ptPolyline.x = pVpoint[i].x;
				ptPolyline.y = pVpoint[i].y;
				//ptPolyline.x = pPolyline->GetPolylinePoint(i).x;
				//ptPolyline.y = pPolyline->GetPolylinePoint(i).y;

				//检查上一个顶点i-1的凸度
				if (0 == fConvexityLast)
				{
					//顶点i-1 to 顶点i 是直线
					vecPolylinePtBuf.push_back(ObjPoint(ptPolyline.x, ptPolyline.y));
				}
				else
				{
					//顶点i-1 to 顶点i 是圆弧
					ObjPoint ArcCenter;
					double fArcRadius, fArcAngleStart, fArcAngleEnd;
					int nArcDir;

					pWPolyline->TranPolylineToArc(
						ptPolylineLast, ptPolyline, fConvexityLast,
						&ArcCenter, &fArcRadius, &fArcAngleStart, &fArcAngleEnd);
					nArcDir = (fArcAngleStart > fArcAngleEnd) ? (int)CW : (int)CCW;

					//解析圆弧上插补坐标
					int nCountPt;
					double fAngleDelt = fArcAngleEnd - fArcAngleStart;
					double fMarkArcStepAngle;
					fMarkArcStepAngle = m_fMarkArcStep / fArcRadius * 180 / M_PI;
					ObjPoint ptTmp;

					if (0 == nArcDir)//顺时针
					{
						if (0 <= fAngleDelt)//劣弧
							nCountPt = (int)(((M_PI * (360 - fAngleDelt) / 180) * fArcRadius) / m_fMarkArcStep) + 1;
						else//优弧
							nCountPt = (int)(((M_PI * abs(fAngleDelt) / 180) * fArcRadius) / m_fMarkArcStep) + 1;

						for (int i = 0; i < nCountPt - 1; i++)
						{
							fArcAngleStart -= fMarkArcStepAngle;
							ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
							ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
							vecPolylinePtBuf.push_back(ptTmp);
						}
						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
						vecPolylinePtBuf.push_back(ptTmp);
					}
					else if (1 == nArcDir)//逆时针
					{
						if (0 <= fAngleDelt)//优弧
							nCountPt = (int)(((M_PI * fAngleDelt / 180) * fArcRadius) / m_fMarkArcStep) + 1;
						else//劣弧
							nCountPt = (int)(((M_PI * (360 - abs(fAngleDelt)) / 180) * fArcRadius) / m_fMarkArcStep) + 1;

						for (int i = 0; i < nCountPt - 1; i++)
						{
							fArcAngleStart += fMarkArcStepAngle;
							ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
							ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
							vecPolylinePtBuf.push_back(ptTmp);
						}
						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
						vecPolylinePtBuf.push_back(ptTmp);
					}
				}

				//更新ptPolylineLast，fConvexityLast
				if (i < nVertCount - 1)
				{
					ptPolylineLast = ptPolyline;
					fConvexityLast = pVpoint[i].convexity;
					//fConvexityLast = pPolyline->GetPolylinePoint(i).convexity;
				}
			}

			//所有多段线读取完成,一起写入缓冲区
			int nCountPtPolyline;
			double(*ptBuf)[2];

			nCountPtPolyline = (int)vecPolylinePtBuf.size();
			ptBuf = new double[nCountPtPolyline][2];
			for (int i = 0; i < nCountPtPolyline; i++)
			{
				ptBuf[i][0] = vecPolylinePtBuf[i].x;
				ptBuf[i][1] = vecPolylinePtBuf[i].y;
			}
			AddEntityLines(vecEntities, nCountPtPolyline, ptBuf, nLayer);
			delete[] ptBuf;
		}
		break;

	case MachineObj_Type_FillPolyline:
		break;

	case MachineObj_Type_Group:
		//***DrawGroup***
		CMachineObjGroup* pGroup;
		CMachineObjList* pList;
		pGroup = (CMachineObjGroup*)pObj;
		pList = pGroup->GetObjList();
		POSITION pos;
		pos = pList->GetHeadPosition();
		while (pos)
		{
			CMachineObj_Comm* pObj;
			pObj = pList->GetNext(pos);
			LoadMachineObj(vecEntities, pObj);
		}
		break;
		//*****新类型待加****
	default:
		break;
	}

}

void CPreProcess::AddEntityLines(VecEntities& vecEntities, int nCount, double ptPos[][2], int nPenNo)
{
	//由objList导入时，mlineTmp只能可能是连续多段线
	//而在分格后导入板卡缓冲区时，mlineTmp可能裂变为多条多段线

	PreProcessEntity myEntity;
	myEntity.nPenNo = nPenNo;

	MLine_T	mlineTmp;
	mlineTmp.resize(1);
	for (int i = 0; i < nCount; i++)
	{
		bg::append(mlineTmp[0], Point_T(ptPos[i][0], ptPos[i][1]));
	}
	myEntity.mlineObj = mlineTmp;

	vecEntities.push_back(myEntity);
}
void CPreProcess::AddEntityCircle(VecEntities& vecEntities, double ptCenterPos[2], double fRadius, double fAngleStart, int nDir, int nPenNo)
{
	//解析圆上插补坐标
	int nCountPt;
	double(*ptBuf)[2];
	double fMarkArcStepAngle;

	fMarkArcStepAngle = m_fMarkArcStep / fRadius * 180 / M_PI;
	nCountPt = (int)((2 * M_PI * fRadius) / m_fMarkArcStep) + 2;
	ptBuf = new double[nCountPt][2];

	if (0 == nDir)//顺时针
	{
		for (int i = 0; i < nCountPt - 1; i++)
		{
			ptBuf[i][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleStart / 180);
			ptBuf[i][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleStart / 180);
			fAngleStart -= fMarkArcStepAngle;
		}

		ptBuf[nCountPt - 1][0] = ptBuf[0][0];
		ptBuf[nCountPt - 1][1] = ptBuf[0][1];
	}
	else if (1 == nDir)//逆时针
	{
		for (int i = 0; i < nCountPt - 1; i++)
		{
			ptBuf[i][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleStart / 180);
			ptBuf[i][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleStart / 180);
			fAngleStart += fMarkArcStepAngle;
		}

		ptBuf[nCountPt - 1][0] = ptBuf[0][0];
		ptBuf[nCountPt - 1][1] = ptBuf[0][1];
	}
	else
	{
		return;
	}


	AddEntityLines(vecEntities, nCountPt, ptBuf, nPenNo);

}


void CPreProcess::AddEntityArc(VecEntities& vecEntities, double ptCenterPos[2], double fRadius, double fAngleStart, double fAngleEnd, int nDir, int nPenNo)
{
	//解析圆弧上插补坐标
	int nCountPt;
	double(*ptBuf)[2];
	double fAngleDelt = fAngleEnd - fAngleStart;
	double fMarkArcStepAngle;
	fMarkArcStepAngle = m_fMarkArcStep / fRadius * 180 / M_PI;

	if (0 == nDir)//顺时针
	{
		if (0 <= fAngleDelt)//劣弧
			nCountPt = (int)(((M_PI * (360 - fAngleDelt) / 180) * fRadius) / m_fMarkArcStep) + 2;
		else//优弧
			nCountPt = (int)(((M_PI * abs(fAngleDelt) / 180) * fRadius) / m_fMarkArcStep) + 2;

		ptBuf = new double[nCountPt][2];

		for (int i = 0; i < nCountPt - 1; i++)
		{
			ptBuf[i][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleStart / 180);
			ptBuf[i][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleStart / 180);
			fAngleStart -= fMarkArcStepAngle;
		}

		ptBuf[nCountPt - 1][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleEnd / 180);
		ptBuf[nCountPt - 1][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleEnd / 180);
	}
	else if (1 == nDir)//逆时针
	{
		if (0 <= fAngleDelt)//优弧
			nCountPt = (int)(((M_PI * fAngleDelt / 180) * fRadius) / m_fMarkArcStep) + 2;
		else//劣弧
			nCountPt = (int)(((M_PI * (360 - abs(fAngleDelt)) / 180) * fRadius) / m_fMarkArcStep) + 2;

		ptBuf = new double[nCountPt][2];

		for (int i = 0; i < nCountPt - 1; i++)
		{
			ptBuf[i][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleStart / 180);
			ptBuf[i][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleStart / 180);
			fAngleStart += fMarkArcStepAngle;
		}

		ptBuf[nCountPt - 1][0] = ptCenterPos[0] + fRadius * cos(M_PI * fAngleEnd / 180);
		ptBuf[nCountPt - 1][1] = ptCenterPos[1] + fRadius * sin(M_PI * fAngleEnd / 180);
	}
	else
	{
		return;
	}


	AddEntityLines(vecEntities, nCountPt, ptBuf, nPenNo);

}

//抓靶相关
int CPreProcess::GenMarkPoints(std::vector <CPointF> vPtPosDestinedMark, std::vector <HalconModel> vMarkPointModel, CMachineListContainer* pObjList)
{
	vPtPosDestinedMark.resize(0);
	vMarkPointModel.resize(0);

	POSITION pos;
	CMachineObj_Comm* pObj;
	ObjRect objRectTmp;
	int nLayerMark, nLayerTmp;
	int nIndexMarkPoint = 0;

	nLayerMark = pObjList->FindLayerByName(LayerName_Mark);
	if (0 > nLayerMark)
		return FALSE;

	pos = pObjList->GetObjHeadPosition();
	while (pos)
	{
		pObj = pObjList->GetObjNext(pos);
		nLayerTmp = pObj->m_ObjByLayer;
		if (nLayerTmp != nLayerMark)
			continue;

		//找到一个mark点，获得mark点中心坐标
		objRectTmp = pObj->GetObjBound();
		CPointF ptTmp((FLOAT)(objRectTmp.max_x + objRectTmp.min_x) / 2,
			(FLOAT)(objRectTmp.max_y + objRectTmp.min_y) / 2);
		vPtPosDestinedMark.push_back(ptTmp);

		//生成mark点模板
		HalconModel hModelTmp;
		if (FALSE == GenMarkPointModel(&hModelTmp, pObj))
		{
			continue;
		}
		vMarkPointModel.push_back(hModelTmp);

		nIndexMarkPoint++;
		if (2 <= nIndexMarkPoint)
			break;
	}

	if (2 != nIndexMarkPoint)
	{
		AfxMessageBox(_T("请设置至少两个mark点\nMark点类型为圆或十字叉"));
		return 0;
	}

	return nIndexMarkPoint;
}
BOOL CPreProcess::GenMarkPointModel(HalconModel* pHalconModel, CMachineObj_Comm* pObj, double fCrossWidth)
{
	if (MachineObj_Type_Circle == pObj->GetObjType())
	{
		CMachineObjCircle* pObjCircle = (CMachineObjCircle*)pObj;
		double fRadius = pObjCircle->GetCircleRadius();
		double fPixelSize = ReadDevCameraPixelSize();

		HalconModel hModelTmp(_T("圆"), fRadius, fPixelSize);
		*pHalconModel = hModelTmp;
	}
	else if (MachineObj_Type_Group == pObj->GetObjType())
	{
		ObjRect rtBorder = pObj->GetObjBound();
		double fCrossLength = rtBorder.max_x - rtBorder.min_x;
		double fPixelSize = ReadDevCameraPixelSize();

		HalconModel hModelTmp(_T("十字叉"), fCrossLength, fCrossWidth, fPixelSize);
		*pHalconModel = hModelTmp;
	}

	return TRUE;
}

int CPreProcess::GenMarkPoints(std::vector <CPointF>& vPtPosDestinedMark, std::vector <ModelBase>& vModelBase, CMachineListContainer* pObjList)
{
	vPtPosDestinedMark.resize(0);
	vModelBase.resize(0);

	POSITION pos;
	CMachineObj_Comm* pObj;
	ObjRect objRectTmp;
	int nLayerMark, nLayerTmp;
	int nIndexMarkPoint = 0;

	nLayerMark = pObjList->FindLayerByName(LayerName_Mark);
	if (0 > nLayerMark)
		return FALSE;

	pos = pObjList->GetObjHeadPosition();
	while (pos)
	{
		pObj = pObjList->GetObjNext(pos);
		nLayerTmp = pObj->m_ObjByLayer;
		if (nLayerTmp != nLayerMark)
			continue;

		//找到一个mark点
		//生成mark点模板
		ModelBase* pModel = NULL;
		if (FALSE == GenMarkPointModel(&pModel, pObj, pObjList))
		{
			continue;
		}
		if (NULL != pModel)
		{
			vModelBase.push_back(*pModel);
			delete pModel;
			pModel = NULL;
		}

		//获得mark点中心坐标
		objRectTmp = pObj->GetObjBound();
		CPointF ptTmp((FLOAT)(objRectTmp.max_x + objRectTmp.min_x) / 2,
			(FLOAT)(objRectTmp.max_y + objRectTmp.min_y) / 2);
		vPtPosDestinedMark.push_back(ptTmp);

		nIndexMarkPoint++;
		if (2 <= nIndexMarkPoint)
			break;
	}

	if (2 != nIndexMarkPoint)
	{
		AfxMessageBox(_T("请设置至少两个mark点\nMark点类型为圆或十字叉"));
		return 0;
	}

	return nIndexMarkPoint;
}
BOOL CPreProcess::GenMarkPointModel(ModelBase **ppModel, CMachineObj_Comm* pObj, CMachineListContainer* pList)
{
	double fPixelSize = ReadDevCameraPixelSize();
	double fCameraPosX = ReadDevCameraPosX();
	double fCameraPosY = ReadDevCameraPosY();
	CPointF ptCameraPos(fCameraPosX, fCameraPosY);
	ObjRect rtListBorder = pList->GetObjBound();
	CPointF ptListCenter((rtListBorder.max_x + rtListBorder.min_x) / 2, (rtListBorder.max_y + rtListBorder.min_y) / 2);

	if (MachineObj_Type_Circle == pObj->GetObjType())
	{
		CMachineObjCircle* pObjCircle = (CMachineObjCircle*)pObj;
		double fRadius = pObjCircle->GetCircleRadius();
		CPointF ptObjCenter(pObjCircle->GetCircleCenter().x, pObjCircle->GetCircleCenter().y);

		*ppModel = ModelFactory::creatModel(ModelType::MT_Circle, fPixelSize, fRadius);
		(*ppModel)->SetMatchDomain(ptObjCenter - ptListCenter + ptCameraPos, 4);
	}
	else if (MachineObj_Type_Group == pObj->GetObjType())
	{
		ObjRect rtObjBorder = pObj->GetObjBound();
		double fCrossLength = rtObjBorder.max_x - rtObjBorder.min_x;
		CPointF ptObjCenter((rtObjBorder.max_x + rtObjBorder.min_x) / 2, (rtObjBorder.max_y + rtObjBorder.min_y) / 2);

		*ppModel = ModelFactory::creatModel(ModelType::MT_Cross, fPixelSize, fCrossLength, 0.25);
		(*ppModel)->SetMatchDomain(ptObjCenter - ptListCenter + ptCameraPos, 4);
	}
	else
		return FALSE;

	return TRUE;
}
int CPreProcess::FindMarkPoints(std::vector <CPointF>& vPtPosRealMark, std::vector <ModelBase>& vModelBase)
{
	vPtPosRealMark.clear();

	int nFinded = 0;
	int nIndex = 0;
	CString strCaption, strText;

	for (auto valModel : vModelBase)
	{
		nIndex++;
		CPointF ptFinded(0,0);
		std::vector <CPointF> vPtPos;
		std::vector <double> vFAngle;
		valModel.LocateModel(vPtPos, vFAngle);

		int nCount = vPtPos.size();
		if (0 >= nCount)
		{
			strText.Format(_T("没有匹配对象"));
			strCaption.Format(_T("第%d个mark点"), nIndex);
			::MessageBox(NULL, strText, strCaption, MB_OK);
		}
		else if (1 < nCount)
		{
			strText.Format(_T("共匹配到%d个对象，请重新设置"), nCount);
			strCaption.Format(_T("第%d个mark点"), nIndex);
			::MessageBox(NULL, strText, strCaption, MB_OK);
		}
		else
		{
			nFinded++;
			ptFinded = vPtPos.front();
		}
		vPtPosRealMark.push_back(ptFinded);
	}

	return nFinded;
}


BOOL CPreProcess::AutoPreProcess1(CMachineListContainer* pList, BOOL bLocate)
{
	if (FALSE == DoSingleGrid(pList))
		return FALSE;

	//判断是否需要抓靶定位
	int nCountMarkPoints = 0;
	std::vector <CPointF> vPtPosDestinedMark;
	std::vector <CPointF> vPtPosRealMark;
	std::vector <ModelBase> vModeBase;
	if (bLocate == FALSE)
	{
		nCountMarkPoints = 0;
		vPtPosDestinedMark.resize(0);
		vPtPosRealMark.resize(0);
		vModeBase.resize(0);
	}
	else
	{
		//抓靶定位流程
		//读mark层生成mark点理论坐标及其model,只支持两个mark点
		nCountMarkPoints = GenMarkPoints(vPtPosDestinedMark, vModeBase, pList);
		if (2 != nCountMarkPoints)
			return FALSE;
		//依次抓靶生成mark点实际坐标
		nCountMarkPoints = FindMarkPoints(vPtPosRealMark, vModeBase);
		if (2 != nCountMarkPoints)
			return FALSE;
	}

	//计算平移旋转
	DoSingleTrans(pList, nCountMarkPoints, vPtPosDestinedMark, vPtPosRealMark);


	return TRUE;
}

BOOL CPreProcess::AutoPreProcess2(CMachineListContainer* pList, BOOL bLocate)
{
	if (FALSE == DoSingleGrid(pList))
		return FALSE;

	//判断是否需要抓靶定位
	double ptOrg[2], ptScale[2], fRotateDegree, ptReal[2];
	if (bLocate == FALSE)
	{

		//计算平移旋转拉伸
		//无mark点 以链表外框中心为锚点，以相机视角dxf位置计算旋转偏移
		ObjRect rtBound = pList->GetObjBound();
		ptOrg[0] = (rtBound.max_x + rtBound.min_x) / 2;
		ptOrg[1] = (rtBound.max_y + rtBound.min_y) / 2;
		ptScale[0] = 1;
		ptScale[1] = 1;
		fRotateDegree = -g_fDxfRotate;		//ccw to cw
		ptReal[0] = g_ptDxfTranslate.x;
		ptReal[1] = g_ptDxfTranslate.y;
	}
	else
	{
		//抓靶定位流程
		//生成border层多段线点阵
		std::vector<CPointF> vPtBorder;
		vPtBorder = GetBorderPtArray(pList);
		if (0 == vPtBorder.size())
			return FALSE;

		//根据border点阵生成匹配模板
		int nCountFinded = 0;
		double fRadius, fPixelSize, fScaleMin, fScaleMax, fMinScore, fPosCameraX, fPosCameraY;
		fRadius = ReadDevCameraMarkCircleRadius();
		fPixelSize = ReadDevCameraPixelSize();
		fMinScore = ReadDevCameraMarkCircleFindMinScore();
		fScaleMin = ReadDevCameraMarkCircleFindScaleMin();
		fScaleMax = ReadDevCameraMarkCircleFindScaleMax();
		fPosCameraX = ReadDevCameraPosX();
		fPosCameraY = ReadDevCameraPosY();

		ModelBase* pModel = ModelFactory::creatModel(ModelType::MT_ClosedPolyline, fPixelSize, vPtBorder);
		pModel->SetScale(fScaleMin, fScaleMax);
		pModel->SetMinScore(fMinScore);
		std::vector<CPointF> vecPtPos;
		std::vector <double> vFAngle;
		nCountFinded = pModel->LocateModel(vecPtPos, vFAngle);
		delete pModel;
		pModel = NULL;

		if (1 != nCountFinded)
		{
			AfxMessageBox(_T("没有找到唯一的加工对象外框"));
			return FALSE;
		}

		CPointF ptBorderCenter;
		if (FALSE == GetBorderCenter(pList, &ptBorderCenter))
		{
			AfxMessageBox(_T("无法获得外框中心"));
			return FALSE;
		}

		ptOrg[0] = ptBorderCenter.x;
		ptOrg[1] = ptBorderCenter.y;
		ptScale[0] = 1;
		ptScale[1] = 1;
		fRotateDegree = -vFAngle.front();		//ccw to cw
		ptReal[0] = vecPtPos.front().x + fPosCameraX;
		ptReal[1] = vecPtPos.front().y + fPosCameraY;
	}

	//进行平移旋转
	//DoSingleTrans(pList, nCountMarkPoints, vPtPosDestinedMark, vPtPosRealMark);
	DoTrans(ptOrg, ptScale, fRotateDegree, ptReal);


	return TRUE;
}


std::vector<CPointF> CPreProcess::GetBorderPtArray(CMachineListContainer* pList)
{
	POSITION pos;
	CMachineObj_Comm* pObj;
	pos = pList->GetObjHeadPosition();
	while (pos)
	{
		pObj = pList->GetObjNext(pos);
		if (pObj->m_ObjByLayer == LayerNum_Border)
			break;
	}

	if (NULL != pObj && LayerNum_Border != pObj->m_ObjByLayer)
		return std::vector<CPointF>();

	if (MachineObj_Type_Polyline != pObj->GetObjType())
		return std::vector<CPointF>();

	ObjRect rtBound;
	CPointF ptRTCenter;
	ObjPoint ptPolylineLast, ptPolyline;
	//int nLayer = pObj->m_ObjByLayer;
	std::vector<CPointF> vecPolylinePtBuf;

	rtBound = pObj->GetObjBound();
	ptRTCenter.x = (rtBound.max_x + rtBound.min_x) / 2;
	ptRTCenter.y = (rtBound.max_y + rtBound.min_y) / 2;
	double fMatchBorderArcStep = 1;

			//***DrawPolyline***
			CMachineObjPolyline* pPolyline;
			pPolyline = (CMachineObjPolyline*)pObj;

			int nVertCount;
			double fConvexityLast;
			//ObjPoint ptPolylineLast, ptPolyline;

			nVertCount = pPolyline->GetPolylineVertexCount();
			ptPolylineLast.x = pPolyline->GetPolylineStart().x;
			ptPolylineLast.y = pPolyline->GetPolylineStart().y;
			fConvexityLast = pPolyline->GetPolylineStart().convexity;

			vecPolylinePtBuf.clear();
			vecPolylinePtBuf.push_back(CPointF(ptPolylineLast.x, ptPolylineLast.y) - ptRTCenter);

			for (int i = 1; i < nVertCount; i++)
			{
				//读当前顶点i
				ptPolyline.x = pPolyline->GetPolylinePoint(i).x;
				ptPolyline.y = pPolyline->GetPolylinePoint(i).y;

				//检查上一个顶点i-1的凸度
				if (0 == fConvexityLast)
				{
					//顶点i-1 to 顶点i 是直线
					vecPolylinePtBuf.push_back(CPointF(ptPolyline.x, ptPolyline.y) - ptRTCenter);
				}
				else
				{
					//顶点i-1 to 顶点i 是圆弧
					ObjPoint ArcCenter;
					double /*ptArcCenterPos[2],*/ fArcRadius, fArcAngleStart, fArcAngleEnd;
					int nArcDir;

					pPolyline->TranPolylineToArc(
						ptPolylineLast, ptPolyline, fConvexityLast,
						&ArcCenter, &fArcRadius, &fArcAngleStart, &fArcAngleEnd);
					nArcDir = (fArcAngleStart > fArcAngleEnd) ? (int)CW : (int)CCW;

					//AddEntityArc(vecEntities, ptArcCenterPos, fArcRadius, fArcAngleStart, fArcAngleEnd, nArcDir, nLayer);

					//解析圆弧上插补坐标
					int nCountPt;
					double fAngleDelt = fArcAngleEnd - fArcAngleStart;
					double fMarkArcStepAngle;
					fMarkArcStepAngle = fMatchBorderArcStep / fArcRadius * 180 / M_PI;
					CPointF ptTmp;

					if (0 == nArcDir)//顺时针
					{
						if (0 <= fAngleDelt)//劣弧
							nCountPt = (int)(((M_PI * (360 - fAngleDelt) / 180) * fArcRadius) / fMatchBorderArcStep) + 1;
						else//优弧
							nCountPt = (int)(((M_PI * abs(fAngleDelt) / 180) * fArcRadius) / fMatchBorderArcStep) + 1;

						//ptBuf = new double[nCountPt][2];

						for (int i = 0; i < nCountPt - 1; i++)
						{
							fArcAngleStart -= fMarkArcStepAngle;
							ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
							ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
							vecPolylinePtBuf.push_back(ptTmp - ptRTCenter);
						}

						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
						vecPolylinePtBuf.push_back(ptTmp - ptRTCenter);
					}
					else if (1 == nArcDir)//逆时针
					{
						if (0 <= fAngleDelt)//优弧
							nCountPt = (int)(((M_PI * fAngleDelt / 180) * fArcRadius) / fMatchBorderArcStep) + 1;
						else//劣弧
							nCountPt = (int)(((M_PI * (360 - abs(fAngleDelt)) / 180) * fArcRadius) / fMatchBorderArcStep) + 1;

						for (int i = 0; i < nCountPt - 1; i++)
						{
							fArcAngleStart += fMarkArcStepAngle;
							ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleStart / 180);
							ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleStart / 180);
							vecPolylinePtBuf.push_back(ptTmp - ptRTCenter);
						}
						ptTmp.x = ArcCenter.x + fArcRadius * cos(M_PI * fArcAngleEnd / 180);
						ptTmp.y = ArcCenter.y + fArcRadius * sin(M_PI * fArcAngleEnd / 180);
						vecPolylinePtBuf.push_back(ptTmp - ptRTCenter);
					}
				}

				//更新ptPolylineLast，fConvexityLast
				if (i < nVertCount - 1)
				{
					ptPolylineLast = ptPolyline;
					fConvexityLast = pPolyline->GetPolylinePoint(i).convexity;
				}
			}


			return vecPolylinePtBuf;
}

BOOL CPreProcess::GetBorderCenter(CMachineListContainer* pList, CPointF* ptBorderCenter)
{
	POSITION pos;
	CMachineObj_Comm* pObj;
	pos = pList->GetObjHeadPosition();
	while (pos)
	{
		pObj = pList->GetObjNext(pos);
		if (pObj->m_ObjByLayer == LayerNum_Border)
			break;
	}

	if (LayerNum_Border != pObj->m_ObjByLayer)
		return FALSE;

	if (MachineObj_Type_Polyline != pObj->GetObjType())
		return FALSE;

	ObjRect rtBound;
	CPointF ptRTCenter;
	rtBound = pObj->GetObjBound();
	ptRTCenter.x = (rtBound.max_x + rtBound.min_x) / 2;
	ptRTCenter.y = (rtBound.max_y + rtBound.min_y) / 2;

	*ptBorderCenter = ptRTCenter;
	return TRUE;
}


