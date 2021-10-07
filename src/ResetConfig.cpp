/**********************************************************************
 
  Audacity: A Digital Audio Editor

  ResetConfig.cpp

  Jithin John

*******************************************************************/
/**
 
\class ResetConfigDialog
\brief ResetConfigDialog is used for resetting Audacity Preferences.

*/
/*******************************************************************/
#include "prefs/ThemePrefs.h"
#include <wx/checkbox.h>
#include "ProjectSettings.h"
#include "AudacityFileConfig.h"
#include "ProjectSelectionManager.h"
#include "toolbars/ToolManager.h"
#include "prefs/PrefsDialog.h"
#include "ShuttleGui.h"
#include "TempDirectory.h"
#include "widgets/KeyView.h"

class ResetConfigDialog final : public wxDialogWrapper
{
public:
   // constructors and destructors
   ResetConfigDialog(wxWindow *parent, AudacityProject &project, const CommandContext &context);

   void MakeResetConfigDialog();

private:
   // WDR: handler declarations
   void OnProceed(wxCommandEvent &event);
   void OnClose(wxCommandEvent &event);
   void OnToggleKeyboardSet(wxCommandEvent & event);
   void OnToggleAllConfiguration(wxCommandEvent & event);

   bool Commit();
   void FilterKeys(std::vector<NormalizedKeyString> &arr);
   void RefreshBindings(bool bSort);
   
   AudacityProject &mProject;
   const CommandContext &mcontext;

   CommandIDs mNames;
   CommandManager *mManager;

   std::vector<NormalizedKeyString> mDefaultKeys; // The full set.
   std::vector<NormalizedKeyString> mNewKeys;     // Used for work in progress.
   std::vector<NormalizedKeyString> mStandardDefaultKeys; // The reduced set.
   std::vector<NormalizedKeyString> mKeys;

   wxCheckBox *mDirectoriesCheckbox;
   wxCheckBox *mInterfaceCheckBox;
   wxCheckBox *mKeyboardCheckBox;
   wxCheckBox *mRecPlayCheckBox;
   wxCheckBox *mAllConfigurationsCheckbox;
   wxRadioButton *mFullCheckBox;
   wxRadioButton *mStandardCheckBox;
   
private:
   DECLARE_EVENT_TABLE()
};

void RunResetConfig(wxWindow *parent, AudacityProject &project, const CommandContext &context)
{
   ResetConfigDialog dlog{parent, project, context};

   dlog.CentreOnParent();

   dlog.ShowModal();
}

//
// ResetConfigDialog
//

enum
{
   ProceedID = 1000,
   IdDirectoriesCheckbox,
   IdInterfaceCheckBox,
   IdKeyboardCheckBox,
   IdMouseCheckBox,
   IdAllConfigurationCheckbox,
   IdCaptureToolbars,
   IdCaptureEffects,
};

BEGIN_EVENT_TABLE(ResetConfigDialog, wxDialogWrapper)
EVT_BUTTON(ProceedID, ResetConfigDialog::OnProceed)
EVT_BUTTON(wxID_CANCEL, ResetConfigDialog::OnClose)
EVT_CHECKBOX(IdKeyboardCheckBox, ResetConfigDialog::OnToggleKeyboardSet)
EVT_CHECKBOX(IdAllConfigurationCheckbox, ResetConfigDialog::OnToggleAllConfiguration)
END_EVENT_TABLE()

ResetConfigDialog::ResetConfigDialog(
    wxWindow *parent, AudacityProject &project, const CommandContext &context)
    :
      wxDialogWrapper(parent, 0, XO("Reset Configuration"),
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE |
                          wxRESIZE_BORDER),
      mProject(project), mcontext(context)
{
   mDirectoriesCheckbox = NULL;
   mInterfaceCheckBox = NULL;
   mKeyboardCheckBox = NULL;
   mRecPlayCheckBox = NULL;
   mStandardCheckBox = NULL;
   mFullCheckBox = NULL;
mAllConfigurationsCheckbox = NULL;
//mAllConfigurationsCheckbox->SetValue(true);
   SetName();
   MakeResetConfigDialog();
}

// WDR: handler implementations for ResetConfigDialog

void ResetConfigDialog::OnClose(wxCommandEvent &WXUNUSED(event))
{
   EndModal(0);
}

