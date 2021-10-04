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

#include "ProjectSettings.h"
#include "ProjectSelectionManager.h"
#include "toolbars/ToolManager.h"
#include "commands/CommandContext.h"
#include "prefs/PrefsDialog.h"
#include "toolbars/TranscriptionToolBar.h"
#include <wx/valgen.h>
#include <wx/valtext.h>
#include "ShuttleGui.h"
#include "Menus.h"
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
   std::vector<NormalizedKeyString> mDefaultKeys; // The full set.
   std::vector<NormalizedKeyString> mNewKeys;     // Used for work in progress.
   CommandIDs mNames;
   KeyView *mView;
   std::vector<NormalizedKeyString> mStandardDefaultKeys; // The reduced set.
   std::vector<NormalizedKeyString> mKeys;
   AudacityProject &mProject;
   const CommandContext &mcontext;
   CommandManager *mManager;
   void FilterKeys(std::vector<NormalizedKeyString> &arr);
   void RefreshBindings(bool bSort);
   bool Commit();
   wxCheckBox *mDirectoriesCheckbox;
   wxCheckBox *mInterfaceCheckBox;
   wxCheckBox *mKeyboardCheckbox;
   wxCheckBox *mMouseCheckbox;
   wxCheckBox *mAllConfigurationsCheckbox;
   
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
};

BEGIN_EVENT_TABLE(ResetConfigDialog, wxDialogWrapper)
EVT_BUTTON(ProceedID, ResetConfigDialog::OnProceed)
EVT_BUTTON(wxID_CANCEL, ResetConfigDialog::OnClose)
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
   ShuttleGui S(this, eIsCreating);
   S.StartStatic(XO("\nWARNING : This can reset everything to default such as the \nsettings, effects and the preferences you have set.\n\n") + XO("Select the configurations you want to reset\n\n"));
   {
      S.StartVerticalLay(true);
      {
         S.SetBorder(8);

         //
         mDirectoriesCheckbox = S.Id(IdDirectoriesCheckbox).AddCheckBox(XXO("Directories Preferences"), false);
         mInterfaceCheckBox = S.Id(IdInterfaceCheckBox).AddCheckBox(XXO("Interface"), false);
         mKeyboardCheckbox = S.Id(IdKeyboardCheckBox).AddCheckBox(XXO("Keyboard Preferences"), false);
         mMouseCheckbox = S.Id(IdMouseCheckBox).AddCheckBox(XXO("Recording/Playback Preferences"), false);
         mAllConfigurationsCheckbox = S.Id(IdAllConfigurationCheckbox).AddCheckBox(XXO("All Configurations"), false);
         S.SetBorder(45);
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

void ResetConfigDialog::OnProceed(wxCommandEvent &WXUNUSED(event))
{
   if (mDirectoriesCheckbox &&
       mDirectoriesCheckbox->GetValue())
   {
      /*auto &menuManager = MenuManager::Get(mProject);
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
      */
      //////////////////////////////////////////////////////////////////////////
      mManager = &CommandManager::Get(mProject);

      RefreshBindings(false);
      gPrefs->DeleteEntry(wxT("/GUI/Shortcuts/FullDefaults"));
      gPrefs->Flush();

      mNewKeys = mDefaultKeys;

      if (2 == 1)
         FilterKeys(mNewKeys);
      for (size_t i = 0; i < mNewKeys.size(); i++)
      {
         mManager->SetKeyFromIndex(i, mNewKeys[i]);
      }
      RefreshBindings(true);
      bool verna = Commit();
      ////////////////////////////////////////////////////////////////////////////////
      // These are necessary to preserve the newly correctly laid out toolbars.
      // In particular the Device Toolbar ends up short on next restart,
      // if they are left out.
      /*gPrefs->Write(wxT("/PrefsVersion"), wxString(wxT(AUDACITY_PREFS_VERSION_STRING)));
      // write out the version numbers to the prefs file for future checking
      gPrefs->Write(wxT("/Version/Major"), AUDACITY_VERSION);
      gPrefs->Write(wxT("/Version/Minor"), AUDACITY_RELEASE);
      gPrefs->Write(wxT("/Version/Micro"), AUDACITY_REVISION);
      gPrefs->Flush();
      ProjectSelectionManager::Get(mProject)
          .AS_SetSnapTo(gPrefs->ReadLong("/SnapTo", SNAP_OFF));
      ProjectSelectionManager::Get(mProject)
          .AS_SetRate(gPrefs->ReadDouble("/DefaultProjectSampleRate", 44100.0));*/
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
   // if (!mProject)
   // {
   //    return true;
   // }

   // ShuttleGui S(this, eIsSavingToPrefs);
   // PopulateOrExchange(S);

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

//for clearing the pluginregistry
//wxRemoveFile()
//TranscriptionToolBar kollam;
//resetting the playback speed
//kollam.SetPlaySpeed( 1.00 );