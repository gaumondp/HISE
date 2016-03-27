/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#define toggleVisibility(x) {x->setVisible(!x->isVisible()); owner->setComponentShown(info.commandID, x->isVisible());}

#define SET_COMMAND_TARGET(result, name, active, ticked, shortcut) { result.setInfo(name, name, "Target", 0); \\
															 result.setActive(active); \\
															 result.setTicked(ticked); \\
															 result.addDefaultKeypress(shortcut, ModifierKeys::commandModifier); }


BackendCommandTarget::BackendCommandTarget(BackendProcessor *owner_):
owner(owner_),
currentColumnMode(OneColumn)
{
	

	createMenuBarNames();
}


void BackendCommandTarget::setEditor(BackendProcessorEditor *editor)
{
	bpe = dynamic_cast<BackendProcessorEditor*>(editor);

	mainCommandManager = owner->getCommandManager();
	mainCommandManager->registerAllCommandsForTarget(this);
	mainCommandManager->getKeyMappings()->resetToDefaultMappings();

	bpe->addKeyListener(mainCommandManager->getKeyMappings());
	mainCommandManager->setFirstCommandTarget(this);
	mainCommandManager->commandStatusChanged();
}

void BackendCommandTarget::getAllCommands(Array<CommandID>& commands)
{
	const CommandID id[] = { ModulatorList,
		CustomInterface,
		DebugPanel,
		ViewPanel,
		Mixer,
		Macros,
		Keyboard,
		Settings,
		MenuNewFile,
		MenuOpenFile,
		MenuSaveFile,
		MenuSaveFileAsXmlBackup,
		MenuOpenXmlBackup,
		MenuProjectNew,
		MenuProjectLoad,
		MenuCloseProject,
		MenuProjectShowInFinder,
		MenuFileSettingsPreset,
		MenuFileSettingsProject,
		MenuFileSettingsUser,
		MenuFileSettingsCompiler,
		MenuFileSettingCheckSanity,
		MenuReplaceWithClipboardContent,
		MenuExportFileAsPlugin,
		MenuFileQuit,
		MenuEditCopy,
		MenuEditPaste,
		MenuEditCreateScriptVariable,
        MenuEditCloseAllChains,
        MenuEditPlotModulator,
		MenuViewShowSelectedProcessorInPopup,
		
		MenuToolsRecompile,
        MenuToolsClearConsole,
		MenuToolsSetCompileTimeOut,
		MenuToolsUseBackgroundThreadForCompile,
		MenuToolsRecompileScriptsOnReload,
		MenuToolsResolveMissingSamples,
		MenuToolsDeleteMissingSamples,
		MenuToolsUseRelativePaths,
		MenuToolsCollectExternalFiles,
        MenuToolsRedirectSampleFolder,
        MenuViewFullscreen,
		MenuViewBack,
		MenuViewForward,
        MenuOneColumn,
		MenuTwoColumns,
		MenuThreeColumns,
		MenuViewShowPluginPopupPreview,
        MenuAddView,
        MenuDeleteView,
        MenuRenameView,
        MenuViewSaveCurrentView,
        MenuToolsCheckDuplicate,
		MenuHelpShowAboutPage
		/*        MenuRevertFile,
		
		
		MenuExportFileAsPlayerLibrary,
		MenuEditOffset,
		MenuViewOffset,
		MenuOneColumn,
		MenuTwoColumns,
		MenuThreeColumns,
		MenuAddView,
		MenuDeleteView,
		MenuRenameView*/
	};

	commands.addArray(id, numElementsInArray(id));
}

void BackendCommandTarget::createMenuBarNames()
{
	menuNames.clear();

	menuNames.add("File");
	menuNames.add("Edit " + (currentCopyPasteTarget.get() == nullptr ? "" : currentCopyPasteTarget->getObjectTypeName()));
	menuNames.add("Tools");
	menuNames.add("View");
	menuNames.add("Help");

	jassert(menuNames.size() == numMenuNames);
}

