/**********************************************************************

  Audacity: A Digital Audio Editor

  ResetConfig.cpp

  Jithin John

*******************************************************************//**

\class ResetConfigDialog
\brief ResetConfigDialog is used for measuring performance and accuracy
of sample block storage.

*//*******************************************************************/


#include "AudioIO.h"
#include "BatchProcessDialog.h"
#include "Benchmark.h"
#include "ResetConfig.h"
#include "CommonCommandFlags.h"
#include "Journal.h"
#include "Menus.h"
#include "PluginManager.h"
#include "PluginRegistrationDialog.h"
#include "Prefs.h"
#include "Project.h"
#include "ProjectSettings.h"
#include "ProjectWindow.h"
#include "ProjectWindows.h"
#include "ProjectSelectionManager.h"
#include "toolbars/ToolManager.h"
#include "Screenshot.h"
#include "TempDirectory.h"
#include "UndoManager.h"
#include "commands/CommandContext.h"
#include "commands/CommandManager.h"
#include "commands/ScreenshotCommand.h"
#include "effects/EffectManager.h"
#include "effects/EffectUI.h"
#include "effects/RealtimeEffectManager.h"
#include "prefs/EffectsPrefs.h"
#include "prefs/PrefsDialog.h"
#include "widgets/AudacityMessageBox.h"

#include "ResetConfig.h"

#include <wx/app.h>
#include <wx/log.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/utils.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/intl.h>

#include "SampleBlock.h"
#include "ShuttleGui.h"
#include "Project.h"
#include "WaveClip.h"
#include "WaveTrack.h"
#include "Sequence.h"
#include "Prefs.h"
#include "Menus.h"
#include "TempDirectory.h"
#include "ProjectRate.h"
#include "ViewInfo.h"

#include "FileNames.h"
#include "SelectFile.h"
#include "widgets/AudacityMessageBox.h"
#include "widgets/wxPanelWrapper.h"

// Change these to the desired format...should probably make the
// choice available in the dialog
#define SampleType short
#define SampleFormat int16Sample

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

   void Printf(const TranslatableString &str);
   void HoldPrint(bool hold);
   void FlushPrint();

   AudacityProject &mProject;
   const CommandContext &mcontext;
   const ProjectRate &mRate;

   bool      mHoldPrint;
   wxString  mToPrint;

   wxString  mBlockSizeStr;
   wxString  mDataSizeStr;
   wxCheckBox *mathil;
   wxString  mNumEditsStr;
   wxString  mRandSeedStr;

   bool mBlockDetttail;
   bool      mEditDetail;

   wxTextCtrl  *mText;

private:
   DECLARE_EVENT_TABLE()
};

void RunResetConfig( wxWindow *parent, AudacityProject &project, const CommandContext &context)
{
   /*
   int action = AudacityMessageBox(
XO("This will close all project windows (without saving)\nand open the Audacity ResetConfig dialog.\n\nAre you sure you want to do this?"),
      XO("ResetConfig"),
      wxYES_NO | wxICON_EXCLAMATION,
      NULL);

   if (action != wxYES)
      return;

   for ( auto pProject : AllProjects{} )
      GetProjectFrame( *pProject ).Close();
   */

   ResetConfigDialog dlog{ parent, project, context };

   dlog.CentreOnParent();

   dlog.ShowModal();
}

//
// ResetConfigDialog
//

enum {
   RunID = 1000,
   BSaveID,
   ClearID,
   StaticTextID,
   BlockSizeID,
   DataSizeID,
   NumEditsID,
   IdDelayCheckBox,
   RandSeedID
};

BEGIN_EVENT_TABLE(ResetConfigDialog, wxDialogWrapper)
   EVT_BUTTON( BSaveID,  ResetConfigDialog::OnSave )
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
   , mProject(project), mcontext(context)
   , mRate{ ProjectRate::Get(project) }
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

   // Strings don't need to be translated because this class doesn't
   // ever get used in a stable release.
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
            S.Id(BSaveID).AddButton(XXO("Proceed"));
            /* i18n-hint verb; to empty or erase */
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
S.EndStatic();}

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