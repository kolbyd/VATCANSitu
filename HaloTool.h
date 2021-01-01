#pragma once
#include "EuroScopePlugIn.h"
#include "VATCANSitu.h"
#include "CSiTRadar.h"

using namespace std;
using namespace EuroScopePlugIn;

class HaloTool :
    public CRadarScreen
{
public:

    static inline double degtorad(double deg) {
        double ans = deg * PI / 180;
        return ans;
    }

    static CPosition calcPTL(CPosition origin, double ptlLen, double gs, double bearing) {
        double lat2;
        double long2;
        double earthRad = 3440; // in nautical miles

        lat2 = asin(sin(origin.m_Latitude * PI / 180) * cos((ptlLen * gs / 60) / earthRad) + cos(origin.m_Latitude * PI / 180) * sin((ptlLen * gs / 60) / earthRad) * cos(bearing * PI / 180));
        long2 = degtorad(origin.m_Longitude) + atan2((sin(bearing * PI / 180)) * sin((ptlLen * gs / 60) / earthRad) * cos(origin.m_Latitude * PI / 180), (cos(sin(bearing * PI / 180) / earthRad) - sin(origin.m_Latitude * PI / 180) * sin(lat2)));
        CPosition ptlEnd;

        ptlEnd.m_Latitude = lat2 * 180 / PI;
        ptlEnd.m_Longitude = long2 * 180 / PI;

        return ptlEnd;
    };

    static double calcBearing(CPosition origin, CPosition end) {
        double y = sin(degtorad(end.m_Longitude) - degtorad(origin.m_Longitude)) * cos(degtorad(end.m_Latitude));
        double x = cos(degtorad(origin.m_Latitude)) * sin(degtorad(end.m_Latitude)) - sin(degtorad(origin.m_Latitude)) * cos(degtorad(end.m_Latitude)) * cos(degtorad(end.m_Longitude) - degtorad(origin.m_Longitude));
        double phi = atan2(y, x);
        double bearing = fmod((phi * 180 / PI + 360), 360);

        return bearing;
    }

    static void drawHalo(CDC* dc, POINT p, double r, double pixpernm) 
    {
        int sDC = dc->SaveDC();

        //calculate pixels per nautical mile

        int pixoffset = (int)round(pixpernm * r);

       // draw the halo around point p with radius r in NM
        COLORREF targetPenColor = RGB(202, 205, 169);
        HPEN targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
        dc->SelectObject(targetPen);
        dc->SelectStockObject(HOLLOW_BRUSH);
        dc->Ellipse(p.x - pixoffset, p.y - pixoffset, p.x + pixoffset, p.y + pixoffset); 

        DeleteObject(targetPen);
        
        dc->RestoreDC(sDC);
    };

    static void drawPTL(CDC* dc, CRadarTarget radtar, CRadarScreen* radscr, POINT p, double ptlTime)
    {
        int sDC = dc->SaveDC();
        
        CPosition pos1 = radtar.GetPreviousPosition(radtar.GetPosition()).GetPosition();
        CPosition pos2 = radtar.GetPosition().GetPosition();
        double theta = calcBearing(pos1, pos2);
        CPosition ptl = calcPTL(radtar.GetPosition().GetPosition(), ptlTime, radtar.GetPosition().GetReportedGS(), theta);
        POINT p2 = radscr->ConvertCoordFromPositionToPixel(ptl);

        radscr->GetPlugIn()->DisplayUserMessage("VATCAN Situ", "Bearing", to_string(theta).c_str(), true, true, false, true, false);

        COLORREF targetPenColor = C_PTL_GREEN;
        HPEN targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
        dc->SelectObject(targetPen);

        dc->MoveTo(p);
        dc->LineTo(p2);

        DeleteObject(targetPen);

        dc->RestoreDC(sDC);
    };
};