void BackendCommandTarget::getCommandInfo(CommandID commandID, ApplicationCommandInfo &result)
{
	switch (commandID)
	{

	case CustomInterface: 
		setCommandTarget(result, "Frontend", true, (bpe->interfaceComponent != nullptr && !bpe->interfaceComponent->isVisible()), 'X', false);
		break;
	case Macros:
		setCommandTarget(result, "Show Macro Controls", true, (bpe->macroKnobs != nullptr && !bpe->macroKnobs->isVisible()), 'X', false);
		break;
	case DebugPanel:
		setCommandTarget(result, "Show Debug Panel", true, (bpe->referenceDebugArea != nullptr && !bpe->referenceDebugArea->isVisible()), 'X', false);
		break;
	case ViewPanel:
		setCommandTarget(result, "Show View Panel", true, true, 'X', false);
		break;
	case Mixer:
		setCommandTarget(result, "Show Mixer", false, true, 'X', false);
		break;
	case Keyboard:
		setCommandTarget(result, "Show Keyboard", true, (bpe->keyboard != nullptr && !bpe->keyboard->isVisible()), 'X', false);
		break;
	case Settings:

#if IS_STANDALONE_APP
		setCommandTarget(result, "Show Audio Device Settings", true, bpe->currentDialog == nullptr, 'X', false);
#else
		setCommandTarget(result, "Show Audio Device Settings (disabled for plugins)", false, bpe->currentDialog == nullptr, '8');
#endif
		break;
	case ModulatorList:
		setCommandTarget(result, "Show Processor List", true, (bpe->popupEditor == nullptr), 'x', false);
		break;
	case MenuNewFile:
		setCommandTarget(result, "New File", true, false, 'N');
		break;
	case MenuOpenFile:
		setCommandTarget(result, "Open File", true, false, 'O');
		break;
	case MenuSaveFile:
		setCommandTarget(result, "Save", true, false, 'S');
		break;
	case MenuSaveFileAsXmlBackup:
		setCommandTarget(result, "Save File as XML Backup", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
	case MenuOpenXmlBackup:
		setCommandTarget(result, "Load XML Backup", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
	case MenuProjectNew:
		setCommandTarget(result, "Create new Project folder", true, false, 'X', false);
		break;
	case MenuProjectLoad:
		setCommandTarget(result, "Load Project", true, false, 'X', false);
		break;
	case MenuCloseProject:
		setCommandTarget(result, "Close Project", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
	case MenuProjectShowInFinder:
		setCommandTarget(result, "Show Project folder in " + String((SystemStats::getOperatingSystemType() == SystemStats::OperatingSystemType::MacOSX) ? "Finder" : "Explorer"), GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
    case MenuExportFileAsPlugin:
        setCommandTarget(result, "Export as VST/AU plugin", true, false, 'X', false);
        break;
	case MenuFileSettingsPreset:
		setCommandTarget(result, "Preset Properties", true, false, 'X', false);
		break;
	case MenuFileSettingsProject:
		setCommandTarget(result, "Project Properties", true, false, 'X', false);
		break;
	case MenuFileSettingsUser:
		setCommandTarget(result, "User Settings", true, false, 'X', false);
		break;
	case MenuFileSettingsCompiler:
		setCommandTarget(result, "Compiler Settings", true, false, 'X', false);
		break;
	case MenuFileSettingCheckSanity:
		setCommandTarget(result, "Check for missing properties", true, false, 'X', false);
		break;
	case MenuReplaceWithClipboardContent:
		setCommandTarget(result, "Replace with clipboard content", Actions::hasProcessorInClipboard(), false, 'X', false);
		break;
	case MenuFileQuit:
		setCommandTarget(result, "Quit", true, false, 'X', false); break;
	case MenuEditCopy:
		setCommandTarget(result, "Copy", currentCopyPasteTarget.get() != nullptr, false, 'C');
		break;
	case MenuEditPaste:
		setCommandTarget(result, "Paste", currentCopyPasteTarget.get() != nullptr, false, 'V');
		break;
	case MenuEditCreateScriptVariable:
		setCommandTarget(result, "Create script variable", clipBoardNotEmpty(), false, 'C', true, ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
    case MenuEditPlotModulator:
        {
            BetterProcessorEditor * editor = dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget.get());
            
            bool active = false;
            
            bool ticked = false;
            
            if(editor != nullptr)
            {
                Modulator *mod = dynamic_cast<Modulator*>(editor->getProcessor());
                
                if(mod != nullptr)
                {
                    active = true;
                    
                    ticked = mod->isPlotted();
                }
            }
            
            setCommandTarget(result, "Plot Modulator", active, ticked, 'P');
            break;
        }

    case MenuEditCloseAllChains:
        setCommandTarget(result, "Close all chains", clipBoardNotEmpty(), false, 'X', false);
        break;
	case MenuToolsRecompile:
        setCommandTarget(result, "Recompile all scripts", true, false, 'X', false);
        break;
	case MenuToolsSetCompileTimeOut:
		setCommandTarget(result, "Change compile time out duration", true, false, 'X', false);
		break;
	case MenuToolsUseBackgroundThreadForCompile:
		setCommandTarget(result, "Use background thread for script compiling", true, bpe->getBackendProcessor()->isUsingBackgroundThreadForCompiling(), 'X', false);
		break;
	case MenuToolsRecompileScriptsOnReload:
		setCommandTarget(result, "Recompile all scripts on preset load", true, bpe->getBackendProcessor()->isCompilingAllScriptsOnPresetLoad(), 'X', false);
		break;
	case MenuToolsDeleteMissingSamples:
		setCommandTarget(result, "Delete missing samples", true, false, 'X', false);
		break;
	case MenuToolsResolveMissingSamples:
		setCommandTarget(result, "Resolve missing samples", true, false, 'X', false);
		break;
	case MenuToolsUseRelativePaths:
		setCommandTarget(result, "Use relative paths to project folder", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), bpe->getBackendProcessor()->getSampleManager().shouldUseRelativePathToProjectFolder(), 'X', false);
		break;
	case MenuToolsCollectExternalFiles:
		setCommandTarget(result, "Collect external files into Project folder", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
    case MenuToolsRedirectSampleFolder:
		setCommandTarget(result, "Redirect sample folder", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(),
			GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isRedirected(ProjectHandler::SubDirectories::Samples), 'X', false);
        break;
    case MenuViewFullscreen:
        setCommandTarget(result, "Toggle Fullscreen", true, bpe->isFullScreenMode(), 'F');
        break;
	case MenuViewBack:
		setCommandTarget(result, "Back: " + bpe->getViewUndoManager()->getUndoDescription(), bpe->getViewUndoManager()->canUndo(), true, (char)KeyPress::backspaceKey, true, ModifierKeys::noModifiers);
		break;
	case MenuViewForward:
		setCommandTarget(result, "Forward: " + bpe->getViewUndoManager()->getRedoDescription(), bpe->getViewUndoManager()->canRedo(), true, (char)KeyPress::backspaceKey, true, ModifierKeys::shiftModifier);
		break;
	case MenuOneColumn:
		setCommandTarget(result, "One Column", true, currentColumnMode == OneColumn, '1', true, ModifierKeys::altModifier);
		break;
	case MenuTwoColumns:
		setCommandTarget(result, "Two Columns", true, currentColumnMode == TwoColumns, '2', true, ModifierKeys::altModifier);
		break;
	case MenuThreeColumns:
		setCommandTarget(result, "Three Columns", true, currentColumnMode == ThreeColumns, '3', true, ModifierKeys::altModifier);
		break;
	case MenuViewShowPluginPopupPreview:
		setCommandTarget(result, "Open Plugin Preview Window", bpe->isPluginPreviewCreatable(), bpe->isPluginPreviewShown(), 'X', false);
		break;
    case MenuAddView:
        setCommandTarget(result, "Add new view", true, false, 'X', false);
        break;
    case MenuDeleteView:
        setCommandTarget(result, "Delete current view", viewActive(), false, 'X', false);
        break;
    case MenuRenameView:
        setCommandTarget(result, "Rename current view", viewActive(), false, 'X', false);
        break;
    case MenuViewSaveCurrentView:
        setCommandTarget(result, "Save current view", viewActive(), false, 'X', false);
        break;
	case MenuViewShowSelectedProcessorInPopup:
		setCommandTarget(result, "Show Processor in full screen", dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget.get()) != nullptr, false, 'X', false);
		result.addDefaultKeypress(KeyPress::F11Key, ModifierKeys::noModifiers);
		break;
    case MenuToolsCheckDuplicate:
        setCommandTarget(result, "Check duplicate IDs", true, false, 'X', false);
        break;
    case MenuToolsClearConsole:
        setCommandTarget(result, "Clear Console", true, false, 'X', false);
        break;
	case MenuHelpShowAboutPage:
		setCommandTarget(result, "About HISE", true, false, 'X', false);

		break;
	default:					jassertfalse; return;
	}
}

bool BackendCommandTarget::perform(const InvocationInfo &info)
{
	switch (info.commandID)
	{
	case CustomInterface:               toggleVisibility(bpe->interfaceComponent);
                                        bpe->viewedComponentChanged();
                                        return true;
	case DebugPanel:                    toggleVisibility(bpe->referenceDebugArea);
                                        bpe->viewedComponentChanged();
                                        return true;
	case Keyboard:                      toggleVisibility(bpe->keyboard);
                                        bpe->viewedComponentChanged();
                                        return true;
	case Macros:                        toggleVisibility(bpe->macroKnobs);
                                        bpe->viewedComponentChanged();
                                        return true;
	case ModulatorList:                 bpe->showModulatorTreePopup(); return true;
	case ViewPanel:                     bpe->showViewPanelPopup(); return true;
	case Settings:                      bpe->showSettingsWindow(); return true;
	case MenuNewFile:                   if (PresetHandler::showYesNoWindow("New File", "Do you want to start a new preset?"))
                                            bpe->clearPreset(); return true;
	case MenuOpenFile:                  Actions::openFile(bpe); return true;
	case MenuSaveFile:                  Actions::saveFile(bpe); return true;
	case MenuSaveFileAsXmlBackup:		Actions::saveFileAsXml(bpe); return true;
	case MenuOpenXmlBackup:				Actions::openFileFromXml(bpe); return true;
	case MenuProjectNew:				Actions::createNewProject(bpe); updateCommands();  return true;
	case MenuProjectLoad:				Actions::loadProject(bpe); updateCommands(); return true;
	case MenuCloseProject:				Actions::closeProject(bpe); updateCommands(); return true;
	case MenuProjectShowInFinder:		Actions::showProjectInFinder(bpe); return true;
	case MenuFileSettingsPreset:		Actions::showFilePresetSettings(bpe); return true;
	case MenuFileSettingsProject:		Actions::showFileProjectSettings(bpe); return true;
	case MenuFileSettingsUser:			Actions::showFileUserSettings(bpe); return true;
	case MenuFileSettingsCompiler:		Actions::showFileCompilerSettings(bpe); return true;
	case MenuFileSettingCheckSanity:	Actions::checkSettingSanity(bpe); return true;
	case MenuReplaceWithClipboardContent: Actions::replaceWithClipboardContent(bpe); return true;
	case MenuFileQuit:                  if (PresetHandler::showYesNoWindow("Quit Application", "Do you want to quit?"))
                                            JUCEApplicationBase::quit(); return true;
	case MenuEditCopy:                  if (currentCopyPasteTarget) currentCopyPasteTarget->copyAction(); return true;
	case MenuEditPaste:                 if (currentCopyPasteTarget) currentCopyPasteTarget->pasteAction(); return true;
	case MenuEditCreateScriptVariable:  Actions::createScriptVariableDeclaration(currentCopyPasteTarget); return true;
    case MenuEditPlotModulator:         Actions::plotModulator(currentCopyPasteTarget.get()); updateCommands(); return true;
    case MenuEditCloseAllChains:        Actions::closeAllChains(bpe); return true;
	case MenuToolsRecompile:            Actions::recompileAllScripts(bpe); return true;
	case MenuToolsSetCompileTimeOut:	Actions::setCompileTimeOut(bpe); return true;
	case MenuToolsUseBackgroundThreadForCompile: Actions::toggleUseBackgroundThreadsForCompiling(bpe); updateCommands(); return true;
	case MenuToolsRecompileScriptsOnReload: Actions::toggleCompileScriptsOnPresetLoad(bpe); updateCommands(); return true;
    case MenuToolsCheckDuplicate:       Actions::checkDuplicateIds(bpe); return true;
	case MenuToolsDeleteMissingSamples: Actions::deleteMissingSamples(bpe); return true;
	case MenuToolsResolveMissingSamples:Actions::resolveMissingSamples(bpe); return true;
	case MenuToolsUseRelativePaths:		Actions::toggleRelativePath(bpe); updateCommands();  return true;
	case MenuToolsCollectExternalFiles:	Actions::collectExternalFiles(bpe); return true;
    case MenuToolsRedirectSampleFolder: Actions::redirectSampleFolder(bpe); updateCommands(); return true;
    case MenuViewFullscreen:            Actions::toggleFullscreen(bpe); updateCommands(); return true;
	case MenuViewBack:					bpe->getViewUndoManager()->undo(); updateCommands(); return true;
	case MenuViewForward:				bpe->getViewUndoManager()->redo(); updateCommands(); return true;
	case MenuViewShowPluginPopupPreview: Actions::togglePluginPopupWindow(bpe); updateCommands(); return true;
    case MenuExportFileAsPlugin:        CompileExporter::exportMainSynthChainAsPackage(owner->getMainSynthChain()); return true;
    case MenuAddView:                   Actions::addView(bpe); updateCommands();return true;
    case MenuDeleteView:                Actions::deleteView(bpe); updateCommands();return true;
    case MenuRenameView:                Actions::renameView(bpe); updateCommands();return true;
    case MenuViewSaveCurrentView:       Actions::saveView(bpe); updateCommands(); return true;
    case MenuToolsClearConsole:         owner->clearConsole(); return true;
	case MenuHelpShowAboutPage:			Actions::showAboutPage(bpe); return true;
	case MenuOneColumn:					Actions::setColumns(bpe, this, OneColumn);  updateCommands(); return true;
	case MenuTwoColumns:				Actions::setColumns(bpe, this, TwoColumns);  updateCommands(); return true;
	case MenuThreeColumns:				Actions::setColumns(bpe, this, ThreeColumns);  updateCommands(); return true;
	case MenuViewShowSelectedProcessorInPopup: Actions::showProcessorInPopup(bpe, dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget.get())); return true;
	}

	return false;
}

struct FileModificationComparator
{
	static int compareElements(const File &first,
		const File &second)
	{
		const int64 firstTime = first.getLastAccessTime().toMilliseconds();

		const int64 secondTime = second.getLastAccessTime().toMilliseconds();

		if (firstTime > secondTime)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
};


PopupMenu BackendCommandTarget::getMenuForIndex(int topLevelMenuIndex, const String &/*menuName*/)
{
	MenuNames m = (MenuNames)topLevelMenuIndex;

	PopupMenu p;

	switch (m)
	{
	case BackendCommandTarget::FileMenu: {
		p.addCommandItem(mainCommandManager, MenuNewFile);

		p.addCommandItem(mainCommandManager, MenuOpenFile);
		p.addCommandItem(mainCommandManager, MenuSaveFile);
		p.addCommandItem(mainCommandManager, MenuReplaceWithClipboardContent);

		PopupMenu filesInProject;

		if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
		{
			File presetDir = GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::Presets);

			recentFileList.clear();

			presetDir.findChildFiles(recentFileList, File::findFiles, false, "*.hip");
            
            FileModificationComparator comparator;
            
			recentFileList.sort(comparator, false);

			for (int i = 0; i < recentFileList.size(); i++)
			{
				filesInProject.addItem(MenuOpenFileFromProjectOffset+i, recentFileList[i].getFileNameWithoutExtension(), true, false);
			}
		}

		p.addSubMenu("Open File from Project", filesInProject, filesInProject.getNumItems() != 0);

		p.addSeparator();

		p.addCommandItem(mainCommandManager, MenuProjectNew);
		p.addCommandItem(mainCommandManager, MenuProjectLoad);
		p.addCommandItem(mainCommandManager, MenuCloseProject);
		p.addCommandItem(mainCommandManager, MenuProjectShowInFinder);

		PopupMenu recentProjects;

		StringArray recentProjectDirectories = GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getRecentWorkDirectories();

		String currentProject = GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getWorkDirectory().getFullPathName();

		for (int i = 0; i < recentProjectDirectories.size(); i++)
		{
			recentProjects.addItem(MenuProjectRecentOffset + i, recentProjectDirectories[i], true, currentProject == recentProjectDirectories[i]);
		}

		p.addSubMenu("Recent projects", recentProjects);

		p.addSeparator();

		p.addCommandItem(mainCommandManager, MenuOpenXmlBackup);
		p.addCommandItem(mainCommandManager, MenuSaveFileAsXmlBackup);

		p.addSeparator();

		PopupMenu settingsSub;

		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsPreset);
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsProject);
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsUser);
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsCompiler);
		settingsSub.addSeparator();
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingCheckSanity);

		p.addSubMenu("Settings", settingsSub);

		PopupMenu exportSub;

        exportSub.addCommandItem(mainCommandManager, MenuExportFileAsPlugin);
		exportSub.addItem(4, "Export as HISE Player library");

		p.addSubMenu("Export", exportSub);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuFileQuit);
		break; }
	case BackendCommandTarget::EditMenu:
        if(dynamic_cast<JavascriptCodeEditor*>(bpe->currentCopyPasteTarget.get()))
        {
            dynamic_cast<JavascriptCodeEditor*>(bpe->currentCopyPasteTarget.get())->addPopupMenuItems(p, nullptr);
            
        }
		else if (dynamic_cast<SampleMapEditor*>(mainCommandManager->getFirstCommandTarget(SampleMapEditor::Undo)))
		{
			dynamic_cast<SampleMapEditor*>(mainCommandManager->getFirstCommandTarget(SampleMapEditor::Undo))->fillPopupMenu(p);
		}
        else
        {
            p.addCommandItem(mainCommandManager, MenuEditCopy);
            p.addCommandItem(mainCommandManager, MenuEditPaste);
            p.addSeparator();
            
            const int chainOffset = 0x6000;
            
            BetterProcessorEditor * editor = dynamic_cast<BetterProcessorEditor*>(bpe->currentCopyPasteTarget.get());
            if(editor != nullptr)
            {
                for(int i = 0; i < editor->getProcessor()->getNumInternalChains(); i++)
                {
                    Processor *child =  editor->getProcessor()->getChildProcessor(i);
                    
                    p.addItem(chainOffset, "Show " + child->getId(), true, child->getEditorState(Processor::EditorState::Visible));
                }
            }
            
            p.addCommandItem(mainCommandManager, MenuEditCreateScriptVariable);
            p.addCommandItem(mainCommandManager, MenuEditCloseAllChains);
            p.addCommandItem(mainCommandManager, MenuEditPlotModulator);
        }
		break;
	case BackendCommandTarget::ToolsMenu:
	{
		p.addSectionHeader("Scripting Tools");
		p.addCommandItem(mainCommandManager, MenuToolsRecompile);
		p.addCommandItem(mainCommandManager, MenuToolsCheckDuplicate);
		p.addCommandItem(mainCommandManager, MenuToolsClearConsole);
		p.addCommandItem(mainCommandManager, MenuToolsRecompileScriptsOnReload);
		p.addCommandItem(mainCommandManager, MenuToolsSetCompileTimeOut);
		p.addCommandItem(mainCommandManager, MenuToolsUseBackgroundThreadForCompile);

		PopupMenu sub;

		Array<File> files;
		StringArray processors;

		bpe->getBackendProcessor()->fillExternalFileList(files, processors);

		for (int i = 0; i < files.size(); i++)
		{
			sub.addItem(MenuToolsExternalScriptFileOffset + i, processors[i] + ": " + files[i].getFileName());
		}

		p.addSubMenu("Edit external script files", sub, files.size() != 0);

		p.addSeparator();
		p.addSectionHeader("Sample Management");
		p.addCommandItem(mainCommandManager, MenuToolsResolveMissingSamples);
		p.addCommandItem(mainCommandManager, MenuToolsDeleteMissingSamples);
		p.addCommandItem(mainCommandManager, MenuToolsUseRelativePaths);
		p.addCommandItem(mainCommandManager, MenuToolsCollectExternalFiles);
		p.addCommandItem(mainCommandManager, MenuToolsRedirectSampleFolder);
		break;
	}
	case BackendCommandTarget::ViewMenu: {
		p.addCommandItem(mainCommandManager, MenuViewBack);
		p.addCommandItem(mainCommandManager, MenuViewForward);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuViewFullscreen);
		p.addCommandItem(mainCommandManager, MenuViewShowSelectedProcessorInPopup);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuOneColumn);
		p.addCommandItem(mainCommandManager, MenuTwoColumns);
		p.addCommandItem(mainCommandManager, MenuThreeColumns);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuViewShowPluginPopupPreview);
		p.addCommandItem(mainCommandManager, CustomInterface);
		p.addCommandItem(mainCommandManager, DebugPanel);
		p.addCommandItem(mainCommandManager, Macros);
		p.addCommandItem(mainCommandManager, Keyboard);
		p.addCommandItem(mainCommandManager, Settings);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuAddView);
		p.addCommandItem(mainCommandManager, MenuDeleteView);
		p.addCommandItem(mainCommandManager, MenuRenameView);
		p.addCommandItem(mainCommandManager, MenuViewSaveCurrentView);

		if (viewActive())
		{
			p.addSeparator();
			p.addSectionHeader("Current View: " + owner->synthChain->getCurrentViewInfo()->getViewName());

			for (int i = 0; i < owner->synthChain->getNumViewInfos(); i++)
			{
				ViewInfo *info = owner->synthChain->getViewInfo(i);

				p.addItem(MenuViewOffset + i, info->getViewName(), true, info == owner->synthChain->getCurrentViewInfo());
			}

		}

		PopupMenu processorList;

		Processor::Iterator<Processor> iter(owner->getMainSynthChain());

		int i = 0;

		while (Processor *pl = iter.getNextProcessor())
		{
			if (ProcessorHelpers::is<ModulatorChain>(pl)) continue;
			if (ProcessorHelpers::is<MidiProcessorChain>(pl)) continue;
			if (ProcessorHelpers::is<EffectProcessorChain>(pl)) continue;

			processorList.addItem(MenuViewProcessorListOffset + i, pl->getId());

			i++;
		}

		if (processorList.containsAnyActiveItems())
		{
			p.addSeparator();
			p.addSubMenu("Solo Processor", processorList);
		}

		break;
		}
	case BackendCommandTarget::HelpMenu:
			p.addCommandItem(mainCommandManager, MenuHelpShowAboutPage);
		break;
	default:
		break;
	}

	return p;
}

