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

juce::Image HierarchicalPresetMenu::createBackgroundImage(int width, int height)
{
    juce::Image img(juce::Image::ARGB, width, height, true);
    juce::Graphics g(img);
    
    auto bounds = juce::Rectangle<float>(0, 0, width, height);
    
    // Enhanced color palette with metallic tones (matching PluginEditor)
    juce::Colour darkPlum(61, 21, 46);
    juce::Colour midPlum(72, 28, 55);
    juce::Colour lightPlum(89, 34, 68);
    juce::Colour peachPink = juce::Colour(255, 201, 190).withMultipliedBrightness(0.6f);
    juce::Colour roseGold = juce::Colour(232, 193, 185).withMultipliedBrightness(0.6f);
    juce::Colour warmPlum = juce::Colour(198, 109, 139).withMultipliedBrightness(0.7f);
    
    // Metallic sheen colors
    juce::Colour metalHighlight = juce::Colour(255, 255, 255).withAlpha(0.03f);
    juce::Colour metalShadow = juce::Colours::black.withAlpha(0.05f);

    // Base gradient (just like in PluginEditor)
    juce::ColourGradient baseGradient(
        darkPlum.brighter(0.05f),
        bounds.getCentreX(), bounds.getCentreY(),
        midPlum.darker(0.05f),
        bounds.getRight(), bounds.getBottom(),
        true);
    
    baseGradient.addColour(0.3, midPlum);
    baseGradient.addColour(0.7, lightPlum);
    
    g.setGradientFill(baseGradient);
    g.fillAll();

    // Add initial sheen
    juce::ColourGradient sheenGradient(
        metalHighlight,
        bounds.getCentreX(), bounds.getCentreY(),
        metalShadow,
        bounds.getRight(), bounds.getBottom(),
        true);
    
    g.setGradientFill(sheenGradient);
    g.fillAll();
    
    // Add Perlin noise for texture
    const float fixedSeed = 42.0f;
    
    struct LayerConfig {
        float scale;
        float alpha;
        float amplitude;
        juce::Colour color;
        float offset;
    };

    // Define layers with the same configuration as PluginEditor
    std::array<LayerConfig, 3> layers = {{
        { 0.003f, 0.08f, 8.0f, warmPlum, 0.0f },
        { 0.005f, 0.06f, 6.0f, roseGold, 50.0f },
        { 0.008f, 0.04f, 5.0f, peachPink, 100.0f },
    }};

    // Generate texture layers
    for (const auto& layer : layers)
    {
        juce::Path swirlyPath;
        
        for (float y = 0; y < bounds.getHeight(); y += 4.0f)
        {
            swirlyPath.startNewSubPath(0, y);
            
            for (float x = 0; x < bounds.getWidth(); x += 4.0f)
            {
                float noise1 = PerlinNoise::noise(x * layer.scale, y * layer.scale, fixedSeed + layer.offset);
                float noise2 = PerlinNoise::noise(x * layer.scale * 1.7f, y * layer.scale * 1.7f, fixedSeed + layer.offset + 10.0f);
                
                float combinedNoise = (noise1 + noise2 * 0.5f) * juce::MathConstants<float>::pi * 4;
                
                float offsetX = std::sin(combinedNoise) * layer.amplitude;
                float offsetY = std::cos(combinedNoise) * layer.amplitude;
                
                swirlyPath.lineTo(x + offsetX, y + offsetY);
            }
        }
        
        g.setColour(layer.color.withAlpha(layer.alpha));
        g.strokePath(swirlyPath, juce::PathStrokeType(1.5f));
    }

    // Add metal flakes (subtle sparkle effect)
    juce::Random random;
    for (int i = 0; i < 1000; ++i) // Reduced count for menu background
    {
        float x = random.nextFloat() * width;
        float y = random.nextFloat() * height;
        float size = random.nextFloat() * 1.4f;
        float alpha = random.nextFloat() * 0.05f;

        if (random.nextBool()) {
            g.setColour(juce::Colours::white.withAlpha(alpha));
        } else {
            g.setColour(metalShadow.withAlpha(alpha * 0.8f));
        }
        
        g.fillEllipse(x, y, size, size);
    }

    // Add a final sheen
    juce::ColourGradient finalSheen(
        metalHighlight.withAlpha(0.02f),
        bounds.getCentreX(), bounds.getCentreY(),
        metalShadow.withAlpha(0.02f),
        bounds.getRight(), bounds.getBottom(),
        true);
    
    g.setGradientFill(finalSheen);
    g.fillRoundedRectangle(bounds, cornerRadius);
    
    return img;
}
