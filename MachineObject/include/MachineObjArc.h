// CMachineObjArc 加工对象圆类
//
#pragma once
#include "MachineObj_Comm.h"

// CMachineObjArc命令目标
class CMachineObjArc :public CMachineObj_Comm
{
// 构造
public:
	CMachineObjArc();
	CMachineObjArc(ObjPoint ArcCenter, double m_ArcRadius,
		double StartAngle, double EndAngle,
		int ArcByLayer = LayerNum_Default, ObjDir dir = CCW);
	~CMachineObjArc();
	virtual void Serialize(CArchive& ar);

// 特性
protected:
	ObjPoint m_ArcCenter;
	double m_ArcRadius;
	double m_StartAngle;
	double m_EndAngle;

// 操作 
private:
	int Quadrant(double x, double y);
protected:
	void ReSizeBound();
public:
	void MoveArcCenter(ObjPoint point);
	void MoveArcStart(ObjPoint point);
	void MoveArcEnd(ObjPoint point);
	void MoveArc(double X_shift, double Y_shift);
	void SetArcRadius(double radius);
	void SetStartAndEndAngle(double angle1, double angle2, ObjDir dir);
	void ExchangeStartAndEnd(CMachineObjArc* pObj);
	ObjPoint GetArcCenter();
	ObjPoint GetArcStart();
	ObjPoint GetArcEnd();
	double GetStartAngle();
	double GetEndAngle();
	ObjPoint GetArcPoint(double Angle);
	double GetArcRadius();
	ObjDir GetArcDir();
	double CalDirAngle(double Angle);
	
};