void BackendCommandTarget::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
{
	if (menuItemID >= MenuOpenFileFromProjectOffset && menuItemID < ((int)(MenuOpenFileFromProjectOffset) + 50))
	{
		if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
		{
			File f = recentFileList[menuItemID - (int)MenuOpenFileFromProjectOffset];

			if (f.existsAsFile())
			{
				bpe->loadNewContainer(f);
			}
		}
	}
	else if (menuItemID >= MenuProjectRecentOffset && menuItemID < (int)MenuProjectRecentOffset + 12)
	{
		const int index = menuItemID - MenuProjectRecentOffset;

		if (PresetHandler::showYesNoWindow("Switch projects?", "Do you want to switch projects? The current preset will be cleared"))
		{
			String file = GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getRecentWorkDirectories()[index];

			bpe->clearPreset();

			GET_PROJECT_HANDLER(bpe->getMainSynthChain()).setWorkingProject(file);
		}
	}

	else if (menuItemID >= MenuViewOffset && menuItemID < (int)MenuViewOffset + 200)
	{
		ViewInfo *info = owner->synthChain->getViewInfo(menuItemID - MenuViewOffset);

		if (info != nullptr)
		{
			info->restore();

			bpe->setRootProcessor(info->getRootProcessor());

			owner->synthChain->setCurrentViewInfo(menuItemID - MenuViewOffset);
		}
	}

	else if (menuItemID >= MenuViewProcessorListOffset && menuItemID < (int)MenuViewProcessorListOffset + 200)
	{
		Processor::Iterator<Processor> iter(owner->synthChain);

		int i = 0;

		const int indexToLookFor = (menuItemID - (int)MenuViewProcessorListOffset);

		while (Processor *p = iter.getNextProcessor())
		{
			if (ProcessorHelpers::is<ModulatorChain>(p)) continue;
			if (ProcessorHelpers::is<MidiProcessorChain>(p)) continue;
			if (ProcessorHelpers::is<EffectProcessorChain>(p)) continue;

			if (i == indexToLookFor)
			{
				bpe->showProcessorPopup(p, ProcessorHelpers::findParentProcessor(p, false));
			}

			i++;
		}
	}
	else if (menuItemID >= MenuToolsExternalScriptFileOffset && menuItemID < (MenuToolsExternalScriptFileOffset + 50))
	{
		Array<File> files;
		StringArray processors;

		bpe->getBackendProcessor()->fillExternalFileList(files, processors);

		File f = files[menuItemID - MenuToolsExternalScriptFileOffset];

		Processor::Iterator<ScriptProcessor> iter(bpe->getMainSynthChain());

		while (ScriptProcessor *sp = iter.getNextProcessor())
		{
			for (int i = 0; i < sp->getNumWatchedFiles(); i++)
			{
				if (sp->getWatchedFile(i) == f)
				{
					sp->showPopupForFile(i);
					return;
				}
			}
		}

		jassertfalse;
	}
}

