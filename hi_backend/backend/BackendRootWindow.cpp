

BackendRootWindow::BackendRootWindow(AudioProcessor *ownerProcessor, var editorState) :
	AudioProcessorEditor(ownerProcessor),
	BackendCommandTarget(static_cast<BackendProcessor*>(ownerProcessor)),
	owner(static_cast<BackendProcessor*>(ownerProcessor))
{

	addAndMakeVisible(floatingRoot = new FloatingTile(nullptr));

	bool loadedCorrectly = true;
	bool objectFound = editorState.isObject();

	int width = 1500;
	int height = 900;

	if (objectFound)
	{
		floatingRoot->setContent(editorState);

		mainEditor = FloatingTileHelpers::findTileWithId<BackendProcessorEditor>(floatingRoot, Identifier("MainColumn"));

		loadedCorrectly = mainEditor != nullptr;

		if (loadedCorrectly)
		{
			auto mws = FloatingTileHelpers::findTileWithId<FloatingTileContainer>(floatingRoot, Identifier("MainWorkspace"));

			if (mws != nullptr)
				workspaces.add(mws->getParentShell());
			else
				loadedCorrectly = false;
		}

		if (loadedCorrectly)
		{
			auto mws = FloatingTileHelpers::findTileWithId<FloatingTileContainer>(floatingRoot, Identifier("ScriptingWorkspace"));

			if (mws != nullptr)
				workspaces.add(mws->getParentShell());
			else
				loadedCorrectly = false;
		}

		if (loadedCorrectly)
		{
			auto mws = FloatingTileHelpers::findTileWithId<FloatingTileContainer>(floatingRoot, Identifier("SamplerWorkspace"));

			if (mws != nullptr)
				workspaces.add(mws->getParentShell());
			else
				loadedCorrectly = false;
		}

		if (loadedCorrectly)
		{
			auto mws = FloatingTileHelpers::findTileWithId<FloatingTileContainer>(floatingRoot, Identifier("CustomWorkspace"));

			if (mws != nullptr)
				workspaces.add(mws->getParentShell());
			else
				loadedCorrectly = false;
		}

		if (loadedCorrectly)
		{
			width = jmax<int>(960, editorState.getDynamicObject()->getProperty("MainWidth"));
			height = jmax<int>(500, editorState.getDynamicObject()->getProperty("MainHeight"));
			const int workspace = editorState.getDynamicObject()->getProperty("CurrentWorkspace");

			if(workspace > 0)
				showWorkspace(workspace);
		}

		setEditor(this);
	}

	if (!loadedCorrectly)
		PresetHandler::showMessageWindow("Error loading Interface", "The interface data is corrupt. The default settings will be loaded", PresetHandler::IconType::Error);

	if(!objectFound || !loadedCorrectly)
	{
		mainEditor = dynamic_cast<BackendProcessorEditor*>(FloatingPanelTemplates::createHiseLayout(floatingRoot));

		jassert(mainEditor != nullptr);

		workspaces.add(FloatingTileHelpers::findTileWithId<VerticalTile>(floatingRoot, Identifier("MainWorkspace"))->getParentShell());
		workspaces.add(FloatingTileHelpers::findTileWithId<FloatingTileContainer>(floatingRoot, Identifier("ScriptingWorkspace"))->getParentShell());
		workspaces.add(FloatingTileHelpers::findTileWithId<FloatingTileContainer>(floatingRoot, Identifier("SamplerWorkspace"))->getParentShell());
		workspaces.add(FloatingTileHelpers::findTileWithId<HorizontalTile>(floatingRoot, Identifier("CustomWorkspace"))->getParentShell());

		showWorkspace(BackendCommandTarget::WorkspaceMain);

		setEditor(this);
	}


	auto consoleParent = FloatingTileHelpers::findTileWithId<ConsolePanel>(getRootFloatingTile(), "MainConsole");

	if (consoleParent != nullptr)
		getBackendProcessor()->getConsoleHandler().setMainConsole(consoleParent->getConsole());
	else
		jassertfalse;

	setOpaque(true);

#if IS_STANDALONE_APP 

	if (owner->callback->getCurrentProcessor() == nullptr)
	{
		showSettingsWindow();
	}

#endif

	PresetHandler::buildProcessorDataBase(owner->getMainSynthChain());

	constrainer = new ComponentBoundsConstrainer();
	constrainer->setMinimumHeight(500);
	constrainer->setMinimumWidth(960);
	constrainer->setMaximumWidth(4000);

	

	addAndMakeVisible(yBorderDragger = new ResizableBorderComponent(this, constrainer));
	addAndMakeVisible(xBorderDragger = new ResizableBorderComponent(this, constrainer));

	BorderSize<int> yBorderSize;
	yBorderSize.setTop(0);
	yBorderSize.setLeft(0);
	yBorderSize.setRight(0);
	yBorderSize.setBottom(7);
	yBorderDragger->setBorderThickness(yBorderSize);


	BorderSize<int> xBorderSize;
	xBorderSize.setTop(0);
	xBorderSize.setLeft(0);
	xBorderSize.setRight(7);
	xBorderSize.setBottom(0);

	xBorderDragger->setBorderThickness(xBorderSize);

	addAndMakeVisible(progressOverlay = new ThreadWithQuasiModalProgressWindow::Overlay());
	owner->setOverlay(progressOverlay);
	progressOverlay->setDialog(nullptr);


#if JUCE_MAC && IS_STANDALONE_APP
	MenuBarModel::setMacMainMenu(this);
#else

	addAndMakeVisible(menuBar = new MenuBarComponent(this));
	menuBar->setLookAndFeel(&plaf);

#endif

	setSize(width, height);

	startTimer(1000);

	updateCommands();
}


