/////////////////////////////////////////////////////////////////////////////
// Name:        src/cocoa/dcscreen.cpp
// Purpose:     wxScreenDC class
// Author:      David Elliott
// Modified by:
// Created:     2003/03/16
// RCS-ID:      $Id: dcscreen.cpp 39123 2006-05-09 13:55:29Z ABX $
// Copyright:   (c) 2002 David Elliott
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/dcscreen.h"

#ifndef WX_PRECOMP
#endif //WX_PRECOMP

//-----------------------------------------------------------------------------
// wxMemoryDC
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxScreenDC,wxDC)

wxScreenDC::wxScreenDC(void)
{
    m_ok = false;
}

wxScreenDC::wxScreenDC( wxDC *WXUNUSED(dc) )
{
    m_ok = false;
}

wxScreenDC::~wxScreenDC(void)
{
}
