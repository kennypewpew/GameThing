#pragma once

// Ballistic trajectories
float CoordsToDistance( const int &x1 , const int &y1 , const int &x2 , const int &y2 );

float VelocityToReach( const float &r );

float HeightAtDistance( const float &dist
                      , const float &vel
                      , const float &angle
                      , const float &y0
                      );

float MaxHeightReached( const float &vel
                      , const float &angle
                      );

float MaxAngleToReach( const float &r , const float &y , const float &v );
float MinAngleToReach( const float &r , const float &y , const float &v );

