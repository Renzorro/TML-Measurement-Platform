#include "TML_lock.h"

int check_lock(Lock *lock, int foot)
{
    if(foot==MAIN_CONTROL)
    {
        return lock->main_control;
    }
    else if(foot==AUX_CONTROL)
    {
        return lock->aux_control;
    }
    else
    {
        return 0;
    }
}

bool try_acquire(Lock *lock, int foot, int controller)
{
    if(foot==MAIN_CONTROL)
    {
        if(controller==DESKTOP_CONTROL)
        {
            if(check_lock(lock, foot) != REMOTE_CONTROL) // if available
            {
                lock->main_control = DESKTOP_CONTROL;
                return true;
            }
            else // not available
            {
                return false;
            }
        }
        if(controller==REMOTE_CONTROL)
        {
            if(check_lock(lock, foot) != DESKTOP_CONTROL) // if available
            {
                lock->main_control = REMOTE_CONTROL;
                return true;
            }
            else // not available
            {
                return false;
            }
        }
    }
    else if(foot==AUX_CONTROL)
    {
        if(controller==DESKTOP_CONTROL)
        {
            if(check_lock(lock, foot) != REMOTE_CONTROL) // if available
            {
                lock->aux_control = DESKTOP_CONTROL;
                return true;
            }
            else // not available
            {
                return false;
            }
        }
        if(controller==REMOTE_CONTROL)
        {
            if(check_lock(lock, foot) != DESKTOP_CONTROL) // if available
            {
                lock->aux_control = REMOTE_CONTROL;
                return true;
            }
            else // not available
            {
                return false;
            }
        }
    }
    return false; // if inputted wrong foot or command value
}

bool try_release(Lock *lock, int foot, int controller)
{
    if(foot==MAIN_CONTROL)
    {
        if(controller==DESKTOP_CONTROL)
        {
            if(check_lock(lock, foot) == DESKTOP_CONTROL) // if holding
            {
                lock->main_control = AVAILABLE;
                return true;
            }
            else // if not holding
            {
                return false;
            }
        }
        if(controller==REMOTE_CONTROL)
        {
            if(check_lock(lock, foot) == REMOTE_CONTROL) // if holding
            {
                lock->main_control = AVAILABLE;
                return true;
            }
            else // not holding
            {
                return false;
            }
        }
    }
    else if(foot==AUX_CONTROL)
    {
        if(controller==DESKTOP_CONTROL)
        {
            if(check_lock(lock, foot) == DESKTOP_CONTROL) // if holding
            {
                lock->aux_control = AVAILABLE;
                return true;
            }
            else // not holding
            {
                return false;
            }
        }
        if(controller==REMOTE_CONTROL)
        {
            if(check_lock(lock, foot) == REMOTE_CONTROL) // if holding
            {
                lock->aux_control = AVAILABLE;
                return true;
            }
            else // not holding
            {
                return false;
            }
        }
    }
    return false; // if inputted wrong foot value
}

bool try_release_all(Lock *lock, int foot)
{
    return (try_release(lock, foot, DESKTOP_CONTROL) && try_release(lock, foot, REMOTE_CONTROL));
}