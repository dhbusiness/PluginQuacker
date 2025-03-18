/*
  ==============================================================================

    PresetManager.cpp
    Created: 21 Feb 2025 7:34:15pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts)
{
    // Set up preset directory in the user's application data folder
    // This ensures the plugin always has write permissions
    
#if JUCE_MAC
    // On macOS, use ~/Library/Application Support
    juce::File appSupportDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    juce::File companyDir = appSupportDir.getChildFile("DeividsHvostovsDSP");
    juce::File pluginDir = companyDir.getChildFile("TremoloViola");
    presetDirectory = pluginDir.getChildFile("Presets");
#elif JUCE_WINDOWS
    // On Windows, use AppData/Roaming
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    juce::File companyDir = appDataDir.getChildFile("DeividsHvostovsDSP");
    juce::File pluginDir = companyDir.getChildFile("TremoloViola");
    presetDirectory = pluginDir.getChildFile("Presets");
#else
    // Linux or other platforms
    juce::File homeDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
    juce::File companyDir = homeDir.getChildFile(".DeividsHvostovsDSP");
    juce::File pluginDir = companyDir.getChildFile("TremoloViola");
    presetDirectory = pluginDir.getChildFile("Presets");
#endif

    createPresetDirectory();
    scanForPresets();
}

PresetManager::~PresetManager()
{
}

void PresetManager::createPresetDirectory()
{
    // Create the entire directory hierarchy
    
    // First, the company directory
    juce::File companyDir = presetDirectory.getParentDirectory().getParentDirectory();
    if (!companyDir.exists())
    {
        const bool companyDirCreated = companyDir.createDirectory();
        if (!companyDirCreated)
        {
            juce::Logger::writeToLog("Failed to create company directory at: " + companyDir.getFullPathName());
            return; // Early return since we can't create parent directory
        }
    }
    
    // Then, the plugin directory
    juce::File pluginDir = presetDirectory.getParentDirectory();
    if (!pluginDir.exists())
    {
        const bool pluginDirCreated = pluginDir.createDirectory();
        if (!pluginDirCreated)
        {
            juce::Logger::writeToLog("Failed to create plugin directory at: " + pluginDir.getFullPathName());
            return; // Early return since we can't create parent directory
        }
    }
    
    // Finally, the presets directory
    if (!presetDirectory.exists())
    {
        const bool presetDirCreated = presetDirectory.createDirectory();
        if (!presetDirCreated)
        {
            juce::Logger::writeToLog("Failed to create preset directory at: " + presetDirectory.getFullPathName());
        }
    }
    
    // Create common subdirectories for organization
    juce::File factoryDir = presetDirectory.getChildFile("Factory");
    if (!factoryDir.exists())
        factoryDir.createDirectory();
        
    juce::File userDir = presetDirectory.getChildFile("User");
    if (!userDir.exists())
        userDir.createDirectory();
}

void PresetManager::scanForPresets()
{
    // Clear existing presets
    presets.clear();
    
    // Recursively scan the preset directory and its subdirectories
    scanDirectory(presetDirectory, "");
    
    // Rebuild the folder hierarchy with the updated presets
    buildFolderHierarchy();
}


void PresetManager::clearFactoryPresets()
{
    for (auto it = presets.begin(); it != presets.end(); )
    {
        if (it->second->category == "Factory")
            it = presets.erase(it);
        else
            ++it;
    }
}

bool PresetManager::savePreset(const juce::String& name, const juce::String& category)
{
    // Create a new preset from current state
    auto currentState = apvts.copyState();
    auto newPreset = std::make_unique<Preset>(name, category, currentState);
    
    // Determine the appropriate directory based on category
    juce::File targetDir;
    
    if (category == "Factory" || category.startsWith("Factory/"))
    {
        // Save to Factory directory
        if (category == "Factory")
        {
            targetDir = presetDirectory.getChildFile("Factory");
        }
        else
        {
            // Create nested directory structure for Factory subcategories
            juce::String subfolderPath = category.substring(8); // Remove "Factory/" prefix
            targetDir = presetDirectory.getChildFile("Factory");
            
            juce::StringArray folders = juce::StringArray::fromTokens(subfolderPath, "/", "");
            for (const auto& folder : folders)
            {
                targetDir = targetDir.getChildFile(folder);
                if (!targetDir.exists())
                    targetDir.createDirectory();
            }
        }
    }
    else
    {
        // Save to User directory
        targetDir = presetDirectory.getChildFile("User");
        
        // Check if this is a user subcategory
        if (category != "User" && !category.isEmpty())
        {
            juce::StringArray folders = juce::StringArray::fromTokens(category, "/", "");
            for (const auto& folder : folders)
            {
                targetDir = targetDir.getChildFile(folder);
                if (!targetDir.exists())
                    targetDir.createDirectory();
            }
        }
    }
    
    // Make sure the directory exists
    if (!targetDir.exists())
    {
        const bool dirCreated = targetDir.createDirectory();
        if (!dirCreated)
        {
            juce::Logger::writeToLog("Failed to create preset directory at: " + targetDir.getFullPathName());
            return false;
        }
    }
    
    // Save preset to file
    juce::File presetFile = targetDir.getChildFile(generateSafeFileName(name));
    
    // Save to file using the updated location
    if (savePresetToFile(*newPreset, presetFile))
    {
        // Add to our preset map
        presets[name] = std::move(newPreset);
        return true;
    }
    
    return false;
}

bool PresetManager::loadPreset(const juce::String& name)
{
    auto it = presets.find(name);
    if (it != presets.end())
    {
        // Create a deep copy of the preset state to avoid modifying the stored version.
        juce::ValueTree presetCopy = it->second->state.createCopy();
        
        // Replace the entire state with the preset's state
        apvts.replaceState(presetCopy);
        
        // Update the current preset name
        currentPresetName = name;
        
        // Save a clean copy for later comparison (used for detecting modifications)
        cleanPresetState = presetCopy.createCopy();
        
        // Apply parameters in the correct order to ensure proper sync behavior
        applyParametersInCorrectOrder();
        
        // Call the callback if it's been set
        if (onPresetLoaded) {
            onPresetLoaded();
        }
        
        return true;
    }
    return false;
}

bool PresetManager::isPresetModified() const
{
    // Retrieve the current state via copyState() to capture all modifications.
    juce::ValueTree currentState = apvts.copyState();
    return !currentState.isEquivalentTo(cleanPresetState);
}

juce::String PresetManager::getDisplayedPresetName() const
{
    return currentPresetName;
}

juce::String PresetManager::getModifiedDisplayName() const
{
    // Compare the current state (using copyState() to capture UI changes)
    juce::ValueTree currentState = apvts.copyState();
    if (!currentState.isEquivalentTo(cleanPresetState))
        return currentPresetName + "*";
    return currentPresetName;
}

void PresetManager::initializeDefaultPresets()
{
    // Here we'll add factory presets
    // This will be implemented in the next step
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : presets)
    {
        names.add(preset.first);
    }
    return names;
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray categories;
    std::set<juce::String> uniqueCategories;
    
    for (const auto& preset : presets)
    {
        uniqueCategories.insert(preset.second->category);
    }
    
    for (const auto& category : uniqueCategories)
    {
        categories.add(category);
    }
    
    return categories;
}

bool PresetManager::savePresetToFile(const Preset& preset, const juce::File& presetFile)
{
    // Convert state to XML
    if (auto xml = getXmlFromState(preset.state))
    {
        // Add metadata
        xml->setAttribute("name", preset.name);
        xml->setAttribute("category", preset.category);
        xml->setAttribute("dateCreated", preset.dateCreated.toISO8601(true));
        
        // Save to the provided file path
        if (xml->writeTo(presetFile))
        {
            return true;
        }
    }
    
    return false;
}

void PresetManager::loadPresetFromFile(const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse(file))
    {
        // Extract metadata
        juce::String name = xml->getStringAttribute("name");
        
        // Determine category based on the file's location relative to the preset directory
        juce::String category = xml->getStringAttribute("category", "");
        
        // If no category is specified in the XML, derive it from the file path
        if (category.isEmpty())
        {
            category = determineCategory(file);
        }
        
        juce::Time dateCreated = juce::Time::fromISO8601(xml->getStringAttribute("dateCreated"));
        
        // Get the state
        juce::ValueTree state = getStateFromXml(*xml);
        
        // Create and store the preset
        presets[name] = std::make_unique<Preset>(name, category, state, dateCreated);
    }
}

juce::String PresetManager::generateSafeFileName(const juce::String& name)
{
    // Remove invalid characters and replace spaces with underscores
    juce::String safeName = name.removeCharacters("\\/:*?\"<>|");
    safeName = safeName.replace(" ", "_");
    return safeName + ".xml";
}

juce::ValueTree PresetManager::getStateFromXml(const juce::XmlElement& xml)
{
    return juce::ValueTree::fromXml(xml);
}

std::unique_ptr<juce::XmlElement> PresetManager::getXmlFromState(const juce::ValueTree& state)
{
    return std::unique_ptr<juce::XmlElement>(state.createXml());
}


juce::StringArray PresetManager::getFactoryPresetNames() const
{
    juce::StringArray names;
    for (const auto& presetPair : presets)
    {
        if (presetPair.second->category == "Factory")
            names.add(presetPair.first);
    }
    return names;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    juce::StringArray names;
    for (const auto& presetPair : presets)
    {
        if (presetPair.second->category != "Factory")
            names.add(presetPair.first);
    }
    return names;
}

juce::String PresetManager::getPresetCategory(const juce::String& presetName) const
{
    auto it = presets.find(presetName);
    if (it != presets.end())
    {
        return it->second->category;
    }
    return "User"; // Default to User category if not found
}

void PresetManager::applyParametersInCorrectOrder()
{
    // Get raw parameter values
    auto* syncParam = apvts.getRawParameterValue("lfoSync");
    auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
    auto* rateParam = apvts.getRawParameterValue("lfoRate");
    
    // First, ensure sync parameter is set
    bool isInSync = syncParam->load() > 0.5f;
    if (auto* sync = apvts.getParameter("lfoSync")) {
        sync->setValueNotifyingHost(isInSync ? 1.0f : 0.0f);
    }
    
    // Set division parameter
    if (auto* division = apvts.getParameter("lfoNoteDivision")) {
        // For a choice parameter with 6 options (0-5), we must normalize to 0-1
        float normalizedValue = divisionParam->load() / 5.0f;
        division->setValueNotifyingHost(normalizedValue);
    }
    
    // Set rate parameter
    if (auto* rate = apvts.getParameter("lfoRate")) {
        rate->setValueNotifyingHost(rate->convertTo0to1(rateParam->load()));
    }
}

std::vector<juce::String> PresetManager::splitFolderPath(const juce::String& path)
{
    std::vector<juce::String> components;
    juce::StringArray tokens = juce::StringArray::fromTokens(path, "/", "");
    
    for (const auto& token : tokens)
        components.push_back(token);
    
    return components;
}

void PresetManager::buildFolderHierarchy()
{
    // Clear existing hierarchy
    presetFolders.clear();
    
    // Create top-level folders
    presetFolders["Factory"] = PresetFolder{"Factory", {}, {}};
    presetFolders["User"] = PresetFolder{"User", {}, {}};
    
    // Process all presets
    for (const auto& preset : presets)
    {
        juce::String presetName = preset.first;
        juce::String category = preset.second->category;
        
        if (category == "Factory")
        {
            // Add to the root Factory folder
            presetFolders["Factory"].addPreset(presetName);
        }
        else if (category.startsWith("Factory/"))
        {
            // Get the subfolder path
            juce::String subfolderPath = category.substring(8); // Remove "Factory/" prefix
            
            // Navigate to the correct subfolder
            PresetFolder* currentFolder = &presetFolders["Factory"];
            auto pathComponents = splitFolderPath(subfolderPath);
            
            for (const auto& component : pathComponents)
            {
                currentFolder = &currentFolder->getOrCreateSubfolder(component);
            }
            
            // Add the preset to this subfolder
            currentFolder->addPreset(presetName);
        }
        else
        {
            // Add to User folder
            presetFolders["User"].addPreset(presetName);
        }
    }
}

juce::StringArray PresetManager::getFactoryCategories() const
{
    juce::StringArray categories;
    
    // First check if "Factory" folder exists
    auto it = presetFolders.find("Factory");
    if (it != presetFolders.end())
    {
        // Add the base Factory category
        categories.add("Factory");
        
        // Helper function to traverse folder hierarchy
        std::function<void(const PresetFolder&, const juce::String&)> traverseFolder =
            [&categories, &traverseFolder](const PresetFolder& folder, const juce::String& path)
        {
            // Add subfolders
            for (const auto& subfolder : folder.subfolders)
            {
                juce::String newPath = path.isEmpty() ? subfolder.first : path + "/" + subfolder.first;
                categories.add("Factory/" + newPath);
                traverseFolder(subfolder.second, newPath);
            }
        };
        
        // Start traversal from the Factory folder with empty path
        traverseFolder(it->second, "");
    }
    
    return categories;
}

juce::StringArray PresetManager::getPresetsInFolder(const juce::String& folderPath) const
{
    juce::StringArray result;
    
    // Handle root folders
    if (folderPath == "Factory" || folderPath == "User")
    {
        auto it = presetFolders.find(folderPath);
        if (it != presetFolders.end())
        {
            // Add all presets from this root folder
            for (const auto& presetName : it->second.presets)
                result.add(presetName);
        }
        return result;
    }
    
    // Handle subfolders
    const PresetFolder* folder = getFolderByPath(folderPath);
    if (folder != nullptr)
    {
        for (const auto& presetName : folder->presets)
            result.add(presetName);
    }
    
    return result;
}

juce::String PresetManager::categoryToFolderPath(const juce::String& category) const
{
    // Factory presets already have a path-like structure
    return category;
}

const PresetManager::PresetFolder* PresetManager::getFolderByPath(const juce::String& path) const
{
    // Split the path
    auto components = splitFolderPath(path);
    
    // Get the root folder
    if (components.empty())
        return nullptr;
        
    auto rootFolderIt = presetFolders.find(components[0]);
    if (rootFolderIt == presetFolders.end())
        return nullptr;
        
    const PresetFolder* currentFolder = &rootFolderIt->second;
    
    // Navigate through the path
    for (size_t i = 1; i < components.size(); ++i)
    {
        auto subfolderIt = currentFolder->subfolders.find(components[i]);
        if (subfolderIt == currentFolder->subfolders.end())
            return nullptr;
            
        currentFolder = &subfolderIt->second;
    }
    
    return currentFolder;
}

void PresetManager::scanDirectory(const juce::File& directory, const juce::String& categoryPrefix)
{
    // Process all XML files in this directory
    for (const auto& file : directory.findChildFiles(juce::File::findFiles, false, "*.xml"))
    {
        loadPresetFromFile(file);
    }
    
    // Recursively process subdirectories
    for (const auto& subdir : directory.findChildFiles(juce::File::findDirectories, false))
    {
        // Determine the category for this subdirectory
        juce::String category;
        
        // Special handling for root Factory and User directories
        if (subdir.getFileName() == "Factory" || subdir.getFileName() == "User")
        {
            category = subdir.getFileName();
        }
        else if (!categoryPrefix.isEmpty())
        {
            category = categoryPrefix + "/" + subdir.getFileName();
        }
        else
        {
            category = subdir.getFileName();
        }
        
        // Scan this subdirectory
        scanDirectory(subdir, category);
    }
}

void PresetManager::setCustomPresetName(const juce::String& name)
{
    currentPresetName = name;
    // Don't update cleanPresetState since this is just for display
}

juce::String PresetManager::determineCategory(const juce::File& file)
{
    // Get the path relative to the preset directory
    juce::String relativePath = file.getParentDirectory().getRelativePathFrom(presetDirectory);
    
    // If the file is directly in the preset directory
    if (relativePath.isEmpty() || relativePath == ".")
    {
        return "User"; // Default to User category
    }
    
    return relativePath;
}