#undef toggleVisibility

bool BackendCommandTarget::Actions::hasProcessorInClipboard()
{
	ScopedPointer<XmlElement> xml = XmlDocument::parse(SystemClipboard::getTextFromClipboard());

	if (xml != nullptr)
	{
		ValueTree v = ValueTree::fromXml(*xml);

		if (v.isValid() && v.getProperty("Type") == "SynthChain")
		{
			return true;
			
		}
	}

	return false;
}

void BackendCommandTarget::Actions::openFile(BackendProcessorEditor *bpe)
{
	FileChooser fc("Load Preset File", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::Presets), "*.hip", true);

	if (fc.browseForFileToOpen()) bpe->loadNewContainer(fc.getResult());
}

void BackendCommandTarget::Actions::saveFile(BackendProcessorEditor *bpe)
{
	if (PresetHandler::showYesNoWindow("Save " + bpe->owner->getMainSynthChain()->getId(), "Do you want to save this preset?"))
	{
		PresetHandler::saveProcessorAsPreset(bpe->owner->getMainSynthChain());
	}
}

void BackendCommandTarget::Actions::replaceWithClipboardContent(BackendProcessorEditor *bpe)
{
	ScopedPointer<XmlElement> xml = XmlDocument::parse(SystemClipboard::getTextFromClipboard());

	if (xml != nullptr)
	{
		ValueTree v = ValueTree::fromXml(*xml);

		if (v.isValid() && v.getProperty("Type") == "SynthChain")
		{
			bpe->loadNewContainer(v);
			return;
		}
	}

	PresetHandler::showMessageWindow("Invalid Preset", "The clipboard does not contain a valid container.");
}

