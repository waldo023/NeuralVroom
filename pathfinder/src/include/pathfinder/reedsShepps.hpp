#ifndef REEDSSHEPPS_H
#define REEDSSHEPPS
#include "../vehicle.hpp"

class ReedsSheppsAction
{
public:
    Steer steer;
    Gear gear;
    float length;

    ReedsSheppsAction(Steer steer, Gear gear, float length);
    void revSteer();
    void revGear();

};

//Single Steering and motion over some length FORMULAS BASED OFF THE REEDS SHEPP PAPER:
//https://projecteuclid.org/download/pdf_1/euclid.pjm/1102645450
//some equations are modified using optimizations from http://msl.cs.uiuc.edu/~lavalle/cs326a/rs.c
class ReedsSheppsCurves
{
    typedef vector<ReedsSheppsAction>(ReedsSheppsCurves::*pathFunction)(float, float, float);
public:
    float calculatePathLength(vector<ReedsSheppsAction> path);
    vector<ReedsSheppsAction> findOptimalPath(Coordinates3D start, Coordinates3D end);
    vector<vector<ReedsSheppsAction>> findAllPaths(Coordinates3D start, Coordinates3D end);
    vector<VehicleState> discretizePath(VehicleState current, vector<ReedsSheppsAction> rsPath, float unit, float length);

private:
    vector<ReedsSheppsAction> one_CSC_SameTurns(float x, float y, float radians);
    vector<ReedsSheppsAction> two_CSC_DiffTurns(float x, float y, float radians);
    vector<ReedsSheppsAction> three_CCC(float x, float y, float radians);
    vector<ReedsSheppsAction> four_C_CC(float x, float y, float radians);
    vector<ReedsSheppsAction> five_CC_C(float x, float y, float radians);
    vector<ReedsSheppsAction> six_CCu_CuC(float x, float y, float radians);
    vector<ReedsSheppsAction> seven_C_CuCu_C(float x, float y, float radians);
    vector<ReedsSheppsAction> eight_C_Cpi2SC_SameTurn(float x, float y, float radians);
    vector<ReedsSheppsAction> nine_C_Cpi2SC_DiffTurn(float x, float y, float radians);
    vector<ReedsSheppsAction> ten_CSCp2_C_SameTurn(float x, float y, float radians);
    vector<ReedsSheppsAction> eleven_CSCp2_C_DiffTurn(float x, float y, float radians);
    vector<ReedsSheppsAction> twelve_C_Cpi2SCpi2_C(float x, float y, float radians);

    vector<pathFunction> pathFunctions = {
        one_CSC_SameTurns,two_CSC_DiffTurns,three_CCC,four_C_CC,
        five_CC_C,six_CCu_CuC,seven_C_CuCu_C,eight_C_Cpi2SC_SameTurn,nine_C_Cpi2SC_DiffTurn,
        ten_CSCp2_C_SameTurn,eleven_CSCp2_C_DiffTurn,twelve_C_Cpi2SCpi2_C};
};





#endif