/**********************************************************************

  Audacity: A Digital Audio Editor

  ResetConfig.cpp

  Jithin John

*******************************************************************//**

\class ResetConfigDialog
\brief ResetConfigDialog is used for measuring performance and accuracy
of sample block storage.

*//*******************************************************************/


#include "ProjectSettings.h"
#include "ProjectSelectionManager.h"
#include "toolbars/ToolManager.h"
#include "commands/CommandContext.h"
#include "prefs/PrefsDialog.h"
#include <wx/valgen.h>
#include <wx/valtext.h>
#include "ShuttleGui.h"
#include "Menus.h"
#include "TempDirectory.h"

class ResetConfigDialog final : public wxDialogWrapper
{
public:
   // constructors and destructors
   ResetConfigDialog( wxWindow *parent, AudacityProject &project, const CommandContext &context);

   void MakeResetConfigDialog();

private:
   // WDR: handler declarations
   void OnSave( wxCommandEvent &event );
   void OnClose( wxCommandEvent &event );

   AudacityProject &mProject;
   const CommandContext &mcontext;

   wxCheckBox *mathil;

   bool mBlockDetttail;
   bool      mEditDetail;

   wxTextCtrl  *mText;

private:
   DECLARE_EVENT_TABLE()
};

void RunResetConfig( wxWindow *parent, AudacityProject &project, const CommandContext &context)
{
   ResetConfigDialog dlog{ parent, project, context };

   dlog.CentreOnParent();

   dlog.ShowModal();
}

//
// ResetConfigDialog
//

enum {
   ProceedID,
   IdDelayCheckBox,
};

BEGIN_EVENT_TABLE(ResetConfigDialog, wxDialogWrapper)
   EVT_BUTTON( ProceedID,  ResetConfigDialog::OnSave )
   EVT_BUTTON( wxID_CANCEL, ResetConfigDialog::OnClose )
END_EVENT_TABLE()

ResetConfigDialog::ResetConfigDialog(
   wxWindow *parent, AudacityProject &project, const CommandContext &context)
   :
      /* i18n-hint: ResetConfig means a software speed test */
      wxDialogWrapper( parent, 0, XO("Reset Configuration"),
                wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_DIALOG_STYLE |
                wxRESIZE_BORDER)
   , mProject(project)
   , mcontext(context)
{
   SetName();

   mEditDetail = false;


   MakeResetConfigDialog();
}

// WDR: handler implementations for ResetConfigDialog

void ResetConfigDialog::OnClose(wxCommandEvent & WXUNUSED(event))
{
   EndModal(0);
}

void ResetConfigDialog::MakeResetConfigDialog()
{
   ShuttleGui S(this, eIsCreating);
   S.StartStatic(XO("This can reset everything to default such \nas the settings, effects and the preferences\n you have set.\n\n")+XO("Select the configurations you want to reset"));
   {
      S.StartVerticalLay(true);
      {
         S.SetBorder(8);

         //
      mathil = S.Id(IdDelayCheckBox).AddCheckBox(
                  XXO("Directories Preferences"),
                  false);
         //
         S.Validator<wxGenericValidator>(&mEditDetail)
            .AddCheckBox(XXO("Effects"),
                              false);
         S.Validator<wxGenericValidator>(&mEditDetail)
            .AddCheckBox(XXO("Settings"),
                              false);
         S.Validator<wxGenericValidator>(&mEditDetail)
            .AddCheckBox(XXO("All Configurations"),
                              true);
      S.StartVerticalLay(true);
      {
         S.Validator<wxGenericValidator>(&mBlockDetttail)
            .AddCheckBox(XXO("Don't ask this question again"),
                              true);
      }
      S.EndVerticalLay();
         //
         S.SetBorder(25);
         S.StartHorizontalLay(wxALIGN_LEFT | wxEXPAND, false);
         {
            S.StartHorizontalLay(wxALIGN_LEFT, false);
            {
               S.Id(ProceedID).AddButton(XXO("Proceed"));
               /* i18n-hint verb; to start resetting */
            }
            S.EndHorizontalLay();

            S.StartHorizontalLay(wxALIGN_CENTER, true);
            {
               // Spacer
            }
            S.EndHorizontalLay();

            S.StartHorizontalLay(wxALIGN_NOT | wxALIGN_LEFT, false);
            {
               /* i18n-hint verb */
               S.Id(wxID_CANCEL).AddButton(XXO("Cancel"));
            }
            S.EndHorizontalLay();
         }
         S.EndHorizontalLay();
      }
      S.EndVerticalLay();

      Fit();
      SetSizeHints(GetSize());
   }
   S.EndStatic();
}

void ResetConfigDialog::OnSave( wxCommandEvent & WXUNUSED(event))
{
   if (mathil && 
         mathil->GetValue())
   {
      auto &menuManager = MenuManager::Get(mProject);
      menuManager.mLastAnalyzerRegistration = MenuCreator::repeattypenone;
      menuManager.mLastToolRegistration = MenuCreator::repeattypenone;
      menuManager.mLastGenerator = "";
      menuManager.mLastEffect = "";
      menuManager.mLastAnalyzer = "";
      menuManager.mLastTool = "";

      ResetPreferences();

      // Directory will be reset on next restart.
      FileNames::UpdateDefaultPath(FileNames::Operation::Temp, TempDirectory::DefaultTempDir());

      // There are many more things we could reset here.
      // Beeds discussion as to which make sense to.
      // Maybe in future versions?
      // - Reset Effects
      // - Reset Recording and Playback volumes
      // - Reset Selection formats (and for spectral too)
      // - Reset Play-at-speed speed to x1
      // - Stop playback/recording and unapply pause.
      // - Set Zoom sensibly.
      gPrefs->Write("/GUI/SyncLockTracks", 0);
      gPrefs->Write("/AudioIO/SoundActivatedRecord", 0);
      gPrefs->Write("/SelectionToolbarMode", 0);
      gPrefs->Flush();
      DoReloadPreferences(mProject);
      ToolManager::OnResetToolBars(mcontext);

      // These are necessary to preserve the newly correctly laid out toolbars.
      // In particular the Device Toolbar ends up short on next restart, 
      // if they are left out.
      gPrefs->Write(wxT("/PrefsVersion"), wxString(wxT(AUDACITY_PREFS_VERSION_STRING)));

      // write out the version numbers to the prefs file for future checking
      gPrefs->Write(wxT("/Version/Major"), AUDACITY_VERSION);
      gPrefs->Write(wxT("/Version/Minor"), AUDACITY_RELEASE);
      gPrefs->Write(wxT("/Version/Micro"), AUDACITY_REVISION);

      gPrefs->Flush();

      ProjectSelectionManager::Get( mProject )
         .AS_SetSnapTo(gPrefs->ReadLong("/SnapTo", SNAP_OFF));
      ProjectSelectionManager::Get( mProject )
         .AS_SetRate(gPrefs->ReadDouble("/DefaultProjectSampleRate", 44100.0));
   }
   EndModal(0);
}