void BackendCommandTarget::Actions::createScriptVariableDeclaration(CopyPasteTarget *currentCopyPasteTarget)
{
	BetterProcessorEditor *editor = dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget);

	if (editor != nullptr)
	{
		ProcessorHelpers::getScriptVariableDeclaration(editor->getProcessor());
	}
}

void BackendCommandTarget::Actions::recompileAllScripts(BackendProcessorEditor * bpe)
{
	bpe->owner->compileAllScripts();
}

void BackendCommandTarget::Actions::toggleFullscreen(BackendProcessorEditor * bpe)
{
#if IS_STANDALONE_APP
    
    Component *window = bpe->getParentComponent()->getParentComponent();
    
    if (bpe->isFullScreenMode())
    {
        Desktop::getInstance().setKioskModeComponent(nullptr);
        
        const int height = Desktop::getInstance().getDisplays().getMainDisplay().userArea.getHeight() - 70;
        bpe->setSize(bpe->referenceDebugArea->isVisible() ? 1280 : 900, height);
        bpe->resized();
        
        window->centreWithSize(bpe->getWidth(), bpe->getHeight());
        
        bpe->setAlwaysOnTop(false);
        bpe->borderDragger->setVisible(true);
    }
    else
    {
        Desktop::getInstance().setKioskModeComponent(window);
        
        bpe->borderDragger->setVisible(false);
        bpe->setAlwaysOnTop(true);
        
        bpe->setSize(Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getWidth(),
                Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getHeight());
        
        bpe->resized();
        
    }
#endif
}

