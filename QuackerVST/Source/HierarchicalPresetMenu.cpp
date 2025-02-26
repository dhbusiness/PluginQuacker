/*
  ==============================================================================

    HierarchicalPresetMenu.cpp
    Created: 26 Feb 2025 9:32:02am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "HierarchicalPresetMenu.h"


HierarchicalPresetMenu::HierarchicalPresetMenu(PresetManager& pm)
    : presetManager(pm), mainButton("Default")
{
    // Set up the main button as before
    mainButton.setLookAndFeel(&transparentButtonLookAndFeel);
    mainButton.setColour(juce::TextButton::textColourOffId, textColour);
    mainButton.setColour(juce::TextButton::textColourOnId, textColour);
    mainButton.addListener(this);
    addAndMakeVisible(mainButton);
    
    // Create normal state drawables
    auto normalLeftArrow = std::make_unique<juce::DrawablePath>();
    auto normalRightArrow = std::make_unique<juce::DrawablePath>();
    
    // Create hover state drawables
    auto hoverLeftArrow = std::make_unique<juce::DrawablePath>();
    auto hoverRightArrow = std::make_unique<juce::DrawablePath>();
    
    // Create down state drawables
    auto downLeftArrow = std::make_unique<juce::DrawablePath>();
    auto downRightArrow = std::make_unique<juce::DrawablePath>();
    
    // Create the arrow paths
    juce::Path leftArrowPath;
    leftArrowPath.addTriangle(10.0f, 10.0f, 20.0f, 5.0f, 20.0f, 15.0f);
    
    juce::Path rightArrowPath;
    rightArrowPath.addTriangle(20.0f, 10.0f, 10.0f, 5.0f, 10.0f, 15.0f);
    
    // Set up the left arrow drawables
    normalLeftArrow->setPath(leftArrowPath);
    normalLeftArrow->setFill(juce::Colour(232, 193, 185).withAlpha(0.8f));
    
    hoverLeftArrow->setPath(leftArrowPath);
    hoverLeftArrow->setFill(juce::Colour(19, 224, 139));
    
    downLeftArrow->setPath(leftArrowPath);
    downLeftArrow->setFill(juce::Colour(19, 224, 139));
    
    // Set up the right arrow drawables
    normalRightArrow->setPath(rightArrowPath);
    normalRightArrow->setFill(juce::Colour(232, 193, 185).withAlpha(0.8f));
    
    hoverRightArrow->setPath(rightArrowPath);
    hoverRightArrow->setFill(juce::Colour(19, 224, 139));
    
    downRightArrow->setPath(rightArrowPath);
    downRightArrow->setFill(juce::Colour(19, 224, 139));
    
    // Set the images on the buttons
    leftArrowButton.setImages(normalLeftArrow.get(), hoverLeftArrow.get(), downLeftArrow.get());
    rightArrowButton.setImages(normalRightArrow.get(), hoverRightArrow.get(), downRightArrow.get());
    
    // Make the button backgrounds transparent
    //leftArrowButton.setBackgroundColours(juce::Colours::transparentBlack, juce::Colours::transparentBlack);
    //rightArrowButton.setBackgroundColours(juce::Colours::transparentBlack, juce::Colours::transparentBlack);
    
    leftArrowButton.addListener(this);
    rightArrowButton.addListener(this);
    
    addAndMakeVisible(leftArrowButton);
    addAndMakeVisible(rightArrowButton);
    
    // Rest of the initialization as before
    updatePresetDisplay();
    startTimer(100);
    juce::LookAndFeel::setDefaultLookAndFeel(&menuLookAndFeel);
}

HierarchicalPresetMenu::~HierarchicalPresetMenu()
{
    mainButton.setLookAndFeel(nullptr);
    mainButton.removeListener(this);
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void HierarchicalPresetMenu::paint(juce::Graphics& g)
{
    // Draw nothing for the background/border

}

void HierarchicalPresetMenu::resized()
{
    auto bounds = getLocalBounds();
    
    // Calculate sizes for arrows
    const int arrowWidth = 20;
    const int arrowPadding = 10;
    
    // Position left arrow on the left side
    leftArrowButton.setBounds(arrowPadding, 0, arrowWidth, bounds.getHeight());
    
    // Position right arrow on the right side
    rightArrowButton.setBounds(bounds.getWidth() - arrowWidth - arrowPadding, 0,
                              arrowWidth, bounds.getHeight());
    
    // Position main button in the center
    mainButton.setBounds(leftArrowButton.getRight(), 0,
                        rightArrowButton.getX() - leftArrowButton.getRight(),
                        bounds.getHeight());
}

void HierarchicalPresetMenu::timerCallback()
{
    // Check if the preset modification status has changed
    bool newModified = presetManager.isPresetModified();
    juce::String newName = presetManager.getDisplayedPresetName();
    
    if (isModified != newModified || currentDisplayName != newName)
    {
        updatePresetDisplay();
        repaint();
    }
}

void HierarchicalPresetMenu::buttonClicked(juce::Button* button)
{
    if (button == &mainButton)
    {
        showRootMenu();
    }
    else if (button == &leftArrowButton)
    {
        // Navigate to previous preset
        navigatePresets(false);
    }
    else if (button == &rightArrowButton)
    {
        // Navigate to next preset
        navigatePresets(true);
    }
}

void HierarchicalPresetMenu::navigatePresets(bool goForward)
{
    // First, define a logical preset order that matches your menu structure
    juce::StringArray orderedPresets;
    
    // Add Factory root presets first (starting with Default and Classic Tremolo)
    juce::StringArray priorityPresets = {"Default", "Classic Tremolo"};
    for (const auto& preset : priorityPresets)
    {
        if (presetManager.getPresetCategory(preset) == "Factory")
            orderedPresets.add(preset);
    }
    
    // Add remaining Factory root presets
    juce::StringArray factoryRootPresets = presetManager.getPresetsInFolder("Factory");
    for (const auto& preset : factoryRootPresets)
    {
        if (!orderedPresets.contains(preset))
            orderedPresets.add(preset);
    }
    
    // Add presets from each Factory subcategory in the same order as the menu
    juce::StringArray categories = presetManager.getFactoryCategories();
    for (const auto& category : categories)
    {
        if (category == "Factory")
            continue; // Skip the root factory category which we've already handled
            
        // Add all presets from this category
        juce::StringArray categoryPresets = presetManager.getPresetsInFolder(category);
        for (const auto& preset : categoryPresets)
            orderedPresets.add(preset);
    }
    
    // Finally add user presets
    juce::StringArray userPresets = presetManager.getPresetsInFolder("User");
    for (const auto& preset : userPresets)
        orderedPresets.add(preset);
    
    // Now navigate using this ordered list
    if (orderedPresets.isEmpty())
        return;
    
    // Find the current preset's index in our ordered list
    int currentIndex = orderedPresets.indexOf(currentDisplayName);
    if (currentIndex < 0)
        currentIndex = 0;
    
    // Calculate the new index
    int newIndex;
    if (goForward)
    {
        newIndex = (currentIndex + 1) % orderedPresets.size();
    }
    else
    {
        newIndex = (currentIndex - 1 + orderedPresets.size()) % orderedPresets.size();
    }
    
    // Load the new preset
    presetManager.loadPreset(orderedPresets[newIndex]);
    updatePresetDisplay();
}

void HierarchicalPresetMenu::updatePresetDisplay()
{
    currentDisplayName = presetManager.getDisplayedPresetName();
    isModified = presetManager.isPresetModified();
    
    // Set the button text with an asterisk if the preset is modified
    if (isModified)
        mainButton.setButtonText("* " + currentDisplayName + " *");
    else
        mainButton.setButtonText(currentDisplayName);
}

void HierarchicalPresetMenu::showRootMenu()
{
    juce::PopupMenu menu;
    menuIDToPresetMap.clear();
    nextMenuID = PresetIDOffset;
    
    // Factory Presets section with submenus
    menu.addSectionHeader("Factory Presets");
    
    // Special handling for default presets
    juce::StringArray rootFactoryPresets = presetManager.getPresetsInFolder("Factory");
    
    // First add Default and Classic Tremolo presets if they exist
    const juce::StringArray priorityPresets = {"Default", "Classic Tremolo"};
    for (const auto& priorityPreset : priorityPresets)
    {
        for (int i = 0; i < rootFactoryPresets.size(); ++i)
        {
            if (rootFactoryPresets[i] == priorityPreset)
            {
                menuIDToPresetMap[nextMenuID] = priorityPreset;
                menu.addItem(nextMenuID++, priorityPreset, true, priorityPreset == currentDisplayName);
                rootFactoryPresets.remove(i);
                break;
            }
        }
    }
    
    // Then add remaining factory presets
    for (const auto& preset : rootFactoryPresets)
    {
        menuIDToPresetMap[nextMenuID] = preset;
        menu.addItem(nextMenuID++, preset, true, preset == currentDisplayName);
    }
    
    // Add factory subcategories as submenus
    juce::StringArray categories = presetManager.getFactoryCategories();
    for (const auto& category : categories)
    {
        if (category == "Factory")
            continue; // Skip the root factory category which we've already handled
            
        // Create a submenu for this category
        juce::PopupMenu subMenu = buildFolderMenu(category);
        
        // Extract the subfolder name from the category path
        juce::String folderName = category.fromLastOccurrenceOf("/", false, false);
        if (folderName.isEmpty())
            folderName = category;
            
        menu.addSubMenu(folderName, subMenu);
    }
    
    // User Presets section
    menu.addSectionHeader("User Presets");
    juce::StringArray userPresets = presetManager.getPresetsInFolder("User");
    for (const auto& preset : userPresets)
    {
        menuIDToPresetMap[nextMenuID] = preset;
        menu.addItem(nextMenuID++, preset, true, preset == currentDisplayName);
    }
    
    // Utility section
    menu.addSectionHeader("Utility");
    menu.addItem(SavePresetID, "Save Current...");
    menu.addItem(OpenFolderID, "Open Preset Folder");
    
    // Show the menu with correct width constraints
    const int menuWidth = 250; // Set this to match your original design
    menu.showMenuAsync(juce::PopupMenu::Options()
        .withTargetComponent(this)
        .withMinimumWidth(menuWidth)  // This is valid
        .withMaximumNumColumns(1)
        .withStandardItemHeight(24),
        [this](int result) { menuItemSelected(result); });
}

juce::PopupMenu HierarchicalPresetMenu::buildFolderMenu(const juce::String& folderPath)
{
    juce::PopupMenu menu;
    
    // Add all presets in this folder
    juce::StringArray folderPresets = presetManager.getPresetsInFolder(folderPath);
    for (const auto& preset : folderPresets)
    {
        menuIDToPresetMap[nextMenuID] = preset;
        menu.addItem(nextMenuID++, preset, true, preset == currentDisplayName);
    }
    
    return menu;
}

void HierarchicalPresetMenu::menuItemSelected(int menuItemID)
{
    if (menuItemID == SavePresetID)
    {
        savePreset();
    }
    else if (menuItemID == OpenFolderID)
    {
        openPresetFolder();
    }
    else if (menuIDToPresetMap.find(menuItemID) != menuIDToPresetMap.end())
    {
        // Directly load the preset by name rather than by index
        presetManager.loadPreset(menuIDToPresetMap[menuItemID]);
        updatePresetDisplay();
    }
}

void HierarchicalPresetMenu::savePreset()
{
    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Save Preset",
        presetManager.getCurrentPresetDirectory(),
        "*.xml"
    );

    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode, [this, fileChooser](const juce::FileChooser& fc)
    {
        juce::File file = fc.getResult();
        if (file != juce::File{})  // Ensure the user selected a file
        {
            juce::String presetName = file.getFileNameWithoutExtension();
            if (presetName.isNotEmpty())
            {
                if (presetManager.savePreset(presetName))
                {
                    // After saving, rescan for presets to refresh the list
                    presetManager.scanForPresets();
                    
                    // Update the display with the new preset name
                    updatePresetDisplay();
                    
                    // Load the newly saved preset
                    presetManager.loadPreset(presetName);
                }
            }
        }
    });
}

void HierarchicalPresetMenu::openPresetFolder()
{
    juce::File presetDir = presetManager.getCurrentPresetDirectory();
    if (presetDir.exists())
        presetDir.revealToUser();
}
