/* -*- C++ -*-

   This file implements the InConstructionState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: InConstructionState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "InConstructionState.h"

#include "ThreadWeaver.h"
#include "WeaverImpl.h"

namespace ThreadWeaver {

    void InConstructionState::suspend()
    {
        // this request is not handled in InConstruction state
    }

    void InConstructionState::resume()
    {
        // this request is not handled in InConstruction state
    }

    Job* InConstructionState::applyForWork ( Thread *th,  Job *previous)
    {
        // As long as we are in the construction state, no jobs will be given
        // to the worker threads. The threads will be suspended. They will
        // return from the blocked state when jobs are queued. By then, we
        // should not be in InConstruction state anymore, and we hand the job
        // application over to the then active state.
        while ( m_weaver->state().stateId() == InConstruction )
        {
            m_weaver->waitForAvailableJob ( th);
        }
        return m_weaver->applyForWork ( th,  previous );
    }

    void InConstructionState::waitForAvailableJob ( Thread * th)
    {
        m_weaver->blockThreadUntilJobsAreBeingAssigned ( th );
    }

}