void BackendCommandTarget::Actions::addView(BackendProcessorEditor *bpe)
{
    String view = PresetHandler::getCustomName("View");
    
    ViewInfo *info = new ViewInfo(bpe->owner->synthChain, bpe->currentRootProcessor, view);
    
    bpe->owner->synthChain->addViewInfo(info);
}

void BackendCommandTarget::Actions::deleteView(BackendProcessorEditor *bpe)
{
    bpe->owner->synthChain->removeCurrentViewInfo();
}

void BackendCommandTarget::Actions::renameView(BackendProcessorEditor *bpe)
{
    String view = PresetHandler::getCustomName("View");
    
    bpe->owner->synthChain->getCurrentViewInfo()->setViewName(view);
}

void BackendCommandTarget::Actions::saveView(BackendProcessorEditor *bpe)
{
    String view = bpe->owner->synthChain->getCurrentViewInfo()->getViewName();
    
    ViewInfo *info = new ViewInfo(bpe->owner->synthChain, bpe->currentRootProcessor, view);
    
    bpe->owner->synthChain->replaceCurrentViewInfo(info);
}

void BackendCommandTarget::Actions::closeAllChains(BackendProcessorEditor *bpe)
{
    BetterProcessorEditor *editor = dynamic_cast<BetterProcessorEditor*>(bpe->currentCopyPasteTarget.get());
    
    if(editor != nullptr)
    {
        editor->getChainBar()->closeAll();
    }
}