void ResetConfigDialog::MakeResetConfigDialog()
{
   ShuttleGui S(this, eIsCreatingFromPrefs);
   S.StartStatic(XO("\nWARNING : This can reset everything to default\nsuch as the settings, effects and the preferences\nyou have set.\n\n") + XO("Select the configurations you want to reset\n\n"));
   {
      S.StartVerticalLay(true);
      {
         S.SetBorder(8);

         //
         mAllConfigurationsCheckbox = S.Id(IdAllConfigurationCheckbox).AddCheckBox(XXO("All Configurations\n(Does not reset Keyboard Preferences)"), true);
         mDirectoriesCheckbox = S.Id(IdDirectoriesCheckbox).Disable(mAllConfigurationsCheckbox->GetValue())
                                 .AddCheckBox(XXO("Directories Preferences"), false);
         mInterfaceCheckBox = S.Id(IdInterfaceCheckBox).Disable(mAllConfigurationsCheckbox->GetValue())
                                 .AddCheckBox(XXO("Interface Preferences"), false);
         mRecPlayCheckBox = S.Id(IdMouseCheckBox).Disable(mAllConfigurationsCheckbox->GetValue())
                                 .AddCheckBox(XXO("Playback and Recording Preferences"), false);
         mKeyboardCheckBox = S.Id(IdKeyboardCheckBox).AddCheckBox(XXO("Keyboard Preferences"), false);
         S.StartHorizontalLay();
         {
               mStandardCheckBox = S.Id(IdCaptureEffects)
                  .Disable(!mKeyboardCheckBox->GetValue())
                  .AddRadioButtonToGroup(XXO("Standard"));
               mFullCheckBox = S.Id(IdCaptureToolbars)
                  .Disable(!mKeyboardCheckBox->GetValue())
                  .AddRadioButtonToGroup(XXO("Full"));
         }
         S.EndHorizontalLay();
         S.SetBorder(40);
         S.StartHorizontalLay(wxALIGN_LEFT | wxEXPAND, false);
         {
            S.StartHorizontalLay(wxALIGN_LEFT, false);
            {
               S.Id(wxID_CANCEL).AddButton(XXO("Cancel"));
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
               S.Id(ProceedID).AddButton(XXO("Proceed"));

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

void ResetConfigDialog::OnToggleKeyboardSet(wxCommandEvent & /* Evt */)
{
   bool mKeyboardCheckBoxVal = (mKeyboardCheckBox->GetValue());
   mStandardCheckBox->Enable(mKeyboardCheckBoxVal);
   mFullCheckBox->Enable(mKeyboardCheckBoxVal);
}

void ResetConfigDialog::OnToggleAllConfiguration(wxCommandEvent & /* Evt */)
{
   bool mAllConfigurationsCheckBoxVal = !(mAllConfigurationsCheckbox->GetValue());
   mDirectoriesCheckbox->Enable(mAllConfigurationsCheckBoxVal);
   mInterfaceCheckBox->Enable(mAllConfigurationsCheckBoxVal);
   mRecPlayCheckBox->Enable(mAllConfigurationsCheckBoxVal);
}

void ResetConfigDialog::OnProceed(wxCommandEvent &WXUNUSED(event))
{
   if (mKeyboardCheckBox->GetValue())
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
      gPrefs->Write("/GUI/SyncLockTracks", 0);
      gPrefs->Write("/AudioIO/SoundActivatedRecord", 0);
      gPrefs->Write("/SelectionToolbarMode", 0);
      gPrefs->Flush();

      DoReloadPreferences(mProject);
      ToolManager::OnResetToolBars(mcontext);

      // Reset Keyboard Preferences to Standard/Full.
      mManager = &CommandManager::Get(mProject);

      RefreshBindings(false);
      gPrefs->DeleteEntry(wxT("/GUI/Shortcuts/FullDefaults"));
      gPrefs->Flush();

      mNewKeys = mDefaultKeys;

      if (1 == 1)
         FilterKeys(mNewKeys);
      for (size_t i = 0; i < mNewKeys.size(); i++)
      {
         mManager->SetKeyFromIndex(i, mNewKeys[i]);
      }
      RefreshBindings(true);
      bool verna = Commit();

      // Clear pluginregistry.
      wxRemoveFile(FileNames::PluginRegistry());
   
      // These are necessary to preserve the newly correctly laid out toolbars.
      // In particular the Device Toolbar ends up short on next restart,
      // if they are left out.
      gPrefs->Write(wxT("/PrefsVersion"), wxString(wxT(AUDACITY_PREFS_VERSION_STRING)));

      // write out the version numbers to the prefs file for future checking
      gPrefs->Write(wxT("/Version/Major"), AUDACITY_VERSION);
      gPrefs->Write(wxT("/Version/Minor"), AUDACITY_RELEASE);
      gPrefs->Write(wxT("/Version/Micro"), AUDACITY_REVISION);
      gPrefs->Flush();

      ProjectSelectionManager::Get(mProject)
          .AS_SetSnapTo(gPrefs->ReadLong("/SnapTo", SNAP_OFF));
      ProjectSelectionManager::Get(mProject)
          .AS_SetRate(gPrefs->ReadDouble("/DefaultProjectSampleRate", 44100.0));
   }
   if(mInterfaceCheckBox->GetValue())
   {
      // lang was not usable and is not system language.  We got overridden.
      gPrefs->Write(wxT("/Locale/Language"), "en");
      gPrefs->Write(wxT("/GUI/Help"), "Local");
      gPrefs->Write(wxT("/GUI/Theme"), "light");
      gPrefs->Write(wxT("/GUI/EnvdBRange"), "60");
      gPrefs->Write(wxT("/GUI/ShowSplashScreen"), "0");
      gPrefs->Write(wxT("/GUI/ShowExtraMenus"), "0");
      gPrefs->Write(wxT("/GUI/BeepOnCompletion"), "0");
      gPrefs->Write(wxT("/GUI/RetainLabels"), "0");
      gPrefs->Write(wxT("/GUI/BlendThemes"), "1");
      gPrefs->Write(wxT("/GUI/RtlWorkaround"), "1");
      gPrefs->Write(wxT("/QuickPlay/ToolTips"), "1");
      gPrefs->Write(wxT("/QuickPlay/ScrubbingEnabled"), "1");
      gPrefs->Flush();
      ThemePrefs::ApplyUpdatedImages();
      DoReloadPreferences(mProject);
   }

   if(mRecPlayCheckBox->GetValue())
   {
      //Resetting the Playback Preferences
      gPrefs->Write(wxT("/AudioIO/EffectsPreviewLen"), "6");
      gPrefs->Write(wxT("/AudioIO/CutPreviewBeforeLen"), "2");
      gPrefs->Write(wxT("/AudioIO/CutPreviewAfterLen"), "1");
      gPrefs->Write(wxT("/AudioIO/SeekShortPeriod"), "1");
      gPrefs->Write(wxT("/AudioIO/SeekLongPeriod"), "15");
      gPrefs->Write(wxT("/AudioIO/VariSpeedPlay"), "1");
      gPrefs->Write(wxT("/AudioIO/Microfades"), "0");
      gPrefs->Write(wxT("/AudioIO/UnpinnedScrubbing"), "1");

      //Resetting the Recording preferences
      gPrefs->Write(wxT("/AudioIO/Duplex"), "1");
      gPrefs->Write(wxT("/AudioIO/SWPlaythrough"), "0");
      gPrefs->Write(wxT("/GUI/PreferNewTrackRecord"), "0");
      gPrefs->Write(wxT("/Warnings/DropoutDetected"), "1");
      gPrefs->Write(wxT("/AudioIO/SoundActivatedRecord"), "0");
      gPrefs->Write(wxT("/AudioIO/SilenceLevel"), "-50");
      gPrefs->Write(wxT("GUI/TrackNames/RecordingNameCustom"), "0");
      gPrefs->Write(wxT("GUI/TrackNames/TrackNumber"), "0");
      gPrefs->Write(wxT("GUI/TrackNames/DateStamp"), "0");
      gPrefs->Write(wxT("GUI/TrackNames/TimeStamp"), "0");
      gPrefs->Write(wxT("/AudioIO/PreRoll"), "5");
      gPrefs->Write(wxT("/AudioIO/Crossfade"), "10");
   }
   EndModal(0);
}
void ResetConfigDialog::FilterKeys(std::vector<NormalizedKeyString> &arr)
{
   const auto &MaxListOnly = CommandManager::ExcludedList();

   // Remove items that are in MaxList.
   for (size_t i = 0; i < arr.size(); i++)
   {
      if (std::binary_search(MaxListOnly.begin(), MaxListOnly.end(), arr[i]))
         arr[i] = {};
   }
}

bool ResetConfigDialog::Commit()
{
   // On the Mac, preferences may be changed without any active
   // projects.  This means that the CommandManager isn't available
   // either.  So we can't attempt to save preferences, otherwise
   // NULL ptr dereferences will happen in ShuttleGui because the
   // radio buttons are never created.  (See Populate() above.)

   bool bFull = gPrefs->ReadBool(wxT("/GUI/Shortcuts/FullDefaults"), false);
   for (size_t i = 0; i < mNames.size(); i++)
   {
      const auto &dkey = bFull ? mDefaultKeys[i] : mStandardDefaultKeys[i];
      // using GET to interpret CommandID as a config path component
      auto name = wxT("/NewKeys/") + mNames[i].GET();
      const auto &key = mNewKeys[i];

      if (gPrefs->HasEntry(name))
      {
         if (key != NormalizedKeyString{gPrefs->ReadObject(name, key)})
         {
            gPrefs->Write(name, key);
         }
         if (key == dkey)
         {
            gPrefs->DeleteEntry(name);
         }
      }
      else
      {
         if (key != dkey)
         {
            gPrefs->Write(name, key);
         }
      }
   }

   return gPrefs->Flush();
}

void ResetConfigDialog::RefreshBindings(bool bSort)
{
   TranslatableStrings Labels;
   TranslatableStrings Categories;
   TranslatableStrings Prefixes;

   mNames.clear();
   mKeys.clear();
   mDefaultKeys.clear();
   mStandardDefaultKeys.clear();
   mManager->GetAllCommandData(
       mNames,
       mKeys,
       mDefaultKeys,
       Labels,
       Categories,
       Prefixes,
       true); // True to include effects (list items), false otherwise.

   mStandardDefaultKeys = mDefaultKeys;
   FilterKeys(mStandardDefaultKeys);

   mNewKeys = mKeys;
}

// There are many more things we could reset here.
// Beeds discussion as to which make sense to.
// Maybe in future versions?
// - Reset Effects
// - Reset Recording and Playback volumes
// - Reset Selection formats (and for spectral too)
// - Reset Play-at-speed speed to x1
// - Stop playback/recording and unapply pause.
// - Set Zoom sensibly.