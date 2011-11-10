/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  thread_event_class.h
// Created:  11/3/2009
// Author:  K. Loux
// Description:  Eventually might house an actual event class - for now just an event
//				 declaration.  This event is used for communication from the worker threads
//				 to the GUI thread.
// History:

#ifndef _THREAD_EVENT_CLASS_H_
#define _THREAD_EVENT_CLASS_H_

// wxWidgets headers
#include <wx/event.h>

// Declaration of the EVT_THREAD event
DECLARE_LOCAL_EVENT_TYPE(EVT_THREAD, -1)

#endif// _THREAD_EVENT_CLASS_H_