void BackendCommandTarget::Actions::checkDuplicateIds(BackendProcessorEditor *bpe)
{
    PresetHandler::checkProcessorIdsForDuplicates(bpe->owner->synthChain, false);

}

void BackendCommandTarget::Actions::showAboutPage(BackendProcessorEditor * bpe)
{
	bpe->aboutPage->showAboutPage();
}

void BackendCommandTarget::Actions::setColumns(BackendProcessorEditor * bpe, BackendCommandTarget* target, ColumnMode columns)
{
	target->currentColumnMode = columns;

	switch (columns)
	{
	case BackendCommandTarget::OneColumn:
		bpe->setSize(900, bpe->getHeight());
		break;
	case BackendCommandTarget::TwoColumns:
		bpe->setSize(jmin<int>(Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getWidth(), 1280), bpe->getHeight());
		break;
	case BackendCommandTarget::ThreeColumns:
		bpe->setSize(jmin<int>(Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getWidth(), 1650), bpe->getHeight());
		break;
	default:
		break;
	}

	bpe->resized();
	
}

void BackendCommandTarget::Actions::showProcessorInPopup(BackendProcessorEditor * bpe, BetterProcessorEditor* editor)
{
	bpe->showProcessorPopup(editor->getProcessor(), editor->getParentEditor() != nullptr ? editor->getParentEditor()->getProcessor() : nullptr);
}

void BackendCommandTarget::Actions::plotModulator(CopyPasteTarget *currentCopyPasteTarget)
{
    BetterProcessorEditor *editor = dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget);
                                                                         
    if(editor != nullptr)
    {
        Modulator *mod = dynamic_cast<Modulator*>(editor->getProcessor());
        if(mod != nullptr)
        {
            if(mod->isPlotted())
            {
                mod->getMainController()->removePlottedModulator(mod);
            }
            else
            {
                mod->getMainController()->addPlottedModulator(mod);
            }
        }
    }
}

void BackendCommandTarget::Actions::resolveMissingSamples(BackendProcessorEditor *bpe)
{
	bpe->getBackendProcessor()->getSampleManager().getModulatorSamplerSoundPool()->resolveMissingSamples(bpe);
}

void BackendCommandTarget::Actions::deleteMissingSamples(BackendProcessorEditor *bpe)
{
	bpe->getBackendProcessor()->getSampleManager().getModulatorSamplerSoundPool()->deleteMissingSamples();
}

void BackendCommandTarget::Actions::setCompileTimeOut(BackendProcessorEditor * bpe)
{
	AlertWindowLookAndFeel alaf;

	AlertWindow newTime("Enter new compile time out duration", "Current time out: " + String(bpe->getBackendProcessor()->getCompileTimeOut(),1) + " seconds.", AlertWindow::QuestionIcon, bpe);

	newTime.setLookAndFeel(&alaf);

	newTime.addTextEditor("time", String(bpe->getBackendProcessor()->getCompileTimeOut(), 1), "", false);
	
	newTime.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	newTime.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	if (newTime.runModalLoop())
	{
		double value = newTime.getTextEditor("time")->getText().getDoubleValue();

		if (value != 0.0)
		{
			bpe->getBackendProcessor()->setCompileTimeOut(value);
		}
	}
}

