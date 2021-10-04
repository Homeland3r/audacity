/**********************************************************************

Audacity: A Digital Audio Editor

GUISettings.h

Paul Licameli

**********************************************************************/
#ifndef __AUDACITY_GUI_SETTINGS__
#define __AUDACITY_GUI_SETTINGS__

// Right to left languages fail in many wx3 dialogs with missing buttons.
// The workaround is to use LTR in those dialogs.
#ifndef __WXMAC__
#define RTL_WORKAROUND( pWnd ) \
   if ( RtlWorkaround.Read() ) \
       pWnd->SetLayoutDirection(wxLayout_LeftToRight);
#else
   #define RTL_WORKAROUND( pWnd )
#endif

#endif