BackendRootWindow::~BackendRootWindow()
{
	getBackendProcessor()->getCommandManager()->clearCommands();
	getBackendProcessor()->getConsoleHandler().setMainConsole(nullptr);

	saveInterfaceData();
	

	clearModalComponent();

	modalComponent = nullptr;

	// Remove the resize stuff

	constrainer = nullptr;
	yBorderDragger = nullptr;
	xBorderDragger = nullptr;
	currentDialog = nullptr;

	// Remove the menu

#if JUCE_MAC && IS_STANDALONE_APP
	MenuBarModel::setMacMainMenu(nullptr);
#else
	menuBar->setModel(nullptr);
	menuBar = nullptr;
#endif

	floatingRoot = nullptr;

	mainEditor = nullptr;


}

bool BackendRootWindow::isFullScreenMode() const
{
#if IS_STANDALONE_APP
	if (getParentComponent() == nullptr) return false;

	Component *kioskComponent = Desktop::getInstance().getKioskModeComponent();

	Component *parentparent = getParentComponent()->getParentComponent();

	return parentparent == kioskComponent;
#else
	return false;
#endif
}

void BackendRootWindow::saveInterfaceData()
{
	if (resetOnClose)
	{
		getBackendProcessor()->setEditorData({});
	}
	else
	{
		var editorData = getRootFloatingTile()->getCurrentFloatingPanel()->toDynamicObject();

		if (auto obj = editorData.getDynamicObject())
		{
			obj->setProperty("MainWidth", getWidth());
			obj->setProperty("MainHeight", getHeight());
			obj->setProperty("CurrentWorkspace", currentWorkspace);
		}

		getBackendProcessor()->setEditorData(editorData);
	}

}

void BackendRootWindow::resized()
{

#if IS_STANDALONE_APP
	if (getParentComponent() != nullptr)
	{
		getParentComponent()->getParentComponent()->setSize(getWidth(), getHeight());
	}
#endif

	progressOverlay->setBounds(0, 0, getWidth(), getHeight());

	const int menuBarOffset = menuBar == nullptr ? 0 : 20;

	if (menuBarOffset != 0)
		menuBar->setBounds(getLocalBounds().withHeight(menuBarOffset));

	const float dpiScale = Desktop::getInstance().getGlobalScaleFactor();

	floatingRoot->setBounds(4, menuBarOffset + 4, getWidth() - 8, getHeight() - menuBarOffset - 8);

#if IS_STANDALONE_APP

	if (currentDialog != nullptr)
	{
		currentDialog->centreWithSize(700, 500);
	}

#else

	yBorderDragger->setBounds(getBounds());
	xBorderDragger->setBounds(getBounds());

#endif

}

void BackendRootWindow::showSettingsWindow()
{
	jassert(owner->deviceManager != nullptr);

	if (owner->deviceManager != nullptr && currentDialog == nullptr)
	{
		addAndMakeVisible(currentDialog = new AudioDeviceDialog(owner));

		currentDialog->centreWithSize(700, 500);
	}
	else
	{
		currentDialog = nullptr;
	}
}

void BackendRootWindow::timerCallback()
{
	stopTimer();

	if (!GET_PROJECT_HANDLER(mainEditor->getMainSynthChain()).isActive() && PresetHandler::showYesNoWindow("Welcome to HISE", "Do you want to create a new project?\nA project is a folder which contains all external files needed for a sample library."))
	{
		owner->setChanged(false);

		BackendCommandTarget::Actions::createNewProject(this);
	}
}

void BackendRootWindow::resetInterface()
{
	resetOnClose = true;

	PresetHandler::showMessageWindow("Workspace Layout Reset", "Close and open this instance to reset the interface", PresetHandler::IconType::Info);
}

void BackendRootWindow::loadNewContainer(ValueTree & v)
{
	FloatingTile::Iterator<PanelWithProcessorConnection> iter(getRootFloatingTile());

	while (auto p = iter.getNextPanel())
		p->setContentWithUndo(nullptr, 0);

	mainEditor->loadNewContainer(v);

	

}

void BackendRootWindow::loadNewContainer(const File &f)
{
	FloatingTile::Iterator<PanelWithProcessorConnection> iter(getRootFloatingTile());

	while (auto p = iter.getNextPanel())
		p->setContentWithUndo(nullptr, 0);

	mainEditor->loadNewContainer(f);
}