void BackendCommandTarget::Actions::toggleUseBackgroundThreadsForCompiling(BackendProcessorEditor * bpe)
{
	const bool lastState = bpe->getBackendProcessor()->isUsingBackgroundThreadForCompiling();

	bpe->getBackendProcessor()->setShouldUseBackgroundThreadForCompiling(!lastState);
}

void BackendCommandTarget::Actions::toggleCompileScriptsOnPresetLoad(BackendProcessorEditor * bpe)
{
	const bool lastState = bpe->getBackendProcessor()->isCompilingAllScriptsOnPresetLoad();

	bpe->getBackendProcessor()->setEnableCompileAllScriptsOnPresetLoad(!lastState);
}


void BackendCommandTarget::Actions::toggleRelativePath(BackendProcessorEditor * bpe)
{
	const bool state = bpe->getBackendProcessor()->getSampleManager().shouldUseRelativePathToProjectFolder();

	bpe->getBackendProcessor()->getSampleManager().setShouldUseRelativePathToProjectFolder(!state);
}


void BackendCommandTarget::Actions::collectExternalFiles(BackendProcessorEditor * bpe)
{
	ExternalResourceCollector *resource = new ExternalResourceCollector(bpe->getBackendProcessor());

	resource->setModalComponentOfMainEditor(bpe);
}

void BackendCommandTarget::Actions::saveFileAsXml(BackendProcessorEditor * bpe)
{
	if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
	{
		FileChooser fc("Select XML file to load", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::XMLPresetBackups), "*.xml", true);

		if (fc.browseForFileToSave(true))
		{
			ValueTree v = bpe->owner->getMainSynthChain()->exportAsValueTree();

            v.setProperty("BuildVersion", BUILD_SUB_VERSION, nullptr);
            
			fc.getResult().replaceWithText(v.toXmlString());

			debugToConsole(bpe->owner->getMainSynthChain(), "Exported as XML");
		}
	}
}

void BackendCommandTarget::Actions::openFileFromXml(BackendProcessorEditor * bpe)
{
	if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
	{
		FileChooser fc("Select XML file to load", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::XMLPresetBackups), "*.xml", true);

		if (fc.browseForFileToOpen())
		{
			File f = fc.getResult();

			ScopedPointer<XmlElement> xml = XmlDocument::parse(f);

			ValueTree v = ValueTree::fromXml(*xml);

			bpe->loadNewContainer(v);
		}
	}
}

void BackendCommandTarget::Actions::createNewProject(BackendProcessorEditor *bpe)
{
	FileChooser fc("Create new project directory");

	if (fc.browseForDirectory())
	{
		File f = fc.getResult();

		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).createNewProject(f);
	}
}

void BackendCommandTarget::Actions::loadProject(BackendProcessorEditor *bpe)
{
	FileChooser fc("Load project (set as working directory)");

	if (fc.browseForDirectory())
	{
		File f = fc.getResult();

		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).setWorkingProject(f);
	}
}

void BackendCommandTarget::Actions::closeProject(BackendProcessorEditor *bpe)
{
	GET_PROJECT_HANDLER(bpe->getMainSynthChain()).setWorkingProject(File::nonexistent);
}

void BackendCommandTarget::Actions::showProjectInFinder(BackendProcessorEditor *bpe)
{
	if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
	{
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getWorkDirectory().revealToUser();
	}
}

void BackendCommandTarget::Actions::redirectSampleFolder(BackendProcessorEditor *bpe)
{
    FileChooser fc("Redirect sample folder to the following location");
    
    if (fc.browseForDirectory())
    {
        File f = fc.getResult();
        
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).createLinkFile(ProjectHandler::SubDirectories::Samples, f);
    }
}

void BackendCommandTarget::Actions::showFilePresetSettings(BackendProcessorEditor * /*bpe*/)
{
	
}

void BackendCommandTarget::Actions::showFileProjectSettings(BackendProcessorEditor * bpe)
{
	SettingWindows::ProjectSettingWindow *window = new SettingWindows::ProjectSettingWindow(&GET_PROJECT_HANDLER(bpe->getMainSynthChain()));

	window->setModalComponentOfMainEditor(bpe);
}

void BackendCommandTarget::Actions::showFileUserSettings(BackendProcessorEditor * bpe)
{
	SettingWindows::UserSettingWindow *window = new SettingWindows::UserSettingWindow();

	window->setModalComponentOfMainEditor(bpe);
}

void BackendCommandTarget::Actions::showFileCompilerSettings(BackendProcessorEditor * bpe)
{
	SettingWindows::CompilerSettingWindow *window = new SettingWindows::CompilerSettingWindow();

	window->setModalComponentOfMainEditor(bpe);
}

void BackendCommandTarget::Actions::checkSettingSanity(BackendProcessorEditor * bpe)
{
	SettingWindows::checkAllSettings(&GET_PROJECT_HANDLER(bpe->getMainSynthChain()));
}

void BackendCommandTarget::Actions::togglePluginPopupWindow(BackendProcessorEditor * bpe)
{
	if (bpe->isPluginPreviewShown())
	{
		bpe->setPluginPreviewWindow(nullptr);
	}
	else
	{
		bpe->setPluginPreviewWindow(new PopupPluginPreview(bpe));
	}
}