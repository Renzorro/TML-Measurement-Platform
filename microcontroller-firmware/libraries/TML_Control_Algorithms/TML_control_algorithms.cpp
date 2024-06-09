#include "TML_control_algorithms.h"

// Global variable definitions
bool autoMainRunning = false;
bool autoAuxRunning = false;
double stopThreshold_N = 20.0;
bool homeMainRunning = false;
bool homeAuxRunning = false;

// Auto

void auto_start(int foot)
{
    if(foot==0) // Main platform
    {
        autoMainRunning = true;
    }
    else
    {
        autoAuxRunning = true;
    }
    
    return;
}

void auto_stop(int foot)
{
    if(foot==0) // Main platform
    {
        autoMainRunning = false;
    }
    else
    {
        autoAuxRunning = false;
    }
    
    return;
}

bool auto_state_update(int foot, double force, bool override = false)
{
    if(foot==0) // Main foot
    {
        // if(!autoMainRunning)
        // {
        //     return false;
        // }
        if(override)
        {
            autoMainRunning = false;
            return false;
        }
        if(force > stopThreshold_N)
        {
            autoMainRunning = false;
            return false;
        }
    }
    else // Aux foot
    {
        // if(!autoAuxRunning)
        // {
        //     return false;
        // }
        if(override)
        {
            autoAuxRunning = false;
            return false;
        }
        if(force > stopThreshold_N)
        {
            autoAuxRunning = false;
            return false;
        }
    }
    
    return true;
}

// Home

void home_start(int foot)
{
    if(foot==0) // Main platform
    {
        homeMainRunning = true;
    }
    else
    {
        homeAuxRunning = true;
    }
    
    return;
}

void home_stop(int foot)
{
    if(foot==0) // Main platform
    {
        homeMainRunning = false;
    }
    else
    {
        homeAuxRunning = false;
    }
    
    return;
}

bool home_state_update(int foot, double angle, bool override = false)
{
    if(foot==0) // Main foot
    {
        // if(!homeMainRunning)
        // {
        //     return false;
        // }
        if(override)
        {
            homeMainRunning = false;
            return false;
        }
        if(angle < HOME_ANGLE)
        {
            homeMainRunning = false;
            return false;
        }
    }
    else // Aux foot
    {
        // if(!homeAuxRunning)
        // {
        //     return false;
        // }
        if(override)
        {
            homeAuxRunning = false;
            return false;
        }
        if(angle < HOME_ANGLE)
        {
            homeAuxRunning = false;
            return false;
        }
    }
    
    return true;
}