void BackendRootWindow::showWorkspace(int workspace)
{
	currentWorkspace = workspace;

	int workspaceIndex = workspace - BackendCommandTarget::WorkspaceMain;

	for (int i = 0; i < workspaces.size(); i++)
	{
		workspaces[i].getComponent()->getLayoutData().setVisible(i == workspaceIndex);
	}

	getRootFloatingTile()->refreshRootLayout();
}

VerticalTile* BackendPanelHelpers::getMainTabComponent(FloatingTile* root)
{
	static const Identifier id("PersonaContainer");

	return FloatingTileHelpers::findTileWithId<VerticalTile>(root, id);
}

HorizontalTile* BackendPanelHelpers::getMainLeftColumn(FloatingTile* root)
{
	static const Identifier id("MainLeftColumn");

	return FloatingTileHelpers::findTileWithId<HorizontalTile>(root, id);
}

HorizontalTile* BackendPanelHelpers::getMainRightColumn(FloatingTile* root)
{
	static const Identifier id("MainRightColumn");

	return FloatingTileHelpers::findTileWithId<HorizontalTile>(root, id);
}

void BackendPanelHelpers::showWorkspace(BackendRootWindow* root, Workspace workspaceToShow, NotificationType notifyCommandManager)
{
	if (notifyCommandManager = sendNotification)
	{
		root->getBackendProcessor()->getCommandManager()->invokeDirectly(BackendCommandTarget::WorkspaceMain + (int)workspaceToShow, false);
	}
	else
	{
		root->showWorkspace(BackendCommandTarget::WorkspaceMain + (int)workspaceToShow);
	}
}

bool BackendPanelHelpers::isMainWorkspaceActive(FloatingTile* root)
{
	return true;
}

FloatingTile* BackendPanelHelpers::ScriptingWorkspace::get(BackendRootWindow* rootWindow)
{
	return FloatingTileHelpers::findTileWithId<FloatingTileContainer>(rootWindow->getRootFloatingTile(), "ScriptingWorkspace")->getParentShell();
}

void BackendPanelHelpers::ScriptingWorkspace::setGlobalProcessor(BackendRootWindow* rootWindow, JavascriptProcessor* jsp)
{
	auto workspace = get(rootWindow);

	FloatingTile::Iterator<GlobalConnectorPanel<JavascriptProcessor>> iter(workspace);

	if (auto connector = iter.getNextPanel())
	{
		connector->setContentWithUndo(dynamic_cast<Processor*>(jsp), 0);
	}
}

void BackendPanelHelpers::ScriptingWorkspace::showEditor(BackendRootWindow* rootWindow, bool shouldBeVisible)
{
	auto workspace = get(rootWindow);

	auto editor = FloatingTileHelpers::findTileWithId<FloatingTileContainer>(workspace, "ScriptingWorkspaceCodeEditor");

	if (editor != nullptr)
	{
		editor->getParentShell()->getLayoutData().setVisible(shouldBeVisible);
		editor->getParentShell()->refreshRootLayout();
	}
    
    auto toggleBar = FloatingTileHelpers::findTileWithId<VisibilityToggleBar>(workspace, "ScriptingWorkspaceToggleBar");
    
    if(toggleBar != nullptr)
    {
        toggleBar->refreshButtons();
    }
}

void BackendPanelHelpers::ScriptingWorkspace::showInterfaceDesigner(BackendRootWindow* rootWindow, bool shouldBeVisible)
{
    auto workspace = get(rootWindow);
    
    auto editor = FloatingTileHelpers::findTileWithId<FloatingTileContainer>(workspace, "ScriptingWorkspaceInterfaceDesigner");
    
    if (editor != nullptr)
    {
        editor->getParentShell()->getLayoutData().setVisible(shouldBeVisible);
        editor->getParentShell()->refreshRootLayout();
    }
    
    auto toggleBar = FloatingTileHelpers::findTileWithId<VisibilityToggleBar>(workspace, "ScriptingWorkspaceToggleBar");
    
    if(toggleBar != nullptr)
    {
        toggleBar->refreshButtons();
    }
    else
        jassertfalse;
    
}


FloatingTile* BackendPanelHelpers::SamplerWorkspace::get(BackendRootWindow* rootWindow)
{
	return FloatingTileHelpers::findTileWithId<FloatingTileContainer>(rootWindow->getRootFloatingTile(), "SamplerWorkspace")->getParentShell();
}

void BackendPanelHelpers::SamplerWorkspace::setGlobalProcessor(BackendRootWindow* rootWindow, ModulatorSampler* sampler)
{
	auto workspace = get(rootWindow);

	FloatingTile::Iterator<GlobalConnectorPanel<ModulatorSampler>> iter(workspace);

	if (auto connector = iter.getNextPanel())
	{
		connector->setContentWithUndo(dynamic_cast<Processor*>(sampler), 0);
	}
}
